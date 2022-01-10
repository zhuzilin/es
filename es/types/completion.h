#ifndef ES_TYPES_COMPLETION
#define ES_TYPES_COMPLETION

#include <es/types/base.h>

namespace es {

// NOTE(zhuzilin) Completion is the spec type to represent
// the evaluation result of the statement.
// It won't interact with other types, so does not need to
// inherit JSValue.
struct Completion {
  enum Type {
    NORMAL,
    BREAK,
    CONTINUE,
    RETURN,
    THROW,
  };

  Completion() : Completion(NORMAL, nullptr, nullptr) {}

  Completion(Type type, JSValue* value, JSValue* target) :
    type(type), value(value), target(target) {}

  bool IsAbruptCompletion() { return type != NORMAL; }

  Type type;
  JSValue* value;
  JSValue* target;
};

}  // namespace es

#endif  // ES_TYPES_COMPLETION