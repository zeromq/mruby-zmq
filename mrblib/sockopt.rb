module ZMQ
  class Socket
    [
      "backlog", "events", "fd", "handshake_ivl", "linger", "multicast_hops", "rate", "rcvhwm", "rcvtimeo", "reconnect_ivl", "reconnect_ivl_max",
      "recovery_ivl", "sndbuf", "sndhwm", "sndtimeo", "tcp_keepalive", "tcp_keepalive_cnt", "tcp_keepalive_idle", "tcp_keepalive_intvl", "tos"
    ].each do |int|
      if LibZMQ.const_defined?(int.upcase)
        const = LibZMQ.const_get(int.upcase)
        define_method(int) do
          LibZMQ.getsockopt(self, const, Fixnum)
        end
      end
    end

    alias :to_i :fd

    ["gssapi_plaintext", "gssapi_server", "immediate", "ipv6", "plain_server", "rcvmore"].each do |boolean|
      if LibZMQ.const_defined?(boolean.upcase)
        const = LibZMQ.const_get(boolean.upcase)
        define_method("#{boolean}?") do
          LibZMQ.getsockopt(self, const, TrueClass)
        end
      end
    end

    ["gssapi_principal", "gssapi_service_principal", "last_endpoint", "plain_password", "plain_username", "zap_domain"].each do |char|
      if LibZMQ.const_defined?(char.upcase)
        const = LibZMQ.const_get(char.upcase)
        define_method(char) do
          LibZMQ.getsockopt(self, const, String).chop!
        end
      end
    end

    ["curve_publickey", "curve_secretkey", "curve_serverkey"].each do |curve|
      if LibZMQ.const_defined?(curve.upcase)
        const = LibZMQ.const_get(curve.upcase)
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

    ["affinity", "maxmsgsize"].each do |int64|
      if LibZMQ.const_defined?(int64.upcase)
        const = LibZMQ.const_get(int64.upcase)
        define_method(int64) do
          LibZMQ.getsockopt(self, const, Float)
        end
      end
    end

    [
      "backlog", "handshake_ivl", "linger", "multicast_hops", "rate", "rcvhwm", "rcvtimeo", "reconnect_ivl", "reconnect_ivl_max", "recovery_ivl",
      "sndbuf", "sndhwm", "sndtimeo", "tcp_keepalive", "tcp_keepalive_cnt", "tcp_keepalive_idle", "tcp_keepalive_intvl", "tos"
    ].each do |int|
      if LibZMQ.const_defined?(int.upcase)
        const = LibZMQ.const_get(int.upcase)
        define_method("#{int}=") do |option_value|
          LibZMQ.setsockopt(self, const, option_value.to_int)
        end
      end
    end

    [
      "connect_rid", "curve_publickey", "curve_secretkey", "curve_serverkey", "gssapi_principal", "gssapi_service_principal", "plain_password",
      "plain_username", "subscribe", "unsubscribe", "zap_domain"
    ].each do |data|
      if LibZMQ.const_defined?(data.upcase)
        const = LibZMQ.const_get(data.upcase)
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
      if LibZMQ.const_defined?(int64.upcase)
        const = LibZMQ.const_get(int64.upcase)
        define_method("#{int64}=") do |option_value|
          LibZMQ.setsockopt(self, const, option_value.to_f)
        end
      end
    end

    [
      "conflate", "curve_server", "gssapi_plaintext", "gssapi_server", "immediate", "ipv6", "plain_server", "probe_router", "req_collerate",
      "req_relaxed", "router_handhover", "router_mandatory", "xpub_verbose"
    ].each do |boolean|
      if LibZMQ.const_defined?(boolean.upcase)
        const = LibZMQ.const_get(boolean.upcase)
        define_method("#{boolean}=") do |option_value|
          LibZMQ.setsockopt(self, const, option_value ? true : false)
        end
      end
    end
  end
end
