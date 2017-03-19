MessagePack.register_pack_type(1, Symbol) { |sym| String(sym) }
MessagePack.register_unpack_type(1) { |data| data.to_sym }
MessagePack.register_pack_type(2, Class) { |cls| String(cls) }
MessagePack.register_unpack_type(2) { |data| data.constantize }
MessagePack.register_pack_type(3, Exception) do |exe|
  {
    class: exe.class,
    message: exe.message,
    backtrace: exe.backtrace
  }.to_msgpack
end
MessagePack.register_unpack_type(3) do |data|
  data = MessagePack.unpack(data)
  exe = data[:class].new(data[:message])
  exe.set_backtrace(data[:backtrace])
  exe
end

module ZMQ
  class Thread_fn
    if ZMQ.const_defined?("Poller")
      def setup
        @poller = ZMQ::Poller.new
        @poller << @pipe
        @interrupted = false
        @instances = {}
        if @options[:auth].is_a?(Hash)
          @auth = Zap.new(authenticator: @options[:auth].fetch(:class).new(*@options[:auth].fetch(:args) { [] } ))
          @poller << @auth
        end
      end
    else
      def setup
        @interrupted = false
        @instances = {}
      end
    end


    def initialize(options = {})
      @options = {}.merge(options)
      setup
    end

    if ZMQ.const_defined?("Poller")
      def run
        until @interrupted
          @poller.wait do |socket, events|
            case socket
            when @pipe
              handle_pipe
            when @auth
              @auth.handle_zap
            end
          end
        end
      end
    else
      def run
        until @interrupted
          handle_pipe
        end
      end
    end

    TERM = "TERM$".freeze

    def handle_pipe
      msg = @pipe.recv.to_str(true)
      if msg == TERM
        @interrupted = true
      else
        msg = MessagePack.unpack(msg)
        begin
          case msg[:type]
          when :new
            instance = msg[:class].new(*msg[:args])
            id = instance.__id__
            LibZMQ.send(@pipe, {type: :instance, object_id: id}.to_msgpack, 0)
            @instances[id] = instance
          when :send
            LibZMQ.send(@pipe, {type: :result, result: @instances.fetch(msg[:object_id]).__send__(msg[:method], *msg[:args])}.to_msgpack, 0)
          when :async
            if (instance = @instances[msg[:object_id]])
              begin
                instance.__send__(msg[:method], *msg[:args])
              rescue => e
                ZMQ.logger.crash(e)
              end
            end
          when :finalize
            @instances.delete(msg[:object_id])
          end
        rescue => e
          LibZMQ.send(@pipe, {type: :exception, exception: e}.to_msgpack, 0)
        end
      end
    end
  end

  class Thread
    def new(mrb_class, *args)
      if block_given?
        raise ArgumentError, "blocks cannot be migrated"
      end
      LibZMQ.send(@pipe, {type: :new, class: mrb_class, args: args}.to_msgpack, 0)
      msg = MessagePack.unpack(@pipe.recv.to_str(true))
      case msg[:type]
      when :instance
        ThreadProxy.new(self, msg[:object_id])
      when :exception
        raise msg[:exception]
      end
    end

    def send(object_id, method, *args)
      LibZMQ.send(@pipe, {type: :send, object_id: object_id, method: method, args: args}.to_msgpack, 0)
      msg = MessagePack.unpack(@pipe.recv.to_str(true))
      case msg[:type]
      when :result
        msg[:result]
      when :exception
        raise msg[:exception]
      end
    end

    def async(object_id, method, *args)
      LibZMQ.send(@pipe, {type: :async, object_id: object_id, method: method, args: args}.to_msgpack, 0)
    end

    def finalize(object_id)
      LibZMQ.send(@pipe, {type: :finalize, object_id: object_id}.to_msgpack, 0)
    end

    def close(blocky = true)
      LibZMQ.threadclose(self, blocky)
    end
  end

  class ThreadProxy
    attr_reader :object_id

    def initialize(thread, object_id)
      @thread = thread
      @object_id = object_id
    end

    def send(m, *args)
      if block_given?
        raise ArgumentError, "blocks cannot be migrated"
      end
      @thread.send(@object_id, m, *args)
    end

    def async(m, *args)
      if block_given?
        raise ArgumentError, "blocks cannot be migrated"
      end
      @thread.async(@object_id, m, *args)
      self
    end

    def finalize
      @thread.finalize(@object_id)
      remove_instance_variable(:@thread)
      remove_instance_variable(:@object_id)
      nil
    end

    def respond_to?(m)
      super(m) || @thread.send(@object_id, :respond_to?, m)
    end

    def method_missing(m, *args)
      if block_given?
        raise ArgumentError, "blocks cannot be migrated"
      end
      @thread.send(@object_id, m, *args)
    end
  end
end
