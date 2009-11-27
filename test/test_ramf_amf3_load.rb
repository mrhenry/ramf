require 'helper'

class Test_RAMF_AMF3_load < Test::Unit::TestCase
  
  def deserialize(str)
    AMF::C::AMF3Deserializer.new.deserialize(str)
  end
  
  def test_load_integer_types
    assert_equal  12, deserialize("\x04\x0c")
    assert_equal 300, deserialize("\x04\x82\x2c")
  end
  
  def test_load_true_type
    assert_equal true, deserialize("\x03")
  end
  
  def test_load_false_type
    assert_equal false, deserialize("\x02")
  end
  
  def test_load_nil_type
    assert_equal nil, deserialize("\x01")
  end
  
  def test_load_undef_type
    assert_equal nil, deserialize("\x00")
  end
  
  def test_load_double_type
    assert_equal 5.5, deserialize([0x05, 5.5].pack('CG'))
    assert_equal 1073741824, deserialize([0x05, 1073741824].pack('CG'))
  end
  
  def test_load_string_type
    assert_equal "hello", deserialize([0x06, 0b1011, "hello"].pack('CCa5'))
    assert_equal "bye",   deserialize([0x06, 0b111,  "bye"  ].pack('CCa3'))
  end
  
  def test_load_time_type
    t = Time.now
    assert_in_delta t.to_f, deserialize([0x08, 0x01, (t.to_f * 1000).to_i].pack('CCG')).to_f, 0.001
  end
  
  def test_load_array_type
    assert_equal [], deserialize([0x09, 0x01, 0x01].pack('CCC'))
    assert_equal [nil], deserialize([0x09, 0x03, 0x01, 0x01].pack('CCCC'))
    assert_equal ['bye'], deserialize([0x09, 0x03, 0x01, 0x06, 0b111, "bye"].pack('CCCCCa3'))
  end
  
  def test_dump_hash_type
    assert_equal({"bye" => "bye"}, deserialize([0x09, 0x01, 0b111, "bye", 0x06, 0b111, "bye", 0x01].pack('CCCa3CCa3C')))
  end
  
end
