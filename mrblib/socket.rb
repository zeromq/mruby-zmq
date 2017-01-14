module ZMQ
  class Socket
    def bind(endpoint)
      LibZMQ.bind(self, endpoint)
    end

    def close(blocky = true)
      LibZMQ.close(self, blocky)
    end

    def connect(endpoint)
      LibZMQ.connect(self, endpoint)
    end

    def disconnect(endpoint)
      LibZMQ.disconnect(self, endpoint)
    end

    def send(data, flags = 0)
      case data
      when Array
        i = 0
        size = data.size - 1
        while i < size
          LibZMQ.send(self, data[i], LibZMQ::SNDMORE)
          i += 1
        end
        LibZMQ.send(self, data[i], 0)
      else
        LibZMQ.send(self, data, flags)
      end
      self
    end

    def unbind(endpoint)
      LibZMQ.unbind(self, endpoint)
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
    def initialize(endpoint = nil, bind = false)
      super(LibZMQ::SUB)
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
    def initialize(endpoint = nil, connect = false)
      super(LibZMQ::PUSH)
      if endpoint
        if connect
          LibZMQ.connect(self, endpoint)
        else
          LibZMQ.bind(self, endpoint)
        end
      end
    end
  end

  class Pull < Socket
    def initialize(endpoint = nil, bind = false)
      super(LibZMQ::PULL)
      if endpoint
        if bind
          LibZMQ.bind(self, endpoint)
        else
          LibZMQ.connect(self, endpoint)
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
end
