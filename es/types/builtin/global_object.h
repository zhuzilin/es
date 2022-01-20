#ifndef ES_TYPES_BUILTIN_GLOBAL_OBJECT
#define ES_TYPES_BUILTIN_GLOBAL_OBJECT

#include <es/types/object.h>

namespace es {

std::u16string ToString(Error* e, JSValue* input);
double StringToNumber(std::u16string source);

// 15.1 The Global Object
class GlobalObject : public JSObject {
 public:
  static GlobalObject* Instance() {
    static GlobalObject singleton;
    return &singleton;
  }

  bool direct_eval() { return direct_eval_; }
  void SetDirectEval(bool direct_eval) {
    direct_eval_ = direct_eval;
  }

  // 15.1.2.1 eval(X)
  static JSValue* eval(Error* e, JSValue* this_arg, std::vector<JSValue*> vals);

  // 15.1.2.2 parseInt (string , radix)
  static JSValue* parseInt(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    // TODO(zhuzilin) use parseFloat at the moment. fix later
    return GlobalObject::parseFloat(e, this_arg, vals);
  }

  // 15.1.2.3 parseFloat (string)
  static JSValue* parseFloat(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    if (vals.size() == 0)
      return Number::NaN();
    std::u16string input_string = ::es::ToString(e, vals[0]);
    size_t i = 0;
    while (i < input_string.size() && character::IsWhiteSpace(i))
      i++;
    std::u16string trimmed_string = input_string.substr(i);
    Lexer lexer(trimmed_string);
    // TODO(zhuzilin) parseFloat should not be able to parse hex integer
    Token token = lexer.Next();
    if (token.type() == Token::TK_NUMBER) {
      return new Number(StringToNumber(token.source()));
    } else {
      return Number::NaN();
    }
  }

  // 15.1.2.4 isNaN (number)
  static JSValue* isNaN(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  // 15.1.2.5 isFinite (number)
  static JSValue* isFinite(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  inline std::string ToString() override { return "GlobalObject"; }

 private:
  GlobalObject() :
    JSObject(
      OBJ_GLOBAL,
      // 15.1 The values of the [[Prototype]] and [[Class]]
      // of the global object are implementation-dependent.
      u"Global",
      // NOTE(zhuzilin) global object need to have [[Extensible]] as true,
      // otherwise we cannot define variable in global code, as global varaibles
      // are the property of global object.
      true, nullptr, false, false
    ), direct_eval_(false) {}

  bool direct_eval_;
};

class DirectEvalGuard {
  public:
    DirectEvalGuard() {
      GlobalObject::Instance()->SetDirectEval(true);
    }

    ~DirectEvalGuard() {
      GlobalObject::Instance()->SetDirectEval(false);
    }
};

}  // namespace es

#endif  // ES_TYPES_BUILTIN_GLOBAL_OBJECT