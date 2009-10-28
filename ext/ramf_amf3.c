
#include "ruby.h"
#include "ramf_amf3.h"
#include "emitter.h"

#include <stddef.h>
#include <sys/types.h>
#include <netinet/in.h>

typedef enum {
  RAMF_AMF_UNDEFINED_TYPE  = 0x00,
  RAMF_AMF_NULL_TYPE       = 0x01,
  RAMF_AMF_FALSE_TYPE      = 0x02,
  RAMF_AMF_TRUE_TYPE       = 0x03,
  RAMF_AMF_INTEGER_TYPE    = 0x04,
  RAMF_AMF_DOUBLE_TYPE     = 0x05,
  RAMF_AMF_STRING_TYPE     = 0x06,
  RAMF_AMF_XML_DOC_TYPE    = 0x07,
  RAMF_AMF_DATE_TYPE       = 0x08,
  RAMF_AMF_ARRAY_TYPE      = 0x09,
  RAMF_AMF_OBJECT_TYPE     = 0x0A,
  RAMF_AMF_XML_TYPE        = 0x0B,
  RAMF_AMF_BYTE_ARRAY_TYPE = 0x0C
} RAMF_AMF_TYPE;

typedef struct {
  emitter_t emitter;
  VALUE strings;
  int string_id;
  VALUE objects;
  int object_id;
  VALUE traits;
  int trait_id;
} ramf_dump_context_t;

#define emitter (&(context->emitter))

inline void emit_value(ramf_dump_context_t* context, VALUE value);
inline void emit_type(ramf_dump_context_t* context, RAMF_AMF_TYPE type);
inline void emit_ruby_integer(ramf_dump_context_t* context, VALUE integer);
inline void emit_ruby_float(ramf_dump_context_t* context, VALUE flt);
inline void emit_ruby_string(ramf_dump_context_t* context, VALUE string);
inline void emit_ruby_time(ramf_dump_context_t* context, VALUE time);
inline void emit_u29(ramf_dump_context_t* context, uint32_t integer);
inline void emit_double(ramf_dump_context_t* context, double num);
inline void emit_cstring(ramf_dump_context_t* context, u_char* string, size_t len);
inline void emit_cstring_ref(ramf_dump_context_t* context, int ref);
inline void emit_cdate(ramf_dump_context_t* context, uint64_t millisec);
inline void emit_cdate_ref(ramf_dump_context_t* context, int ref);
inline void emit_ruby_array(ramf_dump_context_t* context, VALUE array);
inline void emit_ruby_hash(ramf_dump_context_t* context, VALUE hash);

inline int cached_string(ramf_dump_context_t* context, VALUE string) {
  VALUE key   = ULONG2NUM((uint64_t)string);
  VALUE cache = context->strings;
  VALUE id    = rb_hash_aref(cache, key);
  if (id == Qnil) {
    id = INT2FIX(context->string_id);
    context->string_id ++;
    rb_hash_aset(cache, key, id);
    return -1;
  } else {
    return NUM2INT(id);
  }
}

inline int cached_object(ramf_dump_context_t* context, VALUE object) {
  VALUE key   = ULONG2NUM((uint64_t)object);
  VALUE cache = context->objects;
  VALUE id    = rb_hash_aref(cache, key);
  if (id == Qnil) {
    id = INT2FIX(context->object_id);
    context->object_id ++;
    rb_hash_aset(cache, key, id);
    return -1;
  } else {
    return NUM2INT(id);
  }
}

inline int cached_trait(ramf_dump_context_t* context, VALUE trait) {
  VALUE key   = ULONG2NUM((uint64_t)trait);
  VALUE cache = context->traits;
  VALUE id    = rb_hash_aref(cache, key);
  if (id == Qnil) {
    id = INT2FIX(context->trait_id);
    context->trait_id ++;
    rb_hash_aset(cache, key, id);
    return -1;
  } else {
    return NUM2INT(id);
  }
}

VALUE ramf_dump_amf3(VALUE self, VALUE value)
{
  ramf_dump_context_t ctx;
  ctx.strings = rb_hash_new();
  ctx.string_id = 0;
  ctx.objects = rb_hash_new();
  ctx.object_id = 0;
  ctx.traits  = rb_hash_new();
  ctx.trait_id = 0;
  ramf_dump_context_t * context = &ctx;
  emitter_initialize(emitter);
  
  emit_value(context, value);
  
  VALUE data = emitter_to_rstring(emitter);
  emitter_finalize(emitter);
  return data;
}

inline void emit_value(ramf_dump_context_t* context, VALUE value)
{
  if (0) { /* ignore */ }
  
  else if (rb_obj_is_kind_of(value, rb_cInteger))
  { emit_ruby_integer(context, value);
  }
  
  else if (rb_obj_is_kind_of(value, rb_cFloat))
  { emit_type(context, RAMF_AMF_DOUBLE_TYPE);
    emit_ruby_float(context, value);
  }
  
  else if (rb_obj_is_kind_of(value, rb_cString))
  { emit_type(context, RAMF_AMF_STRING_TYPE);
    emit_ruby_string(context, value);
  }
  
  else if (rb_obj_is_kind_of(value, rb_cTime))
  { emit_type(context, RAMF_AMF_DATE_TYPE);
    emit_ruby_time(context, value);
  }
  
  else if (rb_obj_is_kind_of(value, rb_cArray))
  { emit_type(context, RAMF_AMF_ARRAY_TYPE);
    emit_ruby_array(context, value);
  }
  
  else if (rb_obj_is_kind_of(value, rb_cHash))
  { emit_type(context, RAMF_AMF_ARRAY_TYPE);
    emit_ruby_hash(context, value);
  }
  
  else if (value == Qfalse)
  { emit_type(context, RAMF_AMF_FALSE_TYPE);
  }
  
  else if (value == Qtrue)
  { emit_type(context, RAMF_AMF_TRUE_TYPE);
  }
  
  else if (value == Qundef)
  { emit_type(context, RAMF_AMF_UNDEFINED_TYPE);
  }
  
  else if (value == Qnil)
  { emit_type(context, RAMF_AMF_NULL_TYPE);
  }
}

inline void emit_type(ramf_dump_context_t* context, RAMF_AMF_TYPE type)
{
  emitter_write_byte(emitter, type);
}

inline void emit_ruby_integer(ramf_dump_context_t* context, VALUE integer)
{
  uint64_t cint = NUM2ULONG(integer);
  if (1073741824 > cint) {
    emit_type(context, RAMF_AMF_INTEGER_TYPE);
    emit_u29(context, (uint32_t)cint);
  } else {
    emit_type(context, RAMF_AMF_DOUBLE_TYPE);
    emit_double(context, (double)cint);
  }
}

inline void emit_ruby_float(ramf_dump_context_t* context, VALUE flt)
{
  emit_double(context, NUM2DBL(flt));
}

inline void emit_ruby_string(ramf_dump_context_t* context, VALUE string)
{
  int cache_id = cached_string(context, string);
  if (cache_id < 0) {
    long length = 0;
    u_char* cstr = rb_str2cstr(string, &length);
    emit_cstring(context, cstr, length);
  } else {
    emit_cstring_ref(context, cache_id);
  }
}

inline void emit_ruby_time(ramf_dump_context_t* context, VALUE time)
{
  int cache_id = cached_object(context, time);
  if (cache_id < 0) {
    uint64_t millisec = NUM2DBL(rb_funcall(time, rb_intern("to_f"), 0)) * 1000.0;
    emit_cdate(context, millisec);
  } else {
    emit_cdate_ref(context, cache_id);
  }
}

inline void emit_ruby_array(ramf_dump_context_t* context, VALUE array)
{
  int cache_id = cached_object(context, array);
  if (cache_id < 0) {
    long i=0, c=RARRAY(array)->len;
    
    emit_u29(context, (c << 1) | 0x01);
    emit_cstring(context, "", 0);
    for (;i<c;i++) {
      emit_value(context, rb_ary_entry(array, i));
    }
  } else {
    emit_u29(context, (cache_id << 1) & 0xfffffffe);
  }
}

int emit_ruby_hash_iterator(VALUE key, VALUE value, VALUE extra) {
  ramf_dump_context_t* context = (ramf_dump_context_t*)extra;
  key = rb_String(key);
  long key_length = 0;
  u_char* key_cstr = rb_str2cstr(key, &key_length);
  emit_cstring(context, key_cstr, key_length);
  emit_value(context, value);
  return 0;
}

inline void emit_ruby_hash(ramf_dump_context_t* context, VALUE hash)
{
  int cache_id = cached_object(context, hash);
  if (cache_id < 0) {
    emit_u29(context, 0x01);
    rb_hash_foreach(hash, emit_ruby_hash_iterator, (VALUE)context);
    emit_cstring(context, "", 0);
  } else {
    emit_u29(context, (cache_id << 1) & 0xfffffffe);
  }
}

/*
  object-marker
  ( U29O-ref
  | ( U29O-traits-ext class-name *(U8) )
  | U29O-traits-ref
  | ( U29O-traits class-name *(UTF-8-vr) )
  )
  *(value-type)
  *(dynamic-member) */

inline void emit_u29(ramf_dump_context_t* context, uint32_t v)
{
  u_char buffer[4];
  size_t len = 0;
  
  if (v < 128) {
    buffer[0] = ((v >>  0) & 0x7f) | 0x00;
    len = 1;
  } else if (v < 16384) {
    buffer[1] = ((v >>  0) & 0x7f) | 0x00;
    buffer[0] = ((v >>  7) & 0x7f) | 0x80;
    len = 2;
  } else if (v < 2097152) {
    buffer[2] = ((v >>  0) & 0x7f) | 0x00;
    buffer[1] = ((v >>  7) & 0x7f) | 0x80;
    buffer[0] = ((v >> 14) & 0x7f) | 0x80;
    len = 3;
  } else if (v < 1073741824) {
    buffer[3] = ((v >>  0));
    buffer[2] = ((v >>  8) & 0x7f) | 0x80;
    buffer[1] = ((v >> 15) & 0x7f) | 0x80;
    buffer[0] = ((v >> 22) & 0x7f) | 0x80;
    len = 4;
  }
  
  emitter_write_bytes(emitter, buffer, len);
}

inline void emit_double(ramf_dump_context_t* context, double num)
{
  union aligned {
    double dval;
    char cval[8];
  } d;
  
  const char * number = d.cval;
  d.dval = num;
  u_char buffer[8];
  
  if(1) // bigendian
  {
    buffer[0] = number[7];
    buffer[1] = number[6];
    buffer[2] = number[5];
    buffer[3] = number[4];
    buffer[4] = number[3];
    buffer[5] = number[2];
    buffer[6] = number[1];
    buffer[7] = number[0];
  } else {
    buffer[0] = number[0];
    buffer[1] = number[1];
    buffer[2] = number[2];
    buffer[3] = number[3];
    buffer[4] = number[4];
    buffer[5] = number[5];
    buffer[6] = number[6];
    buffer[7] = number[7];
  }
  
  emitter_write_bytes(emitter, buffer, 8);
}

inline void emit_cstring(ramf_dump_context_t* context, u_char* string, size_t len)
{
  emit_u29(context, (len << 1) | 0x00000001);
  emitter_write_bytes(emitter, string, len);
}

inline void emit_cstring_ref(ramf_dump_context_t* context, int ref)
{
  emit_u29(context, (ref << 1) & 0xfffffffe);
}

inline void emit_cdate(ramf_dump_context_t* context, uint64_t millisec)
{
  emit_u29(context, 0x01);
  emit_double(context, (double)millisec);
}

inline void emit_cdate_ref(ramf_dump_context_t* context, int ref)
{
  emit_u29(context, (ref << 1) & 0xfffffffe);
}


VALUE ramf_load_amf3(VALUE self, VALUE io_or_string)
{
  return Qundef;
}