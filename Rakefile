MRUBY_CONFIG=File.expand_path(ENV["MRUBY_CONFIG"] || "build_config.rb")

file :deps do
  sh "git clone --depth=1 git://github.com/zeromq/libzmq.git"
  sh "git clone --depth=1 git://github.com/mruby/mruby.git"
end

desc "test"
task :test => :deps do
  sh "cd libzmq && ./autogen.sh && ./configure --without-docs && make -j2 && sudo make install"
  if ENV['TRAVIS_OS_NAME'] == 'linux'
    sh "sudo ldconfig"
  end
  sh "cd mruby && MRUBY_CONFIG=#{MRUBY_CONFIG} rake all test"
end

task :default => :test
