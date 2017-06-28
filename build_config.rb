MRuby::Build.new do |conf|
  toolchain :gcc
  enable_debug
  conf.enable_test
  conf.gembox 'full-core'
  conf.gem File.expand_path(File.dirname(__FILE__))
  conf.cc.defines << 'MRB_INT64'
  conf.cxx.defines << 'MRB_INT64'
  conf.cc.defines << 'MRB_USE_ETEXT_EDATA'
  conf.cxx.defines << 'MRB_USE_ETEXT_EDATA'
end
