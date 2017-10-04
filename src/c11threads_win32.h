#ifndef C11THREADS_IMPL_WIN32_H
#define C11THREADS_IMPL_WIN32_H

MRB_INLINE int
thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
  uintptr_t handle;
  if (!thr) return thrd_error;
  handle = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)func, arg, 0, NULL);
  if (handle == 0) {
      if (errno == EAGAIN || errno == EACCES) {
        return thrd_nomem;
      }
      return thrd_error;
  }
  *thr = (thrd_t)handle;
  return thrd_success;
}

MRB_INLINE int
thrd_join(thrd_t thr, int *res)
{
    DWORD w, code;
    w = WaitForSingleObject(thr, INFINITE);
    if (w != WAIT_OBJECT_0)
        return thrd_error;
    if (res) {
        if (!GetExitCodeThread(thr, &code)) {
            CloseHandle(thr);
            return thrd_error;
        }
        *res = (int)code;
    }
    CloseHandle(thr);
    return thrd_success;
}

#endif
