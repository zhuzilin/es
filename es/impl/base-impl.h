#ifndef ES_IMPL_BASE_IMPL_H
#define ES_IMPL_BASE_IMPL_H

#include <es/types/base.h>
#include <es/error.h>
#include <es/utils/macros.h>

namespace es {

void CheckObjectCoercible(Handle<Error>& e, Handle<JSValue> val) {
  if (val.val()->IsUndefined() || val.val()->IsNull()) {
    e = Error::TypeError(u"undefined or null is not coercible");
  }
}

bool JSValue::IsCallable() {
  return (IsObject() && READ_VALUE(this, JSObject::kIsCallableOffset, bool)) || IsGetterSetter();
}

bool JSValue::IsConstructor() {
  return IsObject() && READ_VALUE(this, JSObject::kIsConstructorOffset, bool);
}

std::string JSValue::ToString(JSValue* jsval) {
  switch (jsval->type()) {
    case JS_UNINIT:
      return "Unitialized HeapObject";
    case JS_UNDEFINED:
      return "Undefined";
    case JS_NULL:
      return "Null";
    case JS_BOOL:
      return static_cast<Bool*>(jsval)->data() ? "true" : "false";
    case JS_LONG_STRING:
    case JS_STRING:
      return log::ToString(static_cast<String*>(jsval)->data());
    case JS_NUMBER:
      return NumberToStdString(static_cast<Number*>(jsval)->data());
    case JS_REF: {
      String* name = READ_VALUE(jsval, Reference::kReferenceNameOffset, String*);
      return "ref(" + ToString(name) + ")";
    }
    case JS_PROP_DESC: {
      PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(jsval);
      std::string res = "PropertyDescriptor{";
      if (desc->HasValue()) res += "v: " + ToString(READ_VALUE(jsval, PropertyDescriptor::kValueOffset, JSValue*)) + ", ";
      if (desc->HasWritable()) res += "w: " + log::ToString(desc->Writable()) + ", ";
      if (desc->HasGet()) res += "get: " + ToString(READ_VALUE(jsval, PropertyDescriptor::kGetOffset, JSValue*)) + ", ";
      if (desc->HasSet()) res += "set: " + ToString(READ_VALUE(jsval, PropertyDescriptor::kSetOffset, JSValue*)) + ", ";
      if (desc->HasEnumerable()) res += "e: " + log::ToString(desc->Enumerable()) + ", ";
      if (desc->HasConfigurable()) res += "c: " + log::ToString(desc->Configurable());
      res += '}';
      return res;
    }
    case JS_ENV_REC_DECL:
      return "DeclarativeEnvRec(" + log::ToString(jsval) + ")";
    case JS_ENV_REC_OBJ:
      return "ObjectEnvRec(" + log::ToString(jsval) + ")";
    case JS_LEX_ENV:
      return "LexicalEnvironment";
    case JS_GET_SET:
      return "GetterSetter(" + ToString(READ_VALUE(jsval, GetterSetter::kReferenceOffset, Reference*)) + ")";
    case ERROR:
      return static_cast<Error*>(jsval)->IsOk() ?
        "ok" :
        ToString(READ_VALUE(jsval, Error::kValueOffset, JSValue*));
    case FIXED_ARRAY:
      return "FixedArray(" + std::to_string(READ_VALUE(jsval, FixedArray::kSizeOffset, size_t)) + ")";
    case HASHMAP:
      return "HashMap(" + std::to_string(READ_VALUE(jsval, HashMap::kSizeOffset, size_t)) + ")";
    case BINDING:
      return "Binding(" + ToString(READ_VALUE(jsval, Binding::kValueOffset, JSValue*)) + ")";
    case LIST_NODE:
      return "ListNode(" + ToString(READ_VALUE(jsval, ListNode::kKeyOffset, String*)) + ")";
    case OBJ_ARRAY: {
      size_t num = ToNumber(
        Error::Empty(),
        Get(Error::Empty(), Handle<JSObject>(static_cast<JSObject*>(jsval)), String::Length())
      );
      return "Array(" + std::to_string(num) + ")";
    }
    case OBJ_FUNC: {
      std::string result = "Function(";
      FixedArray* params = READ_VALUE(jsval, FunctionObject::kFormalParametersOffset, FixedArray*);
      if (params->size() > 0) {
        result += ToString(params->GetRaw(0));
        for (size_t i = 1; i < params->size(); i++) {
          result += "," + ToString(params->GetRaw(i));
        }
      }
      result += ")";
      return result;
    }
    case OBJ_BIND_FUNC: {
      return "BindFunctionObject";
    }
    case OBJ_REGEXP: {
      return "/" + ToString(READ_VALUE(jsval, RegExpObject::kPatternOffset, String*)) +
             "/" + ToString(READ_VALUE(jsval, RegExpObject::kFlagOffset, String*));
    }
    case OBJ_ERROR: {
      return ToString(READ_VALUE(jsval, ErrorObject::kErrorOffset, Error*));
    }
    default:
      if (jsval->IsObject()) {
        return ToString(READ_VALUE(jsval, JSObject::kClassOffset, String*));
      }
      assert(false);
  }
}

}  // namespace es

#endif  // ES_IMPL_BASE_IMPL_H