assert('ZMQ::Thread') do
  15.times do
    th = ZMQ::Thread.new
    str = th.new(String, "hallo")
    assert_equal("hallo", str.send(:to_str))
  end
end

assert('PubSub') do
  publisher = ZMQ::Pub.new(ZMQ.ipv6? ? "tcp://[::1]:*" : "tcp://127.0.0.1:*")
  publisher.sndtimeo = 500
  subscriber = ZMQ::Sub.new(publisher.last_endpoint, "hallo")
  sleep 2
  publisher.send("hallo ballo")
  subscriber.rcvtimeo = 500
  msg = subscriber.recv
  assert_equal("hallo ballo", msg.to_str)
end

assert('XPubXSub') do
  publisher = ZMQ::XPub.new(ZMQ.ipv6? ? "tcp://[::1]:*" : "tcp://127.0.0.1:*")
  publisher.rcvtimeo = 500
  publisher.sndtimeo = 500
  subscriber = ZMQ::XSub.new(publisher.last_endpoint)
  subscriber.rcvtimeo = 500
  subscriber.sndtimeo = 500
  subscriber.send("\001hallo")
  publisher.recv
  publisher.send("hallo ballo")
  msg = subscriber.recv
  assert_equal("hallo ballo", msg.to_str)
end

assert('PullPush') do
  pull1 = ZMQ::Pull.new(ZMQ.ipv6? ? "tcp://[::1]:*" : "tcp://127.0.0.1:*")
  pull1.rcvtimeo = 500
  pull2 = ZMQ::Pull.new(ZMQ.ipv6? ? "tcp://[::1]:*" : "tcp://127.0.0.1:*")
  pull2.rcvtimeo = 500
  push = ZMQ::Push.new(pull1.last_endpoint)
  push.connect(pull2.last_endpoint)
  push.sndtimeo = 500

  push.send("1")
  push.send("2")
  assert_equal("1", pull1.recv.to_str)
  assert_equal("2", pull2.recv.to_str)
end

assert('RouterDealer') do
  router = ZMQ::Router.new(ZMQ.ipv6? ? "tcp://[::1]:*" : "tcp://127.0.0.1:*")
  router.rcvtimeo = 500
  router.sndtimeo = 500
  dealer = ZMQ::Dealer.new
  dealer.identity = "#{dealer.object_id}"
  dealer.rcvtimeo = 500
  dealer.sndtimeo = 500
  dealer.connect(router.last_endpoint)
  dealer.send("hallo")
  peer, msg = router.recv
  assert_equal(peer.to_str, "#{dealer.object_id}")
  assert_equal("hallo", msg.to_str)
  peer.send(router, LibZMQ::SNDMORE)
  msg.send(router)
  msg = dealer.recv
  assert_equal("hallo", msg.to_str)
end

if ZMQ.const_defined?("Dish")
  assert('DishRadio') do
    dish = ZMQ::Dish.new("udp://239.0.0.1:5555", "test_zmq", :bind)
    dish.rcvtimeo = 500
    radio = ZMQ::Radio.new("udp://239.0.0.1:5555", :connect)
    radio.sndtimeo = 500
    msg = ZMQ::Msg.new("hallo")
    msg.group = "test_zmq"
    msg.send(radio)
    msg = dish.recv
    assert_equal("hallo", msg.to_str)
    assert_equal("test_zmq", msg.group)
  end
end

assert('Msg') do
  # Ensures optional parameter works.
  ZMQ::Msg.new()
  ZMQ::Msg.new("hallo")
end
