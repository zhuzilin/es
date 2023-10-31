#ifndef ES_TYPES_BUILTIN_OBJECT_OBJECT
#define ES_TYPES_BUILTIN_OBJECT_OBJECT

#include <es/types/object.h>
#include <es/runtime.h>

namespace es {

Handle<String> ToString(Handle<Error>& e, Handle<JSValue> input);
Handle<JSValue> FromPropertyDescriptor(StackPropertyDescriptor value);
StackPropertyDescriptor ToPropertyDescriptor(Handle<Error>& e, Handle<JSValue> obj);

class ObjectProto : public JSObject {
 public:
  static Handle<ObjectProto> Instance() {
    static Handle<ObjectProto> singleton = ObjectProto::New<GCFlag::SINGLE>();
    return singleton;
  }

  static Handle<JSValue> toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    if (val.val()->IsUndefined())
      return String::New(u"[object Undefined]");
    if (val.val()->IsNull())
      return String::New(u"[object Null]");
    Handle<JSObject> obj = ToObject(e, val);
    return String::New(u"[object " + obj.val()->Class() + u"]");
  }

  static Handle<JSValue> toLocaleString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> valueOf(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    Handle<JSObject> O = ToObject(e, val);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    // TODO(zhuzilin) Host object
    return O;
  }

  static Handle<JSValue> hasOwnProperty(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<String> P;
    if (vals.size() == 0)
      P = ::es::ToString(e, Undefined::Instance());
    else
      P = ::es::ToString(e, vals[0]);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();

    Handle<JSValue> val = Runtime::TopValue();
    Handle<JSObject> O = ToObject(e, val);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();

    StackPropertyDescriptor desc = GetOwnProperty(O, P);
    return Bool::Wrap(!desc.IsUndefined());
  }

  static Handle<JSValue> isPrototypeOf(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (unlikely(vals.size() == 0 || !vals[0].val()->IsObject()))
      return Bool::False();
    Handle<JSValue> val = Runtime::TopValue();
    Handle<JSObject> O = ToObject(e, val);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    Handle<JSValue> V = static_cast<Handle<JSObject>>(vals[0]).val()->Prototype();
    while (!V.val()->IsNull()) {
      if (V.val() == O.val())
        return Bool::True();
      V = static_cast<Handle<JSObject>>(V).val()->Prototype();
    }
    return Bool::False();
  }

  static Handle<JSValue> propertyIsEnumerable(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

 private:
  template<flag_t flag>
  static Handle<ObjectProto> New() {
    Handle<JSObject> jsobj = JSObject::New<0, flag>(
      u"Object", true, Handle<JSValue>(), false, false, nullptr);

    jsobj.val()->SetType(OBJ_OBJECT);
    return Handle<ObjectProto>(jsobj);
  }
};

class Object : public JSObject {
 public:
  static Handle<Object> New(size_t num_properties = 0) {
    Handle<JSObject> jsobj = JSObject::New<0>(
      u"Object", true, Handle<JSValue>(), false, false, nullptr, 0, 0, num_properties
    );

    jsobj.val()->SetType(OBJ_OBJECT);
    Handle<Object> obj = Handle<Object>(jsobj);
    obj.val()->SetPrototype(ObjectProto::Instance());
    return obj;
  }
};

class ObjectConstructor : public JSObject {
 public:
  static Handle<ObjectConstructor> Instance() {
    static Handle<ObjectConstructor> singleton = ObjectConstructor::New<GCFlag::SINGLE>();
    return singleton;
  }

  // 15.2.3.2 Object.getPrototypeOf ( O )
  static Handle<JSValue> getPrototypeOf(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() < 1 || !vals[0].val()->IsObject()) {
      e = Error::TypeError();
      return Handle<JSValue>();
    }
    return static_cast<Handle<JSObject>>(vals[0]).val()->Prototype();
  }

  // 15.2.3.3 Object.getOwnPropertyDescriptor ( O, P )
  static Handle<JSValue> getOwnPropertyDescriptor(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() < 1 || !vals[0].val()->IsObject()) {
      e = Error::TypeError(u"Object.create called on non-object");
      return Handle<JSValue>();
    }
    Handle<String> name = vals.size() < 2 ?
      ::es::ToString(e, Undefined::Instance()) : ::es::ToString(e, vals[1]);
    StackPropertyDescriptor desc = GetOwnProperty(static_cast<Handle<JSObject>>(vals[0]), name);
    return FromPropertyDescriptor(desc);
  }

  static Handle<JSValue> getOwnPropertyNames(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> create(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() < 1 || (!vals[0].val()->IsObject() && !vals[0].val()->IsNull())) {
      e = Error::TypeError(u"Object.create called on non-object");
      return Handle<JSValue>();
    }
    Handle<Object> obj = Object::New();
    obj.val()->SetPrototype(vals[0]);
    if (vals.size() > 1 && !vals[1].val()->IsUndefined()) {
      ObjectConstructor::defineProperties(e, this_arg, vals);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    }
    return obj;
  }

  static Handle<JSValue> defineProperty(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() < 1 || !vals[0].val()->IsObject()) {
      e = Error::TypeError(u"Object.defineProperty called on non-object");
      return Handle<JSValue>();
    }
    Handle<JSObject> O = static_cast<Handle<JSObject>>(vals[0]);
    if (vals.size() < 2) {
      e = Error::TypeError(u"Object.defineProperty need 3 arguments");
      return Handle<JSValue>();
    }
    Handle<String> name = ::es::ToString(e, vals[1]);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    StackPropertyDescriptor desc = ToPropertyDescriptor(e, vals[2]);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    DefineOwnProperty(e, O, name, desc, true);
    return O;
  }

  static Handle<JSValue> defineProperties(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> seal(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> freeze(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> preventExtensions(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() == 0 || !vals[0].val()->IsObject()) {
      e = Error::TypeError(u"Object.preventExtensions called on non-object");
      return Handle<JSValue>();
    }
    Handle<JSObject> obj = static_cast<Handle<JSObject>>(vals[0]);
    obj.val()->SetExtensible(false);
    return obj;
  }

  static Handle<JSValue> isSealed(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> isFrozen(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> isExtensible(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() < 1 || !vals[0].val()->IsObject()) {
      e = Error::TypeError(u"Object.isExtensible called on non-object");
      return Handle<JSValue>();
    }
    Handle<JSObject> obj = static_cast<Handle<JSObject>>(vals[0]);
    return Bool::Wrap(obj.val()->Extensible());
  }

  static Handle<JSValue> keys(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  // ES6
  static Handle<JSValue> setPrototypeOf(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() < 2) {
      e = Error::TypeError(u"Object.preventExtensions need 2 arguments");
      return Handle<JSValue>(); 
    }
    CheckObjectCoercible(e, vals[0]);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    if (!(vals[1].val()->IsNull() || vals[1].val()->IsObject())) {
      e = Error::TypeError(u"");
      return Handle<JSValue>();
    }
    if (!vals[0].val()->IsObject()) {
      return vals[0];
    }
    static_cast<Handle<JSObject>>(vals[0]).val()->SetPrototype(vals[1]);
    return vals[0];
  }

  static Handle<JSValue> toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"function Object() { [native code] }");
  }

 private:
  template<flag_t flag>
  static Handle<ObjectConstructor> New() {
    Handle<JSObject> jsobj = JSObject::New<0, flag>(
      u"Object", true, Handle<JSValue>(), true, true, nullptr);

    jsobj.val()->SetType(OBJ_OBJECT_CONSTRUCTOR);
    return Handle<ObjectConstructor>(jsobj);
  }
};

Handle<JSValue> Call__ObjectConstructor(Handle<Error>& e, Handle<ObjectConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {});
Handle<JSObject> Construct__ObjectConstructor(Handle<Error>& e, Handle<ObjectConstructor> O, std::vector<Handle<JSValue>> arguments);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_OBJECT_OBJECT