#ifndef ES_TYPES_BUILTIN_ARRAY_OBJECT
#define ES_TYPES_BUILTIN_ARRAY_OBJECT

#include <algorithm>

#include <es/types/object.h>
#include <es/types/builtin/object_object.h>
#include <es/utils/helper.h>

namespace es {

bool ToBoolean(JSValue input);
double ToNumber(JSValue& e, JSValue input);
double ToInteger(JSValue& e, JSValue input);
double ToUint32(JSValue& e, JSValue input);
std::u16string ToU16String(JSValue& e, JSValue input);
std::u16string NumberToU16String(double m);
double StringToNumber(JSValue source);
JSValue ToObject(JSValue& e, JSValue input);

bool IsArrayIndex(JSValue P) {
  return string::data(P) == string::data(NumberToString(uint32_t(StringToNumber(P))));
}

namespace array_proto {

inline JSValue New(flag_t flag) {
  JSValue jsobj = js_object::New(
    u"Array", true, JSValue(), false, false, nullptr, 0, flag);

  jsobj.SetType(OBJ_OTHER);
  return jsobj;
}

inline  JSValue Instance() {
  static  JSValue singleton = array_proto::New(GCFlag::SINGLE);
  return singleton;
}

// 15.4.4.2 Array.prototype.toString ( )
inline JSValue toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals);

inline JSValue toLocaleString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue concat(JSValue& e, JSValue this_arg, std::vector<JSValue> vals);

inline JSValue join(JSValue& e, JSValue this_arg, std::vector<JSValue> vals);

// 15.4.4.6 Array.prototype.pop ( )
inline JSValue pop(JSValue& e, JSValue this_arg, std::vector<JSValue> vals);

// 15.4.4.7 Array.prototype.push ( [ item1 [ , item2 [ , … ] ] ] )
inline JSValue push(JSValue& e, JSValue this_arg, std::vector<JSValue> vals);

inline JSValue reverse(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

// 15.4.4.9 Array.prototype.shift ( )
inline JSValue shift(JSValue& e, JSValue this_arg, std::vector<JSValue> vals);

inline JSValue slice(JSValue& e, JSValue this_arg, std::vector<JSValue> vals);

inline JSValue sort(JSValue& e, JSValue this_arg, std::vector<JSValue> vals);

inline JSValue splice(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue unshift(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue indexOf(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue lastIndexOf(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue every(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue some(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue forEach(JSValue& e, JSValue this_arg, std::vector<JSValue> vals);

inline JSValue toLocaleUpperCase(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue map(JSValue& e, JSValue this_arg, std::vector<JSValue> vals);

inline JSValue filter(JSValue& e, JSValue this_arg, std::vector<JSValue> vals);

inline JSValue reduce(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

inline JSValue reduceRight(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  assert(false);
}

}  // namespace array_proto

namespace array_object {

inline JSValue New(double len) {
  JSValue jsobj = js_object::New(
    u"Array", true, JSValue(), false, false, nullptr, 0
  );

  jsobj.SetType(OBJ_ARRAY);
  js_object::SetPrototype(jsobj, array_proto::Instance());
  JSValue desc = property_descriptor::New();
  // Not using AddValueProperty here to by pass the override DefineOwnProperty
  JSValue len_num = number::New(len);
  property_descriptor::SetDataDescriptor(desc, len_num, true, false, false);
  JSValue e = error::Empty();
  DefineOwnProperty__Base(e, jsobj, string::Length(), desc, false);
  return jsobj;
}

}  // namespace array_object

namespace array_constructor {

inline JSValue New(flag_t flag) {
  JSValue jsobj = js_object::New(
    u"Array", true, JSValue(), true, true, nullptr, 0, flag);

  jsobj.SetType(OBJ_ARRAY_CONSTRUCTOR);
  return jsobj;
}

inline JSValue Instance() {
  static JSValue singleton = array_constructor::New(GCFlag::SINGLE);
  return singleton;
}

inline JSValue isArray(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() == 0 || !vals[0].IsObject())
    return boolean::False();
  JSValue obj = vals[0];
  return boolean::New(js_object::Class(obj) == u"Array");
}

inline JSValue toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  return string::New(u"function Array() { [native code] }");
}

}  // namespace array_constructor

bool DefineOwnProperty__Array(JSValue& e, JSValue O, JSValue P, JSValue desc, bool throw_flag);
JSValue Construct__ArrayConstructor(JSValue& e, JSValue O,  std::vector<JSValue> arguments);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_ARRAY_OBJECT