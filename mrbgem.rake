MRuby::Gem::Specification.new('mruby-zmq') do |spec|
  spec.license = 'MPL-2.0'
  spec.author  = 'Hendrik Beskow'
  spec.summary = 'mruby bindings for libzmq4'
  spec.add_conflict 'mruby-czmq'
  spec.add_dependency 'mruby-errno'
  spec.add_dependency 'mruby-simplemsgpack'
  spec.add_dependency 'mruby-objectspace'
  spec.add_dependency 'mruby-pack'
  spec.add_dependency 'mruby-env'
  spec.add_dependency 'mruby-print'
  spec.add_dependency 'mruby-time'
  spec.add_dependency 'mruby-sprintf'
  spec.add_dependency 'mruby-class-ext'
  spec.add_test_dependency 'mruby-sleep'

  if spec.cc.search_header_path 'ifaddrs.h'
    spec.cc.defines << 'HAVE_IFADDRS'
  end
  if spec.build.toolchains.include? 'visualcpp'
    spec.linker.libraries << 'libzmq'
  else
    `pkg-config --cflags libzmq`.split("\s").each do |cflag|
      spec.cc.flags << cflag
    end
    `pkg-config --libs libzmq`.split("\s").each do |lib|
      spec.linker.flags_before_libraries << lib
    end
  end
end
