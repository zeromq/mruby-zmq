#ifndef MRB_LIBZMQ_H
#define MRB_LIBZMQ_H

#include "mruby/zmq.h"
#include <stdlib.h>
#include <zmq.h>
#if ZMQ_VERSION < ZMQ_MAKE_VERSION(4,1,0)
  #error "mruby-zmq needs at least libzmq-4.1"
#endif
#include <mruby/data.h>
#include <mruby/error.h>
#include <mruby/variable.h>
#include <mruby/class.h>
#include <mruby/string.h>
#include <mruby/hash.h>
#include <mruby/array.h>
#include <mruby/value.h>
#include <mruby/throw.h>
#include <mruby/gc.h>

#if !defined(SOCKET) && !defined(_WIN32)
#define SOCKET int
#endif

#if (__GNUC__ >= 3) || (__INTEL_COMPILER >= 800) || defined(__clang__)
# define likely(x) __builtin_expect(!!(x), 1)
# define unlikely(x) __builtin_expect(!!(x), 0)
#else
# define likely(x) (x)
# define unlikely(x) (x)
#endif

#ifdef _WIN32
#define mrb_zmq_errno() zmq_errno()
#else
#define mrb_zmq_errno() errno
#endif

#define MRB_LIBZMQ_CONTEXT() (mrb_cptr(mrb_const_get(mrb, mrb_obj_value(mrb_module_get(mrb, "LibZMQ")), mrb_intern_lit(mrb, "_Context"))))

MRB_INLINE void
mrb_zmq_handle_error(mrb_state *mrb, const char *func)
{
  switch(mrb_zmq_errno()) {
    case EFSM: {
      mrb_raisef(mrb, E_EFSM_ERROR, "%S: %S", mrb_str_new_cstr(mrb, func), mrb_str_new_cstr(mrb, zmq_strerror(mrb_zmq_errno())));
    } break;
    case ENOCOMPATPROTO: {
      mrb_raisef(mrb, E_ENOCOMPATPROTO_ERROR, "%S: %S", mrb_str_new_cstr(mrb, func), mrb_str_new_cstr(mrb, zmq_strerror(mrb_zmq_errno())));
    } break;
    case ETERM: {
      mrb_raisef(mrb, E_ETERM_ERROR, "%S: %S", mrb_str_new_cstr(mrb, func), mrb_str_new_cstr(mrb, zmq_strerror(mrb_zmq_errno())));
    } break;
    case EMTHREAD: {
      mrb_raisef(mrb, E_EMTHREAD_ERROR, "%S: %S", mrb_str_new_cstr(mrb, func), mrb_str_new_cstr(mrb, zmq_strerror(mrb_zmq_errno())));
    } break;
    default: {
      mrb_sys_fail(mrb, func);
    }
  }
}

static void
mrb_zmq_gc_close(mrb_state *mrb, void *socket)
{
  if (likely(socket)) {
    int disable = 0;
    zmq_setsockopt(socket, ZMQ_LINGER, &disable, sizeof(disable));
    zmq_close(socket);
  }
}

static const struct mrb_data_type mrb_zmq_socket_type = {
  "$i_mrb_zmq_socket_type", mrb_zmq_gc_close
};

static void
mrb_zmq_gc_msg_close(mrb_state *mrb, void *msg)
{
  zmq_msg_close((zmq_msg_t *) msg);
  mrb_free(mrb, msg);
}

static const struct mrb_data_type mrb_zmq_msg_type = {
  "$i_mrb_zmq_msg_type", mrb_zmq_gc_msg_close
};

typedef struct {
  void *frontend;
  void *backend;
  char *argv_packed;
  mrb_int argv_len;
  void *thread;
  void *backend_ctx;
} mrb_zmq_thread_data_t;

static void
mrb_zmq_gc_threadclose(mrb_state *mrb, void *mrb_zmq_thread_data_)
{
  if (likely(mrb_zmq_thread_data_)) {
    mrb_zmq_thread_data_t *mrb_zmq_thread_data = (mrb_zmq_thread_data_t *) mrb_zmq_thread_data_;
    if (likely(mrb_zmq_thread_data->frontend)) {
      int disable = 0;
      zmq_setsockopt(mrb_zmq_thread_data->frontend, ZMQ_SNDTIMEO, &disable, sizeof(disable));
      zmq_send(mrb_zmq_thread_data->frontend, "TERM$", 5, 0);
      zmq_setsockopt(mrb_zmq_thread_data->frontend, ZMQ_LINGER, &disable, sizeof(disable));
      zmq_close(mrb_zmq_thread_data->frontend);
    }
    if (likely(mrb_zmq_thread_data->backend_ctx))
      zmq_ctx_shutdown(mrb_zmq_thread_data->backend_ctx);
    if (likely(mrb_zmq_thread_data->backend))
      zmq_close(mrb_zmq_thread_data->backend);
    if (likely(mrb_zmq_thread_data->thread))
      zmq_threadclose(mrb_zmq_thread_data->thread);
    mrb_free(mrb, mrb_zmq_thread_data->argv_packed);
    mrb_free(mrb, mrb_zmq_thread_data_);
  }
}

static const struct mrb_data_type mrb_zmq_thread_type = {
  "$i_mrb_zmq_thread_type", mrb_zmq_gc_threadclose
};

#ifdef ZMQ_HAVE_POLLER
static void
mrb_zmq_gc_poller_destroy(mrb_state *mrb, void *poller)
{
  zmq_poller_destroy(&poller);
}

static const struct mrb_data_type mrb_zmq_poller_type = {
  "$i_mrb_zmq_poller_type", mrb_zmq_gc_poller_destroy
};
#endif //ZMQ_HAVE_POLLER

static void
mrb_zmq_thread_close_gem_final(mrb_state *mrb, struct RBasic *obj, void *target_module)
{
  /* filter dead objects */
  if (mrb_object_dead_p(mrb, obj)) {
    return;
  }

  /* filter internal objects */
  switch (obj->tt) {
  case MRB_TT_ENV:
  case MRB_TT_ICLASS:
    return;
  default:
    break;
  }

  /* filter half baked (or internal) objects */
  if (!obj->c) return;

  if (mrb_obj_is_kind_of(mrb, mrb_obj_value(obj), (struct RClass *)target_module)) {
    mrb_value thread_val = mrb_obj_value(obj);
    mrb_iv_remove(mrb, thread_val, mrb_intern_lit(mrb, "@pipe"));
    mrb_zmq_gc_threadclose(mrb, DATA_PTR(thread_val));
    mrb_data_init(thread_val, NULL, NULL);
  }
}

static void
mrb_zmq_zmq_close_gem_final(mrb_state *mrb, struct RBasic *obj, void *target_module)
{
  /* filter dead objects */
  if (mrb_object_dead_p(mrb, obj)) {
    return;
  }

  /* filter internal objects */
  switch (obj->tt) {
  case MRB_TT_ENV:
  case MRB_TT_ICLASS:
    return;
  default:
    break;
  }

  /* filter half baked (or internal) objects */
  if (!obj->c) return;

  if (mrb_obj_is_kind_of(mrb, mrb_obj_value(obj), (struct RClass *)target_module)) {
    mrb_value socket_val = mrb_obj_value(obj);
    mrb_zmq_gc_close(mrb, DATA_PTR(socket_val));
    mrb_data_init(socket_val, NULL, NULL);
  }
}

#endif
