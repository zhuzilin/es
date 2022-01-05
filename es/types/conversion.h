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

JSBool* ToBoolean(JSValue* input) {
  assert(input->IsLanguageType());
  switch (input->type()) {
    case JSValue::JS_UNDEFINED:
    case JSValue::JS_NULL:
      return JSBool::False();
    case JSValue::JS_BOOL:
      return JSBool::False();
    case JSValue::JS_NUMBER:
      JSNumber* num = static_cast<JSNumber*>(input);
      if (num->data() == 0 || num->data() == JSNumber::NaN) {
        return JSBool::False();
      }
      return JSBool::True();
    case JSValue::JS_STRING:
      JSString* str = static_cast<JSString*>(input);
      return JSBool::Wrap(str->data() != u"");
    case JSValue::JS_OBJECT:
      return JSBool::True();
  }
}

JSNumber* ToNumber(JSValue* input, Error* e) {
  assert(input->IsLanguageType());
  switch (input->type()) {
    case JSValue::JS_UNDEFINED:
      return new JSNumber(JSNumber::NaN);
    case JSValue::JS_NULL:
      return new JSNumber(0);
    case JSValue::JS_BOOL:
      JSBool* b = static_cast<JSBool*>(input);
      return new JSNumber(b->data() ? 1 : 0);
    case JSValue::JS_NUMBER:
      return static_cast<JSNumber*>(input);
    case JSValue::JS_STRING:
      // TODO(zhuzilin)
      // ...
    case JSValue::JS_OBJECT:
      JSValue* prim_value = ToPrimitive(input, u"Number", e);
      return ToNumber(prim_value, e);
  }
}

JSNumber* ToInteger(JSValue* input, Error* e) {
  JSNumber* num = ToNumber(input, e);
  if (num->IsNaN()) {
    return JSNumber::Zero();
  }
  if (num->IsInfinity() || std::fpclassify(num->data()) == 0) {
  }
}

JSNumber* ToInt32(JSValue* input);
JSNumber* ToUint32(JSValue* input);
JSNumber* ToUint16(JSValue* input);
JSString* ToString(JSValue* input);
JSObject* ToObject(JSValue* input);

bool SameValue(JSValue* x, JSValue* y);


}  // namespace es

#endif  // ES_TYPES_CONVERSION_H