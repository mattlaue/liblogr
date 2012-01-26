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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/wait.h>

#define SYSLOG_NAMES
#include "logr.h"

#define LOGR_SAVELOG_WAITPID_MAX 10
#define _XARGS file, line, func, pretty_func

#define STREQ(name, field, size)					\
    ((sizeof(name)-1 == size) && (strncasecmp(name, field, size) == 0))

static const char *logr_default_timestamp_fmt = LOGR_DEFAULT_DATE_FORMAT;

struct logr {
    FILE *f;
    char *path;
    pthread_mutex_t lock;
    char *prefix_fmt;
    char *timestamp_fmt;
    int prefix_len;
    int need_date;
    unsigned int level;
    off_t size;
    off_t threshold;
    pid_t savelog_pid;
    unsigned int savelog_waitpid_retries;
    logr_ops_t ops;
};

static struct logr logr = {
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .level = LOG_ERR,
};

logr_t *
logr_getlogger() {
    return &logr;
}

static inline void
_logr_init(logr_t *logr)
{
    memset(logr, 0, sizeof(struct logr));
    pthread_mutex_init(&logr->lock, NULL);
    logr->level = LOG_ERR;
}

static inline int
_logr_errno(int x)
{
    errno = x;
    return -1;
}

/*
 * Wrapper around abort() that gets called for 'bad' format strings.
 * This approach mimics libc's handling of indexed directives.
 */
static void 
_logr_fatal(const char *msg)
{
    fputs(msg, stderr);
    abort();
}

static inline bool
_logr_field_char(const char c)
{
    if ((c == '{') || (c == '}') || (c == '%')) {
	return false;
    }
    if (isprint(c)) {
	return true;
    }
    return false;
}

static inline bool
_logr_specifier_char(const char c)
{
    return isalpha(c);
}

static inline void
logr_lock(logr_t *logr)
{
    pthread_mutex_lock(&logr->lock);
}

static inline void
logr_unlock(logr_t *logr)
{
    pthread_mutex_unlock(&logr->lock);
}

const char *
logr_util_priority(logr_t *unused, int level) 
{
    int i = 0;
    CODE *p;

    for (p = &prioritynames[i]; p->c_name != NULL; p = &prioritynames[++i]) {
	if (p->c_val == level)
	    return p->c_name;
    }
    return "unknown";
}

int
logr_set_level(logr_t *logr, unsigned int level)
{
    if (logr == NULL) {
	return _logr_errno(EINVAL);
    }
    logr->level = level;
    return 0;
}

static void
_logr_sigchild(int sig, siginfo_t *si, void *unused)
{
    pid_t pid;

    do {
	pid = waitpid(-1, NULL, WNOHANG);
    } while (pid > 0);
}

int 
logr_set_threshold(logr_t *logr, off_t threshold)
{
    struct sigaction sa;

    if (logr == NULL) {
	return _logr_errno(EINVAL);
    }
    logr->threshold = threshold;
    if (threshold == 0) {
	return 0;
    }
    sigaction(SIGCHLD, NULL, &sa);
    if (sa.sa_handler == NULL && sa.sa_sigaction == NULL) {
	sa.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = _logr_sigchild;
	sigaction(SIGCHLD, &sa, NULL);
    }
    
    return 0;
}

void
logr_free(logr_t *logr)
{
    int tmp = errno;

    if (logr == NULL)
	return;
    logr_lock(logr);
    if (logr->f != NULL) {
	fclose(logr->f);
    }
    if (logr->prefix_fmt != NULL) {
	free(logr->prefix_fmt);
    }
    if (logr->timestamp_fmt != NULL) {
	free(logr->timestamp_fmt);
    }
    logr_unlock(logr);
    free(logr);

    errno = tmp;
}

static void
_logr_close(logr_t *logr)
{
    int tmp = errno;

    if (logr->f != NULL) {
	fclose(logr->f);
	logr->f = NULL;
    }
    if (logr->path != NULL) {
	free(logr->path);
	logr->path = NULL;
    }
    errno = tmp;
}

int
logr_open(logr_t *logr, const char *p)
{
    FILE *f;
    char *path;
    long pos;

    if (logr == NULL) {
	return _logr_errno(EINVAL);
    }

    /* just use stderr */
    if (p == NULL) {
	_logr_close(logr);
	return 0;
    }

    path = strdup(p);
    if (path == NULL) {
	return -1;
    }

    f = fopen(path, "a");
    if (f == NULL) {
	free(path);
	return -1;
    }

    /* file position is guaranteed to be at the end with 'a' */
    pos = ftell(f);
    if (pos < 0) {
	fclose(f);
	free(path);
	return -1;
    }

    _logr_close(logr);
    logr->f = f;
    logr->path = path;
    logr->size = pos;
    
    return 0;
}

logr_t *
logr_alloc(const char *path)
{
    int retval;
    
    logr_t *logr = (logr_t *)malloc(sizeof(logr_t));
    if (logr == NULL) {
	return NULL;
    }

    _logr_init(logr);

    retval = logr_open(logr, path);
    if (retval < 0) {
	logr_free(logr);
	return NULL;
    }

    return logr;
}

int
logr_set_prefix_format(logr_t *logr, const char *fmt)
{
    char *prefix_fmt;
    int need_date;

    if (logr == NULL || fmt == NULL)
	return _logr_errno(EINVAL);

    prefix_fmt = strdup(fmt);
    if (prefix_fmt == NULL)
	return _logr_errno(ENOMEM);

    if (strstr(prefix_fmt, "%5$s") != NULL) {
	need_date = 1;
    } else {
	need_date = 0;
    }

    logr_lock(logr);
    free(logr->prefix_fmt);
    logr->prefix_fmt = prefix_fmt;
    logr->prefix_len = strlen(prefix_fmt);
    logr->need_date = need_date;
    logr_unlock(logr);

    return 0;
}

int
logr_set_timestamp_format(logr_t *logr, const char *fmt)
{
    char *timestamp_fmt, *tmp = NULL;

    if (logr == NULL || fmt == NULL) {
	return _logr_errno(EINVAL);
    }

    timestamp_fmt = strdup(fmt);
    if (timestamp_fmt == NULL) {
	return _logr_errno(ENOMEM);
    }

    logr_lock(logr);
    if ((logr->timestamp_fmt != NULL) && 
	(logr->timestamp_fmt != logr_default_timestamp_fmt)) {
	tmp = logr->timestamp_fmt;
    }
    logr->timestamp_fmt = timestamp_fmt;
    logr_unlock(logr);

    if (tmp != NULL) {
	free(tmp);
    }

    return 0;
}

int 
logr_set_ops(logr_t *logr, logr_ops_t *ops)
{
    if (logr == NULL) {
	return -1;
    }
    if (ops == NULL) {
	return 0;
    }
    logr->ops = *ops;
    return 0;
}

pid_t
_logr_savelog(const char *path) 
{
    pid_t pid;
    

    pid = fork();
    if (pid == -1) {
	return -1;
    }

    if (pid == 0) {
	/* child */
	int retval;
	retval = execl("/usr/bin/savelog", "-d", "-j", "-q", "-p", path, NULL);
	exit(retval);
    }
    return pid;
}

static int
_logr_fputs(const char *p, FILE *f)
{
    int retval, c, n;

    for (n = 0; ((c = *p) != 0); p++,n++){
	retval = fputc(c, f);
	if (retval == EOF) {
	    return -1;
	}
    }
    return n;
}

static int
_logr_timestamp(const char *fmt, char specifier, FILE *f)
{
    int retval;
    char buf[LOGR_MAX_TIMESTAMP_SIZE];
    struct tm tm;
    time_t t;
    
    time(&t);

    if (specifier == 'd') {
	return fprintf(f, "%ld", (long)t);
    } else if (specifier == 'u') {
	return fprintf(f, "%lu", (unsigned long)t);
    } else if (specifier != 's') {
	return 0;
    }
    
    localtime_r(&t, &tm);
    if ((fmt == NULL) || (fmt[0] == 0)) {
	fmt = LOGR_DEFAULT_DATE_FORMAT;
    }
    
    retval = strftime(buf, sizeof(buf), fmt, &tm);
    if (retval <= 0) {
	return -1;
    }
    buf[LOGR_MAX_TIMESTAMP_SIZE-1] = 0;

    retval = _logr_fputs(buf, f);
    if (retval < 0) {
	return -1;
    }
    return retval;
}

int
logr_util_process(LOGR_XARGV, logr_t *logr, int level, FILE *f,
		  const char *field, size_t size, char specifier)
{
    /* call user specified conversion function, if applicable */

    if (STREQ("file", field, size)) {
	return (specifier == 's') ? _logr_fputs(file, f) : 0;
    } else if (STREQ("line", field, size)) {
	return (specifier == 'd') ? fprintf(f, "%d", line) : 0;
    } else if (STREQ("func", field, size)) {
	return (specifier == 's') ? _logr_fputs(func, f) : 0;
    } else if (STREQ("pretty", field, size)) {
	return (specifier == 's') ? _logr_fputs(pretty_func, f) : 0;
    } else if (STREQ("level", field, size) || STREQ("priority", field, size)) {
	if (specifier == 's') {
	    return _logr_fputs(logr_util_priority(logr, level), f);
	} else if (specifier == 'd') {
	    return fprintf(f, "%d", level);
	} else {
	    return 0;
	}
    } else if (STREQ("pid", field, size)) {
	return (specifier == 'd') ? fprintf(f, "%d", getpid()) : 0;
    } else if (STREQ("timestamp", field, size)) {
	return _logr_timestamp(logr->timestamp_fmt, specifier, f);
    }
    return 0;
}

static int 
logr_prefix(LOGR_XARGV, logr_t *logr, int level, FILE *f, const char *fmt)
{
    int retval, total = 0, n = 0;
    const char *p = fmt, *field = NULL;
    enum {
	CHARACTER,        // plain format char
	DIRECTIVE_SYMBOL, // %
	FIELD,            // between {}
	FIELD_CLOSE       // saw }, need specifier.
    } state = CHARACTER;

    for (; *p != 0; p++) {
	switch (state) {
	case CHARACTER:
	    if (*p == '%') {
		state = DIRECTIVE_SYMBOL;
	    } else if (isprint(*p) || (*p == '\r') || (*p == '\n')) {
		retval = fputc(*p, f);
		if (retval < 0) {
		    return -1;
		}
		total++;
	    } else {
		_logr_fatal("*** invalid format character ***\n");
	    }
	    break;
	case DIRECTIVE_SYMBOL:
	    if (*p == '%') {
		retval = fputc(*p, f);
		if (retval < 0) {
		    return -1;
		}
		total++;
		state = CHARACTER;
	    } else if (*p == '{') {
		state = FIELD;
		n = 0;
	    } else {
		_logr_fatal("*** invalid directive ***\n");
	    }
	    break;
	case FIELD:
	    if (*p == '}') {
		state = FIELD_CLOSE;
		break;
	    } else if (!_logr_field_char(*p)) {
		_logr_fatal("*** invalid field character ***\n");
	    }
	    if (n == 0) {
		field = p;
	    }
	    n++;
	    break;
	case FIELD_CLOSE:
	    if (!_logr_specifier_char(*p)) {
		_logr_fatal("*** invalid specifier ***\n");
	    }
	    if (n != 0) {
		retval = logr_util_process(_XARGS, logr, level, f, 
					   field, n, *p);
		if (retval < 0) {
		    return -1;
		}
		total += retval;
	    }
	    state = CHARACTER;
	    break;
	}
    }

    if (state != CHARACTER) {
	_logr_fatal("*** premature end of format ***\n");
    }

    return total;
}

static inline int
_logr_util_prefix(LOGR_XARGV, logr_t *logr, int level, FILE *f)
{
    if (logr->ops.prefix != NULL) {
	return logr->ops.prefix(_XARGS, logr, level, f, logr->prefix_fmt);
    }
    if (logr->prefix_fmt != NULL) {
	return logr_prefix(_XARGS, logr, level, f, logr->prefix_fmt);
    }
    return 0;
}

int
logr_vxprintf(LOGR_XARGV, logr_t *logr, int level, const char *fmt, va_list ap)
{
    int n = 0, retval;
    FILE *f;
    pid_t pid;

    logr_lock(logr);

    if (logr->savelog_pid > 0) {
	if (logr->savelog_waitpid_retries > 0) {
	    pid = waitpid(logr->savelog_pid, NULL, WNOHANG | WUNTRACED);
	} else {
	    pid = waitpid(logr->savelog_pid, NULL, 0);
	}
	if (pid != 0) {
	    /* anything but zero means the process doesn't exist. */
	    logr->savelog_pid = 0;
	    logr->savelog_waitpid_retries = 0;
	    /* if this fails, we revert to stderr. */
	    logr->f = freopen(logr->path, "a", logr->f);
	    logr->size = 0;
	} else {
	    logr->savelog_waitpid_retries--;
	}
    }

    if (logr->level < level) {
	logr_unlock(logr);
	return 0;
    }

    f = (logr->f != NULL) ? logr->f : stderr;

    retval = _logr_util_prefix(_XARGS, logr, level, f);
    if (retval < 0) {
	logr_unlock(logr);
	return -1;
    }
    n += retval; 

    n += vfprintf(f, fmt, ap);
    logr->size += n;

    if ((logr->savelog_pid == 0) && (logr->threshold != 0) && (
	    logr->size > logr->threshold)) {
	/* do a flush to ensure the file is not erroneously empty. */
	fflush(logr->f);

	pid = _logr_savelog(logr->path);
	if (pid > 0) {
	    logr->savelog_pid = pid;
	    logr->savelog_waitpid_retries = LOGR_SAVELOG_WAITPID_MAX;
 	}
    }
    logr_unlock(logr);

    return n;
}


int
logr_xprintf(LOGR_XARGV, logr_t *logr, int level, const char *fmt, ...)
{
    va_list ap;
    int n = 0;

    va_start(ap, fmt);
    n += logr_vxprintf(_XARGS, logr, level, fmt, ap);
    va_end(ap);    
    return n;
}

int
logr_emerg_(LOGR_XARGV, const char *fmt, ...)
{
    va_list ap;
    int n = 0;

    va_start(ap, fmt);
    n += logr_vxprintf(_XARGS, &logr, LOG_EMERG, fmt, ap);
    va_end(ap);    
    return n;
}

int
logr_alert_(LOGR_XARGV, const char *fmt, ...)
{
    va_list ap;
    int n = 0;

    va_start(ap, fmt);
    n += logr_vxprintf(_XARGS, &logr, LOG_ALERT, fmt, ap);
    va_end(ap);
    return n;
}

int
logr_crit_(LOGR_XARGV, const char *fmt, ...)
{
    va_list ap;
    int n = 0;

    va_start(ap, fmt);
    n += logr_vxprintf(_XARGS, &logr, LOG_CRIT, fmt, ap);
    va_end(ap);    
    return n;
}

int
logr_err_(LOGR_XARGV, const char *fmt, ...)
{
    va_list ap;
    int n = 0;

    va_start(ap, fmt);
    n += logr_vxprintf(_XARGS, &logr, LOG_ERR, fmt, ap);
    va_end(ap);    
    return n;
}

int
logr_warning_(LOGR_XARGV, const char *fmt, ...)
{
    va_list ap;
    int n = 0;

    va_start(ap, fmt);
    n += logr_vxprintf(_XARGS, &logr, LOG_WARNING, fmt, ap);
    va_end(ap);    
    return n;
}

int
logr_notice_(LOGR_XARGV, const char *fmt, ...)
{
    va_list ap;
    int n = 0;

    va_start(ap, fmt);
    n += logr_vxprintf(_XARGS, &logr, LOG_NOTICE, fmt, ap);
    va_end(ap);    
    return n;
}

int
logr_info_(LOGR_XARGV, const char *fmt, ...)
{
    va_list ap;
    int n = 0;

    va_start(ap, fmt);
    n += logr_vxprintf(_XARGS, &logr, LOG_INFO, fmt, ap);
    va_end(ap);    
    return n;
}

int
logr_debug_(LOGR_XARGV, const char *fmt, ...)
{
    va_list ap;
    int n = 0;

    va_start(ap, fmt);
    n += logr_vxprintf(_XARGS, &logr, LOG_DEBUG, fmt, ap);
    va_end(ap);    
    return n;
}
