#ifndef ES_TYPES_LIST_H
#define ES_TYPES_LIST_H

#include <vector>

#include <es/types/base.h>

namespace es {

class List : public JSValue {
 public:
  List() : JSValue(JSValue::JS_LIST) {}

 private:
  std::vector<JSValue*> vals;
};

}  // namespace es

#endif  // ES_TYPES_LIST_H