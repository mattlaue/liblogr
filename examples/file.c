/* Copyright (C) 2012 Akiri Solutions, Inc.
 * For conditions of distribution and use, see copyright notice in logr.h
 */
#include <logr.h>

/* these are just values, by default they are converted to syslog names */
#define	LOG_EMERG	0
#define	LOG_CRIT	2

/* This example demonstrates logging to the filesystem and using a local
 * logr_t instance. */

#define LOGFILE "file.log"

int
main(int argc, char **argv)
{
    logr_t *logr;
    int retval;

    logr = logr_alloc(NULL);
    if (logr == NULL) {
	perror("logr_alloc");
	return -1;
    }

    logr_set_prefix_format(logr, LOGR_PREFIX_FORMAT_BASIC);
    logr_printf(logr, LOG_CRIT, "This output goes to stderr.\n");

    retval = logr_open(logr, LOGFILE);
    if (retval != 0) {
	perror("logr_open");
	logr_free(logr);
	return -1;
    }

    logr_printf(logr, LOG_EMERG, "This output gets appended to %s.\n", LOGFILE);
    logr_free(logr);

    printf("See '%s' for additional output.\n", LOGFILE);
    return 0;
}
