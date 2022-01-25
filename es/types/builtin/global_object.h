#ifndef ES_TYPES_BUILTIN_GLOBAL_OBJECT
#define ES_TYPES_BUILTIN_GLOBAL_OBJECT

#include <es/types/object.h>

namespace es {

std::u16string ToString(Error* e, JSValue* input);
double ToInt32(Error* e, JSValue* input);
double StringToNumber(std::u16string source);

// 15.1 The Global Object
class GlobalObject : public JSObject {
 public:
  static GlobalObject* Instance() {
    static GlobalObject* singleton = new GlobalObject();
    return singleton;
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
    if (vals.size() == 0 || vals[0]->IsUndefined()) {
      *e = *Error::TypeError(u"parseInt called with undefined string");
      return nullptr;
    }
    std::u16string input_string = ::es::ToString(e, vals[0]);
    size_t len = input_string.size();
    if (!e->IsOk()) return nullptr;
    double R = 10;
    bool strip_prefix = true;
    if (vals.size() >= 2 && !vals[1]->IsUndefined()) {
      R = ToInt32(e, vals[0]);
      if (!e->IsOk()) return nullptr;
      if (R < 2 || R > 36)
        return Number::NaN();
      if (R != 0 && R != 16)
        strip_prefix = false;
      if (R == 0)
        R = 10;
    }
    size_t offset = 0;
    while (offset < len && character::IsWhiteSpace(input_string[offset]))
      offset++;
    if (offset == len)
      return Number::NaN();
    double sign = 1;
    if (input_string[offset] == u'-') {
      sign = -1;
      offset++;
    } else if (input_string[offset] == u'+') {
      offset++;
    }
    if (strip_prefix && len - offset >= 2) {
      if (input_string[offset] == u'0' &&
          (input_string[offset + 1] == u'x' || input_string[offset + 1] == u'X')) {
        offset += 2;
        R = 16;
      }
    } 
    size_t Z = offset;
    while (Z < input_string.size() && character::IsRadixDigit(input_string[Z], R))
      Z++;
    if (offset == Z)
      return Number::NaN();
    double number = 0;
    for (size_t i = offset; i < Z; i++) {
      number *= R;
      number += character::Digit(input_string[i]);
    }
    return new Number(number);
  }

  // 15.1.2.3 parseFloat (string)
  static JSValue* parseFloat(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    if (vals.size() == 0)
      return Number::NaN();
    std::u16string input_string = ::es::ToString(e, vals[0]);
    size_t i = 0;
    while (i < input_string.size() && character::IsWhiteSpace(input_string[i]))
      i++;
    size_t j = input_string.size();
    while (j > 0 && character::IsWhiteSpace(input_string[j - 1]))
      j--;
    std::u16string trimmed_string = input_string.substr(i, j - i);
    if (trimmed_string == u"Infinity" || trimmed_string == u"+Infinity")
      return Number::PositiveInfinity();
    if (trimmed_string == u"-Infinity")
      return Number::NegativeInfinity();
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