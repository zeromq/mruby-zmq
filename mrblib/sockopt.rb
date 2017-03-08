module ZMQ
  class Socket
    ["backlog", "events", "fd", "handshake_ivl", "heartbeat_ivl", "heartbeat_ttl", "heartbeat_timeout", "linger", "multicast_hops", "rate",
      "rcvhwm", "rcvtimeo", "reconnect_ivl", "reconnect_ivl_max", "recovery_ivl", "sndbuf", "sndhwm", "sndtimeo",
      "tcp_keepalive", "tcp_keepalive_cnt", "tcp_keepalive_idle", "tcp_keepalive_intvl", "tos"
    ].each do |int|
      upint = int.upcase
      if LibZMQ.const_defined?(upint)
        const = LibZMQ.const_get(upint)
        define_method(int) do
          LibZMQ.getsockopt(self, const, Fixnum)
        end
      end
    end

    ["gssapi_plaintext", "gssapi_server", "immediate", "ipv6", "plain_server", "rcvmore"].each do |boolean|
      upbool = boolean.upcase
      if LibZMQ.const_defined?(upbool)
        const = LibZMQ.const_get(upbool)
        define_method("#{boolean}?") do
          LibZMQ.getsockopt(self, const, TrueClass)
        end
      end
    end

    ["gssapi_principal", "gssapi_service_principal", "last_endpoint", "plain_password", "plain_username", "zap_domain"].each do |char|
      upchar = char.upcase
      if LibZMQ.const_defined?(upchar)
        const = LibZMQ.const_get(upchar)
        define_method(char) do
          LibZMQ.getsockopt(self, const, String).chop!
        end
      end
    end

    ["curve_publickey", "curve_secretkey", "curve_serverkey"].each do |curve|
      upcurve = curve.upcase
      if LibZMQ.const_defined?(upcurve)
        const = LibZMQ.const_get(upcurve)
        define_method(curve) do
          LibZMQ.getsockopt(self, const, String, 32)
        end
      end
    end

    def identity
      LibZMQ.getsockopt(self, LibZMQ::IDENTITY, String)
    end

    def identity=(option_value)
      LibZMQ.setsockopt(self, LibZMQ::IDENTITY, option_value.to_str)
    end

    def readable?
      events & LibZMQ::POLLIN != 0
    end

    def writable?
      events & LibZMQ::POLLOUT != 0
    end

    if LibZMQ.respond_to?("join")
      def join(group)
        LibZMQ.join(self, group)
        self
      end

      def leave(group)
        LibZMQ.leave(self, group)
        self
      end
    end

    def subscribe(topic)
      LibZMQ.setsockopt(self, LibZMQ::SUBSCRIBE, topic)
      self
    end

    def unsubscribe(topic)
      LibZMQ.setsockopt(self, LibZMQ::UNSUBSCRIBE, topic)
      self
    end

    if LibZMQ.respond_to?("has?") && LibZMQ.has?("curve")
      def curve_security(options = {})
        if options[:type] == :server
          self.curve_server = true
          self.curve_secretkey = options.fetch(:secret_key)
        elsif options[:type] == :client
          self.curve_serverkey = options.fetch(:server_key)
          self.curve_publickey = options.fetch(:public_key)
          self.curve_secretkey = options.fetch(:secret_key)
        else
          raise ArgumentError, ":type can only be :server or :client"
        end
        self
      end
    end

    ["affinity", "maxmsgsize"].each do |int64|
      upint64 = int64.upcase
      if LibZMQ.const_defined?(upint64)
        const = LibZMQ.const_get(upint64)
        define_method(int64) do
          LibZMQ.getsockopt(self, const, Float)
        end
      end
    end

    ["backlog", "handshake_ivl", "heartbeat_ivl", "heartbeat_ttl", "heartbeat_timeout", "linger", "multicast_hops", "rate", "rcvhwm",
      "rcvtimeo", "reconnect_ivl", "reconnect_ivl_max", "recovery_ivl", "sndbuf", "sndhwm", "sndtimeo",
      "tcp_keepalive", "tcp_keepalive_cnt", "tcp_keepalive_idle", "tcp_keepalive_intvl", "tos"].each do |int|
        upint = int.upcase
      if LibZMQ.const_defined?(upint)
        const = LibZMQ.const_get(upint)
        define_method("#{int}=") do |option_value|
          LibZMQ.setsockopt(self, const, option_value.to_int)
        end
      end
    end

    ["connect_rid", "curve_publickey", "curve_secretkey", "curve_serverkey", "gssapi_principal", "gssapi_service_principal", "plain_password",
      "plain_username", "zap_domain"].each do |data|
        updata = data.upcase
      if LibZMQ.const_defined?(updata)
        const = LibZMQ.const_get(updata)
        define_method("#{data}=") do |option_value = nil|
          if option_value
            LibZMQ.setsockopt(self, const, option_value.to_str)
          else
            LibZMQ.setsockopt(self, const, nil)
          end
        end
      end
    end

    ["affinity", "maxmsgsize"].each do |int64|
      upint64 = int64.upcase
      if LibZMQ.const_defined?(upint64)
        const = LibZMQ.const_get(upint64)
        define_method("#{int64}=") do |option_value|
          LibZMQ.setsockopt(self, const, option_value.to_f)
        end
      end
    end

    ["conflate", "curve_server", "gssapi_plaintext", "gssapi_server", "immediate", "ipv6", "plain_server", "probe_router", "req_collerate",
      "req_relaxed", "router_handhover", "router_mandatory", "xpub_verbose"].each do |boolean|
        upbool = boolean.upcase
      if LibZMQ.const_defined?(upbool)
        const = LibZMQ.const_get(upbool)
        define_method("#{boolean}=") do |option_value|
          LibZMQ.setsockopt(self, const, option_value ? true : false)
        end
      end
    end
  end
end
