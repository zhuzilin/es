#ifndef ES_IMPL_BUILTIN_OBJECT_OBJECT_IMPL_H
#define ES_IMPL_BUILTIN_OBJECT_OBJECT_IMPL_H

#include <es/types.h>
#include <es/enter_code.h>

namespace es {

JSValue object_constructor::keys(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() < 1 || !vals[0].IsObject()) {
    e = error::TypeError(u"Object.keys called on non-object");
    return JSValue();
  }
  JSValue O = vals[0];
  auto properties = js_object::AllEnumerableProperties(O);
  size_t n = properties.size();
  JSValue arr_obj = array_object::New(n);
  for (size_t index = 0; index < n; index++) {
    AddValueProperty(arr_obj, 
      NumberToString(index), properties[index].first, true, true, true);
  }
  return arr_obj;
}

}  // namespace es

#endif  // ES_IMPL_BUILTIN_OBJECT_OBJECT_IMPL_H