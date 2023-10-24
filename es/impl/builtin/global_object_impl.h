#ifndef ES_IMPL_BUILTIN_GLOBAL_OBJECT_IMPL_H
#define ES_IMPL_BUILTIN_GLOBAL_OBJECT_IMPL_H

#include <es/types.h>
#include <es/enter_code.h>

namespace es {

// 15.1.2.1 eval(X)
JSValue global_object::eval(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  TEST_LOG("enter global_object::eval");
  if (vals.size() == 0)
    return undefined::New();
  if (!vals[0].IsString())
    return vals[0];
  std::u16string x = string::data(vals[0]);
  Parser parser(x);
  AST* program = parser.ParseProgram();
  if (program->IsIllegal()) {
    e = error::SyntaxError(u"failed to parse eval (" + program->source() + u")");
    return JSValue();
  }
  EnterEvalCode(e, program);
  if (unlikely(!error::IsOk(e))) return JSValue();
  Completion result = EvalProgram(program);
  Runtime::Global()->PopContext();

  switch (result.type()) {
    case Completion::NORMAL:
      if (!result.IsEmpty())
        return result.value();
      else
        return undefined::New();
    default: {
      ASSERT(result.type() == Completion::THROW);
      JSValue return_value = result.value();
      if (return_value.IsError()) {
        e = return_value;
      } else {
        e = error::NativeError(return_value);
      }
      return return_value;
    }
  }
}

// 15.1.2.2 parseInt (string , radix)
JSValue global_object::parseInt(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  // TODO(zhuzilin) use parseFloat at the moment. fix later
  if (vals.size() == 0 || vals[0].IsUndefined()) {
    e = error::TypeError(u"parseInt called with undefined string");
    return JSValue();
  }
  std::u16string input_string = ToU16String(e, vals[0]);
  size_t len = input_string.size();
  if (unlikely(!error::IsOk(e))) return JSValue();
  double R = 10;
  bool strip_prefix = true;
  if (vals.size() >= 2 && !vals[1].IsUndefined()) {
    R = ToInt32(e, vals[0]);
    if (unlikely(!error::IsOk(e))) return JSValue();
    if (R < 2 || R > 36)
      return number::NaN();
    if (R != 0 && R != 16)
      strip_prefix = false;
    if (R == 0)
      R = 10;
  }
  size_t offset = 0;
  while (offset < len && character::IsWhiteSpace(input_string[offset]))
    offset++;
  if (offset == len)
    return number::NaN();
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
    return number::NaN();
  double number = 0;
  for (size_t i = offset; i < Z; i++) {
    number *= R;
    number += character::Digit(input_string[i]);
  }
  return number::New(sign * number);
}

// 15.1.2.3 parseFloat (string)
JSValue global_object::parseFloat(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() == 0)
    return number::NaN();
  std::u16string input_string = es::ToU16String(e, vals[0]);
  size_t i = 0;
  while (i < input_string.size() && character::IsWhiteSpace(input_string[i]))
    i++;
  size_t j = input_string.size();
  while (j > 0 && character::IsWhiteSpace(input_string[j - 1]))
    j--;
  std::u16string trimmed_string = input_string.substr(i, j - i);
  if (trimmed_string == u"Infinity" || trimmed_string == u"+Infinity")
    return number::Infinity();
  if (trimmed_string == u"-Infinity")
    return number::NegativeInfinity();
  Lexer lexer(trimmed_string);
  // TODO(zhuzilin) parseFloat should not be able to parse hex integer
  Token token = lexer.Next();
  if (token.type() == Token::TK_NUMBER) {
    return number::New(StringToNumber(token.source()));
  } else {
    return number::NaN();
  }
}

// 15.1.2.4 isNaN (number)
JSValue global_object::isNaN(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  if (vals.size() == 0)
    return boolean::True();
  double num = ToNumber(e, vals[0]);
  if (unlikely(!error::IsOk(e))) return JSValue();
  return boolean::New(isnan(num));
}

}  // namespace es

#endif  // ES_IMPL_BUILTIN_GLOBAL_OBJECT_IMPL_H