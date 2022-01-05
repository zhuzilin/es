#ifndef ES_TYPES_COMPLETION
#define ES_TYPES_COMPLETION

#include <es/types/base.h>

namespace es {

class Completion : public JSValue {
 public:
  enum Type {
    BREAK,
    CONTINUE,
    RETURN,
    THROW,
  };

  Completion(Type type, JSValue* value, JSValue* target) :
    JSValue(JS_COMP), type_(type), value_(value), target_(target) {}

 private:
  Type type_;
  JSValue* value_;
  JSValue* target_;
};

}  // namespace es

#endif  // ES_TYPES_COMPLETION