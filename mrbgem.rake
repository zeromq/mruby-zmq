MRuby::Gem::Specification.new('mruby-zmq') do |spec|
  spec.license = 'MPL-2.0'
  spec.author  = 'Hendrik Beskow'
  spec.summary = 'mruby bindings for libzmq4'
  spec.add_conflict 'mruby-czmq'
  spec.add_dependency 'mruby-errno'
  spec.add_dependency 'mruby-proc-irep-ext'
  spec.add_dependency 'mruby-simplemsgpack'
  spec.add_dependency 'mruby-objectspace'
  spec.add_dependency 'mruby-pack'
  spec.add_dependency 'mruby-env'
  spec.add_dependency 'mruby-print'
  spec.add_dependency 'mruby-time'
  spec.add_dependency 'mruby-sprintf'
  spec.add_dependency 'mruby-class-ext'
  spec.add_dependency 'mruby-metaprog'
  spec.add_test_dependency 'mruby-sleep'

  if spec.cc.search_header_path 'threads.h'
    spec.cc.defines << 'HAVE_THREADS_H'
  end
  if spec.cc.search_header_path 'ifaddrs.h'
    spec.cc.defines << 'HAVE_IFADDRS_H'
  end
  if spec.build.toolchains.include? 'visualcpp'
    spec.linker.libraries << 'libzmq'
  else
    `pkg-config --cflags libzmq 2>/dev/null`.split("\s").each do |cflag|
      spec.cxx.flags << cflag
      spec.cc.flags << cflag
    end
    exitstatus = $?.exitstatus
    `pkg-config --libs libzmq 2>/dev/null`.split("\s").each do |lib|
      spec.linker.flags_before_libraries << lib
    end
    exitstatus += $?.exitstatus
    unless exitstatus == 0
      raise "install libzmq(-dev) before continuing"
    end
    spec.linker.flags_before_libraries << '-pthread' << '-pthread'
  end
end
