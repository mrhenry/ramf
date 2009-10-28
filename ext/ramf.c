
#include "ruby.h"
#include "ramf_amf3.h"

VALUE m_ramf = Qnil;
VALUE m_ramf_amf0 = Qnil;
VALUE m_ramf_amf3 = Qnil;

void Init_ramf_ext();
static VALUE ramf_dump_amf0(VALUE self, VALUE value);
static VALUE ramf_load_amf0(VALUE self, VALUE io_or_string);

void Init_ramf_ext() {
  m_ramf = rb_define_module("RAMF");
  m_ramf_amf0 = rb_define_module_under(m_ramf, "AMF0");
  m_ramf_amf3 = rb_define_module_under(m_ramf, "AMF3");
  rb_define_module_function(m_ramf_amf0, "dump", ramf_dump_amf0, 1);
  rb_define_module_function(m_ramf_amf0, "load", ramf_load_amf0, 1);
  rb_define_module_function(m_ramf_amf3, "dump", ramf_dump_amf3, 1);
  rb_define_module_function(m_ramf_amf3, "load", ramf_load_amf3, 1);
}

static VALUE ramf_dump_amf0(VALUE self, VALUE value)
{
  return Qundef;
}

static VALUE ramf_load_amf0(VALUE self, VALUE io_or_string)
{
  return Qundef;
}