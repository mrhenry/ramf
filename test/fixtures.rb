#
# class Person
#   attr_accessor :name, :age, :gender
#   attr_accessor :occupation
#   attr_reader :age_in_seconds
#
#   amf \
#     :class   => 'be.mrhenry.contacts.Person',
#     :static  => %w( @name age gender ),
#     :ignore  => %w( age_in_seconds )
# end
#
# class Card
#   attr_accessor :phone_numbers
#   amf \
#     :class   => 'be.mrhenry.contacts.Card',
#     :static  => %w( @phone_numbers ),
#     :dynamic => false
#   def initialize(*numbers)
#     @phone_numbers = numbers
#   end
# end
#
# class PhoneNumber
#   attr_accessor :value, :label
#   amf \
#     :class   => 'be.mrhenry.contacts.PhoneNumber',
#     :static  => %w( @value ),
#     :dynamic => true
#   def initialize(value, label=nil)
#     @value = value
#     @label = label if label
#   end
# end
