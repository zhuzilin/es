#ifndef ES_GC_HEAP_OBJECT_H
#define ES_GC_HEAP_OBJECT_H

#include <assert.h>
#include <stdlib.h>

#include <vector>

#include <es/utils/macros.h>
#include <es/gc/handle.h>

namespace es {

void* Allocate(size_t size, flag_t flag);

class HeapObject {
 public:
  enum Type {
    JS_UNINIT = 0,
    JS_UNDEFINED = 1,
    JS_NULL = 2,
    JS_BOOL = 3,
    JS_STRING = 4,
    JS_LONG_STRING = 5,
    JS_NUMBER = 6,

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

    JS_GET_SET      = 1 << 8 | 6,

    NON_JSVALUE  = 1 << 16,

    ERROR       = 1 << 16 | 1,
    FIXED_ARRAY = 1 << 16 | 2,
    HASHMAP     = 1 << 16 | 3,
    BINDING     = 1 << 16 | 4,
    LIST_NODE   = 1 << 16 | 5,
  };


  static Handle<HeapObject> New(size_t size, flag_t flag = 0) {
#ifdef GC_DEBUG
    if (unlikely(log::Debugger::On()))
      std::cout << "HeapObject::New " << size << " " << int(flag) << "\n";
#endif
    Handle<HeapObject> heap_obj(static_cast<HeapObject*>(Allocate(size + kIntSize, flag)));

    // type value should be init by each variable after their member elements
    // are initialized.
    heap_obj.val()->SetType(JS_UNINIT);
    return heap_obj;
  }

  inline Type type() { return READ_VALUE(this, kTypeOffset, Type); }
  inline void SetType(Type t) { SET_VALUE(this, kTypeOffset, t, Type); }

  inline bool IsJSValue() { return (type() & NON_JSVALUE) == 0; }
  inline bool IsLanguageType() { return !IsSpecificationType(); }
  inline bool IsSpecificationType() { return type() & SPEC_TYPE; }

  inline bool IsUndefined() { return type() == JS_UNDEFINED; }
  inline bool IsNull() { return type() == JS_NULL; }
  inline bool IsBool() { return type() == JS_BOOL; }
  inline bool IsString() { return type() == JS_STRING || type() == JS_LONG_STRING; }
  inline bool IsNumber() { return type() == JS_NUMBER; }
  inline bool IsPrimitive() { return !IsObject(); }

  inline bool IsObject() { return (type() & JS_OBJECT) != 0; }

  inline bool IsBoolObject() { return type() == OBJ_BOOL; }
  inline bool IsNumberObject() { return type() == OBJ_NUMBER; }
  inline bool IsArrayObject() { return type() == OBJ_ARRAY; }
  inline bool IsRegExpObject() { return type() == OBJ_REGEXP; }
  inline bool IsErrorObject() { return type() == OBJ_ERROR; }
  inline bool IsFunctionObject() { return type() == OBJ_FUNC || type() == OBJ_BIND_FUNC; }
  inline bool IsStringObject() { return type() == OBJ_STRING; }
  inline bool IsDateObject() { return type() == OBJ_DATE; }
  inline bool IsArgumentsObject() { return type() == OBJ_ARGUMENTS; }

  inline bool IsFunctionProto() { return type() == OBJ_FUNC_PROTO; }

  inline bool IsBoolConstructor() { return type() == OBJ_BOOL_CONSTRUCTOR; }
  inline bool IsNumberConstructor() { return type() == OBJ_NUMBER_CONSTRUCTOR; }
  inline bool IsObjectConstructor() { return type() == OBJ_OBJECT_CONSTRUCTOR; }
  inline bool IsRegExpConstructor() { return type() == OBJ_REGEXP_CONSTRUCTOR; }
  inline bool IsStringConstructor() { return type() == OBJ_STRING_CONSTRUCTOR; }

  inline bool IsReference() { return type() == JS_REF; }
  inline bool IsPropertyDescriptor() { return type() == JS_PROP_DESC; }
  inline bool IsEnvironmentRecord() { return type() == JS_ENV_REC_DECL || type() == JS_ENV_REC_OBJ; }
  inline bool IsLexicalEnvironment() { return type() == JS_LEX_ENV; }

  inline bool IsGetterSetter() { return type() == JS_GET_SET; }

  inline bool IsError() { return type() == ERROR; }

  inline bool IsPrototype() { return IsNull() || IsObject(); }

  void* operator new(size_t) = delete;
  void* operator new[](size_t) = delete;
  void operator delete(void*) = delete;
  void operator delete[](void*) = delete;
  void* operator new(size_t, void* ptr) = delete;

  static std::string ToString(HeapObject* heap_obj);
  static std::vector<HeapObject**> Pointers(HeapObject* heap_obj);

  static std::string ToString(Type type);

 public:
  static constexpr size_t kTypeOffset = 0;
  static constexpr size_t kHeapObjectOffset = kTypeOffset + kIntSize;
};

}  // namespace es

#endif  // ES_GC_HEAP_OBJECT_H