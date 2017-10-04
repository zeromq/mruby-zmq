#ifndef C11THREADS_IMPL_POSIX_H
#define C11THREADS_IMPL_POSIX_H

MRB_INLINE int
thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
  int res;
  if (!thr) return thrd_error;
  res = pthread_create(thr, NULL, (void *(*)(void *))func, arg);
  if (res == 0) {
    return thrd_success;
  }

  return res == EAGAIN ? thrd_nomem : thrd_error;
}

MRB_INLINE int
thrd_join(thrd_t thr, int *res)
{
  void *retval;

  if (pthread_join(thr, &retval) != 0) {
    return thrd_error;
  }
  if (res) {
    *res = (int)retval;
  }
  return thrd_success;
}

#endif
