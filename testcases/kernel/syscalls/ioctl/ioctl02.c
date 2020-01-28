/*
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (c) 2020 Petr Vorel <petr.vorel@gmail.com>
 *
 * Testcase to test the TCGETA, and TCSETA ioctl implementations for the tty
 * driver.
 *
 * ALGORITHM
 *	In this test, the parent and child open the parentty and the childtty
 *	respectively.  After opening the childtty the child flushes the stream
 *	and sends a SIGUSR1 to the parent (thereby asking it to continue its
 *	testing). The parent, which was waiting for this signal to arrive, now
 *	starts the testing. It issues a TCGETA ioctl to get all the tty
 *	parameters. It then changes them to known values by issuing a TCSETA
 *	ioctl.  Then the parent issues a TCGETA ioctl again and compares the
 *	received values with what it had set earlier. The test fails if TCGETA
 *	or TCSETA fails, or if the received values don't match those that were
 *	set. The parent does all the testing, the requirement of the child
 *	process is to moniter the testing done by the parent, and hence the
 *	child just waits for the parent.
 *
 * 07/2001 Ported by Wayne Boyer
 */

#include "config.h"
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include "tst_test.h"

#define	CNUL	0

#ifdef HAVE_STRUCT_TERMIO
static struct termio termio, save_io;

static char *parenttty, *childtty;
static int parentfd, childfd;
static int parentpid, childpid;
static volatile int sigterm, sigusr1, sigusr2;
static int closed = 1;

static int do_child_setup(void);
static int do_parent_setup(void);
static int run_ptest(void);
static int run_ctest(void);
static int chk_tty_parms();
static void do_child(void);
static void sigterm_handler(void);
static void cleanup(void);

static char *device;

static void verify_ioctl(void)
{
	int rval;

	if (!device)
		tst_brk(TBROK, "You must specify a tty device with -D option");

	parenttty = device;
	childtty = device;

	parentpid = getpid();
	childpid = SAFE_FORK();

	// FIXME: debug
	if (childpid == 0) {	/* child */
		do_child();
	}

	while (!sigusr1)
		sleep(1);

	sigusr1 = 0;

	// FIXME: debug
	parentfd = do_parent_setup();
	if (parentfd < 0) {
		kill(childpid, SIGTERM);
		waitpid(childpid, NULL, 0);
		cleanup();
	}

	// FIXME: debug
	/* run the parent test */
	rval = run_ptest();
	if (rval == -1) {
		/*
		 * Parent cannot set/get ioctl parameters.
		 * SIGTERM the child and cleanup.
		 */
		kill(childpid, SIGTERM);
		waitpid(childpid, NULL, 0);
		cleanup();
	}

	if (rval != 0)
		tst_res(TFAIL, "TCGETA/TCSETA tests FAILED with "
			 "%d %s", rval, rval > 1 ? "errors" : "error");
	else
		tst_res(TPASS, "TCGETA/TCSETA tests SUCCEEDED");

	// FIXME: debug
	/* FIXME: check return codes. */
	(void)kill(childpid, SIGTERM);
	(void)waitpid(childpid, NULL, 0);

	/*
	 * Clean up things from the parent by restoring the
	 * tty device information that was saved in setup()
	 * and closing the tty file descriptor.
	 */
	SAFE_IOCTL(parentfd, TCSETA, &save_io);
	SAFE_CLOSE(parentfd);

	closed = 1;
}

static void do_child(void)
{
	childfd = do_child_setup();
	if (childfd < 0)
		_exit(1);
	run_ctest();
	_exit(0);
}

/*
 * run_ptest() - setup the various termio structure values and issue
 *		 the TCSETA ioctl call with the TEST macro.
 */
static int run_ptest(void)
{
	int i, rval;

	/* Use "old" line discipline */
	termio.c_line = 0;

	/* Set control modes */
	termio.c_cflag = B50 | CS7 | CREAD | PARENB | PARODD | CLOCAL;

	/* Set control chars. */
	for (i = 0; i < NCC; i++) {
		if (i == VEOL2)
			continue;
		termio.c_cc[i] = CSTART;
	}

	/* Set local modes. */
	termio.c_lflag =
	    ((unsigned short)(ISIG | ICANON | XCASE | ECHO | ECHOE | NOFLSH));

	/* Set input modes. */
	termio.c_iflag =
	    BRKINT | IGNPAR | INPCK | ISTRIP | ICRNL | IUCLC | IXON | IXANY |
	    IXOFF;

	/* Set output modes. */
	termio.c_oflag = OPOST | OLCUC | ONLCR | ONOCR;

	TEST(ioctl(parentfd, TCSETA, &termio));
	if (TST_RET < 0) {
		tst_res(TFAIL | TTERRNO, "ioctl TCSETA failed");
		return -1;
	}

	/* Get termio and see if all parameters actually got set */
	TEST(ioctl(parentfd, TCGETA, &termio));
	if (TST_RET < 0) {
		tst_res(TFAIL | TTERRNO, "ioctl TCGETA failed");
		return -1;
	}

	return chk_tty_parms();
}

static int run_ctest(void)
{
	/*
	 * Wait till the parent has finished testing.
	 */
	while (!sigterm)
		sleep(1);

	sigterm = 0;

	tst_res(TINFO, "child: Got SIGTERM from parent");
	SAFE_CLOSE(childfd);
	return 0;
}

static int chk_tty_parms(void)
{
	int i, flag = 0;

	if (termio.c_line != 0) {
		tst_res(TINFO, "line discipline has incorrect value %o",
			 termio.c_line);
		flag++;
	}

	for (i = 0; i < NCC; i++) {
		if (i == VEOL2) {
			if (termio.c_cc[VEOL2] == CNUL) {
				continue;
			} else {
				tst_res(TINFO, "control char %d has "
					 "incorrect value %d %d", i,
					 termio.c_cc[i], CNUL);
				flag++;
				continue;
			}
		}

		if (termio.c_cc[i] != CSTART) {
			tst_res(TINFO, "control char %d has incorrect "
				 "value %d.", i, termio.c_cc[i]);
			flag++;
		}
	}

	if (! (termio.c_lflag
	     && (ISIG | ICANON | XCASE | ECHO | ECHOE | NOFLSH))) {
		tst_res(TINFO, "lflag has incorrect value. %o",
			 termio.c_lflag);
		flag++;
	}

	if (! (termio.c_iflag
	     && (BRKINT | IGNPAR | INPCK | ISTRIP | ICRNL | IUCLC | IXON | IXANY
		 | IXOFF))) {
		tst_res(TINFO, "iflag has incorrect value. %o",
			 termio.c_iflag);
		flag++;
	}

	if (!(termio.c_oflag && (OPOST | OLCUC | ONLCR | ONOCR))) {
		tst_res(TINFO, "oflag has incorrect value. %o",
			 termio.c_oflag);
		flag++;
	}

	if (!flag)
		tst_res(TINFO, "termio values are set as expected");

	return flag;
}

static int do_parent_setup(void)
{
	int pfd;

	pfd = SAFE_OPEN(parenttty, O_RDWR, 0777);

	/* unset the closed flag */
	closed = 0;

	/* flush tty queues to remove old output */
	SAFE_IOCTL(pfd, TCFLSH, 2);
	return pfd;
}

static int do_child_setup(void)
{
	int cfd;

	cfd = SAFE_OPEN(childtty, O_RDWR, 0777);

	/* flush tty queues to remove old output */
	SAFE_IOCTL(cfd, TCFLSH, 2);

	/* tell the parent that we're done */
	kill(parentpid, SIGUSR1);

	return cfd;
}

/*
 * Define the signals handlers here.
 */
static void sigterm_handler(void)
{
	sigterm = 1;
}

static void sigusr1_handler(void)
{
	sigusr1 = 1;
}

static void sigusr2_handler(void)
{
	sigusr2 = 1;
}

static void setup(void)
{
	int fd;
	struct sigaction act;

	/* XXX: TERRNO required all over the place */
	fd = SAFE_OPEN(device, O_RDWR, 0777);

	/* Save the current device information - to be restored in cleanup() */
	SAFE_IOCTL(fd, TCGETA, &save_io);

	/* Close the device */
	SAFE_CLOSE(fd);

	/* Set up the signal handlers */
	act.sa_handler = (void *)sigterm_handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	(void)sigaction(SIGTERM, &act, 0);

	act.sa_handler = (void *)sigusr1_handler;
	act.sa_flags = 0;
	(void)sigaction(SIGUSR1, &act, 0);

	act.sa_handler = (void *)sigusr2_handler;
	act.sa_flags = 0;
	(void)sigaction(SIGUSR2, &act, 0);

	act.sa_handler = SIG_IGN;
	act.sa_flags = 0;
	(void)sigaction(SIGTTOU, &act, 0);

	sigterm = sigusr1 = sigusr2 = 0;
}

static void cleanup(void)
{
	if (!closed) {
		if (ioctl(parentfd, TCSETA, &save_io) == -1)
			tst_res(TINFO, "ioctl restore failed in cleanup()");
		SAFE_CLOSE(parentfd);
	}
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_ioctl,
	.options = (struct tst_option[]) {
		{"D:", &device, "-D <tty device> : for example, /dev/tty[0-9]"},
		{}
	}
};

#else
	TST_TEST_TCONF("libc doesn't provide legacy struct termio");
#endif /* HAVE_STRUCT_TERMIO */
