#ifndef MRB_LIBZMQ_H
#define MRB_LIBZMQ_H

#include <stdlib.h>
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

#if !defined(SOCKET) && !defined(_WIN32)
#define SOCKET int
#endif

static void
mrb_zmq_gc_close(mrb_state *mrb, void *socket)
{
  int linger = 0;
  zmq_setsockopt(socket, ZMQ_LINGER, &linger, sizeof(linger));
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

typedef struct {
  void *frontend;
  void *backend;
  char *class_path;
  void *thread;
  void *backend_ctx;
} mrb_zmq_thread_data_t;

static void
mrb_zmq_gc_threadclose(mrb_state *mrb, void *mrb_zmq_thread_data_)
{
  mrb_zmq_thread_data_t *mrb_zmq_thread_data = (mrb_zmq_thread_data_t *) mrb_zmq_thread_data_;
  int sndtimeo = 0;
  zmq_setsockopt(mrb_zmq_thread_data->frontend, ZMQ_SNDTIMEO, &sndtimeo, sizeof(sndtimeo));
  zmq_send(mrb_zmq_thread_data->frontend, "TERM$", 5, 0);
  int linger = 0;
  zmq_setsockopt(mrb_zmq_thread_data->frontend, ZMQ_LINGER, &linger, sizeof(linger));
  zmq_close(mrb_zmq_thread_data->frontend);
  zmq_ctx_shutdown(mrb_zmq_thread_data->backend_ctx);
  zmq_close(mrb_zmq_thread_data->backend);
  zmq_threadclose(mrb_zmq_thread_data->thread);
  free(mrb_zmq_thread_data->class_path);
  mrb_free(mrb, mrb_zmq_thread_data_);
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

#endif
