#ifndef ES_TYPES_BUILTIN_NUMBER_OBJECT
#define ES_TYPES_BUILTIN_NUMBER_OBJECT

#include <es/types/object.h>
#include <es/types/builtin/number_proto.h>

namespace es {

class NumberObject : public JSObject {
 public:
  NumberObject(JSValue* primitive_value) :
    JSObject(
      OBJ_NUMBER,
      u"Number",
      true,  // extensible
      primitive_value,
      false,
      false
    ) {}
};

}

#endif  // ES_TYPES_BUILTIN_NUMBER_OBJECT