#ifndef ES_TYPES_CONVERSION_H
#define ES_TYPES_CONVERSION_H

#include <cmath>
#include <cfloat>

#include <es/types/base.h>
#include <es/types/object.h>
#include <es/error.h>

namespace es {

JSValue* ToPrimitive(JSValue* input, std::u16string_view preferred_type = u"", Error* e) {
  assert(input->IsLanguageType());
  if (input->IsPrimitive()) {
    return input;
  }
  JSObject* obj = static_cast<JSObject*>(input);
  return obj->DefaultValue(preferred_type, e);
}

Bool* ToBoolean(JSValue* input) {
  assert(input->IsLanguageType());
  switch (input->type()) {
    case JSValue::JS_UNDEFINED:
    case JSValue::JS_NULL:
      return Bool::False();
    case JSValue::JS_BOOL:
      return Bool::False();
    case JSValue::JS_NUMBER:
      Number* num = static_cast<Number*>(input);
      if (num->data() == 0 || num->data() == Number::NaN) {
        return Bool::False();
      }
      return Bool::True();
    case JSValue::JS_STRING:
      String* str = static_cast<String*>(input);
      return Bool::Wrap(str->data() != u"");
    case JSValue::JS_OBJECT:
      return Bool::True();
  }
}

Number* ToNumber(JSValue* input, Error* e) {
  assert(input->IsLanguageType());
  switch (input->type()) {
    case JSValue::JS_UNDEFINED:
      return new Number(Number::NaN);
    case JSValue::JS_NULL:
      return new Number(0);
    case JSValue::JS_BOOL:
      Bool* b = static_cast<Bool*>(input);
      return new Number(b->data() ? 1 : 0);
    case JSValue::JS_NUMBER:
      return static_cast<Number*>(input);
    case JSValue::JS_STRING:
      // TODO(zhuzilin)
      // ...
    case JSValue::JS_OBJECT:
      JSValue* prim_value = ToPrimitive(input, u"Number", e);
      return ToNumber(prim_value, e);
  }
}

Number* ToInteger(JSValue* input, Error* e) {
  Number* num = ToNumber(input, e);
  if (num->IsNaN()) {
    return Number::Zero();
  }
  if (num->IsInfinity() || std::fpclassify(num->data()) == 0) {
  }
}

Number* ToInt32(JSValue* input);
Number* ToUint32(JSValue* input);
Number* ToUint16(JSValue* input);
String* ToString(JSValue* input);
JSObject* ToObject(JSValue* input);

bool SameValue(JSValue* x, JSValue* y);


}  // namespace es

#endif  // ES_TYPES_CONVERSION_H