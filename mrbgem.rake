MRuby::Gem::Specification.new('mruby-zmq') do |spec|
  spec.license = 'Apache-2'
  spec.author  = 'Hendrik Beskow'
  spec.summary = 'mruby bindings for libzmq4'
  spec.add_conflict 'mruby-czmq'
  spec.add_dependency 'mruby-errno'
  spec.add_dependency 'mruby-sysrandom'
  spec.add_dependency 'mruby-simplemsgpack'
  spec.add_dependency 'mruby-objectspace'
  spec.add_dependency 'mruby-pack'
  if spec.build.toolchains.include? 'visualcpp'
    spec.cc.flags << 'libzmq'
  else
    `pkg-config --cflags libzmq`.split("\s").each do |cflag|
      spec.cc.flags << cflag
    end
    `pkg-config --libs libzmq`.split("\s").each do |lib|
      spec.linker.flags << lib
    end
  end
end
