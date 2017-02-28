#include "mrb_libzmq.h"

static mrb_value
mrb_zmq_bind(mrb_state *mrb, mrb_value self)
{
  void *socket;
  char *endpoint;
  mrb_get_args(mrb, "dz", &socket, &mrb_zmq_socket_type, &endpoint);

  int rc = zmq_bind(socket, endpoint);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_bind");
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
      mrb_zmq_handle_error(mrb, "zmq_close");
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
    mrb_zmq_handle_error(mrb, "zmq_connect");
  }

  return self;
}

static mrb_value
mrb_zmq_ctx_get(mrb_state *mrb, mrb_value self)
{
  mrb_int option_name;
  mrb_get_args(mrb, "i", &option_name);
  mrb_assert(option_name >= INT_MIN && option_name <= INT_MAX);

  int rc = zmq_ctx_get(MRB_LIBZMQ_CONTEXT(mrb), option_name);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_ctx_get");
  }

  return mrb_fixnum_value(rc);
}

static mrb_value
mrb_zmq_ctx_set(mrb_state *mrb, mrb_value self)
{
  mrb_int option_name, option_value;
  mrb_get_args(mrb, "ii", &option_name, &option_value);
  mrb_assert(option_name >= INT_MIN && option_name <= INT_MAX);
  mrb_assert(option_value >= INT_MIN && option_value <= INT_MAX);

  int rc = zmq_ctx_set(MRB_LIBZMQ_CONTEXT(mrb), option_name, option_value);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_ctx_set");
  }

  return self;
}

static mrb_value
mrb_zmq_curve_keypair(mrb_state *mrb, mrb_value self)
{
  char z85_public_key[41], z85_secret_key[41];

  int rc = zmq_curve_keypair(z85_public_key, z85_secret_key);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_curve_keypair");
  }

  mrb_value public_key = mrb_str_new(mrb, NULL, 32);
  uint8_t *pub = zmq_z85_decode((uint8_t *) RSTRING_PTR(public_key), z85_public_key);
  if (unlikely(!pub)) {
    mrb_zmq_handle_error(mrb, "zmq_z85_decode");
  }

  mrb_value secret_key = mrb_str_new(mrb, NULL, 32);
  uint8_t *sec = zmq_z85_decode((uint8_t *) RSTRING_PTR(secret_key), z85_secret_key);
  if (unlikely(!sec)) {
    mrb_zmq_handle_error(mrb, "zmq_z85_decode");
  }

  mrb_value keypair = mrb_hash_new_capa(mrb, 2);
  mrb_hash_set(mrb, keypair, mrb_symbol_value(mrb_intern_lit(mrb, "public_key")), public_key);
  mrb_hash_set(mrb, keypair, mrb_symbol_value(mrb_intern_lit(mrb, "secret_key")), secret_key);

  return keypair;

}

static mrb_value
mrb_zmq_disconnect(mrb_state *mrb, mrb_value self)
{
  void *socket;
  char *endpoint;
  mrb_get_args(mrb, "dz", &socket, &mrb_zmq_socket_type, &endpoint);

  int rc = zmq_disconnect(socket, endpoint);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_disconnect");
  }

  return self;
}

static mrb_value
mrb_zmq_getsockopt(mrb_state *mrb, mrb_value self)
{
  void *socket;
  mrb_int option_name;
  mrb_value option_type;
  mrb_int string_return_len = 4096;
  mrb_get_args(mrb, "diC|i", &socket, &mrb_zmq_socket_type, &option_name, &option_type, &string_return_len);
  mrb_assert(string_return_len >= 0);

  struct RClass* option_class = mrb_class_ptr(option_type);
  size_t option_len = 0;
  int rc = -1;

  if (option_name == ZMQ_FD) {
    SOCKET fd;
    option_len = sizeof(fd);
    rc = zmq_getsockopt(socket, option_name, &fd, &option_len);
    if (unlikely(rc == -1)) {
      mrb_zmq_handle_error(mrb, "zmq_getsockopt");
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
      mrb_zmq_handle_error(mrb, "zmq_getsockopt");
    }
    return mrb_bool_value(boolean);
  }
  else if (option_class == mrb->fixnum_class) {
    int number;
    option_len = sizeof(number);
    rc = zmq_getsockopt(socket, option_name, &number, &option_len);
    if (unlikely(rc == -1)) {
      mrb_zmq_handle_error(mrb, "zmq_getsockopt");
    }
    return mrb_fixnum_value(number);
  }
  else if (option_class == mrb->float_class) {
    int64_t number;
    option_len = sizeof(number);
    rc = zmq_getsockopt(socket, option_name, &number, &option_len);
    if (unlikely(rc == -1)) {
      mrb_zmq_handle_error(mrb, "zmq_getsockopt");
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
      mrb_zmq_handle_error(mrb, "zmq_getsockopt");
    }

    return mrb_str_resize(mrb, buf, option_len);
  }
  else {
    mrb_raise(mrb, E_TYPE_ERROR, "Expected True-/FalseClass|Fixnum|Float|String");
  }

  return self;
}

static mrb_value
mrb_zmq_msg_new(mrb_state *mrb, mrb_value self)
{
  if (unlikely(DATA_PTR(self))) {
    mrb_free(mrb, DATA_PTR(self));
    mrb_data_init(self, NULL, NULL);
  }

  mrb_value data = mrb_nil_value();
  mrb_get_args(mrb, "|o", &data);

  zmq_msg_t *msg = mrb_malloc(mrb, sizeof(zmq_msg_t));
  memset(msg, 0, sizeof(*msg));
  mrb_data_init(self, msg, &mrb_zmq_msg_type);

  switch (mrb_type(data)) {
    case MRB_TT_FIXNUM: {
      mrb_int size = mrb_fixnum(data);
      mrb_assert(size >= 0 && size <= SIZE_MAX);
      int rc = zmq_msg_init_size(msg, size);
      if (unlikely(rc == -1)) {
        mrb_zmq_handle_error(mrb, "zmq_msg_init_size");
      }
    } break;
    case MRB_TT_STRING: {
      int rc = zmq_msg_init_size(msg, RSTRING_LEN(data));
      if (unlikely(rc == -1)) {
        mrb_zmq_handle_error(mrb, "zmq_msg_init_size");
      }
      memcpy(zmq_msg_data(msg), RSTRING_PTR(data), RSTRING_LEN(data));
    } break;
    case MRB_TT_FALSE: {
      zmq_msg_init(msg);
    } break;
    default: {
      mrb_raise(mrb, E_TYPE_ERROR, "only works (optionally) with Fixnum and String types");
    }
  }

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
    mrb_zmq_handle_error(mrb, "zmq_msg_send");
  }

  return mrb_nil_value();
}

static mrb_value
mrb_zmq_proxy(mrb_state *mrb, mrb_value self)
{
  void *frontend, *backend, *capture = NULL;
  mrb_get_args(mrb, "dd|d!", &frontend, &mrb_zmq_socket_type, &backend, &mrb_zmq_socket_type, &capture, &mrb_zmq_socket_type);

  zmq_proxy(frontend, backend, capture);

  return self;
}

static mrb_value
mrb_zmq_proxy_steerable(mrb_state *mrb, mrb_value self)
{
  void *frontend, *backend, *control, *capture = NULL;
  mrb_get_args(mrb, "ddd|d!", &frontend, &mrb_zmq_socket_type, &backend, &mrb_zmq_socket_type, &control, &mrb_zmq_socket_type,
    &capture, &mrb_zmq_socket_type);

  zmq_proxy_steerable(frontend, backend, capture, control);

  return self;
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
mrb_zmq_msg_eql(mrb_state *mrb, mrb_value self)
{
  zmq_msg_t *other;
  mrb_get_args(mrb, "d", &other, &mrb_zmq_msg_type);

  zmq_msg_t *msg = DATA_PTR(self);

  if (zmq_msg_size(msg) != zmq_msg_size(other)) {
    return mrb_false_value();
  } else {
    return mrb_bool_value(memcmp(zmq_msg_data(msg), zmq_msg_data(other), zmq_msg_size(msg)) == 0);
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
    mrb_zmq_handle_error(mrb, "zmq_send");
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

  int rc = -1;

  switch(mrb_type(option_value)) {
    case MRB_TT_FALSE: {
      if (!mrb_fixnum(option_value)) {
        rc = zmq_setsockopt(socket, option_name, NULL, 0);
      } else {
        int boolean = 0;
        rc = zmq_setsockopt(socket, option_name, &boolean, sizeof(boolean));
      }
    } break;
    case MRB_TT_TRUE: {
      int boolean = 1;
      rc = zmq_setsockopt(socket, option_name, &boolean, sizeof(boolean));
    } break;
    case MRB_TT_FIXNUM: {
      mrb_assert(mrb_fixnum(option_value) >= INT_MIN && mrb_fixnum(option_value) <= INT_MAX);
      int number = (int) mrb_fixnum(option_value);
      rc = zmq_setsockopt(socket, option_name, &number, sizeof(number));
    } break;
    case MRB_TT_FLOAT: {
      mrb_assert(mrb_float(option_value) >= INT64_MIN && mrb_float(option_value) <= INT64_MAX);
      int64_t number = (int64_t) mrb_float(option_value);
      rc = zmq_setsockopt(socket, option_name, &number, sizeof(number));
    } break;
    case MRB_TT_STRING: {
      rc = zmq_setsockopt(socket, option_name, RSTRING_PTR(option_value), RSTRING_LEN(option_value));
    } break;
    default: {
      mrb_raise(mrb, E_TYPE_ERROR, "expected nil|false|true|Fixnum|Float|String");
    }
  }

  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_setsockopt");
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
    mrb_zmq_handle_error(mrb, "zmq_unbind");
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

#ifdef ZMQ_SERVER
static mrb_value
mrb_zmq_msg_routing_id(mrb_state *mrb, mrb_value self)
{
  zmq_msg_t *msg;
  mrb_get_args(mrb, "d", &msg, &mrb_zmq_msg_type);

  return mrb_fixnum_value(zmq_msg_routing_id(msg));
}

static mrb_value
mrb_zmq_msg_set_routing_id(mrb_state *mrb, mrb_value self)
{
  zmq_msg_t *msg;
  mrb_int routing_id;
  mrb_get_args(mrb, "di", &msg, &mrb_zmq_msg_type, &routing_id);

  int rc = zmq_msg_set_routing_id(msg, routing_id);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_msg_set_routing_id");
  }

  return self;
}
#endif // ZMQ_SERVER

#ifdef ZMQ_DISH
static mrb_value
mrb_zmq_join(mrb_state *mrb, mrb_value self)
{
  void *socket;
  char *group;
  mrb_get_args(mrb, "dz", &socket, &mrb_zmq_socket_type, &group);

  int rc = zmq_join(socket, group);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_join");
  }

  return self;
}

static mrb_value
mrb_zmq_leave(mrb_state *mrb, mrb_value self)
{
  void *socket;
  char *group;
  mrb_get_args(mrb, "dz", &socket, &mrb_zmq_socket_type, &group);

  int rc = zmq_leave(socket, group);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_leave");
  }

  return self;
}

static mrb_value
mrb_zmq_msg_group(mrb_state *mrb, mrb_value self)
{
  zmq_msg_t *msg;
  mrb_get_args(mrb, "d", &msg, &mrb_zmq_msg_type);

  return mrb_str_new_cstr(mrb, zmq_msg_group(msg));
}

static mrb_value
mrb_zmq_msg_set_group(mrb_state *mrb, mrb_value self)
{
  zmq_msg_t *msg;
  char *group;
  mrb_get_args(mrb, "dz", &msg, &mrb_zmq_msg_type, &group);

  int rc = zmq_msg_set_group(msg, group);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_msg_set_group");
  }

  return self;
}
#endif // ZMQ_DISH

static mrb_value
mrb_zmq_socket(mrb_state *mrb, mrb_value self)
{
  if (unlikely(DATA_PTR(self))) {
    mrb_free(mrb, DATA_PTR(self));
    mrb_data_init(self, NULL, NULL);
  }

  mrb_int type;
  mrb_get_args(mrb, "i", &type);
  mrb_assert(type >= 0 && type <= INT_MAX);

  void *socket = zmq_socket(MRB_LIBZMQ_CONTEXT(mrb), type);
  if (likely(socket)) {
    mrb_data_init(self, socket, &mrb_zmq_socket_type);
  } else {
    mrb_zmq_handle_error(mrb, "zmq_socket");
  }

  return self;
}

static mrb_value
mrb_zmq_socket_monitor(mrb_state *mrb, mrb_value self)
{
  void *socket;
  char *addr;
  mrb_int events;
  mrb_get_args(mrb, "dzi", &socket, &mrb_zmq_socket_type, &addr, &events);
  mrb_assert(events >= 0 && events <= INT_MAX);

  int rc = zmq_socket_monitor(socket, addr, events);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_socket_monitor");
  }

  return self;
}

static mrb_value
mrb_zmq_socket_recv(mrb_state *mrb, mrb_value self)
{
  mrb_int flags = 0;
  mrb_get_args(mrb, "|i", &flags);
  mrb_assert(flags >= INT_MIN && flags <= INT_MAX);

  mrb_value data = mrb_nil_value();

  int more = 0;
  struct RClass *zmq_msg_class = mrb_class_get_under(mrb, mrb_module_get(mrb, "ZMQ"), "Msg");
  int arena_index = 0;

  do {
    mrb_value msg_val = mrb_obj_new(mrb, zmq_msg_class, 0, NULL);
    zmq_msg_t *msg = DATA_PTR(msg_val);

    int rc = zmq_msg_recv (msg, DATA_PTR(self), flags);
    if (unlikely(rc == -1)) {
      mrb_zmq_handle_error(mrb, "zmq_msg_recv");
    }
    more = zmq_msg_more(msg);
    if (more) {
      if (!mrb_array_p(data)) {
        data = mrb_ary_new_capa(mrb, 2); // We have at least two zmq messages at this point.
        arena_index = mrb_gc_arena_save(mrb);
      }
      mrb_ary_push(mrb, data, msg_val);
      mrb_gc_arena_restore(mrb, arena_index);
    } else {
      if (mrb_array_p(data)) {
        mrb_ary_push(mrb, data, msg_val);
        mrb_gc_arena_restore(mrb, arena_index);
      } else {
        data = msg_val;
      }
    }
  } while (more);

  return data;
}

static void
mrb_zmq_thread_fn(void *mrb_zmq_thread_data_)
{
  mrb_zmq_thread_data_t *mrb_zmq_thread_data = (mrb_zmq_thread_data_t *) mrb_zmq_thread_data_;
  mrb_bool success = FALSE;

  mrb_state *mrb = mrb_open();
  if (likely(mrb)) {
    mrb_zmq_thread_data->backend_ctx = MRB_LIBZMQ_CONTEXT(mrb);
    mrb_value thread_fn = mrb_nil_value();

    struct mrb_jmpbuf* prev_jmp = mrb->jmp;
    struct mrb_jmpbuf c_jmp;

    MRB_TRY(&c_jmp)
    {
      mrb->jmp = &c_jmp;
      mrb_value pipe_val = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_DATA, mrb_class_get_under(mrb, mrb_module_get(mrb, "ZMQ"), "Pair")));
      mrb_data_init(pipe_val, mrb_zmq_thread_data->backend, &mrb_zmq_socket_type);
      mrb_value argv_str = mrb_str_new_static(mrb, mrb_zmq_thread_data->argv_packed, mrb_zmq_thread_data->argv_len);
      mrb_value argv = mrb_funcall(mrb, mrb_obj_value(mrb_module_get(mrb, "MessagePack")), "unpack", 1, argv_str);
      mrb_free(mrb, mrb_zmq_thread_data->argv_packed);
      mrb_zmq_thread_data->argv_packed = NULL;
      mrb_zmq_thread_data->argv_len = 0;

      if (mrb_type(mrb_ary_ref(mrb, argv, 0)) == MRB_TT_CLASS) {
        mrb_value bg_class = mrb_ary_shift(mrb, argv);
        enum mrb_vtype ttype = MRB_INSTANCE_TT(mrb_class_ptr(bg_class));
        if (ttype == 0) ttype = MRB_TT_OBJECT;
        thread_fn = mrb_obj_value(mrb_obj_alloc(mrb, ttype, mrb_class_ptr(bg_class)));
      } else {
        thread_fn = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_OBJECT, mrb_class_get_under(mrb, mrb_module_get(mrb, "ZMQ"), "Thread_fn")));
      }
      mrb_iv_set(mrb, thread_fn, mrb_intern_lit(mrb, "@pipe"), pipe_val);
      mrb_funcall_argv(mrb, thread_fn, mrb_intern_lit(mrb, "initialize"), RARRAY_LEN(argv), RARRAY_PTR(argv));
      success = TRUE;
      int rc = zmq_send(mrb_zmq_thread_data->backend, &success, sizeof(success), 0);
      if (unlikely(rc == -1)) {
        mrb_zmq_handle_error(mrb, "zmq_send");
      }
      mrb->jmp = prev_jmp;
    }
    MRB_CATCH(&c_jmp)
    {
      mrb->jmp = prev_jmp;
      mrb_print_error(mrb);
      success = FALSE;
      zmq_send(mrb_zmq_thread_data->backend, &success, sizeof(success), 0);
    }
    MRB_END_EXC(&c_jmp);

    if (likely(success)) {
      mrb_funcall(mrb, thread_fn, "run", 0, NULL);
    }
    mrb_close(mrb);
  } else {
    zmq_send(mrb_zmq_thread_data->backend, &success, sizeof(success), 0);
    zmq_close(mrb_zmq_thread_data->backend);
  }
}

static mrb_value
mrb_zmq_threadstart(mrb_state *mrb, mrb_value thread_class)
{
  if (MRB_INSTANCE_TT(mrb_class_ptr(thread_class)) != MRB_TT_DATA) {
    mrb_raise(mrb, E_TYPE_ERROR, "thread_class instance_tt is not MRB_TT_DATA");
  }
  mrb_value self = mrb_nil_value();
  mrb_zmq_thread_data_t *mrb_zmq_thread_data = NULL;
  void *backend = NULL;
  mrb_bool success = FALSE;

  mrb_value *argv = NULL;
  mrb_int argv_len = 0;
  mrb_get_args(mrb, "*", &argv, &argv_len);

  struct mrb_jmpbuf* prev_jmp = mrb->jmp;
  struct mrb_jmpbuf c_jmp;

  MRB_TRY(&c_jmp)
  {
    mrb->jmp = &c_jmp;

    self = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_DATA, mrb_class_ptr(thread_class)));
    mrb_value args[2];
    args[0] = mrb_format(mrb, "inproc://mrb-zmq-thread-pipe-%S", mrb_fixnum_value(mrb_obj_id(self)));
    const char *endpoint = mrb_string_value_cstr(mrb, &args[0]);
    args[1] = mrb_true_value();
    mrb_value frontend_val = mrb_obj_new(mrb, mrb_class_get_under(mrb, mrb_module_get(mrb, "ZMQ"), "Pair"), sizeof(args) / sizeof(args[0]), args);
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@pipe"), frontend_val);
    void *frontend = DATA_PTR(frontend_val);
    mrb_zmq_thread_data = mrb_malloc(mrb, sizeof(*mrb_zmq_thread_data));
    memset(mrb_zmq_thread_data, 0, sizeof(*mrb_zmq_thread_data));
    mrb_zmq_thread_data->frontend = frontend;

    backend = zmq_socket(MRB_LIBZMQ_CONTEXT(mrb), ZMQ_PAIR);
    if (likely(backend)) {
      mrb_zmq_thread_data->backend = backend;
    } else {
      mrb_zmq_handle_error(mrb, "zmq_socket");
    }

    int rc = zmq_connect(backend, endpoint);
    mrb_assert(rc != -1);

    int arena_index = mrb_gc_arena_save(mrb);
    mrb_value argv_val = mrb_ary_new_from_values(mrb, argv_len, argv);

    mrb_value argv_str = mrb_funcall(mrb, argv_val, "to_msgpack", 0);
    mrb_zmq_thread_data->argv_packed = mrb_malloc(mrb, RSTRING_LEN(argv_str));
    memcpy(mrb_zmq_thread_data->argv_packed, RSTRING_PTR(argv_str), RSTRING_LEN(argv_str));
    mrb_zmq_thread_data->argv_len = RSTRING_LEN(argv_str);
    mrb_gc_arena_restore(mrb, arena_index);

    void *thread = zmq_threadstart(&mrb_zmq_thread_fn, mrb_zmq_thread_data);
    if (likely(thread)) {
      mrb_zmq_thread_data->thread = thread;
      mrb_data_init(self, mrb_zmq_thread_data, &mrb_zmq_thread_type);
    } else {
      mrb_zmq_handle_error(mrb, "zmq_threadstart");
    }

    zmq_recv(frontend, &success, sizeof(success), 0);
    if (unlikely(!success)) {
      mrb_raise(mrb, E_RUNTIME_ERROR, "Cannot initialize ZMQ Thread");
    }

    mrb_funcall_argv(mrb, self, mrb_intern_lit(mrb, "initialize"), argv_len, argv);
    mrb->jmp = prev_jmp;
  }
  MRB_CATCH(&c_jmp)
  {
    mrb->jmp = prev_jmp;
    if (mrb_zmq_thread_data && !mrb_zmq_thread_data->thread) {
      mrb_free(mrb, mrb_zmq_thread_data->argv_packed);
      mrb_free(mrb, mrb_zmq_thread_data);
    }
    if (backend && !success) {
      mrb_zmq_gc_close(mrb, backend);
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
    mrb_zmq_thread_data_t *mrb_zmq_thread_data = (mrb_zmq_thread_data_t *) DATA_PTR(thread_val);

    mrb_value frontend_val = mrb_iv_remove(mrb, thread_val, mrb_intern_lit(mrb, "@pipe"));
    if (!blocky) {
      int timeo = 0;
      zmq_setsockopt(mrb_zmq_thread_data->frontend, ZMQ_SNDTIMEO, &timeo, sizeof(timeo));
    }
    zmq_send(mrb_zmq_thread_data->frontend, "TERM$", 5, 0);
    if (!blocky) {
      int linger = 0;
      zmq_setsockopt(mrb_zmq_thread_data->frontend, ZMQ_LINGER, &linger, sizeof(linger));
    }
    zmq_close(mrb_zmq_thread_data->frontend);
    mrb_data_init(frontend_val, NULL, NULL);
    if (!blocky) {
      zmq_ctx_shutdown(mrb_zmq_thread_data->backend_ctx);
      zmq_close(mrb_zmq_thread_data->backend);
    }
    zmq_threadclose(mrb_zmq_thread_data->thread);
    mrb_free(mrb, mrb_zmq_thread_data->argv_packed);
    mrb_free(mrb, mrb_zmq_thread_data);
    mrb_data_init(thread_val, NULL, NULL);
  }

  return mrb_nil_value();
}

#ifdef ZMQ_HAVE_POLLER
MRB_INLINE void *
mrb_zmq_get_socket(mrb_state *mrb, mrb_value socket)
{
  switch(mrb_type(socket)) {
    case MRB_TT_CPTR: {
      // No way to check here if its a legitmate zmq socket, if something else is passed libzmq asserts and aborts the programm.
      // Also: when handed a raw c pointer only allow the poller to handle it because we cannot know from which thread this socket comes.
      return mrb_cptr(socket);
    } break;
    case MRB_TT_DATA: {
      if (DATA_TYPE(socket) == &mrb_zmq_socket_type) {
        return DATA_PTR(socket);
      }
    }
    default: {
      mrb_raise(mrb, E_TYPE_ERROR, "Expected a ZMQ Socket");
    }
  }
}

static mrb_value
mrb_zmq_poller_new(mrb_state *mrb, mrb_value self)
{
  if (unlikely(DATA_PTR(self))) {
    mrb_free(mrb, DATA_PTR(self));
    mrb_data_init(self, NULL, NULL);
  }

  mrb_data_init(self, zmq_poller_new(), &mrb_zmq_poller_type);

  return self;
}

static mrb_value
mrb_zmq_poller_add(mrb_state *mrb, mrb_value self)
{
  mrb_value socket, user_data;
  mrb_int events;
  mrb_get_args(mrb, "ooi", &socket, &user_data, &events);
  mrb_assert(events >= SHRT_MIN && events <= SHRT_MAX);

  int rc = zmq_poller_add(DATA_PTR(self), mrb_zmq_get_socket(mrb, socket), mrb_ptr(user_data), events);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_poller_add");
  }

  return self;
}

static mrb_value
mrb_zmq_poller_add_fd(mrb_state *mrb, mrb_value self)
{
  mrb_value socket;
  mrb_int events;
  mrb_get_args(mrb, "oi", &socket, &events);
  mrb_int fd = mrb_fixnum(mrb_Integer(mrb, socket));
  mrb_assert(fd >= INT_MIN&&fd <= INT_MAX);
  mrb_assert(events >= SHRT_MIN && events <= SHRT_MAX);

  int rc = zmq_poller_add_fd(DATA_PTR(self), fd, mrb_ptr(socket), events);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_poller_add_fd");
  }

  return self;
}

static mrb_value
mrb_zmq_poller_modify(mrb_state *mrb, mrb_value self)
{
  mrb_value socket;
  mrb_int events;
  mrb_get_args(mrb, "oi", &socket, &events);
  mrb_assert(events >= SHRT_MIN && events <= SHRT_MAX);

  int rc = zmq_poller_modify(DATA_PTR(self), mrb_zmq_get_socket(mrb, socket), events);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_poller_modify");
  }

  return self;
}

static mrb_value
mrb_zmq_poller_modify_fd(mrb_state *mrb, mrb_value self)
{
  mrb_value socket;
  mrb_int events;
  mrb_get_args(mrb, "oi", &socket, &events);
  mrb_int fd = mrb_fixnum(mrb_Integer(mrb, socket));
  mrb_assert(fd >= INT_MIN&&fd <= INT_MAX);
  mrb_assert(events >= SHRT_MIN && events <= SHRT_MAX);

  int rc = zmq_poller_modify_fd(DATA_PTR(self), fd, events);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_poller_modify_fd");
  }

  return self;
}

static mrb_value
mrb_zmq_poller_remove(mrb_state *mrb, mrb_value self)
{
  mrb_value socket;
  mrb_get_args(mrb, "o", &socket);

  int rc = zmq_poller_remove(DATA_PTR(self), mrb_zmq_get_socket(mrb, socket));
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_poller_remove");
  }

  return self;
}

static mrb_value
mrb_zmq_poller_remove_fd(mrb_state *mrb, mrb_value self)
{
  mrb_value socket;
  mrb_get_args(mrb, "o", &socket);
  mrb_int fd = mrb_fixnum(mrb_Integer(mrb, socket));
  mrb_assert(fd >= INT_MIN&&fd <= INT_MAX);

  int rc = zmq_poller_remove_fd(DATA_PTR(self), fd);
  if (unlikely(rc == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_poller_remove_fd");
  }

  return self;
}

static mrb_value
mrb_zmq_poller_wait(mrb_state *mrb, mrb_value self)
{
  mrb_int timeout;
  mrb_get_args(mrb, "i", &timeout);
  mrb_assert(timeout >= LONG_MIN && timeout <= LONG_MAX);

  zmq_poller_event_t event;
  int rc = zmq_poller_wait(DATA_PTR(self), &event, timeout);
  if (rc == -1) {
    switch(mrb_zmq_errno()) {
      case ETIMEDOUT: {
        return mrb_nil_value();
      } break;
      case EINTR: {
        return mrb_false_value();
      } break;
      default: {
        mrb_zmq_handle_error(mrb, "zmq_poller_wait");
      }
    }
  }

  return mrb_obj_value(event.user_data);
}

static mrb_value
mrb_zmq_poller_wait_all(mrb_state *mrb, mrb_value self)
{
  mrb_int n_events, timeout;
  mrb_value block = mrb_nil_value();
  mrb_get_args(mrb, "ii&", &n_events, &timeout, &block);
  mrb_assert(n_events >= 0 && n_events <= INT_MAX);
  mrb_assert(timeout >= LONG_MIN && timeout <= LONG_MAX);
  if (mrb_nil_p(block)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "no block given");
  }

  int rc = -1;
  if (n_events > 0) {
    zmq_poller_event_t events[n_events];
    rc = zmq_poller_wait_all(DATA_PTR(self), events, n_events, timeout);
    for (int i = 0; i < rc; i++) {
      mrb_value argv[2];
      argv[0] = mrb_obj_value(events[i].user_data);
      argv[1] = mrb_fixnum_value(events[i].events);
      mrb_yield_argv(mrb, block, sizeof(argv) / sizeof(argv[0]), argv);
    }
  } else {
    zmq_poller_event_t event;
    rc = zmq_poller_wait_all(DATA_PTR(self), &event, 0, timeout);
  }

  if (rc == -1) {
    switch(mrb_zmq_errno()) {
      case ETIMEDOUT: {
        return mrb_nil_value();
      } break;
      case EINTR: {
        return mrb_false_value();
      } break;
      default: {
        mrb_zmq_handle_error(mrb, "zmq_poller_wait_all");
      }
    }
  }

  return self;
}
#endif // ZMQ_HAVE_POLLER

void
mrb_mruby_zmq_gem_init(mrb_state* mrb)
{
  void *context = zmq_ctx_new();
  if (unlikely(!context)) {
    mrb_sys_fail(mrb, "zmq_ctx_new");
  }
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
  int arena_index = mrb_gc_arena_save(mrb);

  libzmq_mod = mrb_define_module(mrb, "LibZMQ");
  mrb_define_const(mrb, libzmq_mod, "_Context", mrb_cptr_value(mrb, context));
  mrb_define_module_function(mrb, libzmq_mod, "bind", mrb_zmq_bind, MRB_ARGS_REQ(2));
  mrb_define_module_function(mrb, libzmq_mod, "close", mrb_zmq_close, MRB_ARGS_ARG(1, 1));
  mrb_define_module_function(mrb, libzmq_mod, "connect", mrb_zmq_connect, MRB_ARGS_REQ(2));
  mrb_define_module_function(mrb, libzmq_mod, "ctx_get", mrb_zmq_ctx_get, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, libzmq_mod, "ctx_set", mrb_zmq_ctx_set, MRB_ARGS_REQ(2));
  mrb_define_module_function(mrb, libzmq_mod, "curve_keypair", mrb_zmq_curve_keypair, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, libzmq_mod, "disconnect", mrb_zmq_disconnect, MRB_ARGS_REQ(2));
  mrb_define_module_function(mrb, libzmq_mod, "getsockopt", mrb_zmq_getsockopt, MRB_ARGS_ARG(3, 1));
  mrb_define_module_function(mrb, libzmq_mod, "msg_gets", mrb_zmq_msg_gets, MRB_ARGS_ARG(2, 1));
  mrb_define_module_function(mrb, libzmq_mod, "msg_send", mrb_zmq_msg_send, MRB_ARGS_REQ(3));
  mrb_define_module_function(mrb, libzmq_mod, "proxy", mrb_zmq_proxy, MRB_ARGS_ARG(2, 1));
  mrb_define_module_function(mrb, libzmq_mod, "proxy_steerable", mrb_zmq_proxy_steerable, MRB_ARGS_ARG(3, 1));
  mrb_define_module_function(mrb, libzmq_mod, "send", mrb_zmq_send, MRB_ARGS_REQ(3));
  mrb_define_module_function(mrb, libzmq_mod, "setsockopt", mrb_zmq_setsockopt, MRB_ARGS_REQ(3));
  mrb_define_module_function(mrb, libzmq_mod, "socket_monitor", mrb_zmq_socket_monitor, MRB_ARGS_REQ(3));
  mrb_define_module_function(mrb, libzmq_mod, "threadclose", mrb_zmq_threadclose, MRB_ARGS_ARG(1, 1));
  mrb_define_module_function(mrb, libzmq_mod, "unbind", mrb_zmq_unbind, MRB_ARGS_REQ(2));
#ifdef ZMQ_HAS_CAPABILITIES
  mrb_define_module_function(mrb, libzmq_mod, "has?", mrb_zmq_has, MRB_ARGS_REQ(1));
#endif
#ifdef ZMQ_SERVER
  mrb_define_module_function(mrb, libzmq_mod, "msg_routing_id", mrb_zmq_msg_routing_id, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, libzmq_mod, "msg_set_routing_id", mrb_zmq_msg_set_routing_id, MRB_ARGS_REQ(2));
#endif
#ifdef ZMQ_DISH
  mrb_define_module_function(mrb, libzmq_mod, "join", mrb_zmq_join, MRB_ARGS_REQ(2));
  mrb_define_module_function(mrb, libzmq_mod, "leave", mrb_zmq_leave, MRB_ARGS_REQ(2));
  mrb_define_module_function(mrb, libzmq_mod, "msg_group", mrb_zmq_msg_group, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, libzmq_mod, "msg_set_group", mrb_zmq_msg_set_group, MRB_ARGS_REQ(2));
#endif

  zmq_mod = mrb_define_module(mrb, "ZMQ");
  zmq_msg_class = mrb_define_class_under(mrb, zmq_mod, "Msg", mrb->object_class);
  MRB_SET_INSTANCE_TT(zmq_msg_class, MRB_TT_DATA);
  mrb_define_method(mrb, zmq_msg_class, "initialize", mrb_zmq_msg_new, MRB_ARGS_NONE());
  mrb_define_method(mrb, zmq_msg_class, "to_str", mrb_zmq_msg_to_str, MRB_ARGS_OPT(1));
  mrb_define_method(mrb, zmq_msg_class, "==", mrb_zmq_msg_eql, MRB_ARGS_REQ(1));

  zmq_socket_class = mrb_define_class_under(mrb, zmq_mod, "Socket", mrb->object_class);
  MRB_SET_INSTANCE_TT(zmq_socket_class, MRB_TT_DATA);
  mrb_define_method(mrb, zmq_socket_class, "initialize", mrb_zmq_socket, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, zmq_socket_class, "recv", mrb_zmq_socket_recv, MRB_ARGS_OPT(1));

  zmq_thread_class = mrb_define_class_under(mrb, zmq_mod, "Thread", mrb->object_class);
  MRB_SET_INSTANCE_TT(zmq_thread_class, MRB_TT_DATA);
  mrb_define_class_method(mrb, zmq_thread_class, "new", mrb_zmq_threadstart, MRB_ARGS_ANY());

#ifdef ZMQ_HAVE_POLLER
  struct RClass *zmq_poller_class = mrb_define_class_under(mrb, libzmq_mod, "Poller", mrb->object_class);
  MRB_SET_INSTANCE_TT(zmq_poller_class, MRB_TT_DATA);
  mrb_define_method(mrb, zmq_poller_class, "initialize", mrb_zmq_poller_new, MRB_ARGS_NONE());
  mrb_define_method(mrb, zmq_poller_class, "add", mrb_zmq_poller_add, MRB_ARGS_ARG(2, 1));
  mrb_define_method(mrb, zmq_poller_class, "add_fd", mrb_zmq_poller_add_fd, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, zmq_poller_class, "modify", mrb_zmq_poller_modify, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, zmq_poller_class, "modify_fd", mrb_zmq_poller_modify_fd, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, zmq_poller_class, "remove", mrb_zmq_poller_remove, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, zmq_poller_class, "remove_fd", mrb_zmq_poller_remove_fd, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, zmq_poller_class, "wait", mrb_zmq_poller_wait, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, zmq_poller_class, "wait_all", mrb_zmq_poller_wait_all, (MRB_ARGS_REQ(2)|MRB_ARGS_BLOCK()));
#endif

  mrb_gc_arena_restore(mrb, arena_index);

#define mrb_zmq_define_const(ZMQ_CONST_NAME, ZMQ_CONST) \
  do { \
    mrb_define_const(mrb, libzmq_mod, ZMQ_CONST_NAME, mrb_fixnum_value(ZMQ_CONST)); \
    mrb_gc_arena_restore(mrb, arena_index); \
  } while(0)

#include "zmq_const.cstub"
}

void
mrb_mruby_zmq_gem_final(mrb_state* mrb)
{
  void *context = MRB_LIBZMQ_CONTEXT(mrb);
  zmq_ctx_shutdown(context);
  mrb_objspace_each_objects(mrb, mrb_zmq_thread_close_gem_final, mrb_class_get_under(mrb, mrb_module_get(mrb, "ZMQ"), "Thread"));
  mrb_objspace_each_objects(mrb, mrb_zmq_zmq_close_gem_final, mrb_class_get_under(mrb, mrb_module_get(mrb, "ZMQ"), "Socket"));
  zmq_ctx_term(context);
}
