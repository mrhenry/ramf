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
  
  def test_load_double_type
    assert_equal 5.5, RAMF::AMF3.load([0x05, 5.5].pack('CG'))
    assert_equal 1073741824, RAMF::AMF3.load([0x05, 1073741824].pack('CG'))
  end
  
  def test_load_string_type
    assert_equal "hello", RAMF::AMF3.load([0x06, 0b1011, "hello"].pack('CCa5'))
    assert_equal "bye",   RAMF::AMF3.load([0x06, 0b111,  "bye"  ].pack('CCa3'))
  end
  
  def test_load_time_type
    t = Time.now
    assert_in_delta t.to_f, RAMF::AMF3.load([0x08, 0x01, (t.to_f * 1000).to_i].pack('CCG')).to_f, 0.001
  end
  
end
