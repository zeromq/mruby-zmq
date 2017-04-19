MessagePack.register_pack_type(1, Symbol) { |sym| sym.to_s }
MessagePack.register_unpack_type(1) { |data| data.to_sym }
MessagePack.register_pack_type(2, Class) { |cls| cls.name }
MessagePack.register_unpack_type(2) { |data| data.constantize }
MessagePack.register_pack_type(3, Exception) do |exe|
  [
    exe.class,
    exe.message,
    exe.backtrace
  ].to_msgpack
end
MessagePack.register_unpack_type(3) do |data|
  data = MessagePack.unpack(data)
  exe = data[0].new(data[1])
  exe.set_backtrace(data[2])
  exe
end

module ZMQ
  module ThreadConstants
    TYPE = 0.freeze

    #create a new object
    NEW = 0.freeze
    #send a command
    SEND = 1.freeze
    #async send a command
    ASYNC = 2.freeze
    #"mark" a object for GC by removing it from instances list
    FINALIZE = 3.freeze

    #got a new object
    INSTANCE = 0.freeze
    #got the result of a send
    RESULT = 1.freeze
    #got a exception
    EXCEPTION = 2.freeze
  end

  class Thread
    include ThreadConstants

    def new(mrb_class, *args)
      if block_given?
        raise ArgumentError, "blocks cannot be migrated"
      end
      LibZMQ.send(@pipe, [NEW, mrb_class, args].to_msgpack, 0)
      msg = MessagePack.unpack(@pipe.recv.to_str)
      case msg[TYPE]
      when INSTANCE
        ThreadProxy.new(self, msg[1])
      when EXCEPTION
        raise msg[1]
      end
    end

    def send(object_id, method, *args)
      LibZMQ.send(@pipe, [SEND, object_id, method, args].to_msgpack, 0)
      msg = MessagePack.unpack(@pipe.recv.to_str)
      case msg[TYPE]
      when RESULT
        msg[1]
      when EXCEPTION
        raise msg[1]
      end
    end

    def async(object_id, method, *args)
      LibZMQ.send(@pipe, [ASYNC, object_id, method, args].to_msgpack, 0)
    end

    def finalize(object_id)
      LibZMQ.send(@pipe, [FINALIZE, object_id].to_msgpack, 0)
    end

    def close(blocky = true)
      LibZMQ.threadclose(self, blocky)
    end

    class Thread_fn
      include ThreadConstants

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
          if @options[:auth].is_a?(Hash)
            raise RuntimeError, "authentication: libzmq was compiled without Poller support"
          end
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
        msg = @pipe.recv.to_str
        if msg == TERM
          @interrupted = true
        else
          msg = MessagePack.unpack(msg)
          begin
            case msg[TYPE]
            when NEW
              instance = msg[1].new(*msg[2])
              id = instance.__id__
              LibZMQ.send(@pipe, [INSTANCE, id].to_msgpack, 0)
              @instances[id] = instance
            when SEND
              LibZMQ.send(@pipe, [RESULT, @instances.fetch(msg[1]).__send__(msg[2], *msg[3])].to_msgpack, 0)
            when ASYNC
              if (instance = @instances[msg[1]])
                begin
                  instance.__send__(msg[2], *msg[3])
                rescue => e
                  ZMQ.logger.crash(e)
                end
              end
            when FINALIZE
              @instances.delete(msg[1])
            end
          rescue => e
            LibZMQ.send(@pipe, [EXCEPTION, e].to_msgpack, 0)
          end
        end
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
end
