module ZMQ
  class Socket
    class Monitor
      Events = {}

      # Core events (always present in libzmq 4.x)
      {
        EVENT_CONNECTED:        :connected,
        EVENT_CONNECT_DELAYED:  :connect_delayed,
        EVENT_CONNECT_RETRIED:  :connect_retried,
        EVENT_LISTENING:        :listening,
        EVENT_BIND_FAILED:      :bind_failed,
        EVENT_ACCEPTED:         :accepted,
        EVENT_ACCEPT_FAILED:    :accept_failed,
        EVENT_CLOSED:           :closed,
        EVENT_CLOSE_FAILED:     :close_failed,
        EVENT_DISCONNECTED:     :disconnected,
        EVENT_MONITOR_STOPPED:  :monitor_stopped,
        EVENT_ALL:              :all
      }.each do |const, sym|
        if LibZMQ.const_defined?(const)
          Events[LibZMQ.const_get(const)] = sym
        end
      end

      # Handshake events (may or may not exist depending on libzmq version)
      {
        EVENT_HANDSHAKE_FAILED_NO_DETAIL: :handshake_failed_no_detail,
        EVENT_HANDSHAKE_SUCCEEDED:        :handshake_succeeded,
        EVENT_HANDSHAKE_FAILED_PROTOCOL:  :handshake_failed_protocol,
        EVENT_HANDSHAKE_FAILED_AUTH:      :handshake_failed_auth
      }.each do |const, sym|
        if LibZMQ.const_defined?(const)
          Events[LibZMQ.const_get(const)] = sym
        end
      end

      attr_reader :zmq_socket

      def initialize(endpoint)
        @zmq_socket = ZMQ::Pair.new(endpoint)
      end

      def recv
        msg, endpoint = @zmq_socket.recv
        event, value = msg.to_str.unpack('SL')
        {
          event: Events[event] || event,
          value: value,
          endpoint: endpoint.to_str
        }
      end
    end
  end
end
