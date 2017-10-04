#ifndef C11THREADS_H
#define C11THREADS_H

#ifdef _MSC_VER
#include <windows.h>
#include <process.h>
#include <errno.h>
typedef HANDLE thrd_t;
#else
#include <pthread.h>
typedef pthread_t thrd_t;
#endif

typedef int (*thrd_start_t)(void*);
enum {
  thrd_success,
  thrd_timedout,
  thrd_busy,
  thrd_error,
  thrd_nomem
};


MRB_INLINE int thrd_create(thrd_t *thr, thrd_start_t func, void *arg);

MRB_INLINE int thrd_join(thrd_t thr, int *res);

#ifdef _MSC_VER
#include "c11threads_win32.h"
#else
#include "c11threads_posix.h"
#endif

#endif
