require 'helper'

class Test_RAMF_AMF3_load < Test::Unit::TestCase
  
  def test_load_integer_types
    assert_equal  12, RAMF::AMF3.load("\x04\x0c")
    assert_equal 300, RAMF::AMF3.load("\x04\x82\x2c")
  end
  
  def test_load_true_type
    assert_equal true, RAMF::AMF3.load("\x03")
  end
  
  def test_load_false_type
    assert_equal false, RAMF::AMF3.load("\x02")
  end
  
  def test_load_nil_type
    assert_equal nil, RAMF::AMF3.load("\x01")
  end
  
  def test_load_undef_type
    assert_equal nil, RAMF::AMF3.load("\x00")
  end
  
end
