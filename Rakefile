MRUBY_CONFIG=File.expand_path(ENV["MRUBY_CONFIG"] || "build_config.rb")

file :deps do
  sh "git clone --depth=1 git://github.com/mruby/mruby.git"
end

desc "test"
task :test => :deps do
  sh "cd mruby && MRUBY_CONFIG=#{MRUBY_CONFIG} rake all test"
end

task :default => :test
