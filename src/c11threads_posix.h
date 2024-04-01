#ifndef C11THREADS_IMPL_POSIX_H
#define C11THREADS_IMPL_POSIX_H

MRB_INLINE int
thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
  int res;
  if (!thr) return thrd_error;
  res = pthread_create(thr, NULL, (void *(*)(void *))func, arg);
  if (0 == res) {
    return thrd_success;
  }

  return EAGAIN == res ? thrd_nomem : thrd_error;
}

MRB_INLINE int
thrd_join(thrd_t thr, int *res)
{
  void *retval;

  if (0 != pthread_join(thr, &retval)) {
    return thrd_error;
  }
  if (res) {
    *res = (int)retval;
  }
  return thrd_success;
}

#endif
