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

  Completion() : Completion(NORMAL, nullptr, u"") {}

  Completion(Type type, JSValue* value, std::u16string target) :
    type(type), value(value), target(target) {}

  bool IsAbruptCompletion() { return type != NORMAL; }
  bool IsThrow() { return type == THROW; }

  Type type;
  JSValue* value;
  std::u16string target;
};

}  // namespace es

#endif  // ES_TYPES_COMPLETION