#ifndef ES_TYPES_BUILTIN_OBJECT_PROTO
#define ES_TYPES_BUILTIN_OBJECT_PROTO

#include <es/types/object.h>

namespace es {

class ObjectProto : public JSObject {

};

class ObjectConstructor : public JSObject {
  ObjectConstructor() :
    JSObject(
      ObjType obj_type,
      JSValue* prototype,
      std::u16string_view klass,
      bool extensible,
      JSValue* primitive_value,
      true,
      true,
    )
}

}

#endif  // ES_TYPES_BUILTIN_OBJECT_PROTO