#ifndef __PTHREAD_H__
#define __PTHREAD_H__

#ifndef __WIN32
# error "__WIN32 required."
#endif

#include <windows.h>

#define PTHREAD_MUTEX_INITIALIZER {(void*)-1,-1,0,0,0,0}

typedef void pthread_mutexattr_t;
typedef CRITICAL_SECTION pthread_mutex_t;
static inline int pthread_mutex_lock(pthread_mutex_t *m)
{
	EnterCriticalSection(m);
	return 0;
}

static inline int pthread_mutex_unlock(pthread_mutex_t *m)
{
	LeaveCriticalSection(m);
	return 0;
}

static int inline pthread_mutex_trylock(pthread_mutex_t *m)
{
	return TryEnterCriticalSection(m) ? 0 : EBUSY;
}

static int inline pthread_mutex_init(pthread_mutex_t *m, pthread_mutexattr_t *a)
{
	(void) a;
	InitializeCriticalSection(m);
	return 0;
}

static int inline pthread_mutex_destroy(pthread_mutex_t *m)
{
	DeleteCriticalSection(m);
	return 0;
}

#endif /* __PTHREAD_H__ */
