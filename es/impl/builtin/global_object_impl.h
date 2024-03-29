#ifndef ES_IMPL_BUILTIN_GLOBAL_OBJECT_IMPL_H
#define ES_IMPL_BUILTIN_GLOBAL_OBJECT_IMPL_H

#include <es/types.h>
#include <es/enter_code.h>

namespace es {

// 15.1.2.1 eval(X)
Handle<JSValue> GlobalObject::eval(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
  TEST_LOG("\033[2menter\033[0m GlobalObject::eval");
  if (vals.size() == 0)
    return Undefined::Instance();
  if (!vals[0].val()->IsString())
    return vals[0];
  std::u16string x = static_cast<Handle<String>>(vals[0]).val()->data();
  Parser parser(x);
  AST* program = parser.ParseProgram();
  if (program->IsIllegal()) {
    e = Error::SyntaxError(u"failed to parse eval (" + program->source() + u")");
    return Handle<JSValue>();
  }
  EnterEvalCode(e, program);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  Completion result = EvalProgram(program);
  Runtime::Global()->PopContext();

  switch (result.type()) {
    case Completion::NORMAL:
      if (!result.IsEmpty())
        return result.value();
      else
        return Undefined::Instance();
    default: {
      ASSERT(result.type() == Completion::THROW);
      Handle<JSValue> return_value = result.value();
      if (return_value.val()->IsError()) {
        e = return_value;
      } else {
        e = Error::NativeError(return_value);
      }
      return return_value;
    }
  }
}

// 15.1.2.2 parseInt (string , radix)
Handle<JSValue> GlobalObject::parseInt(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
  // TODO(zhuzilin) use parseFloat at the moment. fix later
  if (vals.size() == 0 || vals[0].val()->IsUndefined()) {
    e = Error::TypeError(u"parseInt called with undefined string");
    return Handle<JSValue>();
  }
  std::u16string input_string = ToU16String(e, vals[0]);
  size_t len = input_string.size();
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  double R = 10;
  bool strip_prefix = true;
  if (vals.size() >= 2 && !vals[1].val()->IsUndefined()) {
    R = ToInt32(e, vals[0]);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
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
  return Number::New(sign * number);
}

// 15.1.2.3 parseFloat (string)
Handle<JSValue> GlobalObject::parseFloat(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
  if (vals.size() == 0)
    return Number::NaN();
  std::u16string input_string = es::ToU16String(e, vals[0]);
  size_t i = 0;
  while (i < input_string.size() && character::IsWhiteSpace(input_string[i]))
    i++;
  size_t j = input_string.size();
  while (j > 0 && character::IsWhiteSpace(input_string[j - 1]))
    j--;
  std::u16string trimmed_string = input_string.substr(i, j - i);
  if (trimmed_string == u"Infinity" || trimmed_string == u"+Infinity")
    return Number::Infinity();
  if (trimmed_string == u"-Infinity")
    return Number::NegativeInfinity();
  Lexer lexer(trimmed_string);
  // TODO(zhuzilin) parseFloat should not be able to parse hex integer
  Token token = lexer.Next();
  if (token.type() == Token::TK_NUMBER) {
    return Number::New(StringToNumber(token.source()));
  } else {
    return Number::NaN();
  }
}

// 15.1.2.4 isNaN (number)
Handle<JSValue> GlobalObject::isNaN(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
  if (vals.size() == 0)
    return Bool::True();
  double num = ToNumber(e, vals[0]);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  return Bool::Wrap(isnan(num));
}

}  // namespace es

#endif  // ES_IMPL_BUILTIN_GLOBAL_OBJECT_IMPL_H