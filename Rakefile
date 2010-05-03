
task :default => :test

require 'rake/testtask'
Rake::TestTask.new(:test) do |test|
  test.libs << 'lib' << 'test'
  test.pattern = 'test/**/test_*.rb'
  test.verbose = true
end

task :test => :build_ext

task :setup_ext do
  Dir.chdir('ext') do
    system 'ruby extconf.rb'
  end
end

task :build_ext do
  Dir.chdir('ext') do
    system 'make'
  end
end
