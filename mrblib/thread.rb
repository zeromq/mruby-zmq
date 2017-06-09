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

    # Rough sketch how the background thread gets started
    #def self.new(*args, &block)
    # instance = super()
    # mrb_zmq_thread_data = malloc(sizeof(mrb_zmq_thread_data_t))
    # endpoint = "inproc://mrb-zmq-thread-pipe-#{instance.object_id}"
    # frontend = ZMQ::Pair.new(endpoint, :bind)
    # mrb_zmq_thread_data->frontend = frontend
    # frontend.sndtimeo = 12000
    # frontend.rcvtimeo = 12000
    # instance.instance_variable_set(:@pipe, frontend)
    # mrb_zmq_thread_data->backend = ZMQ::Pair.new(endpoint)
    # mrb_zmq_thread_data->argv_packed = args.to_msgpack
    # mrb_zmq_thread_data->block_packed = block.to_msgpack
    # thread = zmq_threadstart(&Thread_fn, mrb_zmq_thread_data)
    # if thread
    #   mrb_zmq_thread_data->thread = thread
    # else
    #   sys_fail("zmq_threadstart")
    # end
    # unless frontend.recv
    #   raise RuntimeError, "Cannot initialize ZMQ Thread"
    # end
    # instance.initialize(*args)
    # instance
    #end

    def new(mrb_class, *args, &block)
      LibZMQ.send(@pipe, [NEW, mrb_class, args, block].to_msgpack, 0)
      msg = MessagePack.unpack(@pipe.recv.to_str(true))
      case msg[TYPE]
      when INSTANCE
        ThreadProxy.new(self, msg[1])
      when EXCEPTION
        raise msg[1]
      end
    end

    def send(object_id, method, *args, &block)
      LibZMQ.send(@pipe, [SEND, object_id, method, args, block].to_msgpack, 0)
      msg = MessagePack.unpack(@pipe.recv.to_str(true))
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

    def close(blocky = true)
      LibZMQ.threadclose(self, blocky)
    end

    # this is a rough Sketch how the background thread runs
    #def &Thread_fn(mrb_zmq_thread_data)
    #  success = false
    #  mrb = mrb_open()
    #  if (mrb) {
    #    mrb_zmq_thread_data->backend_ctx = LibZMQ::_Context
    #    thread_fn = nil
    #    begin
    #      pipe = mrb_zmq_thread_data->backend
    #      pipe.sndtimeo = 12000
    #      pipe.rcvtimeo = 12000
    #      argv = MessagePack.unpack(mrb_zmq_thread_data->argv_packed)
    #      block = MessagePack.unpack(mrb_zmq_thread_data->block_packed)
    #      if argv[0].is_a?(Class)
    #        thread_fn = argv.shift.allocate
    #      else
    #        thread_fn = ZMQ::Thread::Thread_fn.allocate
    #      end
    #      thread_fn.instance_variable_set(:@pipe, pipe)
    #      thread_fn.initialize(*argv, &block)
    #      success = true
    #      pipe.send(success)
    #    rescue => e
    #      success = false
    #      mrb_zmq_thread_data->backend.send(success)
    #    end
    #    if success
    #      thread_fn.run
    #    end
    #    if (mrb->exc && mrb_zmq_errno() != ETERM) {
    #      mrb_print_error(mrb)
    #    }
    #    mrb_close(mrb)
    #  } else {
    #    mrb_zmq_thread_data->backend.send(success)
    #    mrb_zmq_thread_data->backend.close
    #  }
    #end

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

      def initialize(options = {}, &block)
        @options = options
        setup
      end

      if ZMQ.const_defined?("Poller")
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
      else #Poller
        def run
          @pipe.rcvtimeo = -1
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
        @thread.finalize(@object_id)
        remove_instance_variable(:@thread)
        remove_instance_variable(:@object_id)
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
