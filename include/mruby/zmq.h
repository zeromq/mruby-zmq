#ifndef MRUBY_ZMQ_H
#define MRUBY_ZMQ_H

#include <mruby.h>

#ifdef MRB_INT16
# error MRB_INT16 is too small for mruby-zmq.
#endif

MRB_BEGIN_DECL

#define E_EFSM_ERROR (mrb_class_get_under(mrb, mrb_module_get(mrb, "LibZMQ"), "EFSMError"))
#define E_ENOCOMPATPROTO_ERROR (mrb_class_get_under(mrb, mrb_module_get(mrb, "LibZMQ"), "ENOCOMPATPROTOError"))
#define E_ETERM_ERROR (mrb_class_get_under(mrb, mrb_module_get(mrb, "LibZMQ"), "ETERMError"))
#define E_EMTHREAD_ERROR (mrb_class_get_under(mrb, mrb_module_get(mrb, "LibZMQ"), "EMTHREADError"))

MRB_API void
mrb_zmq_set_context(mrb_state *mrb, void *zmq_context);

MRB_API void
mrb_zmq_ctx_shutdown_close_and_term(mrb_state* mrb);

MRB_END_DECL

#endif
