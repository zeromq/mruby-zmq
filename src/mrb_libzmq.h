#ifndef MRB_LIBZMQ_H
#define MRB_LIBZMQ_H

#include <stdlib.h>
#include <mruby.h>
#ifdef MRB_INT16
 #error "MRB_INT16 is too small for mruby-libzmq4"
#endif
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

static void
mrb_gc_zmq_close(mrb_state *mrb, void *socket)
{
  int linger = 0;
  zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger));
  zmq_close(socket);
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

typedef struct {
  void *thread;
  void *pipe;
} mrb_zmq_thread_pipe_t;

static void
mrb_gc_zmq_threadclose(mrb_state *mrb, void *thread_pipe_)
{
  mrb_zmq_thread_pipe_t *thread_pipe = (mrb_zmq_thread_pipe_t *) thread_pipe_;
  zmq_send(thread_pipe->pipe, "TERM$", 5, 0);
  zmq_threadclose(thread_pipe->thread);
  mrb_free(mrb, thread_pipe_);
}

static const struct mrb_data_type mrb_zmq_thread_type = {
  "$i_mrb_zmq_thread_type", mrb_gc_zmq_threadclose
};

#endif
