require 'benchmark'
require 'rubygems'
require 'amfora'
require '../ext/ramf_ext'

def bm_amf(t, n, kc, kp, o)
  c = kc.new
  p = kp.new
  td_c = c.deserialize(o).inspect
  td_p = p.deserialize(o).inspect
  cmp  = (td_c == td_p) ? '==' : '<>'
  
  puts "#{t}: #{td_c} #{cmp} #{td_p} (#{n})"
  
  Benchmark.bm do |x|
    x.report('AMF0::C   ') { n.times { c.deserialize(o) } }
    x.report('AMF0::Pure') { n.times { p.deserialize(o) } }
  end
  
  puts
end

d1, d2 = AMF::C::AMF3Deserializer, AMF::Pure::AMF3Deserializer


t = "booleans (true)"
o = "\x03"
bm_amf("read #{t}", 500000, d1, d2, o)


t = "booleans (false)"
o = "\x02"
bm_amf("read #{t}", 500000, d1, d2, o)


t = "doubles"
o = [0x05, 5.5].pack('CG')
bm_amf("read #{t}", 500000, d1, d2, o)


t = "strings"
o = [0x06, 0b1011, "hello"].pack('CCa5')
bm_amf("read #{t}", 500000, d1, d2, o)