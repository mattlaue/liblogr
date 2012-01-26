/* Copyright (C) 2012 Akiri Solutions, Inc.
 * For conditions of distribution and use, see copyright notice in logr.h
 */
#include <sys/utsname.h> // for uname()

#include <logr.h>

/*
 * Format log entries similar to Android's 'logcat':
 *     adb logcat -v time 
 *
 * NOTE: Error values are ignored in the interest of readability.
 */

#define LOGCAT_FATAL 0
#define LOGCAT_ERROR 1
#define LOGCAT_WARNING 2
#define LOGCAT_INFO 3
#define LOGCAT_DEBUG 4
#define LOGCAT_VERBOSE 5
#define LOGCAT_SILENT 6

static const char *
logcat_priority(int priority) 
{
    switch (priority) {
    case LOGCAT_FATAL:
	return "F";
    case LOGCAT_ERROR:
	return "E";
    case LOGCAT_WARNING:
	return "W";
    case LOGCAT_INFO:
	return "I";
    case LOGCAT_DEBUG:
	return "D";
    case LOGCAT_VERBOSE:
	return "V";
    }
    /* We MUST always be returned. */
    return "X";
}

int
main(int argc, char **argv)
{
    logr_t *logr = logr_getlogger();

    static logr_ops_t ops = {
	.priority = logcat_priority,
    };

    logr_set_level(logr, LOGCAT_WARNING);
    logr_set_ops(logr, &ops);

    logr_set_prefix_format(logr, "%{timestamp}s %{level}s/logcat( %{pid}d): ");
    logr_set_timestamp_format(logr, "%m-%d %T.xxx");

    /* This message will NOT be printed since LOG_INFO < LOG_WARNING */
    logr_printf(logr, LOGCAT_INFO, 
		"All work and no play makes %s a dull boy.\n", "Jack");
    /* This message WILL be printed. */
    logr_printf(logr, LOGCAT_ERROR,
		"Come out, come out, where ever you are.\n");
    return 0;
}
