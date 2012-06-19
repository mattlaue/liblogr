/* Copyright (C) 2012 Akiri Solutions, Inc.
 * For conditions of distribution and use, see copyright notice in logr.h
 */
#include <string.h>
#include <logr.h>
#include <assert.h>
#include <errno.h>

#ifdef __WIN32
#include <windows.h>
#include <process.h>
typedef HANDLE thread_t;
#define TRV unsigned int
#define TCALL __stdcall
#ifndef ENOTSUP
#define ENOTSUP 48 /* missing from mingw */
#endif
#else
#include <sched.h>
#include <pthread.h>
typedef pthread_t thread_t;
#define TRV void *
#define TCALL
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#ifdef HAVE_PRCTL_H
#include <sys/prctl.h>
#endif
#endif

/* Demonstrate using logr with threads and over-riding the prefix function. */

static const char *names[] = {
    "Jack", "Wendy", "Danny", "Dick", "Stuart", "Delbert",
    "Lloyd", "Larry", "Bill", "Stanley", "Stephen", "Diane"
};

#ifdef __WIN32
DWORD tls_i;
#endif

#define LOGFILE "file.log"
#define MSG "All work and no play makes Jack a dull boy.\n"
#define NAME_COUNT (sizeof(names) / sizeof(const char *))
#define THRESHOLD (1024 * 1024)

int create_thread(thread_t *t, TRV (TCALL *f)(void *), void *arg);
void wait_for_thread_completion(thread_t t);
void yield_thread();

static int
threads_prefix(const char *file, int line,
	       const char *func, const char *pretty_func,
	       logr_t *logr, int level, FILE *f, const char *fmt)
{
    char buf[17];

    /* get a name on various platforms... */
#ifdef HAVE_PRCTL_H
    memset(buf, 0, sizeof(buf));
    prctl(PR_GET_NAME, buf);
#elif defined(__WIN32)
    snprintf(buf, sizeof(buf), "%s", (char *)TlsGetValue(tls_i));
#else
    static int n = 0;
    snprintf(buf, sizeof(buf), "%d", ++n);
#endif
    return fprintf(f, "%8s[%s]: ", buf, logr_util_priority(logr, level));
}

TRV TCALL
thread_routine(void *arg)
{
    int i, n = (2.5 * THRESHOLD) / strlen(MSG);

#ifdef HAVE_PRCTL_H
    const char *name = (const char *)arg;
    prctl(PR_SET_NAME, name);
#elif defined(__WIN32)
    const char *name = (const char *)arg;
    BOOL b;
    b = TlsSetValue(tls_i, (PVOID)name);
    assert(b);
#endif

    for (i = 0; i < n; i++) {
	logr_err(MSG);
	if ((i % 16) == 15) {
	    /* give up the CPU every 16 lines... */
            yield_thread();
	}
    }

    return (TRV)0;
}

int
main(int argc, char **argv)
{
    int i, retval;
    logr_t *logr = logr_getlogger();
    thread_t threads[NAME_COUNT];

    static logr_ops_t ops = {
	.prefix = threads_prefix,
    };

#ifdef __WIN32
    tls_i = TlsAlloc();
    assert(tls_i != TLS_OUT_OF_INDEXES);
#endif

    logr_set_ops(logr, &ops);

    retval = logr_open(logr, LOGFILE);
    if (retval != 0) {
	perror("logr_open");
	return -1;
    }

    retval = logr_set_threshold(logr, THRESHOLD);
    if (retval < 0) {
        if (errno != ENOTSUP) {
            perror("logr_set_threshold");
            return -1;
        }
    }

    for (i = 0; i < NAME_COUNT; i++) {
	retval = create_thread(&threads[i], thread_routine, (void *)names[i]);
	if (retval != 0) {
	    perror("create_thread");
	    return -1;
	}
    }

    for (i = 0; i < NAME_COUNT; i++) {
	wait_for_thread_completion(threads[i]);
    }

    printf("See '%s' for additional output.\n", LOGFILE);
    return 0;
}

int
create_thread(thread_t *t, TRV (TCALL *f)(void *), void *arg)
{
    int retval = 0;

#ifdef __WIN32
    *t = (HANDLE)_beginthreadex(NULL, 0, f, arg, 0, NULL);
    if (*t == 0) {
        retval = -1;
    }
#else
    retval = pthread_create(t, NULL, f, arg);
#endif
    return retval;
}

void
wait_for_thread_completion(thread_t t)
{
#ifdef __WIN32
    WaitForSingleObject(t, INFINITE);
    CloseHandle(t);
#else
    pthread_join(t, NULL);
#endif
}

void
yield_thread(void)
{
#ifdef __WIN32
    SwitchToThread();
#else
    sched_yield();
#endif
}
