module ZMQ
  class Thread_fn
    def initialize(pipe)
      @pipe = pipe
    end

    def run
      while true
        msg = @pipe.recv
        puts MessagePack.unpack(msg)
      end
    end
  end

  class Thread
    def setup(*args, &block)
      @pipe.send("hallo".to_msgpack)
    end

    def init(mrb_class, *args)
      @pipe.send({type: :init, class: mrb_class, args: args}.to_msgpack)
      msg = MessagePack.unpack(@pipe.recv.to_str)
      case msg[:type]
      when :init_obj
      when :init_error
        raise msg[:class], msg[:msg]
      end
    end
  end
end
