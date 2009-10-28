
require 'ramf_ext'

class Object
  def to_amf0
    RAMF::AMF0.dump(self)
  end
  
  def to_amf3
    RAMF::AMF3.dump(self)
  end
end
