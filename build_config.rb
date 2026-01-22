MRuby::Build.new do |conf|
  toolchain :gcc
  enable_debug
  conf.enable_debug
  conf.enable_sanitizer "address,undefined,leak"
  conf.cc.flags << '-fno-omit-frame-pointer'
  conf.enable_test
  conf.gembox 'default'
  conf.gem File.expand_path(File.dirname(__FILE__))
end
