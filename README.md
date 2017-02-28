# mruby-zmq
mruby bindings for libzmq (v4)

Everything libzmq offers is mapped 1:1 to the LibZMQ namespace, no beautification was done.
High level ruby functions are inside the ZMQ Namespace, you can find most of it inside the mrblib folder, except some of the Msg and Socket classes functions which have to be done in c.

As a bonus a Threading abstraction can be found in the ZMQ::Thread and ZMQ::Thread_fn Namespaces.

Examples
========

Sockets
-------
Client to Server

```ruby
server = ZMQ::Server.new("tcp://127.0.0.1:*") # you can optionally add a boolean argument if it should connect instead of binding
client = ZMQ::Client.new(server.last_endpoint) # you can optionally add a boolean argument if it should bind instead of connecting

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

Threading
=========

```ruby
thread = ZMQ::Thread.new # you can optionally add a background class here which manages the thread, all arguments get passed to the background thread too
string = thread.new(String, "hallo")
puts string.send(:to_str)
puts string.async(:upcase!)
puts string.to_str # this gets routed through method_missing and is as such a bit slower
```

Logging
=======
You can define a environment variable called ZMQ_LOGGER_ENDPOINT which creates a pub sockets which connects to that endpoint.
There is also a ZMQ_LOGGER_IDENT env var which adds a ident to each msg from ZMQ.logger

```ruby
ZMQ.logger.info("hallo")
```

LICENSE
=======
Copyright 2017 Hendrik Beskow

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this Project except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
