/* Copyright (C) 2012 Akiri Solutions, Inc.
 * For conditions of distribution and use, see copyright notice in logr.h
 */
#include <string.h>
#include <sched.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <logr.h>

/* Demonstrate using logr with threads and over-riding the prefix function. */

static const char *names[] = {
    "Jack", "Wendy", "Danny", "Dick", "Stuart", "Delbert",
    "Lloyd", "Larry", "Bill", "Stanley", "Stephen", "Diane"    
};

#define LOGFILE "file.log"
#define MSG "All work and no play makes Jack a dull boy.\n"
#define NAME_COUNT (sizeof(names) / sizeof(const char *))
#define THRESHOLD (1024 * 1024)

static int
threads_prefix(const char *file, int line,
	       const char *func, const char *pretty_func,
	       logr_t *logr, int level, FILE *f, const char *fmt)
{
    char buf[17];

    memset(buf, 0, sizeof(buf));
    prctl(PR_GET_NAME, buf);

    return fprintf(f, "%8s[%s]: ", buf, logr_util_priority(logr, level));
}

static void *
start_routine(void *arg)
{
    const char *name = (const char *)arg;
    int i, n = (2.5 * THRESHOLD) / strlen(MSG);

    prctl(PR_SET_NAME, name);

    for (i = 0; i < n; i++) {
	logr_err(MSG);
	if ((i % 16) == 15) {
	    /* give up the CPU every 16 lines... */
	    sched_yield();
	}
    }

    return NULL;
}

int
main(int argc, char **argv)
{
    int i, retval;
    logr_t *logr = logr_getlogger();
    pthread_t threads[NAME_COUNT];

    static logr_ops_t ops = {
	.prefix = threads_prefix,
    };

    logr_set_ops(logr, &ops);

    retval = logr_open(logr, LOGFILE);
    if (retval != 0) {
	perror("logr_open");
	return -1;
    }

    retval = logr_set_threshold(logr, THRESHOLD);
    if (retval < 0) {
	perror("logr_set_threshold");
	return -1;
    }

    for (i = 0; i < NAME_COUNT; i++) {
	retval = pthread_create(&threads[i], NULL, 
				start_routine, (void *)names[i]);
	if (retval != 0) {
	    perror("pthread_create");
	    return -1;
	}
    }

    for (i = 0; i < NAME_COUNT; i++) {
	pthread_join(threads[i], NULL);
    }

    printf("See '%s' for additional output.\n", LOGFILE);
    return 0;
}
