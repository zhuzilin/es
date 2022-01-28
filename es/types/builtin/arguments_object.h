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
  static Handle<ArgumentsObject> New(Handle<JSObject> parameter_map, size_t len) {
    std::cout << "ArgumentsObject::New" << std::endl;
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_OBJECT, u"Arguments", true, Handle<JSValue>(), false, false, nullptr,
      kParameterMapOffset + kPtrSize - kJSObjectOffset
    );
    SET_HANDLE_VALUE(jsobj.val(), kParameterMapOffset, parameter_map, JSObject);
    Handle<ArgumentsObject> obj(new (jsobj.val()) ArgumentsObject());
    obj.val()->SetPrototype(ObjectProto::Instance());
    obj.val()->AddValueProperty(String::Length(), Number::New(len), true, false, true);
    return obj;
  }

  std::vector<HeapObject**> Pointers() override {
    std::vector<HeapObject**> pointers = JSObject::Pointers();
    pointers.emplace_back(HEAP_PTR(kParameterMapOffset));
    return pointers;
  }

  Handle<JSObject> ParameterMap() { return READ_HANDLE_VALUE(this, kParameterMapOffset, JSObject); }

  Handle<JSValue> Get(Error* e, Handle<String> P) override {
    Handle<JSObject> map = ParameterMap();
    Handle<JSValue> is_mapped = map.val()->GetOwnProperty(P);
    if (is_mapped.val()->IsUndefined()) {  // 3
      Handle<JSValue> v = JSObject::Get(e, P);
      if (!e->IsOk()) return Handle<JSValue>();
      if (P.val()->data() == u"caller") {
        if (v.val()->IsObject()) {
          Handle<JSObject> obj = static_cast<Handle<JSObject>>(v);
          if (obj.val()->IsFunction()) {
            Handle<FunctionObject> func = static_cast<Handle<FunctionObject>>(obj);
            if (func.val()->strict()) {
              *e = *Error::TypeError(u"caller could not be function object");
            }
          }
        }
      }
      return v;
    }
    // 4
    return map.val()->Get(e, P);
  }

  Handle<JSValue> GetOwnProperty(Handle<String> P) override {
    Handle<JSValue> val = JSObject::GetOwnProperty(P);
    if (val.val()->IsUndefined())
      return val;
    Handle<PropertyDescriptor> desc = static_cast<Handle<PropertyDescriptor>>(val);
    Handle<JSObject> map = ParameterMap();
    Handle<JSValue> is_mapped = map.val()->GetOwnProperty(P);
    if (!is_mapped.val()->IsUndefined()) {  // 5
      desc.val()->SetValue(map.val()->Get(nullptr, P));
    }
    return desc;
  }

  bool DefineOwnProperty(Error* e, Handle<String> P, Handle<PropertyDescriptor> desc, bool throw_flag) override {
    Handle<JSObject> map = ParameterMap();
    Handle<JSValue> is_mapped = map.val()->GetOwnProperty(P);
    bool allowed = JSObject::DefineOwnProperty(e, P, desc, false);
    if (!allowed) {
      if (throw_flag) {
        *e = *Error::TypeError(u"DefineOwnProperty " + P.val()->data() + u" failed");
      }
      return false;
    }
    if (!is_mapped.val()->IsUndefined()) {  // 5
      if (desc.val()->IsAccessorDescriptor()) {
        map.val()->Delete(e, P, false);
      } else {
        if (desc.val()->HasValue()) {
          map.val()->Put(e, P, desc.val()->Value(), false);
        }
        if (desc.val()->HasWritable() && !desc.val()->Writable()) {
          map.val()->Delete(e, P, false);
        }
      }
    }
    return true;
  }

  bool Delete(Error* e, Handle<String> P, bool throw_flag) override {
    Handle<JSObject> map = ParameterMap();
    Handle<JSValue> is_mapped = map.val()->GetOwnProperty(P);
    bool result = JSObject::Delete(e, P, throw_flag);
    if (!e->IsOk()) return false;
    if (result && !is_mapped.val()->IsUndefined()) {
      map.val()->Delete(e, P, false);
    }
    return result;
  }

  inline std::string ToString() override { return "ArgumentsObject"; }

 private:
  static constexpr size_t kParameterMapOffset = kJSObjectOffset;
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_ARGUMENTS_OBJECT