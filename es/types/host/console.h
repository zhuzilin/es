#ifndef ES_TYPES_HOST_CONSOLE
#define ES_TYPES_HOST_CONSOLE

#include <iostream>
#include <es/types/object.h>

namespace es {

bool ToBoolean(JSValue);

namespace console {

inline JSValue log(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size())
    std::cout << JSValue::ToString(vals[0]);
  for (size_t i = 1; i < vals.size(); i++) {
    std::cout << " " << JSValue::ToString(vals[i]);
  }
  std::cout << "\n";
  return number::Zero();
}

inline JSValue New(flag_t flag) {
  JSValue jsobj = js_object::New(
    u"Console", true, JSValue(), false, false, nullptr, 0, flag
  );

  jsobj.SetType(OBJ_HOST);
  AddFuncProperty(jsobj, u"log", ::es::console::log, false, false, false);
  return jsobj;
}

inline  JSValue Instance() {
  static JSValue singleton = console::New(GCFlag::SINGLE);
  return singleton;
}

}  // namespace console

}  // namespace es

#endif  // ES_TYPES_HOST_CONSOLE