module ZMQ
  class Socket
    def bind(endpoint)
      LibZMQ.bind(self, endpoint)
      self
    end

    def close(blocky = true)
      LibZMQ.close(self, blocky)
      nil
    end

    def connect(endpoint)
      LibZMQ.connect(self, endpoint)
      self
    end

    def disconnect(endpoint)
      LibZMQ.disconnect(self, endpoint)
      self
    end

    def send(data, flags = 0)
      case data
      when Array
        i = 0
        size = data.size - 1
        while i < size
          LibZMQ.send(self, data[i], LibZMQ::SNDMORE|flags)
          i += 1
        end
        LibZMQ.send(self, data[i], 0|flags)
      else
        LibZMQ.send(self, data, flags)
      end
      self
    end

    def unbind(endpoint)
      LibZMQ.unbind(self, endpoint)
      self
    end

    def readable?
      events & LibZMQ::POLLIN != 0
    end

    def writable?
      events & LibZMQ::POLLOUT != 0
    end

    def monitor(events = LibZMQ::EVENT_ALL)
      LibZMQ.socket_monitor(self, "inproc://mrb-zmq-monitor-#{object_id}", events)
      Monitor.new("inproc://mrb-zmq-mionitor-#{object_id}")
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

    def subscribe(subscribe)
      LibZMQ.setsockopt(self, LibZMQ::SUBSCRIBE, subscribe)
      self
    end

    def unsubscribe(unsubscribe)
      LibZMQ.setsockopt(self, LibZMQ::UNSUBSCRIBE, unsubscribe)
      self
    end

    def curve_security(options = {})
      if options[:type] == :server
        curve_server = true
        curve_publickey = options[:public_key]
        curve_secretkey = options[:secret_key]
        zap_domain = options[:zap_domain]
      elsif options[:type] == :client
        curve_serverkey = options[:server_key]
        curve_publickey = options[:public_key]
        curve_secretkey = options[:secret_key]
      end
      self
    end
  end

  if LibZMQ.const_defined?("CLIENT")
    class Server < Socket
      def initialize(endpoint = nil, connect = false)
        super(LibZMQ::SERVER)
        if endpoint
          if connect
            LibZMQ.connect(self, endpoint)
          else
            LibZMQ.bind(self, endpoint)
          end
        end
      end
    end

    class Client < Socket
      def initialize(endpoint = nil, bind = false)
        super(LibZMQ::CLIENT)
        if endpoint
          if bind
            LibZMQ.bind(self, endpoint)
          else
            LibZMQ.connect(self, endpoint)
          end
        end
      end
    end
  end

  if LibZMQ.const_defined?("DISH")
    class Radio < Socket
      def initialize(endpoint = nil, connect = false)
        super(LibZMQ::RADIO)
        if endpoint
          if connect
            LibZMQ.connect(self, endpoint)
          else
            LibZMQ.bind(self, endpoint)
          end
        end
      end
    end

    class Dish < Socket
      def initialize(endpoint = nil, group = nil, bind = false)
        super(LibZMQ::DISH)
        if endpoint
          if bind
            join(group) if group
            LibZMQ.bind(self, endpoint)
          else
            join(group) if group
            LibZMQ.connect(self, endpoint)
          end
        end
      end
    end
  end

  class Pub < Socket
    def initialize(endpoint = nil, connect = false)
      super(LibZMQ::PUB)
      if endpoint
        if connect
          LibZMQ.connect(self, endpoint)
        else
          LibZMQ.bind(self, endpoint)
        end
      end
    end
  end

  class Sub < Socket
    def initialize(endpoint = nil, subs = nil, bind = false)
      super(LibZMQ::SUB)
      if endpoint
        if bind
          subscribe(subs) if subs
          LibZMQ.bind(self, endpoint)
        else
          subscribe(subs) if subs
          LibZMQ.connect(self, endpoint)
        end
      end
    end
  end

  class XPub < Socket
    def initialize(endpoint = nil, connect = false)
      super(LibZMQ::XPUB)
      if endpoint
        if connect
          LibZMQ.connect(self, endpoint)
        else
          LibZMQ.bind(self, endpoint)
        end
      end
    end
  end

  class XSub < Socket
    def initialize(endpoint = nil, bind = false)
      super(LibZMQ::XSUB)
      if endpoint
        if bind
          LibZMQ.bind(self, endpoint)
        else
          LibZMQ.connect(self, endpoint)
        end
      end
    end
  end

  class Push < Socket
    def initialize(endpoint = nil, bind = false)
      super(LibZMQ::PUSH)
      if endpoint
        if bind
          LibZMQ.bind(self, endpoint)
        else
          LibZMQ.connect(self, endpoint)
        end
      end
    end
  end

  class Pull < Socket
    def initialize(endpoint = nil, connect = false)
      super(LibZMQ::PULL)
      if endpoint
        if connect
          LibZMQ.connect(self, endpoint)
        else
          LibZMQ.bind(self, endpoint)
        end
      end
    end
  end

  class Stream < Socket
    def initialize(endpoint = nil, bind = false)
      super(LibZMQ::STREAM)
      if endpoint
        if bind
          LibZMQ.bind(self, endpoint)
        else
          LibZMQ.connect(self, endpoint)
        end
      end
    end
  end

  class Pair < Socket
    def initialize(endpoint = nil, bind = false)
      super(LibZMQ::PAIR)
      if endpoint
        if bind
          LibZMQ.bind(self, endpoint)
        else
          LibZMQ.connect(self, endpoint)
        end
      end
    end
  end

  class Router < Socket
    def initialize(endpoint = nil, connect = false)
      super(LibZMQ::ROUTER)
      if endpoint
        if connect
          LibZMQ.connect(self, endpoint)
        else
          LibZMQ.bind(self, endpoint)
        end
      end
    end
  end

  class Dealer < Socket
    def initialize(endpoint = nil, bind = false)
      super(LibZMQ::DEALER)
      if endpoint
        if bind
          LibZMQ.bind(self, endpoint)
        else
          LibZMQ.connect(self, endpoint)
        end
      end
    end
  end
end
