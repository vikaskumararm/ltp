// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Joerg Vehlow <joerg.vehlow@aox-tech.de>
 */
#define _GNU_SOURCE
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "old_tmpdir.h"
#include "tst_fifo.h"

#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX	MAXPATHLEN
#else
#define PATH_MAX	1024
#endif
#endif

#define FIFO_ENV_VAR "LTP_FIFO_PATH"

static char *FIFO_DIR = NULL;

#define INIT_FIFO_FUNCTION(req_mode, ack_mode) \
    char path_req[PATH_MAX]; \
    char path_ack[PATH_MAX]; \
    if (!FIFO_DIR) \
        tst_brk(TBROK, "you must call tst_fifo_init first"); \
    snprintf(path_req, sizeof(path_req), "%s/tst_fifo_%s.req", FIFO_DIR, name); \
    snprintf(path_ack, sizeof(path_ack), "%s/tst_fifo_%s.ack", FIFO_DIR, name);
    //if (required_mode && access(path, required_mode)) \
     //   tst_brk(TBROK, "cannot access '%s'", path);

void tst_fifo_init(void)
{
    if (tst_tmpdir_created()) {
        FIFO_DIR = tst_get_tmpdir();
        setenv(FIFO_ENV_VAR, FIFO_DIR, 1);
    } else {
        FIFO_DIR = getenv(FIFO_ENV_VAR);
    }

    if (!FIFO_DIR)
        tst_brk(TBROK, "no tempdir and " FIFO_ENV_VAR " not set");
}

void tst_fifo_create(const char *name)
{
    INIT_FIFO_FUNCTION(0, 0);

    if (mkfifo(path_req, S_IRWXU | S_IRWXG | S_IRWXO))
        tst_brk(TBROK, "mkfifo(%s) failed with %s", path_req, tst_strerrno(errno));

    if (mkfifo(path_ack, S_IRWXU | S_IRWXG | S_IRWXO))
        tst_brk(TBROK, "mkfifo(%s) failed with %s", path_ack, tst_strerrno(errno));
}

void tst_fifo_destroy(const char *name)
{
    INIT_FIFO_FUNCTION(R_OK | W_OK, R_OK | W_OK);

    if (remove(path_req))
        tst_brk(TBROK, "unable to remove fifo '%s'", path_req);
    if (remove(path_ack))
        tst_brk(TBROK, "unable to remove fifo '%s'", path_ack);
}

void tst_fifo_send(const char *name, const char *data)
{
    int fd;
    char ack[2];
    INIT_FIFO_FUNCTION(W_OK, R_OK);

    fd = SAFE_OPEN(path_req, O_WRONLY);
    SAFE_WRITE(1, fd, data, strlen(data));
    SAFE_CLOSE(fd);

    fd = SAFE_OPEN(path_ack, O_RDONLY);
    SAFE_READ(1, fd, ack, 2);
    SAFE_CLOSE(fd);
}

int tst_fifo_receive(const char *name, char *data, int maxlen)
{
    int fd;
    int nbyte;
    int pos;
    INIT_FIFO_FUNCTION(R_OK, W_OK);

    fd = SAFE_OPEN(path_req, O_RDONLY);
    pos = 0;
    while (1) {
        nbyte = SAFE_READ(0, fd, data + pos, maxlen - pos);
        if (nbyte == 0)
            break;
        pos += nbyte;
        if (pos == maxlen)
            tst_brk(TBROK, "buffer is not big enough");
    }

    SAFE_CLOSE(fd);

    fd = SAFE_OPEN(path_ack, O_WRONLY);
    SAFE_WRITE(1, fd, "OK", 2);
    SAFE_CLOSE(fd);

    data[pos] = 0;
    return pos;
}
