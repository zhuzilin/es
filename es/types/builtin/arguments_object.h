#ifndef ES_TYPES_BUILTIN_ARGUMENTS_OBJECT
#define ES_TYPES_BUILTIN_ARGUMENTS_OBJECT

#include <iostream>

#include <es/types/object.h>
#include <es/types/builtin/function_object.h>
#include <es/utils/helper.h>

namespace es {

// 10.6 Arguments Object
class ArgumentsObject : public JSObject {
 public:
  static ArgumentsObject* New(JSObject* parameter_map, size_t len) {
    JSObject* jsobj = JSObject::New(
      OBJ_OBJECT, u"Arguments", true, nullptr, false, false, nullptr,
      kParameterMapOffset + kPtrSize - kJSObjectOffset
    );
    SET_VALUE(jsobj, kParameterMapOffset, parameter_map, JSObject*);
    ArgumentsObject* obj = new (jsobj) ArgumentsObject();
    obj->SetPrototype(ObjectProto::Instance());
    obj->AddValueProperty(String::Length(), Number::New(len), true, false, true);
    return obj;
  }

  JSObject* ParameterMap() { return READ_VALUE(this, kParameterMapOffset, JSObject*); }

  JSValue* Get(Error* e, String* P) override {
    JSObject* map = ParameterMap();
    JSValue* is_mapped = map->GetOwnProperty(P);
    if (is_mapped->IsUndefined()) {  // 3
      JSValue* v = JSObject::Get(e, P);
      if (!e->IsOk()) return nullptr;
      if (P->data() == u"caller") {
        if (v->IsObject()) {
          JSObject* obj = static_cast<JSObject*>(v);
          if (obj->IsFunction()) {
            FunctionObject* func = static_cast<FunctionObject*>(obj);
            if (func->strict()) {
              *e = *Error::TypeError(u"caller could not be function object");
            }
          }
        }
      }
      return v;
    }
    // 4
    return map->Get(e, P);
  }

  JSValue* GetOwnProperty(String* P) override {
    JSValue* val = JSObject::GetOwnProperty(P);
    if (val->IsUndefined())
      return val;
    PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(val);
    JSObject* map = ParameterMap();
    JSValue* is_mapped = map->GetOwnProperty(P);
    if (!is_mapped->IsUndefined()) {  // 5
      desc->SetValue(map->Get(nullptr, P));
    }
    return desc;
  }

  bool DefineOwnProperty(Error* e, String* P, PropertyDescriptor* desc, bool throw_flag) override {
    JSObject* map = ParameterMap();
    JSValue* is_mapped = map->GetOwnProperty(P);
    bool allowed = JSObject::DefineOwnProperty(e, P, desc, false);
    if (!allowed) {
      if (throw_flag) {
        *e = *Error::TypeError(u"DefineOwnProperty " + P->data() + u" failed");
      }
      return false;
    }
    if (!is_mapped->IsUndefined()) {  // 5
      if (desc->IsAccessorDescriptor()) {
        map->Delete(e, P, false);
      } else {
        if (desc->HasValue()) {
          map->Put(e, P, desc->Value(), false);
        }
        if (desc->HasWritable() && !desc->Writable()) {
          map->Delete(e, P, false);
        }
      }
    }
    return true;
  }

  bool Delete(Error* e, String* P, bool throw_flag) override {
    JSObject* map = ParameterMap();
    JSValue* is_mapped = map->GetOwnProperty(P);
    bool result = JSObject::Delete(e, P, throw_flag);
    if (!e->IsOk()) return false;
    if (result && !is_mapped->IsUndefined()) {
      map->Delete(e, P, false);
    }
    return result;
  }

  inline std::string ToString() override { return "ArgumentsObject"; }

 private:
  static constexpr size_t kParameterMapOffset = kJSObjectOffset;
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_ARGUMENTS_OBJECT