#!/usr/bin/env ruby

Dir.chdir(File.dirname($0))

d = File.open("zmq_const.cstub", "w")

IO.readlines("zmq_const.def").each { |name|
  next if name =~ /^#/
  name.strip!

  d.write <<-C
#ifdef ZMQ_#{name}
mrb_zmq_define_const("#{name}", ZMQ_#{name});
#endif
C
}
