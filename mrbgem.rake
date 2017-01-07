MRuby::Gem::Specification.new('mruby-libzmq4') do |spec|
  spec.license = 'Apache-2'
  spec.author  = 'Hendrik Beskow'
  spec.summary = 'hiredis bindings for libzmq4'
  spec.add_dependency 'mruby-errno'
  spec.add_dependency 'mruby-sysrandom'
  spec.add_dependency 'mruby-simplemsgpack'
  spec.add_dependency 'mruby-objectspace'
  spec.add_dependency 'mruby-exit'
  spec.linker.libraries << 'zmq'
end
