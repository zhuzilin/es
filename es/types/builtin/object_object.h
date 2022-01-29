#ifndef ES_TYPES_BUILTIN_OBJECT_OBJECT
#define ES_TYPES_BUILTIN_OBJECT_OBJECT

#include <es/types/object.h>
#include <es/runtime.h>

namespace es {

Handle<String> ToString(Error* e, Handle<JSValue> input);
Handle<PropertyDescriptor> ToPropertyDescriptor(Error* e, Handle<JSValue> obj);

class ObjectProto : public JSObject {
 public:
  static Handle<ObjectProto> Instance() {
    static Handle<ObjectProto> singleton = ObjectProto::New(GCFlag::SINGLE);
    return singleton;
  }

  static Handle<JSValue> toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    if (val.val()->IsUndefined())
      return String::New(u"[object Undefined]");
    if (val.val()->IsNull())
      return String::New(u"[object Null]");
    Handle<JSObject> obj = ToObject(e, val);
    return String::New(u"[object " + obj.val()->Class() + u"]");
  }

  static Handle<JSValue> toLocaleString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> valueOf(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    Handle<JSObject> O = ToObject(e, val);
    if (!e->IsOk()) return Handle<JSValue>();
    // TODO(zhuzilin) Host object
    return O;
  }

  static Handle<JSValue> hasOwnProperty(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> isPrototypeOf(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> propertyIsEnumerable(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

 private:
  static Handle<ObjectProto> New(flag_t flag) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_OBJECT, u"Object", true, Handle<JSValue>(), false, false, nullptr, 0, flag);
    return Handle<ObjectProto>(new (jsobj.val()) ObjectProto());
  }
};

class Object : public JSObject {
 public:
  static Handle<Object> New() {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_OBJECT, u"Object", true, Handle<JSValue>(), false, false, nullptr, 0
    );
    Handle<Object> obj = Handle<Object>(new (jsobj.val()) Object());
    obj.val()->SetPrototype(ObjectProto::Instance());
    return obj;
  }
};

class ObjectConstructor : public JSObject {
 public:
  static Handle<ObjectConstructor> Instance() {
    static Handle<ObjectConstructor> singleton = ObjectConstructor::New(GCFlag::SINGLE);
    return singleton;
  }

  // 15.2.3.2 Object.getPrototypeOf ( O )
  static Handle<JSValue> getPrototypeOf(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() < 1 || !vals[0].val()->IsObject()) {
      *e = *Error::TypeError();
      return Handle<JSValue>();
    }
    return static_cast<Handle<JSObject>>(vals[0]).val()->Prototype();
  }

  // 15.2.3.3 Object.getOwnPropertyDescriptor ( O, P )
  static Handle<JSValue> getOwnPropertyDescriptor(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> getOwnPropertyNames(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> create(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() < 1 || (!vals[0].val()->IsObject() && !vals[0].val()->IsNull())) {
      *e = *Error::TypeError(u"Object.create called on non-object");
      return Handle<JSValue>();
    }
    Handle<Object> obj = Object::New();
    obj.val()->SetPrototype(vals[0]);
    if (vals.size() > 1 && !vals[1].val()->IsUndefined()) {
      ObjectConstructor::defineProperties(e, this_arg, vals);
      if (!e->IsOk()) return Handle<JSValue>();
    }
    return obj;
  }

  static Handle<JSValue> defineProperty(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() < 1 || !vals[0].val()->IsObject()) {
      *e = *Error::TypeError(u"Object.defineProperty called on non-object");
      return Handle<JSValue>();
    }
    Handle<JSObject> O = static_cast<Handle<JSObject>>(vals[0]);
    if (vals.size() < 2) {
      *e = *Error::TypeError(u"Object.defineProperty need 3 arguments");
      return Handle<JSValue>();
    }
    Handle<String> name = ::es::ToString(e, vals[1]);
    if (!e->IsOk()) return Handle<JSValue>();
    Handle<PropertyDescriptor> desc = ToPropertyDescriptor(e, vals[2]);
    if (!e->IsOk()) return Handle<JSValue>();
    DefineOwnProperty(e, O, name, desc, true);
    return O;
  }

  static Handle<JSValue> defineProperties(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> seal(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> freeze(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> preventExtensions(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() == 0 || !vals[0].val()->IsObject()) {
      *e = *Error::TypeError(u"Object.preventExtensions called on non-object");
      return Handle<JSValue>();
    }
    Handle<JSObject> obj = static_cast<Handle<JSObject>>(vals[0]);
    obj.val()->SetExtensible(false);
    return obj;
  }

  static Handle<JSValue> isSealed(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> isFrozen(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> isExtensible(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() < 1 || !vals[0].val()->IsObject()) {
      *e = *Error::TypeError(u"Object.isExtensible called on non-object");
      return Handle<JSValue>();
    }
    Handle<JSObject> obj = static_cast<Handle<JSObject>>(vals[0]);
    return Bool::Wrap(obj.val()->Extensible());
  }

  static Handle<JSValue> keys(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  // ES6
  static Handle<JSValue> setPrototypeOf(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() < 2) {
      *e = *Error::TypeError(u"Object.preventExtensions need 2 arguments");
      return Handle<JSValue>(); 
    }
    vals[0].val()->CheckObjectCoercible(e);
    if (!e->IsOk()) return Handle<JSValue>();
    if (!(vals[1].val()->IsNull() || vals[1].val()->IsObject())) {
      *e = *Error::TypeError(u"");
      return Handle<JSValue>();
    }
    if (!vals[0].val()->IsObject()) {
      return vals[0];
    }
    static_cast<Handle<JSObject>>(vals[0]).val()->SetPrototype(vals[1]);
    return vals[0];
  }

  static Handle<JSValue> toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"function Object() { [native code] }");
  }

 private:
  static Handle<ObjectConstructor> New(flag_t flag) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_OBJECT_CONSTRUCTOR, u"Object", true, Handle<JSValue>(), true, true, nullptr, 0, flag);
    return Handle<ObjectConstructor>(new (jsobj.val()) ObjectConstructor());
  }
};

Handle<JSValue> Call__ObjectConstructor(Error* e, Handle<ObjectConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {});
Handle<JSObject> Construct__ObjectConstructor(Error* e, Handle<ObjectConstructor> O, std::vector<Handle<JSValue>> arguments);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_OBJECT_OBJECT