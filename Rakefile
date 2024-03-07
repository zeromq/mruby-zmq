MRUBY_CONFIG=File.expand_path(ENV["MRUBY_CONFIG"] || "build_config.rb")

file :mruby do
  sh "git clone --recurse-submodules --depth=1 https://github.com/mruby/mruby.git"
  sh "git submodule update --init --recursive"
end

desc "test"
task :test => :mruby do
  sh "cd mruby && MRUBY_CONFIG=#{MRUBY_CONFIG} rake all test"
end

desc "cleanup"
task :clean do
  sh "cd mruby && MRUBY_CONFIG=#{MRUBY_CONFIG} rake deep_clean"
end

task :default => :test
