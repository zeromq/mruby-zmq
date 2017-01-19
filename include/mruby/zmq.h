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

MRB_END_DECL

#endif
