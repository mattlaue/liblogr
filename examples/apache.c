/* Copyright (C) 2012 Akiri Solutions, Inc.
 * For conditions of distribution and use, see copyright notice in logr.h
 */
#ifndef __MINGW32__
#include <sys/utsname.h> // for uname()
#endif

#include <logr.h>

/*
 * This example demonstrates log entries similar to error.log of Apache2.
 * NOTE: Error values are ignored in the interest of readability.
 */

int
main(int argc, char **argv)
{
    logr_t *logr = logr_getlogger();
    const char *release;

#ifndef __MINGW32__
    struct utsname buf;
    uname(&buf);
    release = buf.release;
#else
    release = "mingw";
#endif

    logr_set_prefix_format(logr, LOGR_PREFIX_FORMAT_BASIC);
    logr_set_timestamp_format(logr, "[%a %b %d %H:%M:%S %Y]");

    logr_err("(%s) All work and no play makes %s a dull boy.\n", 
	     release, "Jack");

    return 0;
}
