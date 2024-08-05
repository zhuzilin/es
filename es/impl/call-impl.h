#ifndef ES_TYPES_CALL_IMPL_H
#define ES_TYPES_CALL_IMPL_H

#include <es/types.h>

namespace es {

Handle<JSValue> Call(
  Handle<Error>& e, Handle<JSValue> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  if (O.val()->IsFunctionObject()) {
    Handle<FunctionObject> F = static_cast<Handle<FunctionObject>>(O);
    if (!F.val()->from_bind()) {
      return Call__Function(e, F, this_arg, std::move(arguments));
    } else {
      return Call__BindFunction(e, static_cast<Handle<BindFunctionObject>>(O), this_arg, std::move(arguments));
    }
  } else if (O.val()->IsConstructor()) {
    switch (O.val()->type()) {
      case Type::OBJ_BOOL_CONSTRUCTOR:
        return Call__BoolConstructor(e, static_cast<Handle<BoolConstructor>>(O), this_arg, std::move(arguments));
      case Type::OBJ_NUMBER_CONSTRUCTOR:
        return Call__NumberConstructor(e, static_cast<Handle<NumberConstructor>>(O), this_arg, std::move(arguments));
      case Type::OBJ_OBJECT_CONSTRUCTOR:
        return Call__ObjectConstructor(e, static_cast<Handle<ObjectConstructor>>(O), this_arg, std::move(arguments));
      case Type::OBJ_REGEXP_CONSTRUCTOR:
        return Call__RegExpConstructor(e, static_cast<Handle<RegExpConstructor>>(O), this_arg, std::move(arguments));
      case Type::OBJ_STRING_CONSTRUCTOR:
        return Call__StringConstructor(e, static_cast<Handle<StringConstructor>>(O), this_arg, std::move(arguments));
      case Type::OBJ_FUNC_CONSTRUCTOR:
      case Type::OBJ_ARRAY_CONSTRUCTOR:
      case Type::OBJ_DATE_CONSTRUCTOR:
      case Type::OBJ_ERROR_CONSTRUCTOR:
        // 15.3.1.1 Function (p1, p2, … , pn, body)
        // 15.5.1.1 Array ( [ value ] )
        // 15.9.2.1 Date ( [ year [, month [, date [, hours [, minutes [, seconds [, ms ] ] ] ] ] ] ] )
        // 15.11.1.1 Error (message)
        return Call__Construct(e, O, this_arg, std::move(arguments));
      default:
        assert(false);
    }
  } else if (O.val()->IsFunctionProto()) {
    return Call__FunctionProto(e, static_cast<Handle<FunctionProto>>(O), this_arg, std::move(arguments));
  } else if (O.val()->IsGetterSetter()) {
    return Call__GetterSetter(e, static_cast<Handle<GetterSetter>>(O), this_arg, std::move(arguments));
  } else {
    return Call__Base(e, O, this_arg, std::move(arguments));
  }
}

Handle<JSValue> Call__Base(
  Handle<Error>& e, Handle<JSObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  inner_func callable = O.val()->callable();
  ASSERT(O.val()->IsCallable() && callable != nullptr);
  return callable(e, O, std::move(arguments));
}

Handle<JSValue> Call__Construct(
  Handle<Error>& e, Handle<JSObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  return Construct(e, O, std::move(arguments));
}

// 13.2.1 [[Call]]
Handle<JSValue> Call__Function(
  Handle<Error>& e, Handle<FunctionObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  ProgramOrFunctionBody* const code = O.val()->Code()->body();
  TEST_LOG("\033[1;32menter FunctionObject::Call\033[0m\n", code->source(), "\n");
  Handle<FunctionObject> func = static_cast<Handle<FunctionObject>>(O);
  ASSERT(func.val()->IsFunctionObject());
  Handle<EnvironmentRecord> local_env = ExtracGC::TryPopFunctionEnvRec(code);
  if (local_env.IsNullptr()) {
    local_env = NewDeclarativeEnvironment(func.val()->Scope(), O.val()->Code()->num_decls());
  } else {
    local_env.val()->SetOuter(func.val()->Scope());
  }
  EnterFunctionCode(e, func, code, this_arg, arguments, O.val()->strict(), local_env);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();

  Completion result;
  if (code != nullptr) {
    result = EvalProgram(code);
  }
  Runtime::Global()->PopContext();   // 3
  ExtracGC::TrySaveFunctionEnvRec(code, local_env);
  TEST_LOG("\033[1;32mexit FunctionObject::Call\033[0m");
  switch (result.type()) {
    case Completion::RETURN:
      if (unlikely(log::Debugger::On()))
        log::PrintSource("\033[1;32mexit FunctionObject::Call RETURN\033[0m");
      return result.value();
    case Completion::THROW: {
      if (unlikely(log::Debugger::On()))
        log::PrintSource("\033[1;31mexit FunctionObject::Call THROW\033[0m");
      Handle<JSValue> throw_value = result.value();
      if (throw_value.val()->IsError()) {
        e = throw_value;
        if (unlikely(log::Debugger::On()))
          log::PrintSource("message: " + e.ToString());
        return Handle<JSValue>();
      }
      if (unlikely(log::Debugger::On()))
        log::PrintSource("message: " + throw_value.ToString());
      e = Error::NativeError(throw_value);
      return Handle<JSValue>();
    }
    default:
      if (unlikely(log::Debugger::On()))
        log::PrintSource("\033[1;32mexit FunctionObject::Call NORMAL\033[0m");
      ASSERT(result.type() == Completion::NORMAL);
      return Undefined::Instance();
  }
}

Handle<JSValue> Call__BindFunction(
  Handle<Error>& e, Handle<BindFunctionObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> extra_args
) {
  TEST_LOG("\033[2menter\033[0m BindFunctionObject::Call");
  Handle<FixedArray> bound_args = O.val()->BoundArgs();
  Handle<JSObject> target_function = O.val()->TargetFunction();

  std::vector<Handle<JSValue>> args;
  for (size_t i = 0; i < bound_args.val()->size(); i++) {
    args.emplace_back(bound_args.val()->Get(i));
  }
  args.insert(args.end(), extra_args.begin(), extra_args.end());
  return Call(e, target_function, O.val()->BoundThis(), args);
}

// 15.3.4 The Function prototype object is itself a Function object (its [[Class]] is "Function") that,
// when invoked, accepts any arguments and returns undefined.
Handle<JSValue> Call__FunctionProto(
  Handle<Error>& e, Handle<FunctionProto> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  return Undefined::Instance();
}

// 15.6.1.1 Boolean (value)
Handle<JSValue> Call__BoolConstructor(
  Handle<Error>& e, Handle<BoolConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  bool b;
  if (arguments.size() == 0)
    b = ToBoolean(Undefined::Instance());
  else
    b = ToBoolean(arguments[0]);
  return Bool::Wrap(b);
}

// 15.7.1.1 Number ( [ value ] )
Handle<JSValue> Call__NumberConstructor(
  Handle<Error>& e, Handle<NumberConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  Handle<Number> js_num;
  if (arguments.size() == 0) {
    js_num = Number::Zero();
  } else {
    double num = ToNumber(e, arguments[0]);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    js_num = Number::New(num);
  }
  return js_num;
}

// 15.2.1.1 Object ( [ value ] )
Handle<JSValue> Call__ObjectConstructor(
  Handle<Error>& e, Handle<ObjectConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  if (arguments.size() == 0 || arguments[0].val()->IsNull() || arguments[0].val()->IsUndefined())
    return Construct(e, O, std::move(arguments));
  return ToObject(e, arguments[0]);
}

// 15.10.4.1 new RegExp(pattern, flags)
Handle<JSValue> Call__RegExpConstructor(
  Handle<Error>& e, Handle<RegExpConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  if (arguments.size() == 0) {
    e = Error::TypeError(u"RegExp called with 0 parameters");
    return Handle<JSValue>();
  }
  if ((arguments.size() == 1 || arguments[1].val()->IsUndefined()) && arguments[0].val()->IsRegExpObject()) {
      return arguments[0];
  }
  return Construct(e, O, std::move(arguments));
}

// 15.5.1.1 String ( [ value ] )
Handle<JSValue> Call__StringConstructor(
  Handle<Error>& e, Handle<StringConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  if (arguments.size() == 0)
    return String::Empty();
  return ::es::ToString(e, arguments[0]);
}

Handle<JSValue> Call__GetterSetter(
  Handle<Error>& e, Handle<GetterSetter> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  ASSERT(arguments.size() < 2);
  Handle<GetterSetter> gs = static_cast<Handle<GetterSetter>>(O);
  Handle<JSValue> base = gs.val()->GetBase();
  Handle<String> name = gs.val()->GetReferencedName();
  bool is_strict = gs.val()->IsStrictReference();
  if (arguments.size() == 0) {
    return GetValueEnvRec(e, base, name, is_strict);
  } else {
    PutValueEnvRec(e, base, name, is_strict, arguments[0]);
    return Handle<JSValue>();
  }
}

}  // namespace es

#endif  // ES_TYPES_CALL_IMPL_H
