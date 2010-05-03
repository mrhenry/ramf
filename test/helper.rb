require 'rubygems'
require 'test/unit'

$:.unshift(File.extend_path('../../ext', __FILE__))
$:.unshift(File.extend_path('../../lib', __FILE__))
$:.unshift(File.extend_path('..', __FILE__))

require 'ramf'
require 'fixtures'

class Test::Unit::TestCase
end
