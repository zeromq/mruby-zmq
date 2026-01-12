#ifndef MRB_LIBZMQ_H
#define MRB_LIBZMQ_H

#include <zmq.h>
#if (ZMQ_VERSION < ZMQ_MAKE_VERSION(4,1,0))
  #error "mruby-zmq needs at least libzmq-4.1"
#endif
#include <assert.h>
#include <stdlib.h>
#include "mruby/zmq.h"
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
#include <mruby/numeric.h>
#include <mruby/proc.h>
#ifdef HAVE_IFADDRS_H
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#endif
#include <mruby/presym.h>
#include <mruby/num_helpers.hpp>
#include <mruby/branch_pred.h>
#include <vector>
#include <cstddef>

#define NELEMS(args) (sizeof(args) / sizeof(args[0]))

#if !defined(SOCKET) && !defined(_WIN32)
#define SOCKET int
#endif

#ifdef _WIN32
#define mrb_zmq_errno() zmq_errno()
#else
#define mrb_zmq_errno() errno
#endif


#define MRB_LIBZMQ_CONTEXT(mrb) (mrb_cptr(mrb_const_get(mrb, mrb_obj_value(mrb_module_get_id(mrb, MRB_SYM(LibZMQ))), MRB_SYM(__CTX__))))

#ifdef MRB_EACH_OBJ_OK
static int
#else
static void
#endif
mrb_zmq_zmq_close_gem_final(mrb_state *mrb, struct RBasic *obj, void *socket_class)
{
  /* filter dead objects */
  if (mrb_object_dead_p(mrb, obj)) {
#ifdef MRB_EACH_OBJ_OK
    return MRB_EACH_OBJ_OK;
#else
    return;
#endif
  }

  /* filter internal objects */
  switch (obj->tt) {
  case MRB_TT_ENV:
  case MRB_TT_ICLASS:
#ifdef MRB_EACH_OBJ_OK
    return MRB_EACH_OBJ_OK;
#else
    return;
#endif
  default:
    break;
  }

  /* filter half baked (or internal) objects */
  if (!obj->c) {
#ifdef MRB_EACH_OBJ_OK
    return MRB_EACH_OBJ_OK;
#else
    return;
#endif
  }

  mrb_value socket_val = mrb_obj_value(obj);
  if (mrb_obj_is_kind_of(mrb, socket_val, (struct RClass *)socket_class)) {
    void *socket = DATA_PTR(socket_val);
    if (socket) {
      int wait500ms = 500; // we wait up to 500 miliseconds for each socket to close when mruby is closed via mrb_close(mrb).
      zmq_setsockopt(socket, ZMQ_LINGER, &wait500ms, sizeof(wait500ms));
      zmq_close(socket);
      mrb_data_init(socket_val, NULL, NULL);
    }
  }
#ifdef  MRB_EACH_OBJ_OK
  return MRB_EACH_OBJ_OK;
#endif
}

MRB_API void
mrb_zmq_ctx_shutdown_close_and_term(mrb_state* mrb)
{
  void *context_ = MRB_LIBZMQ_CONTEXT(mrb);
  zmq_ctx_shutdown(context_);
  mrb_objspace_each_objects(mrb, mrb_zmq_zmq_close_gem_final, mrb_class_get_under_id(mrb, mrb_module_get_id(mrb, MRB_SYM(ZMQ)), MRB_SYM(Socket)));
  zmq_ctx_term(context_);
}

MRB_API void
mrb_zmq_set_context(mrb_state *mrb, void *context_)
{
  int rc = zmq_ctx_get (context_, ZMQ_IO_THREADS);
  if (rc == -1) {
    mrb_sys_fail(mrb, "not a zmq context");
  }
  mrb_value foreigen_context = mrb_const_get(mrb, mrb_obj_value(mrb_module_get_id(mrb, MRB_SYM(LibZMQ))), MRB_SYM(__foreigen_context__));
  if (!mrb_bool(foreigen_context)) {
    mrb_zmq_ctx_shutdown_close_and_term(mrb);
  }
  mrb_define_const_id(mrb, mrb_module_get_id(mrb, MRB_SYM(LibZMQ)), MRB_SYM(__CTX__),
                      mrb_cptr_value(mrb, context_));
  mrb_define_const_id(mrb, mrb_module_get_id(mrb, MRB_SYM(LibZMQ)), MRB_SYM(__foreigen_context__),
                      mrb_true_value());
  mrb_load_string(mrb, "ZMQ.logger = ZMQ::Logger.new(ENV['ZMQ_LOGGER_ENDPOINT'], ENV['ZMQ_LOGGER_IDENT'])");
  if (mrb->exc) {
    mrb_exc_raise(mrb, mrb_obj_value(mrb->exc));
  }
}

static void
mrb_zmq_handle_error(mrb_state *mrb, const char *func)
{
  switch(mrb_zmq_errno()) {
    case EFSM:
      mrb_raisef(mrb, E_EFSM_ERROR, "%S: %S", mrb_str_new_cstr(mrb, func), mrb_str_new_cstr(mrb, zmq_strerror(mrb_zmq_errno())));
    case ENOCOMPATPROTO:
      mrb_raisef(mrb, E_ENOCOMPATPROTO_ERROR, "%S: %S", mrb_str_new_cstr(mrb, func), mrb_str_new_cstr(mrb, zmq_strerror(mrb_zmq_errno())));
    case ETERM:
      mrb_raisef(mrb, E_ETERM_ERROR, "%S: %S", mrb_str_new_cstr(mrb, func), mrb_str_new_cstr(mrb, zmq_strerror(mrb_zmq_errno())));
    case EMTHREAD:
      mrb_raisef(mrb, E_EMTHREAD_ERROR, "%S: %S", mrb_str_new_cstr(mrb, func), mrb_str_new_cstr(mrb, zmq_strerror(mrb_zmq_errno())));
    default: {
#ifdef _WIN32
      errno = mrb_zmq_errno();
#endif
      mrb_sys_fail(mrb, func);
    }
  }
}

// mruby gargabe collects objects which are out of scope
// as such the author no longer has a use for the socket
// so we close it immediatily instead of possibly waiting forever to close it
static void
mrb_zmq_gc_close(mrb_state *mrb, void *socket)
{
  int disable = 0;
  zmq_setsockopt(socket, ZMQ_LINGER, &disable, sizeof(disable));
  zmq_close(socket);
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

#ifdef ZMQ_HAVE_POLLER
static void
mrb_zmq_gc_poller_destroy(mrb_state *mrb, void *poller)
{
  zmq_poller_destroy(&poller);
}

static const struct mrb_data_type mrb_zmq_poller_type = {
  "$i_mrb_zmq_poller_type", mrb_zmq_gc_poller_destroy
};
#else
static const struct mrb_data_type mrb_zmq_poller_type = {
  "$i_mrb_zmq_poller_type", mrb_free
};
#endif //ZMQ_HAVE_POLLER

#ifdef ZMQ_HAVE_TIMERS
typedef struct {
  mrb_state *mrb;
  mrb_value timers;
  mrb_value block;
  int timer_id;
} mrb_zmq_timers_fn_t;

static const struct mrb_data_type mrb_zmq_timers_fn_type = {
  "$i_mrb_zmq_timers_fn_type", mrb_free
};

static void
mrb_zmq_gc_timers_destroy(mrb_state *mrb, void *timers)
{
  zmq_timers_destroy(&timers);
}

static const struct mrb_data_type mrb_zmq_timers_type = {
  "$i_mrb_zmq_timers_type", mrb_zmq_gc_timers_destroy
};
#endif //ZMQ_HAVE_TIMERS

#endif
