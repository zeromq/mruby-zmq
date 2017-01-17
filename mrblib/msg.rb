module ZMQ
  class Msg
    def send(socket, flags = 0)
      LibZMQ.msg_send(self, socket, flags)
    end

    def [](property)
      LibZMQ.msg_gets(self, property, false)
    end

    if LibZMQ.respond_to?("routing_id")
      def routing_id
        LibZMQ.msg_routing_id(self)
      end

      def routing_id=(route_id)
        LibZMQ.msg_set_routing_id(self, route_id)
      end
    end

    if LibZMQ.respond_to?("msg_group")
      def group
        LibZMQ.msg_group(self)
      end

      def group=(grp)
        LibZMQ.msg_set_group(self, grp)
      end
    end
  end
end
