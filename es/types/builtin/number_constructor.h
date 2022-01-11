#ifndef ES_TYPES_BUILTIN_NUMBER_CONSTRUCTOR
#define ES_TYPES_BUILTIN_NUMBER_CONSTRUCTOR

#include <es/types/base.h>
#include <es/types/builtin/function_proto.h>
#include <es/types/builtin/number_object.h>
#include <es/types/conversion.h>

#include <es/error.h>

namespace es {

class NumberConstructor : public JSObject {
  NumberConstructor() :
    JSObject(
      OBJ_OTHER,
      "Number",
      false,
      nullptr,
      true,
      true,
    ) {}

  JSObject* Construct(std::vector<JSValue*> arguments) override {
    assert(arguments.size() == 0 || arguments.size() == 1);
    return new NumberObject(arguments.size() == 0 ? Number::Zero(), ToNumber(arguments[0]));
  }

  JSNumber* Call(Error* e, this, std::vector<JSValue*> arguments) override {
    assert(arguments.size() == 1);
    return ToNumber(arguments[0]);
  }
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_NUMBER_CONSTRUCTOR