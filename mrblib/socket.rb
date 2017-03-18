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

    def monitor(events = LibZMQ::EVENT_ALL)
      endpoint = "inproc://mrb-zmq-monitor-#{object_id}"
      LibZMQ.socket_monitor(self, endpoint, events)
      Monitor.new(endpoint)
    end
  end # Socket

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
  end # Client

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
        join(group) if group
        if endpoint
          if bind
            LibZMQ.bind(self, endpoint)
          else
            LibZMQ.connect(self, endpoint)
          end
        end
      end
    end
  end # Dish

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
      subscribe(subs) if subs
      if endpoint
        if bind
          LibZMQ.bind(self, endpoint)
        else
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

  class Rep < Socket
    def initialize(endpoint = nil, connect = false)
      super(LibZMQ::REP)
      if endpoint
        if connect
          LibZMQ.connect(self, endpoint)
        else
          LibZMQ.bind(self, endpoint)
        end
      end
    end
  end

  class Req < Socket
    def initialize(endpoint = nil, bind = false)
      super(LibZMQ::REQ)
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
