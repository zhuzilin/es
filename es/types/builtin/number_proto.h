#ifndef ES_TYPES_BUILTIN_NUMBER_PROTO
#define ES_TYPES_BUILTIN_NUMBER_PROTO

#include <es/types/object.h>
#include <es/types/builtin/object_proto.h>

namespace es {

class NumberProto : public JSObject {
 public:
  NumberProto() :
    JSObject(
      OBJ_OTHER,
      new ObjectProto(),
      u"Number",
      true,  // extensible
      Number::Zero(),
      false,
      false
    ) {}
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_NUMBER_PROTO