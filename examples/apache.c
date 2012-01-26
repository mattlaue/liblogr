/* Copyright (C) 2012 Akiri Solutions, Inc.
 * For conditions of distribution and use, see copyright notice in logr.h
 */
#include <sys/utsname.h> // for uname()

#include <logr.h>

/*
 * This example demonstrates log entries similar to error.log of Apache2.
 * NOTE: Error values are ignored in the interest of readability.
 */

int
main(int argc, char **argv)
{
    struct utsname buf;

    logr_t *logr = logr_getlogger();

    logr_set_prefix_format(logr, LOGR_PREFIX_FORMAT_BASIC);
    logr_set_timestamp_format(logr, "[%a %b %d %T %Y]");

    uname(&buf);
    logr_err("(%s) All work and no play makes %s a dull boy.\n", 
	     buf.release, "Jack");

    return 0;
}
