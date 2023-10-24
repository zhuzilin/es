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

  Completion() : Completion(NORMAL, JSValue(), u"") {}

  Completion(Type type, JSValue value, std::u16string target) :
    type_(type), value_(value), target_(target) {}

  Type type() { return type_; }
  JSValue value() {
    return value_;
  }
  void SetValue(JSValue val) {
    value_ = val;
  }
  std::u16string target() { return target_; }

  bool IsAbruptCompletion() { return type_ != NORMAL; }
  bool IsThrow() { return type_ == THROW; }
  bool IsEmpty() { return value_.type() == JS_UNINIT; }

 private:
  Type type_;
  JSValue value_;
  std::u16string target_;
};

}  // namespace es

#endif  // ES_TYPES_COMPLETION