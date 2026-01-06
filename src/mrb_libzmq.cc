#include "mrb_libzmq.h"

static mrb_value
mrb_zmq_bind(mrb_state *mrb, mrb_value self)
{
  void *socket;
  char *endpoint;
  mrb_get_args(mrb, "dz", &socket, &mrb_zmq_socket_type, &endpoint);

  int rc = zmq_bind(socket, endpoint);
  if (unlikely(-1 == rc)) {
    mrb_zmq_handle_error(mrb, "zmq_bind");
  }

  return self;
}

static mrb_value
mrb_zmq_close(mrb_state *mrb, mrb_value self)
{
  mrb_value socket_val;
  mrb_get_args(mrb, "o", &socket_val);

  if (mrb_type(socket_val) == MRB_TT_DATA && DATA_TYPE(socket_val) == &mrb_zmq_socket_type) {
    int rc = zmq_close(DATA_PTR(socket_val));
    if (unlikely(-1 == rc)) {
      mrb_zmq_handle_error(mrb, "zmq_close");
    }
    mrb_data_init(socket_val, NULL, NULL);
  }

  return mrb_nil_value();
}

static mrb_value
mrb_zmq_close_mark(mrb_state *mrb, mrb_value self)
{
  mrb_value socket_val;
  mrb_get_args(mrb, "o", &socket_val);

  if (mrb_type(socket_val) == MRB_TT_DATA && DATA_TYPE(socket_val) == &mrb_zmq_socket_type) {
    int disable = 0;
    zmq_setsockopt(DATA_PTR(socket_val), ZMQ_LINGER, &disable, sizeof(disable));
    int rc = zmq_close(DATA_PTR(socket_val));
    if (unlikely(-1 == rc)) {
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
  if (unlikely(-1 == rc)) {
    mrb_zmq_handle_error(mrb, "zmq_connect");
  }

  return self;
}

static mrb_value
mrb_zmq_ctx_get(mrb_state *mrb, mrb_value self)
{
  mrb_int option_name;
  mrb_get_args(mrb, "i", &option_name);

  int rc = zmq_ctx_get(MRB_LIBZMQ_CONTEXT(mrb), (int) option_name);
  if (unlikely(-1 == rc)) {
    mrb_zmq_handle_error(mrb, "zmq_ctx_get");
  }

  return mrb_convert_number(mrb, rc);
}

static mrb_value
mrb_zmq_ctx_set(mrb_state *mrb, mrb_value self)
{
  mrb_int option_name, option_value;
  mrb_get_args(mrb, "ii", &option_name, &option_value);

  int rc = zmq_ctx_set(MRB_LIBZMQ_CONTEXT(mrb), (int) option_name, (int) option_value);
  if (unlikely(-1 == rc)) {
    mrb_zmq_handle_error(mrb, "zmq_ctx_set");
  }

  return self;
}

static mrb_value
mrb_zmq_curve_keypair(mrb_state *mrb, mrb_value self)
{
  mrb_value public_key = mrb_str_new(mrb, NULL, 40);
  mrb_value secret_key = mrb_str_new(mrb, NULL, 40);

  int rc = zmq_curve_keypair(RSTRING_PTR(public_key), RSTRING_PTR(secret_key));
  if (unlikely(-1 == rc)) {
    mrb_zmq_handle_error(mrb, "zmq_curve_keypair");
  }

  mrb_value keypair = mrb_hash_new_capa(mrb, 2);
  mrb_hash_set(mrb, keypair, mrb_symbol_value(MRB_SYM(public_key)), public_key);
  mrb_hash_set(mrb, keypair, mrb_symbol_value(MRB_SYM(secret_key)), secret_key);

  return keypair;

}

static mrb_value
mrb_zmq_disconnect(mrb_state *mrb, mrb_value self)
{
  void *socket;
  char *endpoint;
  mrb_get_args(mrb, "dz", &socket, &mrb_zmq_socket_type, &endpoint);

  int rc = zmq_disconnect(socket, endpoint);
  if (unlikely(-1 == rc)) {
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
  if (unlikely(string_return_len <= 0)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "string_return_len must be greater than 0");
  }

  struct RClass* option_class = mrb_class_ptr(option_type);

  size_t option_len;
  int rc;

  if (option_name == ZMQ_FD) {
    SOCKET fd;
    option_len = sizeof(fd);
    rc = zmq_getsockopt(socket, option_name, &fd, &option_len);
    if (unlikely(-1 == rc)) {
      mrb_zmq_handle_error(mrb, "zmq_getsockopt");
   }
    return mrb_convert_number(mrb, fd);
  }
  else if (option_class == mrb->true_class||option_class == mrb->false_class) {
    int boolean;
    option_len = sizeof(boolean);
    rc = zmq_getsockopt(socket, option_name, &boolean, &option_len);
    if (unlikely(-1 == rc)) {
      mrb_zmq_handle_error(mrb, "zmq_getsockopt");
    }
    return mrb_bool_value(boolean);
  }
  else if (option_class == mrb->integer_class) {
    int number;
    option_len = sizeof(number);
    rc = zmq_getsockopt(socket, option_name, &number, &option_len);
    if (unlikely(-1 == rc)) {
      mrb_zmq_handle_error(mrb, "zmq_getsockopt");
    }
    return mrb_convert_number(mrb, number);
  }
  else if (option_class == mrb->float_class) {
    int64_t number;
    option_len = sizeof(number);
    rc = zmq_getsockopt(socket, option_name, &number, &option_len);
    if (unlikely(-1 == rc)) {
      mrb_zmq_handle_error(mrb, "zmq_getsockopt");
    }
    return mrb_convert_number(mrb, number);
  }
  else if (option_class == mrb->string_class) {
    mrb_value buf = mrb_str_new(mrb, NULL, string_return_len);
    option_len = RSTRING_CAPA(buf);
    rc = zmq_getsockopt(socket, option_name, RSTRING_PTR(buf), &option_len);
    if (unlikely(-1 == rc)) {
      mrb_zmq_handle_error(mrb, "zmq_getsockopt");
    }

    return mrb_str_resize(mrb, buf, option_len);
  }
  else {
    mrb_raise(mrb, E_TYPE_ERROR, "Expected True-/FalseClass|Integer|Float|String");
  }

  return self;
}

static mrb_value
mrb_zmq_msg_new(mrb_state *mrb, mrb_value self)
{
  mrb_value data = mrb_nil_value();
  mrb_get_args(mrb, "|o", &data);

  zmq_msg_t *msg = (zmq_msg_t *) mrb_realloc(mrb, DATA_PTR(self), sizeof(*msg));
  mrb_data_init(self, msg, &mrb_zmq_msg_type);

  switch (mrb_type(data)) {
    case MRB_TT_STRING: {
      int rc = zmq_msg_init_size(msg, RSTRING_LEN(data));
      if (unlikely(-1 == rc)) {
        mrb_zmq_handle_error(mrb, "zmq_msg_init_size");
      }
      memcpy(zmq_msg_data(msg), RSTRING_PTR(data), RSTRING_LEN(data));
    } break;
    case MRB_TT_FALSE: {
      zmq_msg_init(msg);
    } break;
    default: {
      mrb_raise(mrb, E_TYPE_ERROR, "(optionally) expected a String");
    }
  }

  return self;
}

static mrb_value
mrb_zmq_msg_copy(mrb_state *mrb, mrb_value copy)
{
  zmq_msg_t* msg_src;
  mrb_get_args(mrb, "d", &msg_src, &mrb_zmq_msg_type);
  if (msg_src && msg_src == DATA_PTR(copy)) return copy;

  if (unlikely(!msg_src)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "uninitialized src msg");
  }

  zmq_msg_t *msg_copy = (zmq_msg_t *) mrb_realloc(mrb, DATA_PTR(copy), sizeof(*msg_copy));
  mrb_data_init(copy, msg_copy, &mrb_zmq_msg_type);
  zmq_msg_init(msg_copy);
  int rc = zmq_msg_copy(msg_copy, msg_src);
  if (unlikely(-1 == rc)) {
    mrb_zmq_handle_error(mrb, "zmq_msg_copy");
  }
  return copy;
}

static mrb_value
mrb_zmq_msg_gets(mrb_state *mrb, mrb_value self)
{
  zmq_msg_t *msg;
  char *property;
  mrb_get_args(mrb, "dz", &msg, &mrb_zmq_msg_type, &property);

  const char *prop = zmq_msg_gets(msg, property);
  if (prop) {
    return mrb_str_new_cstr(mrb, prop);
  } else {
    return mrb_nil_value();
  }
}

static mrb_value
mrb_zmq_msg_send(mrb_state *mrb, mrb_value self)
{
  zmq_msg_t *msg;
  void *socket;
  mrb_int flags;
  mrb_get_args(mrb, "ddi", &msg, &mrb_zmq_msg_type, &socket, &mrb_zmq_socket_type, &flags);
  mrb_assert_int_fit(mrb_int, flags, int, INT_MAX);

  int rc = zmq_msg_send(msg, socket, flags);
  if (unlikely(-1 == rc)) {
    mrb_zmq_handle_error(mrb, "zmq_msg_send");
  }

  return mrb_nil_value();
}

static mrb_value
mrb_zmq_msg_size(mrb_state *mrb, mrb_value self)
{
  zmq_msg_t *msg;
  mrb_get_args(mrb, "d", &msg, &mrb_zmq_msg_type);

  return mrb_convert_number(mrb, zmq_msg_size(msg));
}

static mrb_value
mrb_zmq_proxy(mrb_state *mrb, mrb_value self)
{
  void *frontend, *backend, *capture = NULL;
  mrb_get_args(mrb, "dd|d!", &frontend, &mrb_zmq_socket_type, &backend, &mrb_zmq_socket_type, &capture, &mrb_zmq_socket_type);

  int rc = zmq_proxy(frontend, backend, capture);
  if (-1 == rc) {
    mrb_zmq_handle_error(mrb, "zmq_proxy");
  }

  return self;
}

static mrb_value
mrb_zmq_proxy_steerable(mrb_state *mrb, mrb_value self)
{
  void *frontend, *backend, *control, *capture = NULL;
  mrb_get_args(mrb, "ddd|d!", &frontend, &mrb_zmq_socket_type, &backend, &mrb_zmq_socket_type, &control, &mrb_zmq_socket_type,
    &capture, &mrb_zmq_socket_type);

  int rc = zmq_proxy_steerable(frontend, backend, capture, control);
  if (-1 == rc) {
    mrb_zmq_handle_error(mrb, "zmq_proxy");
  }

  return self;
}

static mrb_value
mrb_zmq_msg_to_str(mrb_state *mrb, mrb_value self)
{
  return mrb_str_new(mrb, (const char *) zmq_msg_data((zmq_msg_t *) DATA_PTR(self)), zmq_msg_size((zmq_msg_t *) DATA_PTR(self)));
}

static mrb_value
mrb_zmq_msg_eql(mrb_state *mrb, mrb_value self)
{
  zmq_msg_t *other;
  mrb_get_args(mrb, "d", &other, &mrb_zmq_msg_type);

  zmq_msg_t *msg = (zmq_msg_t *) DATA_PTR(self);

  if (msg == other) {
    return mrb_true_value();
  } else if (zmq_msg_size(msg) != zmq_msg_size(other)) {
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
  mrb_assert_int_fit(mrb_int, flags, int, INT_MAX);

  message = mrb_str_to_str(mrb, message);

  int rc = zmq_send(socket, RSTRING_PTR(message), RSTRING_LEN(message), flags);
  if (unlikely(-1 == rc)) {
    mrb_zmq_handle_error(mrb, "zmq_send");
  }

  return mrb_convert_number(mrb, rc);
}

static mrb_value
mrb_zmq_setsockopt(mrb_state *mrb, mrb_value self)
{
  void *socket;
  mrb_int option_name;
  mrb_value option_value;
  mrb_get_args(mrb, "dio", &socket, &mrb_zmq_socket_type, &option_name, &option_value);
  mrb_assert_int_fit(mrb_int, option_name, int, INT_MAX);

  int rc = 0;

  switch(mrb_type(option_value)) {
    case MRB_TT_FALSE: {
      if (!mrb_integer(option_value)) {
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
    case MRB_TT_INTEGER: {
      int number = (int) mrb_integer(option_value);
      rc = zmq_setsockopt(socket, option_name, &number, sizeof(number));
    } break;
    case MRB_TT_FLOAT: {
      int64_t number = (int64_t) mrb_float(option_value);
      rc = zmq_setsockopt(socket, option_name, &number, sizeof(number));
    } break;
    case MRB_TT_STRING: {
      rc = zmq_setsockopt(socket, option_name, RSTRING_PTR(option_value), RSTRING_LEN(option_value));
    } break;
    default: {
      mrb_raise(mrb, E_TYPE_ERROR, "expected nil|false|true|Integer|Float|String as value");
    }
  }

  if (unlikely(-1 == rc)) {
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
  if (unlikely(-1 == rc)) {
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

  return mrb_convert_number(mrb, zmq_msg_routing_id(msg));
}

static mrb_value
mrb_zmq_msg_set_routing_id(mrb_state *mrb, mrb_value self)
{
  zmq_msg_t *msg;
  mrb_int routing_id;
  mrb_get_args(mrb, "di", &msg, &mrb_zmq_msg_type, &routing_id);

  int rc = zmq_msg_set_routing_id(msg, (uint32_t) routing_id);
  if (unlikely(-1 == rc)) {
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
  if (unlikely(-1 == rc)) {
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
  if (unlikely(-1 == rc)) {
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
  if (unlikely(-1 == rc)) {
    mrb_zmq_handle_error(mrb, "zmq_msg_set_group");
  }

  return self;
}
#endif // ZMQ_DISH

static mrb_value
mrb_zmq_socket(mrb_state *mrb, mrb_value self)
{
  if (unlikely(DATA_PTR(self))) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "ZMQ::Socket instance already initialized");
  }

  mrb_int type;
  mrb_get_args(mrb, "i", &type);
  mrb_assert_int_fit(mrb_int, type, int, INT_MAX);

  void *socket = zmq_socket(MRB_LIBZMQ_CONTEXT(mrb), (int) type);
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
  mrb_assert_int_fit(mrb_int, events, int, INT_MAX);

  int rc = zmq_socket_monitor(socket, addr, (int) events);
  if (unlikely(-1 == rc)) {
    mrb_zmq_handle_error(mrb, "zmq_socket_monitor");
  }

  return self;
}

static mrb_value
mrb_zmq_socket_recv(mrb_state *mrb, mrb_value self)
{
  mrb_int flags = 0;
  mrb_get_args(mrb, "|i", &flags);
  mrb_assert_int_fit(mrb_int, flags, int, INT_MAX);

  int more;
  mrb_value data = mrb_nil_value();
  struct RClass *zmq_msg_class = mrb_class_get_under_id(mrb, mrb_module_get_id(mrb, MRB_SYM(ZMQ)), MRB_SYM(Msg));
  void *socket = mrb_data_get_ptr(mrb, self, &mrb_zmq_socket_type);

  do {
    mrb_value msg_val = mrb_obj_new(mrb, zmq_msg_class, 0, NULL);
    zmq_msg_t *msg = (zmq_msg_t *) mrb_data_get_ptr(mrb, msg_val, &mrb_zmq_msg_type);

    int rc = zmq_msg_recv (msg, socket, (int) flags);
    if (unlikely(-1 == rc)) {
      mrb_zmq_handle_error(mrb, "zmq_msg_recv");
    }
    more = zmq_msg_more(msg);
    if (more) {
      if (!mrb_array_p(data)) {
        data = mrb_ary_new_capa(mrb, 2); // We have at least two zmq messages at this point.
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

static mrb_value
mrb_zmq_z85_decode(mrb_state *mrb, mrb_value self)
{
  char *string;
  mrb_get_args(mrb, "z", &string);
  size_t string_len = strlen(string);

  if (unlikely(string_len % 5)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "string len must be divisible by 5");
  }

  mrb_int groups = string_len / 5;

  if (groups > MRB_INT_MAX / 4) {
    mrb_raise(mrb, E_RANGE_ERROR, "Z85 output too large");
  }

  mrb_int out_size = groups * 4;

  mrb_value dest = mrb_str_new(mrb, NULL, out_size);


  uint8_t *rc = zmq_z85_decode((uint8_t *) RSTRING_PTR(dest), string);
  if (unlikely(!rc)) {
    mrb_zmq_handle_error(mrb, "zmq_z85_decode");
  }

  return dest;
}

static mrb_value
mrb_zmq_z85_encode(mrb_state *mrb, mrb_value self)
{
  char *data;
  mrb_int size;

  mrb_get_args(mrb, "s", &data, &size);

  if (unlikely(size % 4)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "data size must be divisible by 4");
  }

  mrb_int groups = size / 4;

  if (unlikely(groups > MRB_INT_MAX / 5)) {
    mrb_raise(mrb, E_RANGE_ERROR, "Z85 output too large");
  }

  mrb_int out_size = groups * 5;

  mrb_value dest = mrb_str_new(mrb, NULL, out_size);

  char *rc = zmq_z85_encode(RSTRING_PTR(dest), (uint8_t*)data, size);
  if (unlikely(!rc)) {
    mrb_zmq_handle_error(mrb, "zmq_z85_encode");
  }

  return dest;
}

MRB_INLINE void *
mrb_zmq_get_socket(mrb_state *mrb, mrb_value socket)
{
  switch(mrb_type(socket)) {
    case MRB_TT_CPTR: {
      // No way to check here if its a legitmate zmq socket, if something else is passed libzmq asserts and aborts the programm.
      // Also: when handed a raw c pointer only allow the poller to handle it because we cannot know from which thread this socket comes.
      return mrb_cptr(socket);
    }
    case MRB_TT_DATA: {
      if (&mrb_zmq_socket_type == DATA_TYPE(socket))
        return DATA_PTR(socket);
    }
    default: {
      mrb_raise(mrb, E_TYPE_ERROR, "Expected a ZMQ Socket");
    }
  }
}

#ifdef ZMQ_HAVE_POLLER
static mrb_value
mrb_zmq_poller_new(mrb_state *mrb, mrb_value self)
{
  if (unlikely(DATA_PTR(self))) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "ZMQ::Poller instance already initialized");
  }

  void *poller = zmq_poller_new();
  if (unlikely(!poller)) {
    mrb_zmq_handle_error(mrb, "zmq_poller_new");
  }
  mrb_data_init(self, poller, &mrb_zmq_poller_type);
  mrb_iv_set(mrb, self, MRB_SYM(sockets), mrb_ary_new(mrb));

  return self;
}

static mrb_value
mrb_zmq_poller_add(mrb_state *mrb, mrb_value self)
{
  mrb_value socket;
  mrb_int events = ZMQ_POLLIN;
  mrb_get_args(mrb, "o|i", &socket, &events);
  struct RClass *socket_class = mrb_obj_class(mrb, socket);
  mrb_assert_int_fit(mrb_int, events, short, SHRT_MAX);
  void *poller = mrb_data_get_ptr(mrb, self, &mrb_zmq_poller_type);

  int rc;
  if (mrb_obj_respond_to(mrb, socket_class, MRB_SYM(fileno))) {
      mrb_value fd_val = mrb_type_convert(mrb, socket, MRB_TT_INTEGER, MRB_SYM(fileno));
      mrb_int fd = mrb_integer(fd_val);
      mrb_assert_int_fit(mrb_int, fd, int, INT_MAX);

      rc = zmq_poller_add_fd(poller, (int) fd, mrb_ptr(socket), events);
      if (unlikely(-1 == rc)) {
        mrb_zmq_handle_error(mrb, "zmq_poller_add_fd");
      }
  } else if (mrb_obj_respond_to(mrb, socket_class, MRB_SYM(zmq_socket))) {
    rc = zmq_poller_add(poller, mrb_zmq_get_socket(mrb, mrb_funcall_id(mrb, socket, MRB_SYM(zmq_socket), 0)), mrb_ptr(socket), events);
  } else {
    rc = zmq_poller_add(poller, mrb_zmq_get_socket(mrb, socket), mrb_ptr(socket), events);
  }

  if (unlikely(-1 == rc)) {
    mrb_zmq_handle_error(mrb, "zmq_poller_add");
  }

  mrb_ary_push(mrb, mrb_iv_get(mrb, self, MRB_SYM(sockets)), socket);

  return self;
}

static mrb_value
mrb_zmq_poller_modify(mrb_state *mrb, mrb_value self)
{
  mrb_value socket;
  mrb_int events;
  mrb_get_args(mrb, "oi", &socket, &events);
  struct RClass *socket_class = mrb_obj_class(mrb, socket);
  mrb_assert_int_fit(mrb_int, events, short, SHRT_MAX);
  void *poller = mrb_data_get_ptr(mrb, self, &mrb_zmq_poller_type);

  int rc;
  if (mrb_obj_respond_to(mrb, socket_class, MRB_SYM(fileno))) {
    mrb_value fd_val = mrb_type_convert(mrb, socket, MRB_TT_INTEGER, MRB_SYM(fileno));
    mrb_int fd = mrb_integer(fd_val);
    mrb_assert_int_fit(mrb_int, fd, int, INT_MAX);
    rc = zmq_poller_modify_fd(poller, (int) fd, events);
    if (unlikely(-1 == rc)) {
      mrb_zmq_handle_error(mrb, "zmq_poller_modify_fd");
    }
  } else if (mrb_obj_respond_to(mrb, socket_class, MRB_SYM(zmq_socket))) {
    rc = zmq_poller_modify(poller, mrb_zmq_get_socket(mrb, mrb_funcall_id(mrb, socket, MRB_SYM(zmq_socket), 0)), events);
  } else {
    rc = zmq_poller_modify(poller, mrb_zmq_get_socket(mrb, socket), events);
  }

  if (unlikely(-1 == rc)) {
    mrb_zmq_handle_error(mrb, "zmq_poller_modify");
  }

  return self;
}

static mrb_value
mrb_zmq_poller_remove(mrb_state *mrb, mrb_value self)
{
  mrb_value socket;
  mrb_get_args(mrb, "o", &socket);
  struct RClass *socket_class = mrb_obj_class(mrb, socket);
  void *poller = mrb_data_get_ptr(mrb, self, &mrb_zmq_poller_type);

  int rc;
  if (mrb_obj_respond_to(mrb, socket_class, MRB_SYM(fileno))) {
    mrb_value fd_val = mrb_type_convert(mrb, socket, MRB_TT_INTEGER, MRB_SYM(fileno));
    mrb_int fd = mrb_integer(fd_val);
    mrb_assert_int_fit(mrb_int, fd, int, INT_MAX);
    rc = zmq_poller_remove_fd(poller, (int) fd);
    if (unlikely(-1 == rc)) {
      mrb_zmq_handle_error(mrb, "zmq_poller_remove_fd");
    }
  } else if (mrb_obj_respond_to(mrb, socket_class, MRB_SYM(zmq_socket))) {
    rc = zmq_poller_remove(poller, mrb_zmq_get_socket(mrb, mrb_funcall_id(mrb, socket, MRB_SYM(zmq_socket), 0)));
  } else {
    rc = zmq_poller_remove(poller, mrb_zmq_get_socket(mrb, socket));
  }

  if (unlikely(-1 == rc)) {
    mrb_zmq_handle_error(mrb, "zmq_poller_remove");
  }

  mrb_funcall(mrb, mrb_iv_get(mrb, self, MRB_SYM(sockets)), "delete", 1, socket);

  return self;
}

static mrb_value
mrb_zmq_poller_wait(mrb_state *mrb, mrb_value self)
{
  mrb_int timeout = -1;
  mrb_value block = mrb_nil_value();
  mrb_get_args(mrb, "|i&", &timeout, &block);
  void *poller = mrb_data_get_ptr(mrb, self, &mrb_zmq_poller_type);

  int rc;
  if (mrb_type(block) != MRB_TT_PROC) {
    zmq_poller_event_t event;
    rc = zmq_poller_wait(poller, &event, (long) timeout);
    if (-1 == rc) {
      switch(mrb_zmq_errno()) {
        case ETIMEDOUT: {
          return mrb_nil_value();
        }
        case EINTR: {
          return mrb_false_value();
        }
        default: {
          mrb_zmq_handle_error(mrb, "zmq_poller_wait");
        }
      }
    }

    return mrb_obj_value(event.user_data);
  } else {
    mrb_int n_events = RARRAY_LEN(mrb_iv_get(mrb, self, MRB_SYM(sockets)));
    if (n_events > 0) {
      std::vector<zmq_poller_event_t> events;
      events.resize(static_cast<size_t>(n_events));

      rc = zmq_poller_wait_all(poller,
                              events.data(),
                              n_events,
                              timeout);

      for (int i = 0; i < rc; i++) {
        mrb_value argv[] = {
          mrb_obj_value(events[i].user_data),
          mrb_convert_number(mrb, events[i].events)
        };
        mrb_yield_argv(mrb, block, NELEMS(argv), argv);
      }
    } else {
      zmq_poller_event_t event;
      rc = zmq_poller_wait_all(poller, &event, 0, timeout);
    }


    if (-1 == rc) {
      switch(mrb_zmq_errno()) {
        case ETIMEDOUT: {
          return mrb_nil_value();
        }
        case EINTR: {
          return mrb_false_value();
        }
        default: {
          mrb_zmq_handle_error(mrb, "zmq_poller_wait_all");
        }
      }
    }

    return self;
  }
}
#endif // ZMQ_HAVE_POLLER

#ifdef ZMQ_HAVE_TIMERS
MRB_INLINE void
mrb_zmq_timer_fn(int timer_id, void *arg)
{
  mrb_yield(((mrb_zmq_timers_fn_t *) arg)->mrb, ((mrb_zmq_timers_fn_t *) arg)->block, mrb_convert_number(((mrb_zmq_timers_fn_t *) arg)->mrb, ((mrb_zmq_timers_fn_t *) arg)->timer_id));
}

static mrb_value
mrb_zmq_timers_new(mrb_state *mrb, mrb_value self)
{
  if (unlikely(DATA_PTR(self))) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "ZMQ::Timers instance already initialized");
  }

  void *timers = zmq_timers_new();
  if (unlikely(!timers)) {
    mrb_zmq_handle_error(mrb, "zmq_timers_new");
  }
  mrb_data_init(self, timers, &mrb_zmq_timers_type);
  mrb_iv_set(mrb, self, MRB_SYM(timers), mrb_hash_new(mrb));

  return self;
}

static mrb_value
mrb_zmq_timers_add(mrb_state *mrb, mrb_value self)
{
  mrb_int interval;
  mrb_value block = mrb_nil_value();
  mrb_get_args(mrb, "i&", &interval, &block);
  mrb_assert_int_fit(mrb_int, interval, size_t, SIZE_MAX);
  if (unlikely(mrb_type(block) != MRB_TT_PROC)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "no block given");
  }
  void *timers = mrb_data_get_ptr(mrb, self, &mrb_zmq_timers_type);

  mrb_value timer = mrb_obj_value(mrb_obj_alloc(mrb, MRB_TT_DATA, mrb_class_get_under_id(mrb, mrb_obj_class(mrb, self), MRB_SYM(Timer))));
  mrb_zmq_timers_fn_t *timer_fn_arg = (mrb_zmq_timers_fn_t *) mrb_realloc(mrb, DATA_PTR(timer), sizeof(*timer_fn_arg));
  mrb_data_init(timer, timer_fn_arg, &mrb_zmq_timers_fn_type);
  timer_fn_arg->mrb = mrb;
  timer_fn_arg->timers = self;
  timer_fn_arg->block = block;
  mrb_iv_set(mrb, timer, MRB_SYM(block), block);

  int timer_id = zmq_timers_add(timers, (size_t) interval, mrb_zmq_timer_fn, timer_fn_arg);
  if (unlikely(timer_id == -1)) {
    mrb_zmq_handle_error(mrb, "zmq_timers_add");
  }
  timer_fn_arg->timer_id = timer_id;
  mrb_hash_set(mrb, mrb_iv_get(mrb, self, MRB_SYM(timers)), mrb_convert_number(mrb, timer_id), timer);

  return timer;
}

static mrb_value
mrb_zmq_timers_fn_set_interval(mrb_state *mrb, mrb_value self)
{
  mrb_int interval;
  mrb_get_args(mrb, "i", &interval);
  mrb_assert_int_fit(mrb_int, interval, size_t, SIZE_MAX);

  mrb_zmq_timers_fn_t *timer_fn_arg = (mrb_zmq_timers_fn_t *) mrb_data_get_ptr(mrb, self, &mrb_zmq_timers_fn_type);
  void *timers = mrb_data_get_ptr(mrb, timer_fn_arg->timers, &mrb_zmq_timers_type);
  int rc = zmq_timers_set_interval(timers, timer_fn_arg->timer_id, (size_t) interval);
  if (unlikely(-1 == rc)) {
    mrb_zmq_handle_error(mrb, "zmq_timers_set_interval");
  }

  return self;
}

static mrb_value
mrb_zmq_timers_fn_reset(mrb_state *mrb, mrb_value self)
{
  mrb_zmq_timers_fn_t *timer_fn_arg = (mrb_zmq_timers_fn_t *) mrb_data_get_ptr(mrb, self, &mrb_zmq_timers_fn_type);
  void *timers = mrb_data_get_ptr(mrb, timer_fn_arg->timers, &mrb_zmq_timers_type);
  int rc = zmq_timers_reset(timers, timer_fn_arg->timer_id);
  if (unlikely(-1 == rc)) {
    mrb_zmq_handle_error(mrb, "zmq_timers_reset");
  }

  return self;
}

static mrb_value
mrb_zmq_timers_fn_cancel(mrb_state *mrb, mrb_value self)
{
  if (DATA_PTR(self)) {
    mrb_zmq_timers_fn_t *timer_fn_arg = (mrb_zmq_timers_fn_t *) mrb_data_get_ptr(mrb, self, &mrb_zmq_timers_fn_type);
    void *timers = mrb_data_get_ptr(mrb, timer_fn_arg->timers, &mrb_zmq_timers_type);
    int rc = zmq_timers_cancel(timers, timer_fn_arg->timer_id);
    if (unlikely(-1 == rc)) {
      mrb_zmq_handle_error(mrb, "zmq_timers_cancel");
    }
    mrb_hash_delete_key(mrb, mrb_iv_get(mrb, timer_fn_arg->timers, MRB_SYM(timers)), mrb_convert_number(mrb, timer_fn_arg->timer_id));
    mrb_iv_remove(mrb, self, MRB_SYM(block));
    mrb_free(mrb, timer_fn_arg);
    mrb_data_init(self, NULL, NULL);
  }

  return mrb_nil_value();
}

static mrb_value
mrb_zmq_timers_timeout(mrb_state *mrb, mrb_value self)
{
  void *timers = mrb_data_get_ptr(mrb, self, &mrb_zmq_timers_type);
  long rc = zmq_timers_timeout(timers);
  if (unlikely(-1 == rc)) {
    mrb_zmq_handle_error(mrb, "zmq_timers_timeout");
  }
  return mrb_convert_number(mrb, rc);
}

static mrb_value
mrb_zmq_timers_execute(mrb_state *mrb, mrb_value self)
{
  void *timers = mrb_data_get_ptr(mrb, self, &mrb_zmq_timers_type);
  int rc = zmq_timers_execute(timers);
  if (unlikely(-1 == rc)) {
    mrb_zmq_handle_error(mrb, "zmq_timers_execute");
  }

  return self;
}
#endif //ZMQ_HAVE_TIMERS

#ifdef HAVE_IFADDRS_H
MRB_INLINE mrb_bool
s_valid_flags (unsigned int flags)
{
    return (flags & IFF_UP)             //  Only use interfaces that are running
           && !(flags & IFF_LOOPBACK)   //  Ignore loopback interface
           && (flags & IFF_MULTICAST)
#   if defined (IFF_SLAVE)
           && !(flags & IFF_SLAVE)      //  Ignore devices that are bonding slaves.
#   endif
           && !(flags & IFF_POINTOPOINT); //  Ignore point to point interfaces.
}

static mrb_value
mrb_network_interfaces(mrb_state *mrb, mrb_value self)
{
  const int is_ipv6 = zmq_ctx_get(MRB_LIBZMQ_CONTEXT(mrb), ZMQ_IPV6);
  if (unlikely(is_ipv6 == -1)) {
    mrb_sys_fail(mrb, "zmq_ctx_get");
  }

  mrb_value interfaces_ary = mrb_ary_new(mrb);
  int ai = mrb_gc_arena_save(mrb);
  struct ifaddrs *interfaces = NULL;

  try
  {
    if (likely(getifaddrs(&interfaces) == 0)) {
      struct ifaddrs *interface;
      for (interface = interfaces; interface; interface = interface->ifa_next) {
        if (s_valid_flags(interface->ifa_flags)
          && interface->ifa_addr
          && interface->ifa_addr->sa_family == (is_ipv6 ? AF_INET6 : AF_INET)) {
          mrb_ary_push(mrb, interfaces_ary, mrb_str_new_cstr(mrb, interface->ifa_name));
          mrb_gc_arena_restore(mrb, ai);
        }
      }
      freeifaddrs(interfaces);
    } else {
      mrb_sys_fail(mrb, "getifaddrs");
    }
  }
  catch(...)
  {
    freeifaddrs(interfaces);
    throw;
  }

  return interfaces_ary;
}
#endif //HAVE_IFADDRS_H

MRB_BEGIN_DECL
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

  struct RClass *libzmq_mod, *zmq_mod, *zmq_msg_class, *zmq_socket_class;

  libzmq_mod = mrb_define_module_id(mrb, MRB_SYM(LibZMQ));
  mrb_define_const_id(mrb, libzmq_mod, MRB_SYM(_Context),
                      mrb_cptr_value(mrb, context));

  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(bind),           mrb_zmq_bind,           MRB_ARGS_REQ(2));
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(close),          mrb_zmq_close,          MRB_ARGS_REQ(1));
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM_B(close),        mrb_zmq_close_mark,     MRB_ARGS_REQ(1)); // close!
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(connect),        mrb_zmq_connect,        MRB_ARGS_REQ(2));
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(ctx_get),        mrb_zmq_ctx_get,        MRB_ARGS_REQ(1));
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(ctx_set),        mrb_zmq_ctx_set,        MRB_ARGS_REQ(2));
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(curve_keypair),  mrb_zmq_curve_keypair,  MRB_ARGS_NONE());
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(disconnect),     mrb_zmq_disconnect,     MRB_ARGS_REQ(2));
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(getsockopt),     mrb_zmq_getsockopt,     MRB_ARGS_ARG(3, 1));
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(msg_gets),       mrb_zmq_msg_gets,       MRB_ARGS_ARG(2, 1));
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(msg_send),       mrb_zmq_msg_send,       MRB_ARGS_REQ(3));
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(msg_size),       mrb_zmq_msg_size,       MRB_ARGS_NONE());
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(proxy),          mrb_zmq_proxy,          MRB_ARGS_ARG(2, 1));
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(proxy_steerable),mrb_zmq_proxy_steerable,MRB_ARGS_ARG(3, 1));
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(send),           mrb_zmq_send,           MRB_ARGS_REQ(3));
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(setsockopt),     mrb_zmq_setsockopt,     MRB_ARGS_REQ(3));
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(socket_monitor), mrb_zmq_socket_monitor, MRB_ARGS_REQ(3));
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(unbind),         mrb_zmq_unbind,         MRB_ARGS_REQ(2));

  #ifdef ZMQ_HAS_CAPABILITIES
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM_Q(has),          mrb_zmq_has,            MRB_ARGS_REQ(1)); // has?
  #endif

  #ifdef ZMQ_SERVER
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(msg_routing_id),     mrb_zmq_msg_routing_id,     MRB_ARGS_REQ(1));
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(msg_set_routing_id), mrb_zmq_msg_set_routing_id, MRB_ARGS_REQ(2));
  #endif

  #ifdef ZMQ_DISH
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(join),           mrb_zmq_join,           MRB_ARGS_REQ(2));
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(leave),          mrb_zmq_leave,          MRB_ARGS_REQ(2));
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(msg_group),      mrb_zmq_msg_group,      MRB_ARGS_REQ(1));
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(msg_set_group),  mrb_zmq_msg_set_group,  MRB_ARGS_REQ(2));
  #endif

  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(z85_decode),     mrb_zmq_z85_decode,     MRB_ARGS_REQ(1));
  mrb_define_module_function_id(mrb, libzmq_mod, MRB_SYM(z85_encode),     mrb_zmq_z85_encode,     MRB_ARGS_REQ(1));


  // ZMQ module
  zmq_mod = mrb_define_module_id(mrb, MRB_SYM(ZMQ));


  // ZMQ::Msg
  zmq_msg_class = mrb_define_class_under_id(mrb, zmq_mod, MRB_SYM(Msg), mrb->object_class);
  MRB_SET_INSTANCE_TT(zmq_msg_class, MRB_TT_DATA);

  mrb_define_method_id(mrb, zmq_msg_class, MRB_SYM(initialize),      mrb_zmq_msg_new,   MRB_ARGS_OPT(1));
  mrb_define_method_id(mrb, zmq_msg_class, MRB_SYM(initialize_copy), mrb_zmq_msg_copy,  MRB_ARGS_REQ(1));
  mrb_define_method_id(mrb, zmq_msg_class, MRB_SYM(to_str),          mrb_zmq_msg_to_str,MRB_ARGS_NONE());
  mrb_define_method_id(mrb, zmq_msg_class, MRB_OPSYM(eq),            mrb_zmq_msg_eql,   MRB_ARGS_REQ(1)); // ==


  // ZMQ::Socket
  zmq_socket_class = mrb_define_class_under_id(mrb, zmq_mod, MRB_SYM(Socket), mrb->object_class);
  MRB_SET_INSTANCE_TT(zmq_socket_class, MRB_TT_DATA);

  mrb_define_method_id(mrb, zmq_socket_class, MRB_SYM(initialize), mrb_zmq_socket,     MRB_ARGS_REQ(1));
  mrb_define_method_id(mrb, zmq_socket_class, MRB_SYM(recv),       mrb_zmq_socket_recv,MRB_ARGS_OPT(1));


  // ZMQ::Poller
  #ifdef ZMQ_HAVE_POLLER
  struct RClass *zmq_poller_class =
      mrb_define_class_under_id(mrb, zmq_mod, MRB_SYM(Poller), mrb->object_class);
  MRB_SET_INSTANCE_TT(zmq_poller_class, MRB_TT_DATA);

  mrb_define_const_id(mrb, zmq_poller_class, MRB_SYM(In),  mrb_convert_number(mrb, ZMQ_POLLIN));
  mrb_define_const_id(mrb, zmq_poller_class, MRB_SYM(Out), mrb_convert_number(mrb, ZMQ_POLLOUT));
  mrb_define_const_id(mrb, zmq_poller_class, MRB_SYM(Err), mrb_convert_number(mrb, ZMQ_POLLERR));
  mrb_define_const_id(mrb, zmq_poller_class, MRB_SYM(Pri), mrb_convert_number(mrb, ZMQ_POLLPRI));

  mrb_define_method_id(mrb, zmq_poller_class, MRB_SYM(initialize), mrb_zmq_poller_new,   MRB_ARGS_NONE());
  mrb_define_method_id(mrb, zmq_poller_class, MRB_SYM(add),        mrb_zmq_poller_add,   MRB_ARGS_ARG(1, 1));

  mrb_define_alias_id(mrb, zmq_poller_class,
                      MRB_OPSYM(lshift),   // "<<"
                      MRB_SYM(add));

  mrb_define_method_id(mrb, zmq_poller_class, MRB_SYM(modify),     mrb_zmq_poller_modify,MRB_ARGS_REQ(2));
  mrb_define_method_id(mrb, zmq_poller_class, MRB_SYM(remove),     mrb_zmq_poller_remove,MRB_ARGS_REQ(1));
  mrb_define_method_id(mrb, zmq_poller_class, MRB_SYM(wait),       mrb_zmq_poller_wait,  (MRB_ARGS_OPT(1)|MRB_ARGS_BLOCK()));
  #endif


  // ZMQ::Timers
  #ifdef ZMQ_HAVE_TIMERS
  struct RClass *zmq_timers_class, *zmq_timers_timer_fn_class;

  zmq_timers_class = mrb_define_class_under_id(mrb, zmq_mod, MRB_SYM(Timers), mrb->object_class);
  MRB_SET_INSTANCE_TT(zmq_timers_class, MRB_TT_DATA);

  mrb_define_method_id(mrb, zmq_timers_class, MRB_SYM(initialize), mrb_zmq_timers_new,    MRB_ARGS_NONE());
  mrb_define_method_id(mrb, zmq_timers_class, MRB_SYM(add),        mrb_zmq_timers_add,    (MRB_ARGS_REQ(1)|MRB_ARGS_BLOCK()));
  mrb_define_method_id(mrb, zmq_timers_class, MRB_SYM(timeout),    mrb_zmq_timers_timeout,MRB_ARGS_NONE());
  mrb_define_method_id(mrb, zmq_timers_class, MRB_SYM(execute),    mrb_zmq_timers_execute,MRB_ARGS_NONE());

  zmq_timers_timer_fn_class =
      mrb_define_class_under_id(mrb, zmq_timers_class, MRB_SYM(Timer), mrb->object_class);
  MRB_SET_INSTANCE_TT(zmq_timers_timer_fn_class, MRB_TT_DATA);

  mrb_define_method_id(mrb, zmq_timers_timer_fn_class,
                      MRB_SYM_E(interval),   // interval=
                      mrb_zmq_timers_fn_set_interval,
                      MRB_ARGS_REQ(1));

  mrb_define_method_id(mrb, zmq_timers_timer_fn_class, MRB_SYM(reset),  mrb_zmq_timers_fn_reset,  MRB_ARGS_NONE());
  mrb_define_method_id(mrb, zmq_timers_timer_fn_class, MRB_SYM(cancel), mrb_zmq_timers_fn_cancel, MRB_ARGS_NONE());
  #endif


  #ifdef HAVE_IFADDRS_H
  mrb_define_module_function_id(mrb, zmq_mod, MRB_SYM(network_interfaces),
                                mrb_network_interfaces, MRB_ARGS_NONE());
  #endif

  #define mrb_zmq_define_const(ZMQ_CONST_NAME, ZMQ_CONST) \
    do { \
      mrb_define_const_id(mrb, libzmq_mod, MRB_SYM(ZMQ_CONST_NAME), mrb_convert_number(mrb, ZMQ_CONST)); \
    } while (0)


  #include "zmq_const.cstub"
}

void
mrb_mruby_zmq_gem_final(mrb_state* mrb)
{
  void *context = MRB_LIBZMQ_CONTEXT(mrb);
  zmq_ctx_shutdown(context);
  mrb_objspace_each_objects(mrb, mrb_zmq_zmq_close_gem_final, mrb_class_get_under_id(mrb, mrb_module_get_id(mrb, MRB_SYM(ZMQ)), MRB_SYM(Socket)));
  zmq_ctx_term(context);
}
MRB_END_DECL