module ZMQ
  class Socket
    def close
      LibZMQ.close(self)
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
        LibZMQ.send(self, data.last, 0)
      else
        LibZMQ.send(self, data, flags)
      end
      self
    end
  end

  class Dealer
    def self.new(endpoint, bind = false)
      instance = ZMQ::Socket.new(LibZMQ::DEALER)
      if bind
        LibZMQ.bind(instance, endpoint)
      else
        LibZMQ.connect(instance, endpoint)
      end
      instance
    end
  end
end
