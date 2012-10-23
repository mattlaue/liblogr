/* Copyright (C) 2012 Akiri Solutions, Inc.
   http://www.akirisolutions.com

   logr - customizable logging library for C/C++.

   The logr package is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The logr package is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the logr source code; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* logr provides log file formatting and rotation in a similar fashion to
 * Java's java.util.logging package or Python's logging module. */


#ifndef __LOGR_H__
#define __LOGR_H__

#ifdef __cplusplus
extern "C" {
#endif

/** @file */

#include <stdio.h>
#include <unistd.h>

/**
 * Basic prefix format that prints the datetime and level.
 */
#define LOGR_PREFIX_FORMAT_BASIC "%{timestamp}s [%{priority}s] "

/**
 * Verbose prefix format.
 */
#define LOGR_PREFIX_FORMAT_VERBOSE \
    "%{timestamp}s [%{priority}s] @ %{file}s:%{line}d(%{pretty}s) "

/**
 * Default format to use when printing the datetime.
 * <ISO 8601 date format>-<24-hour time>
 */
#define LOGR_DEFAULT_DATE_FORMAT   "%Y-%m-%d-%H:%M:%S"

/**
 * Maximum number of bytes the date may be once expanded.
 * Additional bytes are silently truncated.
 */
#define LOGR_MAX_TIMESTAMP_SIZE 256

/**
 * Log levels.
 * Based on equivalent ones in syslog.h.
 * We define our own since syslog.h does not exist on windows.
 */

#define LOGR_EMERG   0
#define LOGR_ALERT   1
#define LOGR_CRIT    2
#define LOGR_ERR     3
#define LOGR_WARNING 4
#define LOGR_NOTICE  5
#define LOGR_INFO    6
#define LOGR_DEBUG   7

/// @cond
#define LOGR_XARGV \
    const char *file, int line, const char *func, const char *pretty_func
#define LOGR_XARGS __FILE__, __LINE__, __FUNCTION__, __PRETTY_FUNCTION__
/// @endcond

struct logr;

/**
 * typedef for the opaque structure <i>logr</i>.
 */
typedef struct logr logr_t;

/**
 * Function type converting priority values to strings.
 * NOTE: priority functions MUST return a valid string;
 * they may NOT return NULL.
 */
typedef const char * (*logr_priority_func_t)(int);

/**
 * Function type for printing the log entry prefix.
 * The prefix prints everything before the formatted output.  If this
 * function is overridden all other formatting functions are ignored unless
 * directly invoked using one of the <i>logr_util_*</i> helpers.
 * \see logr_util_priority
 * \see logr_util_process
 */
    typedef int (*logr_prefix_func_t)(const char *, int,
                                      const char *, const char *,
                                      logr_t *, int, FILE *, const char *);

/**
 * Function type for processing directives in a format string.
 * \returns -1 on error or the number of characters output (may be zero).
 */
    typedef int (*logr_process_func_t)(const char *, int,
                                       const char *, const char *,
                                       logr_t *, int, FILE *f,
                                       const char *, size_t);

/**
 * Function overrides for formatting log entries.
 */
    typedef struct logr_ops {
        /**
         * override the priority conversion function.
         * \see logr_priority_func_t
         */
        logr_priority_func_t priority;
        /**
         * override the process function.
         * \see logr_process_func_t
         */
        logr_process_func_t process;
        /**
         * override the prefix function
         * \see logr_prefix_func_t
         */
        logr_prefix_func_t prefix;
    } logr_ops_t;

/**
 * Returns a pointer to the global logger instance.
 *
 * The global logger instance is definied as a static variable in the logr
 * library and is initialized when the shared library is loaded.
 *
 * \returns pointer to a global logr_t variable.
 */
    logr_t *logr_getlogger();

/**
 * Allocate and initialize a logr_t instance.
 * Creates a logr_t instance and associates it with the file specified by the
 * path.
 *
 * \param path The filepath of the log file, NULL for stderr.
 * \returns logr_t instance or NULL on error.
 */
    logr_t *logr_alloc(const char *path);

/**
 * Release the resources and deallocated the specified logr_t instance.
 *
 * \param logr The logr_t instance to use.
 */
    void logr_free(logr_t *logr);

/**
 * (re)open a file for the specified logr_t instance.
 *
 * Previously opened files are automatically closed, if applicable.
 *
 * \param logr The logr_t instance to use.
 * \param path The filepath of the log file, NULL for stderr.
 * \returns 0 on success or -1 on error.
 */
    int logr_open(logr_t *logr, const char *path);

/**
 * Print formatted output to the log.
 *
 * When <i>level</i> is less than or equal to the level specified by
 * <i>logr_set_level</i>, then print the message to the log file.  This
 * routine is implemented as a macro wrapper around <i>logr_xprintf</i>.
 *
 * \param logr The logr_t instance to use.
 * \param level Level for this message.
 * \param fmt <i>printf</i>-style format string.
 * \param args Variable arguments for <i>fmt</i>.
 */
#define logr_printf(logr, level, fmt, args...) \
    logr_xprintf(LOGR_XARGS, logr, level, fmt, ##args)
/// @cond
    int logr_xprintf(LOGR_XARGV, logr_t *logr, int level, const char *fmt, ...);
/// @endcond

/**
 * Set the maximum level to be output.
 *
 * All log messages whose level is strictly greater than the specied level
 * are ignored.
 *
 * \param logr The logr_t instance to use.
 * \param level The maximum level that will be printed.
 * \returns the number of bytes printed to the log or -1 on error.
 */
    int logr_set_level(logr_t *logr, unsigned int level);

/**
 * Get the current maximum level to be output.
 *
 * \returns the current log level.
 */
    unsigned int logr_get_level(logr_t *logr);

/**
 * Set the maximum size the log file may grow to before being rotated.
 *
 * \param logr The logr_t instance to use.
 * \param threshold The maximum file size in bytes.
 * \returns 0 on success or -1 on error.
 */
    int logr_set_threshold(logr_t *logr, off_t threshold);

/**
 * Format the timestamp according to the provided specification.
 *
 * The output format of the date can be set in the same manner as the
 * <i>strftime</i> function.
 *
 * \param logr The logr_t instance to use.
 * \param fmt The output format of the timestamp.
 * \return 0 on success and -1 on error.
 */
    int logr_set_timestamp_format(logr_t *logr, const char *fmt);

/**
 * Override default formatting operations.
 *
 * \param logr The logr_t instance to use.
 * \param ops Pointer to the operations to replace.
 * \return 0 on success and -1 on error;
 */
    int logr_set_ops(logr_t *logr, logr_ops_t *ops);

/**
 * Specify the prefix format for log entries.
 *
 * All entries are prepended with a string specified by the provided
 * format.  The <i>fmt</i> string may contain named directives, which are
 * dynamically replace in the output stream.  The default allowed directives
 * include:
 * \li <tt>%{file}s</tt> - source file name
 * \li <tt>%{line}d</tt> - source line number
 * \li <tt>%{func}s</tt> - calling function
 * \li <tt>%{pretty}</tt>s - 'pretty' function name
 * \li <tt>%{level}d</tt> - log level as an integer
 * \li <tt>%{level}s</tt> - log level name
 * \li <tt>%{priority}d</tt> - same as %{level}d
 * \li <tt>%{priority}s</tt> - same as %{level}s
 * \li <tt>%{pid}d</tt> - process ID of the current process
 * \li <tt>%{timestamp}s</tt> - entry timestamp using the specified format
 *
 * \param logr The logr_t instance to use.
 * \param fmt The format string specifying the prefix.
 * \return 0 on success or -1 on error;
 *
 * \see logr_set_timestamp_format
 * \see logr_priority_func_t
 * \see LOGR_PREFIX_FORMAT_BASIC
 * \see LOGR_PREFIX_FORMAT_VERBOSE
 */
    int logr_set_prefix_format(logr_t *logr, const char *fmt);

/* high-level interface */

/**
 * Shorthand for: logr_printf(logr_getlogger(), LOGR_EMERG, fmt, ...)
 * This routine is implemented as a macro.
 * \see logr_printf
 * \see logr_getlogger
 */
#define logr_emerg(fmt, args...) logr_emerg_(LOGR_XARGS, fmt, ## args)
/// @cond
    int logr_emerg_(LOGR_XARGV, const char *fmt, ...);
/// @endcond

/**
 * Shorthand for: logr_printf(logr_getlogger(), LOGR_ALERT, fmt, ...)
 * This routine is implemented as a macro.
 * \see logr_printf
 * \see logr_getlogger
 */
#define logr_alert(fmt, args...) logr_alert_(LOGR_XARGS, fmt, ## args)
/// @cond
    int logr_alert_(LOGR_XARGV, const char *fmt, ...);
/// @endcond

/**
 * Shorthand for: logr_printf(logr_getlogger(), LOGR_CRIT, fmt, ...)
 * This routine is implemented as a macro.
 * \see logr_printf
 * \see logr_getlogger
 */
#define logr_crit(fmt, args...) logr_crit_(LOGR_XARGS, fmt, ## args)
/// @cond
    int logr_crit_(LOGR_XARGV, const char *fmt, ...);
/// @endcond

/**
 * Shorthand for: logr_printf(logr_getlogger(), LOGR_ERR, fmt, ...)
 * This routine is implemented as a macro.
 * \see logr_printf
 * \see logr_getlogger
 */
#define logr_err(fmt, args...) logr_err_(LOGR_XARGS, fmt, ## args)
/// @cond
    int logr_err_(LOGR_XARGV, const char *fmt, ...);
/// @endcond

/**
 * Shorthand for: logr_printf(logr_getlogger(), LOGR_WARNING, fmt, ...)
 * This routine is implemented as a macro.
 * \see logr_printf
 * \see logr_getlogger
 */
#define logr_warning(fmt, args...) logr_warning_(LOGR_XARGS, fmt, ## args)
/// @cond
    int logr_warning_(LOGR_XARGV, const char *fmt, ...);
/// @endcond

/**
 * Shorthand for: logr_printf(logr_getlogger(), LOGR_NOTICE, fmt, ...)
 * This routine is implemented as a macro.
 * \see logr_printf
 * \see logr_getlogger
 */
#define logr_notice(fmt, args...) logr_notice_(LOGR_XARGS, fmt, ## args)
/// @cond
    int logr_notice_(LOGR_XARGV, const char *fmt, ...);
/// @endcond

/**
 * Shorthand for: logr_printf(logr_getlogger(), LOGR_INFO, fmt, ...)
 * This routine is implemented as a macro.
 * \see logr_printf
 * \see logr_getlogger
 */
#define logr_info(fmt, args...) logr_info_(LOGR_XARGS, fmt, ## args)
/// @cond
    int logr_info_(LOGR_XARGV, const char *fmt, ...);
/// @endcond

/**
 * Shorthand for: logr_printf(logr_getlogger(), LOGR_DEBUG, fmt, ...)
 * This routine is implemented as a macro.
 * \see logr_printf
 * \see logr_getlogger
 */
#define logr_debug(fmt, args...) logr_debug_(LOGR_XARGS, fmt, ## args)
/// @cond
    int logr_debug_(LOGR_XARGV, const char *fmt, ...);
/// @endcond

/**
 * Utility function to invoke the priority callback.
 *
 * \param logr The logr_t instance to use.
 * \param level Level for this message.
 * \returns string representation of <i>level</i> (never NULL).
 */
    const char *logr_util_priority(logr_t *logr, int level);

/**
 * Utility function to invoke the priority callback.
 *
 * \param logr The logr_t instance to use.
 * \param buf The output buffer for the formatted string.
 * \param size The size of the output buffer.
 * \returns 0 on success; -1 on error.
 */
    int logr_util_date(logr_t *logr, void *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* __LOGR_H__ */
