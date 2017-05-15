MRUBY_CONFIG=File.expand_path(ENV["MRUBY_CONFIG"] || "build_config.rb")

file :mruby do
  sh "git clone --depth=1 git://github.com/zeromq/libzmq.git"
  sh "git clone --depth=1 git://github.com/mruby/mruby.git"
end

desc "test"
task :test => :mruby do
  dir2 = "#{Dir.pwd}/build"
  sh "cd libzmq && ./autogen.sh && ./configure --without-docs --prefix=#{dir2} && make -j4 install"
  sh "cd mruby && PKG_CONFIG_PATH=#{dir2}/lib/pkgconfig MRUBY_CONFIG=#{MRUBY_CONFIG} rake all test"
end

task :default => :test
