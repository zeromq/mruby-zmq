# mruby-zmq
mruby bindings for https://github.com/zeromq/libzmq (v4)

Everything libzmq offers is mapped 1:1 to the LibZMQ namespace, no beautification was done.
High level ruby functions are inside the ZMQ Namespace, you can find most of it inside the mrblib folder, except some of the Msg and Socket classes functions which have to be done in c.

As a bonus a Threading abstraction can be found in the ZMQ::Thread and ZMQ::Thread_fn Namespaces.

Installation
============

If you dont have libzmq installed on a non Windows platform we will build it and statically link it into your mruby app.
If you have it installed you need to have pkg-config installed.
Then add

```ruby
conf.gem mgem: 'mruby-zmq'
```
to your build_config.rb

Examples
========

Sockets
-------
Client to Server

```ruby
server = ZMQ::Server.new("tcp://127.0.0.1:*") # you can optionally add a boolean argument if it should connect instead of bind
client = ZMQ::Client.new(server.last_endpoint) # you can optionally add a boolean argument if it should bind instead of connect

client.send("hello")
msg = server.recv
puts msg['Peer-Address'] # This gets you the metadata of a message
puts msg.to_str
msg2 = ZMQ::Msg.new("hello too")
msg2.routing_id = msg.routing_id
msg2.send(server)
puts client.recv.to_str
```

Dealer to Router

```ruby
router = ZMQ::Router.new("tcp://127.0.0.1:*")
dealer = ZMQ::Dealer.new(router.last_endpoint)

dealer.send("hello")
msg = router.recv
puts msg[1].to_str
router.send([msg[0], "hello too"])
puts dealer.recv.to_str
```

Pub to Sub

```ruby
pub = ZMQ::Pub.new("tcp://127.0.0.1:*")
sub = ZMQ::Sub.new(pub.last_endpoint, "testme")
# you have to wait a short time here because of the slow joiner Problem
pub.send("testme hallo")
puts sub.recv.to_str
```

Logging
=======
You can define a environment variable called ZMQ_LOGGER_ENDPOINT to create pub sockets which connect to that endpoint.
There is also a ZMQ_LOGGER_IDENT env var which adds a ident to each msg from ZMQ.logger

```ruby
ZMQ.logger.info("hallo")
```

LICENSE
=======
Copyright 2017,2021 Hendrik Beskow

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  project, You can obtain one at http://mozilla.org/MPL/2.0/.

Contribution Policy
===================

mruby-zmq follows the C4 Process: https://rfc.zeromq.org/spec:42/C4
