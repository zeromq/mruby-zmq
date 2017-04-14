module ZMQ
  ["ipv6", "blocky"].each do |contextopt|
    optup = contextopt.upcase
    if LibZMQ.const_defined?(optup)
      const = LibZMQ.const_get(optup)
      define_singleton_method("#{contextopt}?") do
        LibZMQ.ctx_get(const) == 1
      end
    end
  end

  ["io_threads", "max_sockets", "max_msgsz", "socket_limit", "msg_t_size"].each do |contextopt|
    optup = contextopt.upcase
    if LibZMQ.const_defined?(optup)
      const = LibZMQ.const_get(optup)
      define_singleton_method(contextopt) do
        LibZMQ.ctx_get(const)
      end
    end
  end

  ["blocky", "ipv6"].each do |contextopt|
    optup = contextopt.upcase
    if LibZMQ.const_defined?(optup)
      const = LibZMQ.const_get(optup)
      define_singleton_method("#{contextopt}=") do |option_value|
        LibZMQ.ctx_set(const, option_value ? 1 : 0)
      end
    end
  end

  ["max_msgsz", "max_sockets"].each do |contextopt|
    optup = contextopt.upcase
    if LibZMQ.const_defined?(optup)
      const = LibZMQ.const_get(optup)
      define_singleton_method("#{contextopt}=") do |option_value|
        LibZMQ.ctx_set(const, option_value)
      end
    end
  end
end
