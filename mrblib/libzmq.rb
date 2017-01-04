module LibZMQ
  private

  def self._finalizer
    ObjectSpace.each_object(ZMQ::Thread) do |thread|
      begin
        thread.close
      rescue
      end
    end
    ObjectSpace.each_object(ZMQ::Socket) do |socket|
      begin
        socket.close(false)
      rescue
      end
    end
  end
end
