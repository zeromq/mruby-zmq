#ifndef MRB_LIBZMQ_H
#define MRB_LIBZMQ_H

#include <stdlib.h>
#include <mruby.h>
#include <zmq.h>
#if ZMQ_VERSION < ZMQ_MAKE_VERSION(4,1,0)
  #error "mruby-libzmq4 needs at least libzmq-4.1"
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
#include <mruby/sysrandom.h>
#include <unistd.h>


#define MRB_LIBZMQ_CONTEXT() (mrb_cptr(mrb_const_get(mrb, mrb_obj_value(mrb_module_get(mrb, "LibZMQ")), mrb_intern_lit(mrb, "_Context"))))
#define MRB_LIBZMQ_SOCKETS() (mrb_const_get(mrb, mrb_obj_value(mrb_module_get(mrb, "LibZMQ")), mrb_intern_lit(mrb, "_Sockets")))
#define MRB_LIBZMQ_THREADS() (mrb_const_get(mrb, mrb_obj_value(mrb_module_get(mrb, "LibZMQ")), mrb_intern_lit(mrb, "_Threads")))

static void
mrb_gc_zmq_close(mrb_state *mrb, void *socket)
{
  int linger = 0;
  zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger));
  zmq_close(socket);
  mrb_hash_delete_key(mrb, MRB_LIBZMQ_SOCKETS(), mrb_cptr_value(mrb, socket));
}

static const struct mrb_data_type mrb_zmq_socket_type = {
  "$i_mrb_zmq_socket_type", mrb_gc_zmq_close
};

static void
mrb_gc_zmq_msg_close(mrb_state *mrb, void *msg)
{
  zmq_msg_close((zmq_msg_t *) msg);
  mrb_free(mrb, msg);
}

static const struct mrb_data_type mrb_zmq_msg_type = {
  "$i_mrb_zmq_msg_type", mrb_gc_zmq_msg_close
};

static void
mrb_gc_zmq_threadclose(mrb_state *mrb, void *thread)
{
  zmq_threadclose(thread);
  mrb_hash_delete_key(mrb, MRB_LIBZMQ_THREADS(), mrb_cptr_value(mrb, thread));
}

static const struct mrb_data_type mrb_zmq_thread_type = {
  "$i_mrb_zmq_thread_type", mrb_gc_zmq_threadclose
};

#endif
