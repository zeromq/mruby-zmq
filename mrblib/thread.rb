unless MessagePack.ext_packer_registered?(Exception)
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

    def new(mrb_class, *args, &block)
      LibZMQ.send(@pipe, [NEW, mrb_class, args, block].to_msgpack, 0)
      msg = MessagePack.unpack(@pipe.recv.to_str)
      case msg[TYPE]
      when INSTANCE
        ThreadProxy.new(self, msg[1])
      when EXCEPTION
        raise msg[1]
      end
    end

    def send(object_id, method, *args, &block)
      LibZMQ.send(@pipe, [SEND, object_id, method, args, block].to_msgpack, 0)
      msg = MessagePack.unpack(@pipe.recv.to_str)
      case msg[TYPE]
      when RESULT
        msg[1]
      when EXCEPTION
        raise msg[1]
      end
    end

    def async(object_id, method, *args, &block)
      LibZMQ.send(@pipe, [ASYNC, object_id, method, args, block].to_msgpack, 0)
    end

    def finalize(object_id)
      LibZMQ.send(@pipe, [FINALIZE, object_id].to_msgpack, 0)
    end

    def close
      LibZMQ.threadclose(self)
    end

    def close!
      LibZMQ.threadclose!(self)
    end

    class Thread_fn
      include ThreadConstants

      def initialize(options = {}, &block)
        @options = options
        @interrupted = false
        @instances = {}
        if @options[:auth].is_a?(Hash)
          @auth = Zap.new(authenticator: @options[:auth].fetch(:class).new(*@options[:auth].fetch(:args) { [] } ))
          @poller = ZMQ::Poller.new
          @poller << @pipe << @auth
        end
      end

      def run
        if @auth
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
        else
          @pipe.rcvtimeo = -1
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
          begin
            msg = MessagePack.unpack(msg)
            case msg[TYPE]
            when NEW
              instance = msg[1].new(*msg[2], &msg[3])
              id = instance.__id__
              LibZMQ.send(@pipe, [INSTANCE, id].to_msgpack, 0)
              @instances[id] = instance
            when SEND
              LibZMQ.send(@pipe, [RESULT, @instances[msg[1]].__send__(msg[2], *msg[3], &msg[4])].to_msgpack, 0)
            when ASYNC
              begin
                @instances[msg[1]].__send__(msg[2], *msg[3], &msg[4])
              rescue LibZMQ::ETERMError => e
                raise e
              rescue => e
                ZMQ.logger.crash(e)
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

      def send(m, *args, &block)
        @thread.send(@object_id, m, *args, &block)
      end

      def async(m, *args, &block)
        @thread.async(@object_id, m, *args, &block)
        self
      end

      def finalize
        if instance_variable_defined? :@object_id
          @thread.finalize(@object_id)
          remove_instance_variable(:@thread)
          remove_instance_variable(:@object_id)
        end
        nil
      end

      def respond_to?(m)
        super(m) || @thread.send(@object_id, :respond_to?, m)
      end

      def method_missing(m, *args, &block)
        @thread.send(@object_id, m, *args, &block)
      end
    end
  end
end
