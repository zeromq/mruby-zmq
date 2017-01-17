module ZMQ
  class Authenticator
    def authenticate(domain, address, identity, mechanism, *credentials)
      case mechanism
      when 'NULL'
        null(domain, address, identity)
      when 'PLAIN'
        plain(domain, address, identity, credentials.first.to_str, credentials.last.to_str)
      when 'CURVE'
        curve(domain, address, identity, credentials.first.to_str)
      else
        raise ArgumentError, "Unknown mechanism #{mechanism.dump}"
      end
    end

    def null(domain, address, identity)
      'anonymous'
    end

    def plain(domain, address, identity, username, password)
      'anonymous'
    end

    def curve(domain, address, identity, public_key)
      'anonymous'
    end
  end

  class Auth
    attr_reader :socket

    def initialize(options = {})
      @socket = ZMQ::Socket.new(LibZMQ::ROUTER)
      @socket.bind("inproc://zeromq.zap.01")
      @authenticator = options.fetch(:authenticator) do
        Authenticator.new
      end
    end

    def handle_zap
      socket_identity, _, version, request_id, domain, address, identity, mechanism, *credentials = @socket.recv
      if version.to_str == '1.0'
        user, metadata = @authenticator.authenticate(domain.to_str, address.to_str, identity.to_str, mechanism.to_str, credentials)
        if user
          send_reply(socket_identity, _, version, request_id, 200, 'OK', user, metadata)
        else
          send_reply(socket_identity, _, version, request_id, 400, 'Invalid credentials', nil)
        end
      else
        send_reply(socket_identity, _, 1.0, request_id, 500, 'Version number not valid', nil)
      end
    end

    def send_reply(socket_identity, _, version, request_id, status_code, reason, user, metadata = nil)
      socket_identity.send(@socket, LibZMQ::SNDMORE)
      _.send(@socket, LibZMQ::SNDMORE)
      if version.is_a?(ZMQ::Msg)
        version.send(@socket, LibZMQ::SNDMORE)
      else
        LibZMQ.send(@socket, version, LibZMQ::SNDMORE)
      end
      request_id.send(@socket, LibZMQ::SNDMORE)
      LibZMQ.send(@socket, status_code, LibZMQ::SNDMORE)
      LibZMQ.send(@socket, reason, LibZMQ::SNDMORE)
      LibZMQ.send(@socket, user, LibZMQ::SNDMORE)
      if metadata
        meta = ""
        metadata.each do |key, value|
          key, value = String(key), String(value)
          if key.bytesize > 255
            raise ArgumentError, "metadata keys can only be 8 bit long"
          else
            meta << [key.bytesize].pack('C') << key
            meta << [value.bytesize].pack('N') << value
          end
        end
        LibZMQ.send(@socket, meta, 0)
      else
        LibZMQ.send(@socket, nil, 0)
      end
    end
  end
end
