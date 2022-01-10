#ifndef ES_TYPES_BUILTIN_OBJECT_PROTO
#define ES_TYPES_BUILTIN_OBJECT_PROTO

#include <es/types/object.h>

namespace es {

class ObjectProto : public JSObject {
 public:
  ObjectProto() :
    JSObject(
      OBJ_OTHER,
      Null::Instance(),
      u"Object",
      true,  // extensible
      nullptr,
      false,
      false
    ) {}
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_OBJECT_PROTO