module ZMQ
  class << self
    attr_accessor :logger
  end

  class Logger
    def initialize(endpoint = nil, ident = nil)
      @pub = ZMQ::Pub.new(endpoint, :connect) if endpoint
      @ident = ident if ident
    end

    def log(level, message)
      if @ident
        msg = [level, @ident, Time.now.utc.to_s, message]
      else
        msg = [level, Time.now.utc.to_s, message]
      end
      puts msg.join(", ")
      @pub.send(msg) if @pub
      true
    end

    def debug(message)
      log("D", message)
    end

    def error(message)
      log("E", message)
    end

    def info(message)
      log("I", message)
    end

    def warn(message)
      log("W", message)
    end

    def crash(exception)
      log("E", format_exception(exception))
    end

    def format_exception(exception)
      str = "#{exception.class}: #{exception}\n\t"
      str << if exception.backtrace
               exception.backtrace.join("\n\t")
             else
               "EMPTY BACKTRACE\n\t"
             end
    end
  end
end
