require 'helper'

class Test_RAMF_AMF3_dump < Test::Unit::TestCase
  
  def test_define_RAMF
    assert Object.const_defined?('RAMF')
    assert_equal Module, RAMF.class
  end
  
  def test_dump_integer_types
    assert_equal [0x04, 0b0000_1100].pack('CC*'), RAMF::AMF3.dump(12)
    assert_equal [0x04, 0b1000_0010, 0b0010_1100].pack('CC*'), RAMF::AMF3.dump(300)
  end
  
  def test_dump_double_types
    assert_equal [0x05, 5.5].pack('CG'), RAMF::AMF3.dump(5.5)
    assert_equal [0x05, 1073741824].pack('CG'), RAMF::AMF3.dump(1073741824)
  end
  
  def test_dump_string_types
    assert_equal [0x06, 0b1011, "hello"].pack('CCa5'), RAMF::AMF3.dump("hello")
    assert_equal [0x06, 0b111,  "bye"  ].pack('CCa3'), RAMF::AMF3.dump("bye")
  end
  
  def test_dump_true_type
    assert_equal [0x03].pack('C'), RAMF::AMF3.dump(true)
  end
  
  def test_dump_false_type
    assert_equal [0x02].pack('C'), RAMF::AMF3.dump(false)
  end
  
  def test_dump_nil_type
    assert_equal [0x01].pack('C'), RAMF::AMF3.dump(nil)
  end
  
  def test_dump_time_type
    t = Time.now
    assert_equal [0x08, 0x01, (t.to_f * 1000).to_i].pack('CCG'), RAMF::AMF3.dump(t)
  end
  
  def test_dump_array_type
    assert_equal [0x09, 0x01, 0x01].pack('CCC'), RAMF::AMF3.dump([])
    assert_equal [0x09, 0x03, 0x01, 0x01].pack('CCCC'), RAMF::AMF3.dump([nil])
    assert_equal [0x09, 0x03, 0x01, 0x06, 0b111, "bye"].pack('CCCCCa3'), RAMF::AMF3.dump(["bye"])
  end
  
  def test_dump_string_refs
    str = "bye"
    assert_equal [
      0x09, 0b101, 0x01,
      0x06, 0b111, "bye",
      0x06, 0b0
    ].pack('CCCCCa3CC'), RAMF::AMF3.dump([str, str])
    assert_equal [
      0x09, 0b111, 0x01,
      0x06, 0b111, "bye",
      0x06, 0b0,
      0x06, 0b0
    ].pack('CCCCCa3CCCC'), RAMF::AMF3.dump([str, str, str])
  end
  
  def test_dump_hash_type
    assert_equal [0x09, 0x01, 0x01].pack('CCC'), RAMF::AMF3.dump({})
    assert_equal [0x09, 0x01, 0b111, "bye", 0x06, 0b111, "bye", 0x01].pack('CCCa3CCa3C'), RAMF::AMF3.dump({:bye => "bye"})
  end
  
  def test_dump_array_refs
    arr = []
    arr.push(arr)
    assert_equal [0x09, 0x03, 0x01, 0x09, 0x00].pack('CCCCC'), RAMF::AMF3.dump(arr)
    arr.push(arr)
    assert_equal [0x09, 0b101, 0x01, 0x09, 0x00, 0x09, 0x00].pack('CCCCCCC'), RAMF::AMF3.dump(arr)
  end
  
  def test_dump_object
    object = Person.new
    object.name   = "Simon Menke"
    object.age    = 22
    object.gender = "m"
    
    expected = "\x0A\x37\x35be.mrhenry.contacts.Person\x09name\x07age\x0dgender\x06\x17Simon Menke\x04\x16\x06\x03m"
    assert_equal expected, RAMF::AMF3.dump(object)
    
    object.occupation = "Development"
    
    expected = "\x0A\x3F\x35be.mrhenry.contacts.Person\x09name\x07age\x0dgender\x06\x17Simon Menke\x04\x16\x06\x03m\x15occupation\x06\x17Development\x01"
    assert_equal expected, RAMF::AMF3.dump(object)
  end
  
  def test_dump_object_ref
    n = '+32 488 478 282'
    pn1 = PhoneNumber.new(n, 'home')
    pn2 = PhoneNumber.new(n)
    c   = Card.new(pn1, pn2, pn2)
    
    expected = "\n\0271be.mrhenry.contacts.Card\ephone_numbers\t\a\001\n\037?be.mrhenry.contacts.PhoneNumber\vvalue\006\037+32 488 478 282\vlabel\006\thome\001\n\005\006\000\001\n\006"
    assert_equal expected, RAMF::AMF3.dump(c)
  end
  
end
