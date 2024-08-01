#ifndef ES_IMPL_BUILTIN_OBJECT_OBJECT_IMPL_H
#define ES_IMPL_BUILTIN_OBJECT_OBJECT_IMPL_H

#include <es/types.h>
#include <es/enter_code.h>

namespace es {

Handle<JSValue> ObjectConstructor::keys(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
  if (vals.size() < 1 || !vals[0].val()->IsObject()) {
    e = Error::TypeError(u"Object.keys called on non-object");
    return Handle<JSValue>();
  }
  Handle<JSObject> O = static_cast<Handle<JSObject>>(vals[0]);
  auto keys = O.val()->AllEnumerableKeys();
  size_t n = keys.size();
  Handle<ArrayObject> arr_obj = ArrayObject::New(n);
  for (size_t index = 0; index < n; index++) {
    AddValueProperty(arr_obj, 
      NumberToString(index), keys[index], true, true, true);
  }
  return arr_obj;
}

}  // namespace es

#endif  // ES_IMPL_BUILTIN_OBJECT_OBJECT_IMPL_H