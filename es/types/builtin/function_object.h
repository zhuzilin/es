#ifndef ES_TYPES_BUILTIN_FUNCTION_OBJECT
#define ES_TYPES_BUILTIN_FUNCTION_OBJECT

#include <es/types/object.h>
#include <es/types/lexical_environment.h>

namespace es {

class Function : public JSObject {
 public:
  Function(JSValue* proto, bool extensible) :
    JSObject(
      OBJ_FUNC,
      proto,
      u"Function",
      extensible,
      nullptr,
      true,
      true
    ) {
    
  }

  // Function only internal properties
  bool HasInstance(JSValue* value);
  LexicalEnvironment* Scope() { return scope_; };
  std::vector<std::u16string_view> FormalParameters() { return formal_params_; };
  std::u16string_view Code() { return code_; }

 private:
  LexicalEnvironment* scope_;
  std::vector<std::u16string_view> formal_params_;
  std::u16string_view code_;
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_FUNCTION_OBJECT