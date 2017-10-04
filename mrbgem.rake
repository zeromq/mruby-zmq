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
  spec.add_test_dependency 'mruby-sleep'

  task :clean do
    if File.exists?("#{spec.build_dir}/lib/libzmq.a")
      sh "cd #{spec.build_dir}/build && make uninstall"
      FileUtils.rm_rf "#{spec.build_dir}/build"
    end
  end

  if spec.cc.search_header_path 'threads.h'
    spec.cc.defines << 'HAVE_THREADS_H'
  end
  if spec.cc.search_header_path 'ifaddrs.h'
    spec.cc.defines << 'HAVE_IFADDRS_H'
  end
  if spec.build.toolchains.include? 'visualcpp'
    spec.linker.libraries << 'libzmq'
  elsif build.is_a?(MRuby::CrossBuild)
    unless File.exists?("#{spec.build_dir}/lib/libzmq.a")
      sh "cd #{spec.dir}/libzmq && ./autogen.sh && mkdir -p #{spec.build_dir}/build && cd #{spec.build_dir}/build && #{spec.dir}/libzmq/configure CC=\"#{spec.cc.command}\" CFLAGS=\"#{spec.cc.flags.join(' ')}\" LDFLAGS=\"#{spec.linker.flags.join(' ')}\" CXX=\"#{spec.cxx.command}\" CXXFLAGS=\"#{spec.cxx.flags.join(' ')}\" --host=#{build.host_target} --build=#{build.build_target} --disable-shared --enable-static --without-docs --prefix=#{spec.build_dir} && make -j4 && make install"
    end
    spec.linker.flags_before_libraries << "\"#{spec.build_dir}/lib/libzmq.a\" -pthread"
    spec.linker.libraries << 'stdc++'
    spec.cc.include_paths << "#{spec.build_dir}/include"
    spec.cxx.include_paths << "#{spec.build_dir}/include"
    build.cc.include_paths << "#{spec.build_dir}/include"
    build.cxx.include_paths << "#{spec.build_dir}/include"
    spec.cxx.defines << 'ZMQ_BUILD_DRAFT_API=1'
    spec.cc.defines << 'ZMQ_BUILD_DRAFT_API=1'
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
      unless File.exists?("#{spec.build_dir}/lib/libzmq.a")
        warn "mruby-zmq: cannot find libzmq, building it"
        if spec.cc.search_header_path 'sodium.h'
          sh "cd #{spec.dir}/libzmq && ./autogen.sh && mkdir -p #{spec.build_dir}/build && cd #{spec.build_dir}/build && #{spec.dir}/libzmq/configure CC=\"#{spec.cc.command}\" CFLAGS=\"#{spec.cc.flags.join(' ')}\" LDFLAGS=\"#{spec.linker.flags.join(' ')}\" CXX=\"#{spec.cxx.command}\" CXXFLAGS=\"#{spec.cxx.flags.join(' ')}\" --disable-shared --enable-static --without-docs --with-libsodium --prefix=#{spec.build_dir} && make -j4 && make install"
        else
          sh "cd #{spec.dir}/libzmq && ./autogen.sh && mkdir -p #{spec.build_dir}/build && cd #{spec.build_dir}/build && #{spec.dir}/libzmq/configure CC=\"#{spec.cc.command}\" CFLAGS=\"#{spec.cc.flags.join(' ')}\" LDFLAGS=\"#{spec.linker.flags.join(' ')}\" CXX=\"#{spec.cxx.command}\" CXXFLAGS=\"#{spec.cxx.flags.join(' ')}\" --disable-shared --enable-static --without-docs --prefix=#{spec.build_dir} && make -j4 && make install"
        end
      end
      spec.linker.flags_before_libraries << "\"#{spec.build_dir}/lib/libzmq.a\" -pthread"
      if spec.cc.search_header_path 'sodium.h'
        spec.linker.libraries << 'sodium'
      end
      spec.linker.libraries << 'stdc++'
      spec.cc.include_paths << "#{spec.build_dir}/include"
      spec.cxx.include_paths << "#{spec.build_dir}/include"
      build.cc.include_paths << "#{spec.build_dir}/include"
      build.cxx.include_paths << "#{spec.build_dir}/include"
      spec.cxx.defines << 'ZMQ_BUILD_DRAFT_API=1'
      spec.cc.defines << 'ZMQ_BUILD_DRAFT_API=1'
    end
  end
end
