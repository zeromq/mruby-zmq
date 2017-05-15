MRUBY_CONFIG=File.expand_path(ENV["MRUBY_CONFIG"] || "build_config.rb")

file :mruby do
  sh "git clone --depth=1 git://github.com/zeromq/libzmq.git"
  sh "git clone --depth=1 git://github.com/mruby/mruby.git"
end

desc "test"
task :test => :mruby do
  dir = "#{Dir.pwd}/build"
  sh "cd libzmq && ./autogen.sh && ./configure --without-docs --prefix=#{dir} && make -j4 install"
  ENV['PKG_CONFIG_PATH'] = "#{dir}/lib/pkgconfig"
  sh "cd mruby && MRUBY_CONFIG=#{MRUBY_CONFIG} rake all test"
end

task :default => :test
