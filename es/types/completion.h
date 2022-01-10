#ifndef ES_TYPES_COMPLETION
#define ES_TYPES_COMPLETION

#include <es/types/base.h>

namespace es {

// NOTE(zhuzilin) Completion is the spec type to represent
// the evaluation result of the statement.
// It won't interact with other types, so does not need to
// inherit JSValue.
class Completion {
 public:
  enum Type {
    NORMAL,
    BREAK,
    CONTINUE,
    RETURN,
    THROW,
  };

  Completion(Type type, JSValue* value, JSValue* target) :
    type_(type), value_(value), target_(target) {}

  bool IsAbruptCompletion() { return type_ != NORMAL; }

 private:
  Type type_;
  JSValue* value_;
  JSValue* target_;
};

}  // namespace es

#endif  // ES_TYPES_COMPLETION