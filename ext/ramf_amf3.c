
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
inline void emit_ruby_object(ramf_dump_context_t* context, VALUE object);

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
  
  else if (1)
  { emit_type(context, RAMF_AMF_OBJECT_TYPE);
    emit_ruby_object(context, value);
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
    u_char* cstr = (u_char*)rb_str2cstr(string, &length);
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
    emit_cstring(context, 0, 0);
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
  u_char* key_cstr = (u_char*)rb_str2cstr(key, &key_length);
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
    emit_cstring(context, 0, 0);
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

inline void emit_ruby_object(ramf_dump_context_t* context, VALUE object)
{
  int cache_id = cached_object(context, object);
  if (cache_id < 0) {
    
    // < external?
    
    // < traits ref
    
    long i=0, c=0;
    VALUE attr, key;
    char *attr_name;
    
    long cstr_len = 0;
    u_char* cstr  = 0;
    
    VALUE class   = rb_funcall(object, rb_intern("class"), 0);
    VALUE traits  = rb_funcall(class,  rb_intern("amf_traits"), 0);
    VALUE dynamic = Qnil;
    VALUE amf_cls = Qnil;
    VALUE sealed_attrs   = Qnil;
    VALUE dynamic_attrs  = Qnil;
    VALUE ignored_attrs  = Qnil;
    uint32_t sealed_len  = 0;
    uint32_t dynamic_len = 0;
    uint32_t ignored_len = 0;
    
    if (RTEST(traits)) {
      dynamic       = rb_hash_aref(traits, ID2SYM(rb_intern("dynamic")));
      
      sealed_attrs  = rb_hash_aref(traits, ID2SYM(rb_intern("static")));
      ignored_attrs = rb_hash_aref(traits, ID2SYM(rb_intern("ignore")));
      
      if (RTEST(sealed_attrs))
        sealed_len = RARRAY(sealed_attrs)->len;
      if (RTEST(ignored_attrs))
        ignored_len = RARRAY(ignored_attrs)->len;
      
      amf_cls = rb_hash_aref(traits, ID2SYM(rb_intern("class")));
    }
    
    if (dynamic == Qtrue || dynamic == Qnil) {
      dynamic_attrs = rb_obj_instance_variables(object);
      
      for (i=0;i<ignored_len;i++) {
        key       = rb_String(rb_ary_entry(ignored_attrs, i));
        attr_name = StringValueCStr(key);
        cstr_len  = RSTRING(key)->len;
        if (attr_name[0] != '@') {
          key = rb_str_new2("@");
          rb_str_cat(key, attr_name, cstr_len);
        }
        rb_ary_delete(dynamic_attrs, key);
      }
      
      for (i=0;i<sealed_len;i++) {
        key       = rb_String(rb_ary_entry(sealed_attrs, i));
        attr_name = StringValueCStr(key);
        cstr_len  = RSTRING(key)->len;
        if (attr_name[0] != '@') {
          key = rb_str_new2("@");
          rb_str_cat(key, attr_name, cstr_len);
        }
        rb_ary_delete(dynamic_attrs, key);
      }
      
      if (dynamic == Qnil && (RARRAY(dynamic_attrs)->len == 0)) {
        dynamic = Qfalse;
      } else {
        dynamic = Qtrue;
      }
    }
    
    if (!RTEST(amf_cls)) { amf_cls = class; }
    
    c = sealed_len;
    
    cache_id = cached_trait(context, class);
    if (cache_id < 0) {
      emit_u29(context, ((sealed_len) << 4) | (RTEST(dynamic) ? 0x0000000f : 0x00000007));
      
      cstr = (u_char*)rb_str2cstr(rb_String(amf_cls), &cstr_len);
      emit_cstring(context, cstr, cstr_len);
      
      for (i=0;i<c;i++) {
        key       = rb_String(rb_ary_entry(sealed_attrs, i));
        attr_name = StringValueCStr(key);
        cstr_len  = RSTRING(key)->len;
        if (attr_name[0] == '@') {
          attr_name++;
          emit_cstring(context, (u_char*)attr_name, cstr_len-1);
        } else {
          emit_cstring(context, (u_char*)attr_name, cstr_len);
        }
      }
    } else {
      emit_u29(context, ((cache_id << 2) & (0xffffffff << 2)) | 0x00000001);
    }
    
    for (i=0;i<c;i++) {
      key       = rb_String(rb_ary_entry(sealed_attrs, i));
      attr_name = StringValueCStr(key);
      if (attr_name[0] == '@') {
        attr = rb_iv_get(object, attr_name);
      } else {
        attr = rb_funcall(object, rb_intern(attr_name), 0);
      }
      emit_value(context, attr);
    }
    
    if (dynamic == Qtrue) {
      c = RARRAY(dynamic_attrs)->len;
      for (i=0;i<c;i++) {
        key = rb_String(rb_ary_entry(dynamic_attrs, i));
        attr_name = StringValueCStr(key);
        attr = rb_iv_get(object, attr_name);
        
        attr_name++;
        emit_cstring(context, (u_char*)attr_name, RSTRING(key)->len - 1);
        
        emit_value(context, attr);
      }
      
      emit_cstring(context, 0, 0);
    }
    
    
  } else {
    emit_u29(context, (cache_id << 1) & 0xfffffffe);
  }
}

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
  
  if(BIG_ENDIAN){
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



// ================================
// = Load AMF3 from a Ruby String =
// ================================

typedef struct {
  u_char * buffer;
  u_char * cursor;
  u_char * buffer_end;
  
  VALUE strings;
  int string_id;
  VALUE objects;
  int object_id;
  VALUE traits;
  int trait_id;
} ramf_load_context_t;

inline VALUE load_amf3_value(ramf_load_context_t* context);
inline VALUE load_amf3_integer(ramf_load_context_t* context);
inline VALUE load_amf3_double(ramf_load_context_t* context);

inline int is_eof(ramf_load_context_t* context)
{
  return (context->buffer_end == context->cursor);
}

inline u_char read_byte(ramf_load_context_t* context)
{
  if (is_eof(context)) return 0;
  u_char c = context->cursor[0];
  context->cursor++;
  return c;
}

inline u_char* read_bytes(ramf_load_context_t* context, size_t len)
{
  if ((context->cursor + 8) > context->buffer_end) return 0;
  u_char* buffer = context->cursor;
  context->cursor += 8;
  return buffer;
}

VALUE ramf_load_amf3(VALUE self, VALUE string)
{
  ramf_load_context_t context;
  context.buffer     = (u_char*)RSTRING(string)->ptr;
  context.cursor     = context.buffer;
  context.buffer_end = context.buffer + RSTRING(string)->len;
  
  VALUE result = load_amf3_value(&context);
  
  return result;
}

inline VALUE load_amf3_value(ramf_load_context_t* context)
{
  if (is_eof(context)) { return Qnil; }
  switch (read_byte(context)) {
    case RAMF_AMF_UNDEFINED_TYPE:  return Qnil;
    case RAMF_AMF_NULL_TYPE:       return Qnil;
    case RAMF_AMF_FALSE_TYPE:      return Qfalse;
    case RAMF_AMF_TRUE_TYPE:       return Qtrue;
    case RAMF_AMF_INTEGER_TYPE:    return load_amf3_integer(context);
    case RAMF_AMF_DOUBLE_TYPE:     return load_amf3_double(context);
    case RAMF_AMF_STRING_TYPE:     return Qnil;
    case RAMF_AMF_XML_DOC_TYPE:    return Qnil;
    case RAMF_AMF_DATE_TYPE:       return Qnil;
    case RAMF_AMF_ARRAY_TYPE:      return Qnil;
    case RAMF_AMF_OBJECT_TYPE:     return Qnil;
    case RAMF_AMF_XML_TYPE:        return Qnil;
    case RAMF_AMF_BYTE_ARRAY_TYPE: return Qnil;
  }
}

inline VALUE load_amf3_integer(ramf_load_context_t* context)
{
  uint32_t  i = 0;
  u_char    c = 0;
  char      b = 0;
  
  for (b=0; b<4; b++) {
    if (is_eof(context))
      return Qnil;
    c = read_byte(context);
    
    if (b < 3) {
      i = i << 7;
      i = i  | (c & 0x7F);
      if (!(c & 0x80)) break;
    } else {
      i = i << 8;
      i = i  | c;
    }
  }
  
  return UINT2NUM(i);
}

inline VALUE load_amf3_double(ramf_load_context_t* context)
{
  union aligned {
    double dval;
    u_char cval[8];
  } d;
  
  const u_char * cp = read_bytes(context, 8);
  
  if (cp) {
    if (BIG_ENDIAN) {
      d.cval[0] = cp[7]; d.cval[1] = cp[6]; d.cval[2] = cp[5]; d.cval[3] = cp[4];
      d.cval[4] = cp[3]; d.cval[5] = cp[2]; d.cval[6] = cp[1]; d.cval[7] = cp[0];
    } else {
      MEMCPY(d.cval, cp, u_char, 8);
    }
    
    return rb_float_new(d.dval);
  }
  
  return Qnil;
}
