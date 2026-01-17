require_relative 'mrblib/version'

MRuby::Gem::Specification.new('mruby-zmq') do |spec|
  spec.license = 'MPL-2.0'
  spec.author  = 'Hendrik Beskow'
  spec.summary = 'mruby bindings for libzmq4'
  spec.version = ZMQ::VERSION
  spec.add_conflict 'mruby-czmq'
  spec.add_dependency 'mruby-errno'
  spec.add_dependency 'mruby-objectspace'
  spec.add_dependency 'mruby-pack'
  spec.add_dependency 'mruby-env'
  spec.add_dependency 'mruby-time'
  spec.add_dependency 'mruby-sprintf'
  spec.add_dependency 'mruby-class-ext'
  spec.add_dependency 'mruby-metaprog'
  spec.add_dependency 'mruby-string-ext'
  spec.add_dependency 'mruby-c-ext-helpers' , '>= 0.2.1'
  spec.add_test_dependency 'mruby-sleep'

  def build_libzmq(spec, build)
    unless File.file?("#{spec.build_dir}/build/lib/libzmq.a")
      warn "mruby-zmq: cannot find libzmq, building it"
      cmdline = "mkdir -p #{spec.build_dir}/build && cd #{spec.build_dir}/build && cmake -DBUILD_SHARED=OFF -DBUILD_STATIC=ON -DCMAKE_INSTALL_PREFIX=\"#{spec.build_dir}\""
      if spec.search_package('libsodium')
        puts "mruby-zmq: building with libsodium"
        cmdline << " -DWITH_LIBSODIUM=ON -DWITH_LIBSODIUM_STATIC=ON -DENABLE_CURVE=ON"
      end
      cmdline << " -DENABLE_DRAFTS=ON #{spec.dir}/deps/libzmq/ -DCMAKE_POLICY_VERSION_MINIMUM=3.5 && cmake --build . -j$(nproc) --target libzmq-static && make -j$(nproc) install"
      sh cmdline
    end
    spec.linker.flags_before_libraries << "\"#{spec.build_dir}/build/lib/libzmq.a\""
    `pkg-config --cflags \"#{spec.build_dir}/build/libzmq.pc\"`.split("\s").each do |cflag|
      spec.cxx.flags << cflag
      spec.cc.flags << cflag
    end
    `pkg-config --static --libs \"#{spec.build_dir}/build/libzmq.pc\"`.split("\s").each do |lib|
      next if lib == '-lzmq'
      spec.linker.flags_before_libraries << lib
    end
    src = File.join(spec.build_dir, 'include')
    dst = File.join(File.expand_path(File.dirname(__FILE__)), 'include')
    FileUtils.mkdir_p(dst)
    FileUtils.cp_r("#{src}/.", dst)
  end

  if spec.cxx.search_header_path 'ifaddrs.h'
    spec.cxx.defines << 'HAVE_IFADDRS_H'
  end
  if spec.build.toolchains.include? 'visualcpp'
    spec.linker.libraries << 'libzmq'
  else
    if ENV['BUILD_LIBZMQ']
      build_libzmq(spec, build)
    else
      unless spec.search_package('libzmq')
        build_libzmq(spec, build)
      end

    end
    spec.linker.flags_before_libraries << '-pthread'
  end
end
