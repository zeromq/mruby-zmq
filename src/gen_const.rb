#!/usr/bin/env ruby

Dir.chdir(File.dirname($0))

d = File.open("zmq_const.cstub", "w")

define_match = /^[ \t]*#define ZMQ_(\S+)[ \t]*((?:.*\\\r?\n)*.*)/m
IO.readlines('../deps/libzmq/include/zmq.h').each do |line|
  if (match = define_match.match(line))
    begin
      Integer(match[2])
      d.write <<-C
#ifdef ZMQ_#{match[1]}
mrb_zmq_define_const(#{match[1]}, ZMQ_#{match[1]});
#endif
C
    rescue
    end
  end
end
