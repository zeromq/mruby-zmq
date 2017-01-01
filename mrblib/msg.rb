module ZMQ
  class Msg
    def send(socket, flags = 0)
      LibZMQ.msg_send(self, socket, flags)
    end

    def gets(property, static_string = false)
      LibZMQ.msg_gets(self, property, static_string)
    end
  end
end
