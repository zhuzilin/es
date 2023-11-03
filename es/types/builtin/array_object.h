#ifndef ES_TYPES_BUILTIN_ARRAY_OBJECT
#define ES_TYPES_BUILTIN_ARRAY_OBJECT

#include <algorithm>

#include <es/types/object.h>
#include <es/types/builtin/object_object.h>
#include <es/utils/helper.h>

namespace es {

bool ToBoolean(Handle<JSValue> input);
double ToNumber(Handle<Error>& e, Handle<JSValue> input);
double ToInteger(Handle<Error>& e, Handle<JSValue> input);
double ToUint32(Handle<Error>& e, Handle<JSValue> input);
std::u16string ToU16String(Handle<Error>& e, Handle<JSValue> input);
std::u16string NumberToU16String(double m);
double StringToNumber(Handle<String> source);
Handle<JSObject> ToObject(Handle<Error>& e, Handle<JSValue> input);

class ArrayProto : public JSObject {
 public:
  static  Handle<ArrayProto> Instance() {
    static  Handle<ArrayProto> singleton = ArrayProto::New<GCFlag::SINGLE>();
    return singleton;
  }

  // 15.4.4.2 Array.prototype.toString ( )
  static Handle<JSValue> toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  static Handle<JSValue> toLocaleString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> concat(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  static Handle<JSValue> join(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  // 15.4.4.6 Array.prototype.pop ( )
  static Handle<JSValue> pop(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  // 15.4.4.7 Array.prototype.push ( [ item1 [ , item2 [ , … ] ] ] )
  static Handle<JSValue> push(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  static Handle<JSValue> reverse(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  // 15.4.4.9 Array.prototype.shift ( )
  static Handle<JSValue> shift(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  static Handle<JSValue> slice(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  static Handle<JSValue> sort(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  static Handle<JSValue> splice(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> unshift(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> indexOf(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> lastIndexOf(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> every(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> some(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> forEach(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  static Handle<JSValue> toLocaleUpperCase(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> map(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  static Handle<JSValue> filter(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  static Handle<JSValue> reduce(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> reduceRight(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

 private:
  template<flag_t flag>
  static Handle<ArrayProto> New() {
    Handle<JSObject> jsobj = JSObject::New<0, flag>(
      CLASS_ARRAY, true, Handle<JSValue>(), false, false, nullptr);

    jsobj.val()->SetType(OBJ_OTHER);
    return Handle<ArrayProto>(jsobj);
  }
};

class ArrayObject : public JSObject {
 public:
  static Handle<ArrayObject> New(double len) {
    Handle<JSObject> jsobj = JSObject::New<0, 0>(
      CLASS_ARRAY, true, Handle<JSValue>(), false, false, nullptr,
      len < kMinNumArrayElements ? kMinNumArrayElements : len
    );

    jsobj.val()->SetType(OBJ_ARRAY);
    Handle<ArrayObject> obj(jsobj);
    obj.val()->SetPrototype(ArrayProto::Instance());
    // Not using AddValueProperty here to by pass the override DefineOwnProperty
    Handle<Number> len_num = Number::New(len);
    StackPropertyDescriptor desc = StackPropertyDescriptor::NewDataDescriptor(
      len_num, true, false, false);
    DefineOwnProperty__Base(Error::Empty(), obj, String::Length(), desc, false);
    return obj;
  }

  static constexpr size_t kMinNumArrayElements = 4;
};

class ArrayConstructor : public JSObject {
 public:
  static  Handle<ArrayConstructor> Instance() {
    static  Handle<ArrayConstructor> singleton = ArrayConstructor::New<GCFlag::SINGLE>();
    return singleton;
  }

  static Handle<JSValue> isArray(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    if (vals.size() == 0 || !vals[0].val()->IsObject())
      return Bool::False();
    Handle<JSObject> obj = static_cast<Handle<JSObject>>(vals[0]);
    return Bool::Wrap(obj.val()->Class() == CLASS_ARRAY);
  }

  static Handle<JSValue> toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"function Array() { [native code] }");
  }

 private:
  template<flag_t flag>
  static Handle<ArrayConstructor> New() {
    Handle<JSObject> jsobj = JSObject::New<0, flag>(
      CLASS_ARRAY, true, Handle<JSValue>(), true, true, nullptr);

    jsobj.val()->SetType(OBJ_ARRAY_CONSTRUCTOR);
    return Handle<ArrayConstructor>(jsobj);
  }
};

bool DefineOwnProperty__Array(Handle<Error>& e, Handle<ArrayObject> O, Handle<String> P, StackPropertyDescriptor desc, bool throw_flag);
Handle<JSObject> Construct__ArrayConstructor(Handle<Error>& e, Handle<ArrayConstructor> O,  std::vector<Handle<JSValue>> arguments);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_ARRAY_OBJECT