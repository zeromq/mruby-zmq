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

  def build_libzmq(spec, build)
    unless File.exists?("#{spec.build_dir}/build/lib/libzmq.a")
      warn "mruby-zmq: cannot find libzmq, building it"
      sh "mkdir -p #{spec.build_dir}/build && cd #{spec.build_dir}/build && cmake -DCMAKE_INSTALL_PREFIX=\"#{spec.build_dir}\" -DWITH_LIBSODIUM=OFF -DENABLE_CURVE=ON -DENABLE_DRAFTS=ON #{spec.dir}/deps/libzmq/ && cmake --build . -j4 --target libzmq-static"
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
    spec.cc.include_paths << "#{spec.dir}/deps/libzmq/include"
    spec.cxx.include_paths << "#{spec.dir}/deps/libzmq/include"
    build.cc.include_paths << "#{spec.dir}/deps/libzmq/include"
    build.cxx.include_paths << "#{spec.dir}/deps/libzmq/include"
    spec.cxx.defines << 'ZMQ_BUILD_DRAFT_API=1'
    spec.cc.defines << 'ZMQ_BUILD_DRAFT_API=1'
  end

  if spec.cc.search_header_path 'threads.h'
    spec.cc.defines << 'HAVE_THREADS_H'
  end
  if spec.cc.search_header_path 'ifaddrs.h'
    spec.cc.defines << 'HAVE_IFADDRS_H'
  end
  if spec.build.toolchains.include? 'visualcpp'
    spec.linker.libraries << 'libzmq'
  else
    if ENV['BUILD_LIBZMQ']
      build_libzmq(spec, build)
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
        build_libzmq(spec, build)
      end
    end
    spec.linker.flags_before_libraries << '-pthread'
  end
end
