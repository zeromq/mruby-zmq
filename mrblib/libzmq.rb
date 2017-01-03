module LibZMQ
  def self.finalizer
    ObjectSpace.each_object(ZMQ::Thread) {|thread| thread.close}
    ObjectSpace.each_object(ZMQ::Socket) {|socket| socket.close(false)}
  end
end
