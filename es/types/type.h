namespace {

constexpr uint64_t STACK_MASK = 7;
constexpr uint64_t STACK_SHIFT = 3;

enum Type {
  JS_UNINIT = 0,
  JS_UNDEFINED = 1,
  JS_NULL = 2,
  JS_BOOL = 3,
  JS_NUMBER = 4,
  JS_STRING = 5,
  JS_LONG_STRING = 6,

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

  JS_REF          = 1 << 8 | 1,
  JS_PROP_DESC    = 1 << 8 | 2,
  JS_ENV_REC_DECL = 1 << 8 | 3,
  JS_ENV_REC_OBJ  = 1 << 8 | 4,
  JS_LEX_ENV      = 1 << 8 | 5,

  NON_JSVALUE  = 1 << 16,

  ERROR       = 1 << 16 | 1,
  FIXED_ARRAY = 1 << 16 | 2,
  HASHMAP     = 1 << 16 | 3,
  BINDING     = 1 << 16 | 4,
  LIST_NODE   = 1 << 16 | 5,
  PROPERTY_MAP = 1 << 16 | 6,
};

}  // namespace es
