#ifndef ES_TYPES_CONSTRUCT_IMPL_H
#define ES_TYPES_CONSTRUCT_IMPL_H

#include <es/types.h>

namespace es {

Handle<JSObject> Construct(Handle<Error>& e, Handle<JSObject> O, std::vector<Handle<JSValue>> arguments) {
  if (O.val()->IsFunctionObject()) {
    Handle<FunctionObject> F = static_cast<Handle<FunctionObject>>(O);
    if (!F.val()->from_bind()) {
      return Construct__Function(e, F, std::move(arguments));
    } else {
      return Construct__BindFunction(e, static_cast<Handle<BindFunctionObject>>(O), std::move(arguments));
    }
  } else if (O.val()->IsConstructor()) {
    switch (O.val()->type()) {
      case Type::OBJ_BOOL_CONSTRUCTOR:
        return Construct__BoolConstructor(e, static_cast<Handle<BoolConstructor>>(O), std::move(arguments));
      case Type::OBJ_NUMBER_CONSTRUCTOR:
        return Construct__NumberConstructor(e, static_cast<Handle<NumberConstructor>>(O), std::move(arguments));
      case Type::OBJ_OBJECT_CONSTRUCTOR:
        return Construct__ObjectConstructor(e, static_cast<Handle<ObjectConstructor>>(O), std::move(arguments));
      case Type::OBJ_REGEXP_CONSTRUCTOR:
        return Construct__RegExpConstructor(e, static_cast<Handle<RegExpConstructor>>(O), std::move(arguments));
      case Type::OBJ_STRING_CONSTRUCTOR:
        return Construct__StringConstructor(e, static_cast<Handle<StringConstructor>>(O), std::move(arguments));
      case Type::OBJ_FUNC_CONSTRUCTOR:
        return Construct__FunctionConstructor(e, static_cast<Handle<FunctionConstructor>>(O), std::move(arguments));
      case Type::OBJ_ARRAY_CONSTRUCTOR:
        return Construct__ArrayConstructor(e, static_cast<Handle<ArrayConstructor>>(O), std::move(arguments));
      case Type::OBJ_DATE_CONSTRUCTOR:
        assert(false);
      case Type::OBJ_ERROR_CONSTRUCTOR:
        return Construct__ErrorConstructor(e, static_cast<Handle<ErrorConstructor>>(O), std::move(arguments));
      default:
        assert(false);
    }
  }
  assert(false);
}

// 13.2.2 [[Construct]]
Handle<JSObject> Construct__Function(
  Handle<Error>& e, Handle<FunctionObject> O, std::vector<Handle<JSValue>> arguments
) {
  TEST_LOG("\033[2menter\033[0m FunctionObject::Construct");
  // NOTE(zhuzilin) I'm not sure if the object type should be OBJ_OBJECT or OBJ_OTHER...
  Function* func_ast = O.val()->Code();
  Handle<JSObject> obj = JSObject::New<0>(
    CLASS_OBJECT, true, Handle<JSValue>(), false, false, nullptr, 0, 0, func_ast->body()->num_this_properties()
  );
  obj.val()->SetType(Type::OBJ_OBJECT);
  Handle<JSValue> proto = Get(e, O, String::Prototype());
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  if (proto.val()->IsObject()) {  // 6
    obj.val()->SetPrototype(proto);
  } else {  // 7
    obj.val()->SetPrototype(ObjectProto::Instance());
  }
  Handle<JSValue> result = Call(e, O, obj, std::move(arguments));  // 8
  // get more accurate num_decls from runtime.
  func_ast->body()->SetNumThisProperties(obj.val()->named_properties()->hashmap().val()->occupancy());
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  if (result.val()->IsObject())  // 9
    return static_cast<Handle<JSObject>>(result);
  return obj;  // 10
}

// 13.2.2 [[Construct]]
Handle<JSObject> Construct__BindFunction(
  Handle<Error>& e, Handle<BindFunctionObject> O, std::vector<Handle<JSValue>> extra_args
) {
  auto target_function = O.val()->TargetFunction();
  auto bound_args = O.val()->BoundArgs();
  if (!target_function.val()->IsConstructor()) {
    e = Error::TypeError(u"target function has no [[Construct]] internal method");
    return Handle<JSValue>();
  }
  std::vector<Handle<JSValue>> args;
  for (size_t i = 0; i < bound_args.val()->size(); i++) {
    args.emplace_back(bound_args.val()->Get(i));
  }
  args.insert(args.end(), extra_args.begin(), extra_args.end());
  return Construct(e, target_function, args);
}

// 15.5.2.1 new Array ( [ value ] )
Handle<JSObject> Construct__ArrayConstructor(
  Handle<Error>& e, Handle<ArrayConstructor> O, std::vector<Handle<JSValue>> arguments
) {
  if (arguments.size() == 1 && arguments[0].val()->IsNumber()) {
    Handle<Number> len = static_cast<Handle<Number>>(arguments[0]);
    if (len.val()->data() == ToUint32(e, len)) {
      return ArrayObject::New(len.val()->data());
    } else {
      e = Error::RangeError(u"Invalid array length");
      return Handle<JSValue>();
    }
  }
  Handle<ArrayObject> arr = ArrayObject::New(arguments.size());
  for (size_t i = 0; i < arguments.size(); i++) {
    Handle<JSValue> arg = arguments[i];
    AddValueProperty(arr, NumberToString(i), arg, true, true, true);
  }
  return arr;
}

Handle<JSObject> Construct__BoolConstructor(
  Handle<Error>& e, Handle<BoolConstructor> O, std::vector<Handle<JSValue>> arguments
) {
  bool b;
  if (arguments.size() == 0)
    b = ToBoolean(Undefined::Instance());
  else
    b = ToBoolean(arguments[0]);
  return BoolObject::New(Bool::Wrap(b));
}

Handle<JSObject> Construct__ErrorConstructor(
  Handle<Error>& e, Handle<ErrorConstructor> O, std::vector<Handle<JSValue>> arguments
) {
  if (arguments.size() == 0 || arguments[0].val()->IsUndefined())
    return ErrorObject::New(Error::NativeError(Undefined::Instance()));
  return ErrorObject::New(Error::NativeError(arguments[0]));
}

// 15.3.2.1 new Function (p1, p2, … , pn, body)
Handle<JSObject> Construct__FunctionConstructor(
  Handle<Error>& e, Handle<FunctionConstructor> O, std::vector<Handle<JSValue>> arguments
) {
  TEST_LOG("\033[2menter\033[0m FunctionConstructor::Construct");
  size_t arg_count = arguments.size();
  std::u16string P = u"";
  std::u16string body = u"";
  if (arg_count == 1) {
    body = ToU16String(e, arguments[0]);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  } else if (arg_count > 1) {
    P += ToU16String(e, arguments[0]);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    for (size_t i = 1; i < arg_count - 1; i++) {
      P += u"," + ToU16String(e, arguments[i]);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    }
    body = ToU16String(e, arguments[arg_count - 1]);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  }
  std::u16string P_view = Runtime::Global()->AddSource(std::move(P));
  std::vector<Handle<String>> names;
  // here will have some memory leak.
  AST* body_ast;
  if (P_view.size() > 0) {
    Parser parser(P_view);
    if (!parser.ParseFormalParameterList<GCFlag::CONST>(names)) {
      e = Error::SyntaxError(u"invalid parameter name");
      return Handle<JSValue>();
    }
  }
  std::u16string body_view = Runtime::Global()->AddSource(std::move(body));
  {
    Parser parser(body_view);
    body_ast = parser.ParseFunctionBody(Token::TK_EOS);
    if (body_ast->IsIllegal()) {
      e = Error::SyntaxError(u"failed to parse function body: " + body_ast->source());
      return Handle<JSValue>();
    }
  }
  Handle<EnvironmentRecord> scope = EnvironmentRecord::Global();
  bool strict = static_cast<ProgramOrFunctionBody*>(body_ast)->strict();
  Function* func_ast = new Function(Handle<String>(), std::move(names), body_ast, u"", 0, 0);
  if (strict) {
    // 13.1
    if (func_ast->params_have_duplicated()) {
      e = Error::SyntaxError(u"have duplicate parameter name in strict mode");
      return Handle<JSValue>();
    }
    if (func_ast->params_have_eval_or_arguments()) {
      e = Error::SyntaxError(u"param name cannot be eval or arguments in strict mode");
      return Handle<JSValue>();
    }
  }
  return FunctionObject::New(func_ast, scope, strict);
}

Handle<JSObject> Construct__NumberConstructor(
  Handle<Error>& e, Handle<NumberConstructor> O, std::vector<Handle<JSValue>> arguments
) {
  Handle<Number> js_num;
  if (arguments.size() == 0) {
    js_num = Number::Zero();
  } else {
    double num = ToNumber(e, arguments[0]);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    js_num = Number::New(num);
  }
  return NumberObject::New(js_num);
}

// 15.2.2 The Object Constructor
Handle<JSObject> Construct__ObjectConstructor(
  Handle<Error>& e, Handle<ObjectConstructor> O, std::vector<Handle<JSValue>> arguments
) {
  if (arguments.size() > 0) {  // 1
    Handle<JSValue> value = arguments[0];
    switch (value.val()->type()) {
      case Type::JS_OBJECT:
        // TODO(zhuzilin) deal with host object.
        return static_cast<Handle<JSObject>>(value);
      case Type::JS_LONG_STRING:
      case Type::JS_STRING:
      case Type::JS_BOOL:
      case Type::JS_NUMBER:
        return ToObject(e, value);
      default:
        break;
    }
  }
  ASSERT(arguments.size() == 0 || arguments[0].val()->IsNull() || arguments[0].val()->IsUndefined());
  Handle<JSObject> obj = Object::New();
  return obj;
}

Handle<JSObject> Construct__RegExpConstructor(
  Handle<Error>& e, Handle<RegExpConstructor> O, std::vector<Handle<JSValue>> arguments
) {
  Handle<String> P, F;
  if (arguments.size() == 0) {
    P = String::Empty();
  } else if (arguments[0].val()->IsRegExpObject()) {
    if (arguments.size() > 1 && !arguments[1].val()->IsUndefined()) {
      Handle<RegExpObject> R = static_cast<Handle<RegExpObject>>(arguments[0]);
      P = R.val()->pattern();
      F = R.val()->flag();
    } else {
      e = Error::TypeError(u"new RegExp called with RegExp object and flag.");
      return Handle<JSValue>();
    }
  } else {
    P = ::es::ToString(e, arguments[0]);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  }
  if (arguments.size() < 2 || arguments[1].val()->IsUndefined()) {
    F = String::Empty();
  } else {
    F = ::es::ToString(e, arguments[1]);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  }
  // Check is flag is valid
  std::unordered_map<char16_t, size_t> count;
  bool valid_flag = true;
  if (F.val()->IsArrayIndex()) {
    valid_flag = false;
  } else {
    for (auto c : F.val()->data()) {
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
  }
  if (!valid_flag) {
    e = Error::SyntaxError(u"invalid RegExp flag: " + F.val()->data());
    return Handle<JSValue>();
  }
  return RegExpObject::New(P, F);
}

// 15.5.2.1 String::New ( [ value ] )
Handle<JSObject> Construct__StringConstructor(
  Handle<Error>& e, Handle<StringConstructor> O, std::vector<Handle<JSValue>> arguments
) {
  if (arguments.size() == 0)
    return StringObject::New(String::Empty());
  Handle<String> str = ::es::ToString(e, arguments[0]);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  return StringObject::New(str);
}

}  // namespace es

#endif  // ES_TYPES_CONSTRUCT_IMPL_H
