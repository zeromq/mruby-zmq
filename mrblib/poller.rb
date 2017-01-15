if LibZMQ.const_defined?("Poller")
  module ZMQ
    class Poller
      def initialize
        @poller = LibZMQ::Poller.new
        @sockets = []
      end

      def add(socket, events = LibZMQ::POLLIN)
        if socket.respond_to?(:to_i)
          @poller.add_fd(socket, socket, events)
        elsif socket.respond_to?(:socket)
          @poller.add(socket.socket, socket, events)
        else
          @poller.add(socket, socket, events)
        end
        @sockets << socket
        self
      end

      alias_method :<<, :add

      def modify(socket, events)
        if socket.respond_to?(:to_i)
          @poller.modify_fd(socket, events)
        elsif socket.respond_to?(:socket)
          @poller.modify(socket.socket, events)
        else
          @poller.modify(socket, events)
        end
        self
      end

      def remove(socket)
        if socket.respond_to?(:to_i)
          @poller.remove_fd(socket)
        elsif socket.respond_to?(:socket)
          @poller.remove(socket.socket)
        else
          @poller.remove(socket)
        end
        @sockets.delete(socket)
        self
      end

      def wait(timeout = -1, &block)
        if block
          @poller.wait_all(@sockets.size, timeout, &block)
        else
          @poller.wait(timeout)
        end
      end
    end
  end
end
