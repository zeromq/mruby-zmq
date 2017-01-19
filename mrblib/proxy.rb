module ZMQ
  class Proxy_fn
    def initialize(options = {})
      @background = ZMQ::Socket.new(options[:background][:type])
      @background.bind(options[:background][:endpoint])
      @foreground = ZMQ::Socket.new(options[:foreground][:type])
      @foreground.bind(options[:foreground][:endpoint])
      @control = ZMQ::Pair.new(options[:_control_endpoint])
    end

    def run
      LibZMQ.proxy_steerable(@background, @foreground, @control)
    end
  end

  class Proxy
    def initialize(options = {})
      @control = ZMQ::Pair.new("tcp://127.0.0.1:*", true)
      options[:_control_endpoint] = @control.last_endpoint
      @thread = ZMQ::Thread.new
      @proxy = @thread.new(ZMQ::Proxy_fn, options)
      @proxy.async_send(:run)
    end

    def pause
      @control.send("PAUSE")
      self
    end

    def resume
      @control.send("RESUME")
      self
    end

    def terminate
      @control.send("TERMINATE")
      @control.close
      @proxy.finalize
      @thread.close
      remove_instance_variable(:@control)
      remove_instance_variable(:@thread)
      remove_instance_variable(:@proxy)
      nil
    end
  end
end
