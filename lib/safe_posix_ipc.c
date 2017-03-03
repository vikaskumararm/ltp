#include <mqueue.h>
#include <stdarg.h>

#include "test.h"
#include "safe_posix_ipc.h"

int safe_mq_open(const char *file, const int lineno, void (*cleanup_fn) (void),
              const char *pathname, int oflags, ...)
{
	va_list ap;
	int rval;
	mode_t mode;
	struct mq_attr *attr;

	mode = 0;
	attr = NULL;

	va_start(ap, oflags);

	/* Android's NDK's mode_t is smaller than an int, which results in
	 * SIGILL here when passing the mode_t type.
	 */
#ifndef ANDROID
	mode = va_arg(ap, mode_t);
#else
	mode = va_arg(ap, int);
#endif

	attr = va_arg(ap, struct mq_attr *);

	va_end(ap);

	rval = mq_open(pathname, oflags, mode, attr);
	if (rval == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn,
			 "%s:%d: mq_open(%s,%d,0%o,%p) failed",
			 file, lineno, pathname, oflags, mode, attr);
	}

	return rval;
}
