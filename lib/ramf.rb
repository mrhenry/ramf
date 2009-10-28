
require 'ramf_ext'

class Object
  class << self
    
    def amf(traits={})
      self.amf_traits.merge!(traits)
    end
    
    def amf_traits
      @amf_traits ||= (self.superclass.amf_traits.dup rescue {})
    end
    
  end
  
  def to_amf0
    RAMF::AMF0.dump(self)
  end
  
  def to_amf3
    RAMF::AMF3.dump(self)
  end
end
