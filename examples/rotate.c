/* Copyright (C) 2012 Akiri Solutions, Inc.
 * For conditions of distribution and use, see copyright notice in logr.h
 */
#include <string.h>
#include <logr.h>
#include <errno.h>

#ifndef ENOTSUP
#  define ENOTSUP 48 /* missing from mingw */
#endif

/* This example demonstrates setting a rotate threshold and automatically
   rotating file logs. */

#define LOGFILE "file.log"
#define MSG "All work and no play makes Jack a dull boy.\n"

int
main(int argc, char **argv)
{
    int retval;
    logr_t *logr = logr_getlogger();
    unsigned int threshold = 1024 * 1024; // 1 MB
    int i, n = (2.5 * threshold) / strlen(MSG); // ensure rotating twice...

    retval = logr_set_threshold(logr, threshold);
    if (retval != 0) {
        if (errno == ENOTSUP) {
            fprintf(stderr, "log rotation is not supported on this platform.\n");
        } else {
            perror("logr_set_threshold");
        }
        return -1;
    }

    retval = logr_open(logr, LOGFILE);
    if (retval != 0) {
	perror("logr_open");
	return -1;
    }
    
    for (i = 0; i < n; i++) {
	logr_err(MSG);
    }

    printf("See '%1$s', '%1$s.0' and '%1$s.1.bz2'.\n", LOGFILE);
    printf("Subsequent runs will generate additional .bz2 files.\n");
    return 0;
}
