# -*- encoding: utf-8 -*-
lib = File.expand_path('../lib/', __FILE__)
$:.unshift lib unless $:.include?(lib)

require 'ramf/version'

Gem::Specification.new do |s|
  s.name        = "ramf"
  s.version     = RAMF::VERSION
  s.platform    = Gem::Platform::RUBY
  s.authors     = ["Simon Menke"]
  s.email       = ["simon.menke@gmail.com"]
  s.homepage    = "http://github.com/mrhenry/ramf"
  s.summary     = "AMF (de-)serializer"
  s.description = "AMF (de-)serializer for ruby"

  s.required_rubygems_version = ">= 1.3.6"
  s.rubyforge_project         = "ramf"

  s.files        = Dir.glob("{lib,test}/**/*") + Dir.glob("{ext}/**/*.{h,c,rb}") + %w(LICENSE README.md)
  s.test_files   = Dir.glob("test/**/*")
  s.extensions   = ["ext/extconf.rb"]
  s.require_path = 'lib'
end