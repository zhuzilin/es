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

  Completion() : type_(NORMAL), value_() {}

  Completion(Type type, Handle<JSValue> value, const std::u16string& target) :
    type_(type), value_(value), target_(target) {}

  static Completion Normal() { return Completion(); }
  static Completion Normal(Handle<JSValue> value) {
    Completion c;
    c.value_ = value;
    return c;
  }
  static Completion Throw(Handle<JSValue> value) {
    Completion c;
    c.type_ = THROW;
    c.value_ = value;
    return c;
  }
  static Completion Return(Handle<JSValue> value) {
    Completion c;
    c.type_ = RETURN;
    c.value_ = value;
    return c;
  }

  Type type() { return type_; }
  Handle<JSValue> value() {
    return value_;
  }
  void SetValue(JSValue* val) {
    value_ = Handle<JSValue>(val);
  }
  void SetType(Type type) { type_ = type; }
  void SetTarget(const std::u16string& target) { target_ = target; }
  const std::u16string& target() { return target_; }

  bool IsAbruptCompletion() { return type_ != NORMAL; }
  bool IsThrow() { return type_ == THROW; }
  bool IsEmpty() { return value_.IsNullptr(); }

 private:
  Type type_;
  Handle<JSValue> value_;
  std::u16string target_;
};

}  // namespace es

#endif  // ES_TYPES_COMPLETION