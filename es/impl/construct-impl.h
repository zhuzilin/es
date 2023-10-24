#ifndef ES_TYPES_CONSTRUCT_IMPL_H
#define ES_TYPES_CONSTRUCT_IMPL_H

#include <es/types.h>

namespace es {

JSValue Construct(JSValue& e, JSValue O, std::vector<JSValue> arguments) {
  if (O.IsFunctionObject()) {
    if (!function_object::from_bind(O)) {
      return Construct__Function(e, O, arguments);
    } else {
      return Construct__BindFunction(e, O, arguments);
    }
  } else if (O.IsConstructor()) {
    switch (O.type()) {
      case Type::OBJ_BOOL_CONSTRUCTOR:
        return Construct__BoolConstructor(e, O, arguments);
      case Type::OBJ_NUMBER_CONSTRUCTOR:
        return Construct__NumberConstructor(e, O, arguments);
      case Type::OBJ_OBJECT_CONSTRUCTOR:
        return Construct__ObjectConstructor(e, O, arguments);
      case Type::OBJ_REGEXP_CONSTRUCTOR:
        return Construct__RegExpConstructor(e, O, arguments);
      case Type::OBJ_STRING_CONSTRUCTOR:
        return Construct__StringConstructor(e, O, arguments);
      case Type::OBJ_FUNC_CONSTRUCTOR:
        return Construct__FunctionConstructor(e, O, arguments);
      case Type::OBJ_ARRAY_CONSTRUCTOR:
        return Construct__ArrayConstructor(e, O, arguments);
      case Type::OBJ_DATE_CONSTRUCTOR:
        assert(false);
      case Type::OBJ_ERROR_CONSTRUCTOR:
        return Construct__ErrorConstructor(e, O, arguments);
      default:
        assert(false);
    }
  }
  assert(false);
}

// 13.2.2 [[Construct]]
JSValue Construct__Function(
  JSValue& e, JSValue O, std::vector<JSValue> arguments
) {
  TEST_LOG("enter function_object::Construct");
  // NOTE(zhuzilin) I'm not sure if the object type should be OBJ_OBJECT or OBJ_OTHER...
  JSValue obj = js_object::New(u"Object", true, JSValue(), false, false, nullptr, 0);
  obj.SetType(Type::OBJ_OBJECT);
  JSValue proto = Get(e, O, string::Prototype());
  if (unlikely(!error::IsOk(e))) return JSValue();
  if (proto.IsObject()) {  // 6
    js_object::SetPrototype(obj, proto);
  } else {  // 7
    js_object::SetPrototype(obj, object_proto::Instance());
  }
  JSValue result = Call(e, O, obj, arguments);  // 8
  if (unlikely(!error::IsOk(e))) return JSValue();
  if (result.IsObject())  // 9
    return result;
  return obj;  // 10
}

// 13.2.2 [[Construct]]
JSValue Construct__BindFunction(
  JSValue& e, JSValue O, std::vector<JSValue> extra_args
) {
  auto target_function = bind_function_object::TargetFunction(O);
  auto bound_args = bind_function_object::BoundArgs(O);
  if (!target_function.IsConstructor()) {
    e = error::TypeError(u"target function has no [[Construct]] internal method");
    return JSValue();
  }
  std::vector<JSValue> args;
  for (size_t i = 0; i < fixed_array::size(bound_args); i++) {
    args.emplace_back(fixed_array::Get(bound_args, i));
  }
  args.insert(args.end(), extra_args.begin(), extra_args.end());
  return Construct(e, target_function, args);
}

// 15.5.2.1 new Array ( [ value ] )
JSValue Construct__ArrayConstructor(
  JSValue& e, JSValue O, std::vector<JSValue> arguments
) {
  if (arguments.size() == 1 && arguments[0].IsNumber()) {
    JSValue len = arguments[0];
    if (number::data(len) == ToUint32(e, len)) {
      return array_object::New(number::data(len));
    } else {
      e = error::RangeError(u"Invalid array length");
      return JSValue();
    }
  }
  JSValue arr = array_object::New(arguments.size());
  for (size_t i = 0; i < arguments.size(); i++) {
    JSValue arg = arguments[i];
    AddValueProperty(arr, NumberToU16String(i), arg, true, true, true);
  }
  return arr;
}

JSValue Construct__BoolConstructor(
  JSValue& e, JSValue O, std::vector<JSValue> arguments
) {
  bool b;
  if (arguments.size() == 0)
    b = ToBoolean(undefined::New());
  else
    b = ToBoolean(arguments[0]);
  return bool_object::New(boolean::New(b));
}

JSValue Construct__ErrorConstructor(
  JSValue& e, JSValue O, std::vector<JSValue> arguments
) {
  if (arguments.size() == 0 || arguments[0].IsUndefined())
    return error_object::New(error::NativeError(undefined::New()));
  return error_object::New(error::NativeError(arguments[0]));
}

// 15.3.2.1 new Function (p1, p2, … , pn, body)
JSValue Construct__FunctionConstructor(
  JSValue& e, JSValue O, std::vector<JSValue> arguments
) {
  TEST_LOG("enter function_constructor::Construct");
  size_t arg_count = arguments.size();
  std::u16string P = u"";
  std::u16string body = u"";
  if (arg_count == 1) {
    body = ToU16String(e, arguments[0]);
    if (unlikely(!error::IsOk(e))) return JSValue();
  } else if (arg_count > 1) {
    P += ToU16String(e, arguments[0]);
    if (unlikely(!error::IsOk(e))) return JSValue();
    for (size_t i = 1; i < arg_count - 1; i++) {
      P += u"," + ToU16String(e, arguments[i]);
      if (unlikely(!error::IsOk(e))) return JSValue();
    }
    body = ToU16String(e, arguments[arg_count - 1]);
    if (unlikely(!error::IsOk(e))) return JSValue();
  }
  std::u16string P_view = Runtime::Global()->AddSource(std::move(P));
  std::vector<std::u16string> names;
  AST* body_ast;
  if (P_view.size() > 0) {
    Parser parser(P_view);
    if (!parser.ParseFormalParameterList(names)) {
      e = error::SyntaxError(u"invalid parameter name");
      return JSValue();
    }
  }
  std::u16string body_view = Runtime::Global()->AddSource(std::move(body));
  {
    Parser parser(body_view);
    body_ast = parser.ParseFunctionBody(Token::TK_EOS);
    if (body_ast->IsIllegal()) {
      e = error::SyntaxError(u"failed to parse function body: " + body_ast->source());
      return JSValue();
    }
  }
  JSValue scope = lexical_env::Global();
  bool strict = static_cast<ProgramOrFunctionBody*>(body_ast)->strict();
  if (strict) {
    // 13.1
    if (HaveDuplicate(names)) {
      e = error::SyntaxError(u"have duplicate parameter name in strict mode");
      return JSValue();
    }
    for (auto name : names) {
      if (name == u"eval" || name == u"arguments") {
        e = error::SyntaxError(u"param name cannot be eval or arguments in strict mode");
        return JSValue();
      }
    }
  }
  return function_object::New(names, body_ast, scope, strict);
}

JSValue Construct__NumberConstructor(
  JSValue& e, JSValue O, std::vector<JSValue> arguments
) {
  JSValue js_num;
  if (arguments.size() == 0) {
    js_num = number::Zero();
  } else {
    double num = ToNumber(e, arguments[0]);
    if (unlikely(!error::IsOk(e))) return JSValue();
    js_num = number::New(num);
  }
  return number_object::New(js_num);
}

// 15.2.2 The Object Constructor
JSValue Construct__ObjectConstructor(
  JSValue& e, JSValue O, std::vector<JSValue> arguments
) {
  if (arguments.size() > 0) {  // 1
    JSValue value = arguments[0];
    switch (value.type()) {
      case Type::JS_OBJECT:
        // TODO(zhuzilin) deal with host object.
        return value;
      case Type::JS_STRING:
      case Type::JS_BOOL:
      case Type::JS_NUMBER:
        return ToObject(e, value);
      default:
        break;
    }
  }
  ASSERT(arguments.size() == 0 || arguments[0].IsNull() || arguments[0].IsUndefined());
  JSValue obj = object_object::New();
  return obj;
}

JSValue Construct__RegExpConstructor(
  JSValue& e, JSValue O, std::vector<JSValue> arguments
) {
  JSValue P, F;
  if (arguments.size() == 0) {
    P = string::Empty();
  } else if (arguments[0].IsRegExpObject()) {
    if (arguments.size() > 1 && !arguments[1].IsUndefined()) {
      JSValue R = arguments[0];
      P = regex_object::pattern(R);
      F = regex_object::flag(R);
    } else {
      e = error::TypeError(u"new RegExp called with RegExp object and flag.");
      return JSValue();
    }
  } else {
    P = ::es::ToString(e, arguments[0]);
    if (unlikely(!error::IsOk(e))) return JSValue();
  }
  if (arguments.size() < 2 || arguments[1].IsUndefined()) {
    F = string::Empty();
  } else {
    F = ::es::ToString(e, arguments[1]);
    if (unlikely(!error::IsOk(e))) return JSValue();
  }
  // Check is flag is valid
  std::unordered_map<char16_t, size_t> count;
  bool valid_flag = true;
  for (auto c : string::data_view(F)) {
    if (c != u'g' && c != u'i' && c != u'm') {
      valid_flag = false;
      break;
    }
    if (count[c] > 0) {
      valid_flag = false;
      break;
    }
    count[c]++;
  }
  if (!valid_flag) {
    e = error::SyntaxError(u"invalid RegExp flag: " + string::data(F));
    return JSValue();
  }
  return regex_object::New(P, F);
}

// 15.5.2.1 string::New ( [ value ] )
JSValue Construct__StringConstructor(
  JSValue& e, JSValue O, std::vector<JSValue> arguments
) {
  if (arguments.size() == 0)
    return string_object::New(string::Empty());
  JSValue str = ::es::ToString(e, arguments[0]);
  if (unlikely(!error::IsOk(e))) return JSValue();
  return string_object::New(str);
}

}  // namespace es

#endif  // ES_TYPES_CONSTRUCT_IMPL_H
