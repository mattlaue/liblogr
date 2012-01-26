/* Copyright (C) 2012 Akiri Solutions, Inc.
 * For conditions of distribution and use, see copyright notice in logr.h
 */
#include <logr.h>

/*
 * Demonstrates basic use of the default logging instance.
 * NOTE: Error values are ignored in the interest of readability.
 */

int
main(int argc, char **argv)
{
    /* Print to stderr without a prefix */
    logr_err("All work and no play makes %s a dull boy.\n", "Jack");

    /* 
     * Change the log level so more messages will be printed.
     * logr uses the syslog defines for levels by default.
     */
    logr_set_level(logr_getlogger(), LOG_DEBUG);
    logr_debug("All work and no play makes %s a dull boy.\n", "Jack");

    /*
     * Set a prefix for log entries using the default format. 
     */
    logr_set_prefix_format(logr_getlogger(), LOGR_PREFIX_FORMAT_BASIC);
    logr_notice("All work and no play makes %s a dull boy.\n", "Jack");

    /*
     * Make the prefix more verbose...
     */
    logr_set_prefix_format(logr_getlogger(), LOGR_PREFIX_FORMAT_VERBOSE);
    logr_crit("All work and no play makes %s a dull boy.\n", "Jack");

    return 0;
}
