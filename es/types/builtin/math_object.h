#ifndef ES_TYPES_BUILTIN_MATH_OBJECT
#define ES_TYPES_BUILTIN_MATH_OBJECT

#include <es/types/object.h>

namespace es {

double ToNumber(Error* e, JSValue* input);

class Math : public JSObject {
 public:
  static Math* Instance() {
  static Math* singleton = Math::New();
    return singleton;
  }

  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    return Construct(e, arguments);
  }

  JSObject* Construct(Error* e, std::vector<JSValue*> arguments) override {
    assert(false);
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    return String::New(u"Math");
  }

  static JSValue* max(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    if (vals.size() == 0)
      return Number::NegativeInfinity();
    double value1 = ToNumber(e, vals[0]);
    if (!e->IsOk()) return nullptr;
    double value2;
    if (vals.size() < 2) {
      value2 = ToNumber(e, vals[1]);
      if (!e->IsOk()) return nullptr;
    }
    if (isnan(value1) || isnan(value2))
      return Number::NaN();
    if (value1 == 0 && value2 == 0 && (!signbit(value1) || !signbit(value2)))
      return Number::Zero();
    else if (value1 >= value2)
      return Number::New(value1);
    else
      return Number::New(value2);
  }

 private:
  static Math* New() {
    JSObject* jsobj = JSObject::New(
      OBJ_FUNC, u"Math", true, nullptr, true, true, nullptr, 0);
    return new (jsobj) Math();
  }
};

void InitMath() {
  JSObject* math = Math::Instance();
  math->AddFuncProperty(u"max", Math::max, false, false, false);
}

}  // namespace es

#endif  // ES_TYPES_BUILTIN_MATH_OBJECT