#include "mrb_libzmq.h"

#if (__GNUC__ >= 3) || (__INTEL_COMPILER >= 800) || defined(__clang__)
# define likely(x) __builtin_expect(!!(x), 1)
# define unlikely(x) __builtin_expect(!!(x), 0)
#else
# define likely(x) (x)
# define unlikely(x) (x)
#endif

#define MRB_LIBZMQ_CONTEXT() (mrb_cptr(mrb_const_get(mrb, mrb_obj_value(mrb_module_get(mrb, "LibZMQ")), mrb_intern_lit(mrb, "_Context"))))

static mrb_value
mrb_zmq_bind(mrb_state *mrb, mrb_value self)
{
  void *socket;
  char *endpoint;
  mrb_get_args(mrb, "dz", &socket, &mrb_zmq_socket_type, &endpoint);

  int rc = zmq_bind(socket, endpoint);
  if (unlikely(rc == -1)) {
    mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
  }

  return self;
}

static mrb_value
mrb_zmq_close(mrb_state *mrb, mrb_value self)
{
  mrb_value socket_val;
  mrb_bool blocky = TRUE;
  mrb_get_args(mrb, "o|b", &socket_val, &blocky);

  if (mrb_type(socket_val) == MRB_TT_DATA && DATA_TYPE(socket_val) == &mrb_zmq_socket_type) {
    if (!blocky) {
      int linger = 0;
      zmq_setsockopt(DATA_PTR(socket_val), ZMQ_LINGER, &linger, sizeof(linger));
    }
    int rc = zmq_close(DATA_PTR(socket_val));
    if (unlikely(rc == -1)) {
      mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
    }
    mrb_data_init(socket_val, NULL, NULL);
  }

  return mrb_nil_value();
}

static mrb_value
mrb_zmq_connect(mrb_state *mrb, mrb_value self)
{
  void *socket;
  char *endpoint;
  mrb_get_args(mrb, "dz", &socket, &mrb_zmq_socket_type, &endpoint);

  int rc = zmq_connect(socket, endpoint);
  if (unlikely(rc == -1)) {
    mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
  }

  return self;
}

static mrb_value
mrb_zmq_disconnect(mrb_state *mrb, mrb_value self)
{
  void *socket;
  char *endpoint;
  mrb_get_args(mrb, "dz", &socket, &mrb_zmq_socket_type, &endpoint);

  int rc = zmq_disconnect(socket, endpoint);
  if (unlikely(rc == -1)) {
    mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
  }

  return self;
}

static mrb_value
mrb_zmq_getsockopt(mrb_state *mrb, mrb_value self)
{
  void *socket;
  mrb_int option_name;
  mrb_value option_type;
  mrb_int string_return_len = 4096 - sizeof(mrb_value) - sizeof(struct RString);
  mrb_get_args(mrb, "diC|i", &socket, &mrb_zmq_socket_type, &option_name, &option_type, &string_return_len);
  mrb_assert(string_return_len >= 0);

  struct RClass* option_class = mrb_class_ptr(option_type);
  size_t option_len;
  int rc;

  if (option_name == ZMQ_FD) {
#ifdef _WIN32
    SOCKET fd;
#else
    int fd;
#endif
    option_len = sizeof(fd);
    rc = zmq_getsockopt(socket, option_name, &fd, &option_len);
    if (unlikely(rc == -1)) {
      mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
    }
    if (MRB_INT_MAX < fd) {
      return mrb_float_value(mrb, fd);
    } else {
      return mrb_fixnum_value(fd);
    }
  }
  else if (option_class == mrb->true_class||option_class == mrb->false_class) {
    int boolean;
    option_len = sizeof(boolean);
    rc = zmq_getsockopt(socket, option_name, &boolean, &option_len);
    if (unlikely(rc == -1)) {
      mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
    }
    return mrb_bool_value(boolean);
  }
  else if (option_class == mrb->fixnum_class) {
    int number;
    option_len = sizeof(number);
    rc = zmq_getsockopt(socket, option_name, &number, &option_len);
    if (unlikely(rc == -1)) {
      mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
    }
    return mrb_fixnum_value(number);
  }
  else if (option_class == mrb->float_class) {
    int64_t number;
    option_len = sizeof(number);
    rc = zmq_getsockopt(socket, option_name, &number, &option_len);
    if (unlikely(rc == -1)) {
      mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
    }
#ifdef MRB_INT64
    return mrb_fixnum_value(number);
#else
    return mrb_float_value(mrb, number);
#endif
  }
  else if (option_class == mrb->string_class) {
    mrb_value buf = mrb_str_new(mrb, NULL, string_return_len);
    option_len = RSTRING_CAPA(buf);
    rc = zmq_getsockopt(socket, option_name, RSTRING_PTR(buf), &option_len);
    if (unlikely(rc == -1)) {
      mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
    }

    return mrb_str_resize(mrb, buf, option_len);
  }
  else {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "Expected True-/FalseClass|Fixnum|Float|String");
  }

  return self;
}

#ifdef ZMQ_HAS_CAPABILITIES
static mrb_value
mrb_zmq_has(mrb_state *mrb, mrb_value self)
{
  char *capability;
  mrb_get_args(mrb, "z", &capability);

  return mrb_bool_value(zmq_has(capability));
}
#endif

static mrb_value
mrb_zmq_msg_new(mrb_state *mrb, mrb_value self)
{
  if (unlikely(DATA_PTR(self))) {
    mrb_free(mrb, DATA_PTR(self));
  }

  zmq_msg_t *msg = mrb_malloc(mrb, sizeof(zmq_msg_t));
  zmq_msg_init(msg);
  mrb_data_init(self, msg, &mrb_zmq_msg_type);

  return self;
}

static mrb_value
mrb_zmq_msg_gets(mrb_state *mrb, mrb_value self)
{
  zmq_msg_t *msg;
  char *property;
  mrb_bool static_string = FALSE;
  mrb_get_args(mrb, "dz|b", &msg, &mrb_zmq_msg_type, &property, &static_string);

  const char *prop = zmq_msg_gets(msg, property);
  if (prop) {
    if (static_string) {
      return mrb_str_new_static(mrb, prop, strlen(prop));
    } else {
      return mrb_str_new_cstr(mrb, prop);
    }
  }

  return mrb_nil_value();
}

static mrb_value
mrb_zmq_msg_send(mrb_state *mrb, mrb_value self)
{
  zmq_msg_t *msg;
  void *socket;
  mrb_int flags;
  mrb_get_args(mrb, "ddi", &msg, &mrb_zmq_msg_type, &socket, &mrb_zmq_socket_type, &flags);
  mrb_assert(flags >= INT_MIN && flags <= INT_MAX);

  int rc = zmq_msg_send(msg, socket, flags);
  if (unlikely(rc == -1)) {
    mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
  }

  return mrb_nil_value();
}

static mrb_value
mrb_zmq_msg_to_str(mrb_state *mrb, mrb_value self)
{
  mrb_bool static_string = FALSE;
  mrb_get_args(mrb, "|b", &static_string);

  void *data = zmq_msg_data(DATA_PTR(self));
  size_t size = zmq_msg_size(DATA_PTR(self));

  if (static_string) {
    return mrb_str_new_static(mrb, data, size);
  } else {
    return mrb_str_new(mrb, data, size);
  }
}

static mrb_value
mrb_zmq_send(mrb_state *mrb, mrb_value self)
{
  void *socket;
  mrb_value message;
  mrb_int flags;
  mrb_get_args(mrb, "doi", &socket, &mrb_zmq_socket_type, &message, &flags);
  mrb_assert(flags >= INT_MIN && flags <= INT_MAX);

  message = mrb_str_to_str(mrb, message);

  int rc = zmq_send(socket, RSTRING_PTR(message), RSTRING_LEN(message), flags);
  if (unlikely(rc == -1)) {
    mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
  }

  return mrb_fixnum_value(rc);
}

static mrb_value
mrb_zmq_setsockopt(mrb_state *mrb, mrb_value self)
{
  void *socket;
  mrb_int option_name;
  mrb_value option_value;
  mrb_get_args(mrb, "dio", &socket, &mrb_zmq_socket_type, &option_name, &option_value);
  mrb_assert(option_name >= INT_MIN && option_name <= INT_MAX);

  int rc;

  switch(mrb_type(option_value)) {
    case MRB_TT_FALSE: {
      if (!mrb_fixnum(option_value)) {
        rc = zmq_setsockopt(socket, option_name, NULL, 0);
        if (unlikely(rc == -1)) {
          mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
        }
      } else {
        int boolean = 0;
        rc = zmq_setsockopt(socket, option_name, &boolean, sizeof(boolean));
        if (unlikely(rc == -1)) {
          mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
        }
      }
    } break;
    case MRB_TT_TRUE: {
      int boolean = 1;
      rc = zmq_setsockopt(socket, option_name, &boolean, sizeof(boolean));
      if (unlikely(rc == -1)) {
        mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
      }
    } break;
    case MRB_TT_FIXNUM: {
      mrb_assert(mrb_fixnum(option_value) >= INT_MIN && mrb_fixnum(option_value) <= INT_MAX);
      int number = (int) mrb_fixnum(option_value);
      rc = zmq_setsockopt(socket, option_name, &number, sizeof(number));
      if (unlikely(rc == -1)) {
        mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
      }
    } break;
    case MRB_TT_FLOAT: {
      mrb_assert(mrb_float(option_value) >= INT64_MIN && mrb_float(option_value) <= INT64_MAX);
      int64_t number = (int64_t) mrb_float(option_value);
      rc = zmq_setsockopt(socket, option_name, &number, sizeof(number));
      if (unlikely(rc == -1)) {
        mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
      }
    } break;
    case MRB_TT_STRING: {
      rc = zmq_setsockopt(socket, option_name, RSTRING_PTR(option_value), RSTRING_LEN(option_value));
      if (unlikely(rc == -1)) {
        mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
      }
    } break;
    default: {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "expected nil|false|true|fixnum|float|string");
    }
  }

  return self;
}

static mrb_value
mrb_zmq_unbind(mrb_state *mrb, mrb_value self)
{
  void *socket;
  char *endpoint;
  mrb_get_args(mrb, "dz", &socket, &mrb_zmq_socket_type, &endpoint);

  int rc = zmq_unbind(socket, endpoint);
  if (unlikely(rc == -1)) {
    mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
  }

  return self;
}

static mrb_value
mrb_zmq_socket(mrb_state *mrb, mrb_value self)
{
  if (unlikely(DATA_PTR(self))) {
    mrb_free(mrb, DATA_PTR(self));
  }

  mrb_int type;
  mrb_get_args(mrb, "i", &type);
  mrb_assert(type >= 0 && type <= INT_MAX);

  void *socket = zmq_socket(MRB_LIBZMQ_CONTEXT(), type);
  if (likely(socket)) {
    mrb_data_init(self, socket, &mrb_zmq_socket_type);
  } else {
    mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
  }

  return self;
}

static mrb_value
mrb_zmq_socket_recv(mrb_state *mrb, mrb_value self)
{
  mrb_int flags = 0;
  mrb_get_args(mrb, "|i", &flags);
  mrb_assert(flags >= INT_MIN && flags <= INT_MAX);

  mrb_value data = self;

  int more = 0;

  do {
    mrb_value msg_val = mrb_obj_new(mrb, mrb_class_get_under(mrb, mrb_module_get(mrb, "ZMQ"), "Msg"), 0, NULL);
    zmq_msg_t *msg = DATA_PTR(msg_val);

    int rc = zmq_msg_recv (msg, DATA_PTR(self), flags);
    if (unlikely(rc == -1)) {
      mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
    }
    more = zmq_msg_more(msg);
    if (more) {
      if (!mrb_array_p(data)) {
        data = mrb_ary_new_capa(mrb, 2);
      }
      mrb_ary_push(mrb, data, msg_val);
    } else {
      if (mrb_array_p(data)) {
        mrb_ary_push(mrb, data, msg_val);
      } else {
        data = msg_val;
      }
    }
  } while (more);

  return data;
}

MRB_INLINE void
mrb_zmq_thread_fn(void *mrb_zmq_thread_)
{
  mrb_zmq_thread_t *mrb_zmq_thread = (mrb_zmq_thread_t *) mrb_zmq_thread_;
  mrb_bool success = FALSE;

  mrb_state *mrb = mrb_open();
  if (likely(mrb)) {
    mrb_zmq_thread->backend_ctx = MRB_LIBZMQ_CONTEXT();
    mrb_value pipe_val = mrb_nil_value();
    mrb_value thread_fn = mrb_nil_value();

    struct mrb_jmpbuf* prev_jmp = mrb->jmp;
    struct mrb_jmpbuf c_jmp;

    MRB_TRY(&c_jmp)
    {
      mrb->jmp = &c_jmp;
      pipe_val = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_DATA, mrb_class_get_under(mrb, mrb_module_get(mrb, "ZMQ"), "Pair")));
      mrb_data_init(pipe_val, mrb_zmq_thread->backend, &mrb_zmq_socket_type);
      thread_fn = mrb_obj_new(mrb, mrb_class_get_under(mrb, mrb_module_get(mrb, "LibZMQ"), "Thread_fn"), 0, NULL);
      mrb_iv_set(mrb, thread_fn, mrb_intern_lit(mrb, "@instances"), mrb_hash_new(mrb));
      mrb_iv_set(mrb, thread_fn, mrb_intern_lit(mrb, "@pipe"), pipe_val);
      success = TRUE;
      int rc = zmq_send(mrb_zmq_thread->backend, &success, sizeof(success), 0);
      if (unlikely(rc == -1)) {
        success = FALSE;
        mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
      }
      mrb->jmp = prev_jmp;
    }
    MRB_CATCH(&c_jmp)
    {
      mrb->jmp = prev_jmp;
      zmq_send(mrb_zmq_thread->backend, &success, sizeof(success), 0);
      if (mrb_nil_p(pipe_val)) {
        zmq_close(mrb_zmq_thread->backend);
      }
      mrb_print_error(mrb);
    }
    MRB_END_EXC(&c_jmp);

    if (likely(success)) {
      mrb_funcall(mrb, thread_fn, "run", 0, NULL);
    }
    mrb_close(mrb);
  } else {
    zmq_send(mrb_zmq_thread->backend, &success, sizeof(success), 0);
    zmq_close(mrb_zmq_thread->backend);
  }
}

static mrb_value
mrb_zmq_threadstart(mrb_state *mrb, mrb_value thread_class)
{
  mrb_value self = mrb_nil_value();
  mrb_zmq_thread_t *mrb_zmq_thread = NULL;
  void *backend = NULL;
  mrb_bool success = FALSE;

  struct mrb_jmpbuf* prev_jmp = mrb->jmp;
  struct mrb_jmpbuf c_jmp;

  MRB_TRY(&c_jmp)
  {
    mrb->jmp = &c_jmp;

    self = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_DATA, mrb_class_ptr(thread_class)));
    mrb_value frontend_val = mrb_obj_new(mrb, mrb_class_get_under(mrb, mrb_module_get(mrb, "ZMQ"), "Pair"), 0, NULL);
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@pipe"), frontend_val);
    void *frontend = DATA_PTR(frontend_val);
    mrb_zmq_thread = mrb_malloc(mrb, sizeof(mrb_zmq_thread_t));
    memset(mrb_zmq_thread, 0, sizeof(mrb_zmq_thread_t));
    mrb_zmq_thread->frontend = frontend;

    void *backend = zmq_socket(MRB_LIBZMQ_CONTEXT(), ZMQ_PAIR);
    if (likely(backend)) {
      mrb_zmq_thread->backend = backend;
    } else {
      mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
    }

    char endpoint[256];
    do {
      errno = 0;
      snprintf(endpoint, sizeof(endpoint), "inproc://mrb-actor-pipe-%04x-%04x", mrb_sysrandom_uniform(0x10000), mrb_sysrandom_uniform(0x10000));
      if (likely(zmq_bind(frontend, endpoint) == 0)) {
        break;
      }
      else if (errno != EADDRINUSE) {
        mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
      }
    } while(TRUE);

    int rc = zmq_connect(backend, endpoint);
    mrb_assert(rc != -1);

    void *thread = zmq_threadstart(&mrb_zmq_thread_fn, mrb_zmq_thread);
    if (likely(thread)) {
      mrb_zmq_thread->thread = thread;
      mrb_data_init(self, mrb_zmq_thread, &mrb_zmq_thread_type);
    } else {
      mrb_sys_fail(mrb, zmq_strerror(zmq_errno()));
    }

    zmq_recv(frontend, &success, sizeof(mrb_bool), 0);
    if (unlikely(!success)) {
      mrb_raise(mrb, E_RUNTIME_ERROR, "Cannot initialize ZMQ Thread");
    }

    mrb_value *argv = NULL;
    mrb_int argv_len = 0;
    mrb_value block = mrb_nil_value();
    mrb_get_args(mrb, "*&", &argv, &argv_len, &block);

    mrb_funcall_with_block(mrb, self, mrb_intern_lit(mrb, "initialize"), argv_len, argv, block);
    mrb->jmp = prev_jmp;
  }
  MRB_CATCH(&c_jmp)
  {
    mrb->jmp = prev_jmp;
    if (mrb_zmq_thread && !mrb_zmq_thread->thread) {
      mrb_free(mrb, mrb_zmq_thread);
    }
    if (backend && !success) {
      zmq_close(backend);
    }
    MRB_THROW(mrb->jmp);
  }
  MRB_END_EXC(&c_jmp);

  return self;
}

static mrb_value
mrb_zmq_threadclose(mrb_state *mrb, mrb_value self)
{
  mrb_value thread_val;
  mrb_bool blocky = TRUE;
  mrb_get_args(mrb, "o|b", &thread_val, &blocky);

  if (mrb_type(thread_val) == MRB_TT_DATA && DATA_TYPE(thread_val) == &mrb_zmq_thread_type) {
    mrb_zmq_thread_t *mrb_zmq_thread = (mrb_zmq_thread_t *) DATA_PTR(thread_val);

    mrb_value frontend_val = mrb_iv_remove(mrb, thread_val, mrb_intern_lit(mrb, "@pipe"));
    if (!blocky) {
      int timeo = 0;
      zmq_setsockopt(mrb_zmq_thread->frontend, ZMQ_SNDTIMEO, &timeo, sizeof(timeo));
    }
    zmq_send(mrb_zmq_thread->frontend, "TERM$", 5, 0);
    if (!blocky) {
      int linger = 0;
      zmq_setsockopt(mrb_zmq_thread->frontend, ZMQ_LINGER, &linger, sizeof(linger));
    }
    zmq_close(mrb_zmq_thread->frontend);
    mrb_data_init(frontend_val, NULL, NULL);
    zmq_ctx_shutdown(mrb_zmq_thread->backend_ctx);
    zmq_close(mrb_zmq_thread->backend);
    zmq_threadclose(mrb_zmq_thread->thread);
    mrb_free(mrb, DATA_PTR(thread_val));
    mrb_data_init(thread_val, NULL, NULL);
  }

  return mrb_nil_value();
}

void
mrb_mruby_libzmq4_gem_init(mrb_state* mrb)
{
  void *context = zmq_ctx_new();
  if (getenv("ZMQ_IO_THREADS")) {
    zmq_ctx_set(context, ZMQ_IO_THREADS, atoi(getenv("ZMQ_IO_THREADS")));
  }
  if (getenv("ZMQ_THREAD_SCHED_POLICY")) {
    zmq_ctx_set(context, ZMQ_THREAD_SCHED_POLICY, atoi(getenv("ZMQ_THREAD_SCHED_POLICY")));
  }
  if (getenv("ZMQ_THREAD_PRIORITY")) {
    zmq_ctx_set(context, ZMQ_THREAD_PRIORITY, atoi(getenv("ZMQ_THREAD_PRIORITY")));
  }

  struct RClass *libzmq_mod, *zmq_mod, *zmq_msg_class, *zmq_socket_class, *zmq_thread_class;

  libzmq_mod = mrb_define_module(mrb, "LibZMQ");
  mrb_define_const(mrb, libzmq_mod, "_Context", mrb_cptr_value(mrb, context));
  mrb_define_module_function(mrb, libzmq_mod, "bind", mrb_zmq_bind, MRB_ARGS_REQ(2));
  mrb_define_module_function(mrb, libzmq_mod, "close", mrb_zmq_close, MRB_ARGS_ARG(1, 1));
  mrb_define_module_function(mrb, libzmq_mod, "connect", mrb_zmq_connect, MRB_ARGS_REQ(2));
  mrb_define_module_function(mrb, libzmq_mod, "disconnect", mrb_zmq_disconnect, MRB_ARGS_REQ(2));
  mrb_define_module_function(mrb, libzmq_mod, "getsockopt", mrb_zmq_getsockopt, MRB_ARGS_ARG(3, 1));
#ifdef ZMQ_HAS_CAPABILITIES
  mrb_define_module_function(mrb, libzmq_mod, "has?", mrb_zmq_has, MRB_ARGS_REQ(1));
#endif
  mrb_define_module_function(mrb, libzmq_mod, "msg_gets", mrb_zmq_msg_gets, MRB_ARGS_ARG(2, 1));
  mrb_define_module_function(mrb, libzmq_mod, "msg_send", mrb_zmq_msg_send, MRB_ARGS_REQ(3));
  mrb_define_module_function(mrb, libzmq_mod, "send", mrb_zmq_send, MRB_ARGS_REQ(3));
  mrb_define_module_function(mrb, libzmq_mod, "setsockopt", mrb_zmq_setsockopt, MRB_ARGS_REQ(3));
  mrb_define_module_function(mrb, libzmq_mod, "threadclose", mrb_zmq_threadclose, MRB_ARGS_ARG(1, 1));
  mrb_define_module_function(mrb, libzmq_mod, "unbind", mrb_zmq_unbind, MRB_ARGS_REQ(2));

  zmq_mod = mrb_define_module(mrb, "ZMQ");
  zmq_msg_class = mrb_define_class_under(mrb, zmq_mod, "Msg", mrb->object_class);
  MRB_SET_INSTANCE_TT(zmq_msg_class, MRB_TT_DATA);
  mrb_define_method(mrb, zmq_msg_class, "initialize", mrb_zmq_msg_new, MRB_ARGS_NONE());
  mrb_define_method(mrb, zmq_msg_class, "to_str", mrb_zmq_msg_to_str, MRB_ARGS_OPT(1));

  zmq_socket_class = mrb_define_class_under(mrb, zmq_mod, "Socket", mrb->object_class);
  MRB_SET_INSTANCE_TT(zmq_socket_class, MRB_TT_DATA);
  mrb_define_method(mrb, zmq_socket_class, "initialize", mrb_zmq_socket, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, zmq_socket_class, "recv", mrb_zmq_socket_recv, MRB_ARGS_OPT(1));

  zmq_thread_class = mrb_define_class_under(mrb, zmq_mod, "Thread", mrb->object_class);
  MRB_SET_INSTANCE_TT(zmq_thread_class, MRB_TT_DATA);
  mrb_define_class_method(mrb, zmq_thread_class, "new", mrb_zmq_threadstart, (MRB_ARGS_ANY()|MRB_ARGS_BLOCK()));

#define mrb_zmq_define_const(ZMQ_CONST_NAME, ZMQ_CONST) \
  do { \
    int ai = mrb_gc_arena_save(mrb); \
    mrb_define_const(mrb, libzmq_mod, ZMQ_CONST_NAME, mrb_fixnum_value(ZMQ_CONST)); \
    mrb_gc_arena_restore(mrb, ai); \
  } while(0)

#include "zmq_const.cstub"
}

void
mrb_mruby_libzmq4_gem_final(mrb_state* mrb)
{
  void *context = MRB_LIBZMQ_CONTEXT();
  zmq_ctx_shutdown(context);
  mrb_funcall(mrb, mrb_obj_value(mrb_module_get(mrb, "LibZMQ")), "_finalizer", 0, NULL);
  zmq_ctx_term(context);
}
