#ifndef ES_TYPES_TYPE_H
#define ES_TYPES_TYPE_H

namespace es {

constexpr uint64_t STACK_MASK = 7;
constexpr uint64_t STACK_SHIFT = 3;

enum Type : uint16_t {
  JS_UNINIT = 0,
  JS_UNDEFINED = 1,
  JS_NULL = 2,
  JS_BOOL = 3,
  JS_NUMBER = 4,
  JS_REF = 5,
  JS_STRING = 6,
  JS_LONG_STRING = 7,

  JS_OBJECT = 1 << 6,

  OBJ_GLOBAL    = 1 << 6 | 1,
  OBJ_OBJECT    = 1 << 6 | 2,
  OBJ_FUNC      = 1 << 6 | 3,
  OBJ_BIND_FUNC = 1 << 6 | 4,
  OBJ_ARRAY     = 1 << 6 | 5,
  OBJ_STRING    = 1 << 6 | 6,
  OBJ_BOOL      = 1 << 6 | 7,
  OBJ_NUMBER    = 1 << 6 | 8,
  OBJ_MATH      = 1 << 6 | 9,
  OBJ_DATE      = 1 << 6 | 10,
  OBJ_REGEXP    = 1 << 6 | 11,
  OBJ_JSON      = 1 << 6 | 12,
  OBJ_ERROR     = 1 << 6 | 13,

  OBJ_FUNC_PROTO = 1 << 6 | 14,

  OBJ_ARRAY_CONSTRUCTOR  = 1 << 6 | 15,
  OBJ_BOOL_CONSTRUCTOR   = 1 << 6 | 16,
  OBJ_DATE_CONSTRUCTOR   = 1 << 6 | 17,
  OBJ_ERROR_CONSTRUCTOR  = 1 << 6 | 18,
  OBJ_FUNC_CONSTRUCTOR   = 1 << 6 | 19,
  OBJ_NUMBER_CONSTRUCTOR = 1 << 6 | 20,
  OBJ_OBJECT_CONSTRUCTOR = 1 << 6 | 21,
  OBJ_REGEXP_CONSTRUCTOR = 1 << 6 | 22,
  OBJ_STRING_CONSTRUCTOR = 1 << 6 | 23,

  OBJ_ARGUMENTS = 1 << 6 | 24,

  OBJ_INNER_FUNC = 1 << 6 | 25,
  OBJ_HOST       = 1 << 6 | 26,
  OBJ_OTHER      = 1 << 6 | 27,

  SPEC_TYPE = 1 << 8,

  JS_PROP_DESC    = 1 << 8 | 2,
  JS_ENV_REC_DECL = 1 << 8 | 3,
  JS_ENV_REC_OBJ  = 1 << 8 | 4,
  JS_GET_SET      = 1 << 8 | 5,

  NON_JSVALUE  = 1 << 10,

  ERROR       = 1 << 10 | 1,
  FIXED_ARRAY = 1 << 10 | 2,
  HASHMAP     = 1 << 10 | 3,
  HASHMAP_V2  = 1 << 10 | 4,
  LIST_NODE   = 1 << 10 | 5,
  PROPERTY_MAP = 1 << 10 | 6,
};

enum ClassType : uint8_t {
  CLASS_OBJECT = 1,
  CLASS_ARRAY = 2,
  CLASS_NUMBER = 3,
  CLASS_STRING = 4,
  CLASS_FUNCTION = 5,
  CLASS_ARGUMENTS = 6,
  CLASS_BOOL = 7,
  CLASS_DATE = 8,
  CLASS_ERROR = 9,
  CLASS_INTERNAL_FUNCTION = 10,
  CLASS_GLOBAL = 11,
  CLASS_MATH = 12,
  CLASS_REGEXP = 13,
  CLASS_CONSOLE = 14,
  NUM_CLASS,
};

static_assert(NUM_CLASS <= (1 << 4));

std::u16string ClassToString(ClassType t) {
  switch (t) {
    case CLASS_OBJECT:
      return u"Object";
    case CLASS_ARRAY:
      return u"Array";
    case CLASS_NUMBER:
      return u"Number";
    case CLASS_STRING:
      return u"String";
    case CLASS_FUNCTION:
      return u"Function";
    case CLASS_ARGUMENTS:
      return u"Arguments";
    case CLASS_BOOL:
      return u"Boolean";
    case CLASS_DATE:
      return u"Date";
    case CLASS_ERROR:
      return u"Error";
    case CLASS_INTERNAL_FUNCTION:
      return u"InternalFunc";
    case CLASS_GLOBAL:
      return u"Global";
    case CLASS_MATH:
      return u"Math";
    case CLASS_REGEXP:
      return u"RegExp";
    case CLASS_CONSOLE:
      return u"Console";
    default:
      assert(false);
  }
}

}  // namespace es
#endif
