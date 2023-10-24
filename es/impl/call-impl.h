#ifndef ES_TYPES_CALL_IMPL_H
#define ES_TYPES_CALL_IMPL_H

#include <es/types.h>

namespace es {

JSValue Call(
  JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments
) {
  if (O.IsFunctionObject()) {
    if (!function_object::from_bind(O)) {
      return Call__Function(e, O, this_arg, arguments);
    } else {
      return Call__BindFunction(e, O, this_arg, arguments);
    }
  } else if (O.IsConstructor()) {
    switch (O.type()) {
      case Type::OBJ_BOOL_CONSTRUCTOR:
        return Call__BoolConstructor(e, O, this_arg, arguments);
      case Type::OBJ_NUMBER_CONSTRUCTOR:
        return Call__NumberConstructor(e, O, this_arg, arguments);
      case Type::OBJ_OBJECT_CONSTRUCTOR:
        return Call__ObjectConstructor(e, O, this_arg, arguments);
      case Type::OBJ_REGEXP_CONSTRUCTOR:
        return Call__RegExpConstructor(e, O, this_arg, arguments);
      case Type::OBJ_STRING_CONSTRUCTOR:
        return Call__StringConstructor(e, O, this_arg, arguments);
      case Type::OBJ_FUNC_CONSTRUCTOR:
      case Type::OBJ_ARRAY_CONSTRUCTOR:
      case Type::OBJ_DATE_CONSTRUCTOR:
      case Type::OBJ_ERROR_CONSTRUCTOR:
        // 15.3.1.1 Function (p1, p2, … , pn, body)
        // 15.5.1.1 Array ( [ value ] )
        // 15.9.2.1 Date ( [ year [, month [, date [, hours [, minutes [, seconds [, ms ] ] ] ] ] ] ] )
        // 15.11.1.1 Error (message)
        return Call__Construct(e, O, this_arg, arguments);
      default:
        assert(false);
    }
  } else if (O.IsFunctionProto()) {
    return Call__FunctionProto(e, O, this_arg, arguments);
  } else if (O.IsGetterSetter()) {
    return Call__GetterSetter(e, O, this_arg, arguments);
  } else {
    return Call__Base(e, O, this_arg, arguments);
  }
}

JSValue Call__Base(
  JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments
) {
  inner_func callable = js_object::callable(O);
  ASSERT(O.IsCallable() && callable != nullptr);
  return callable(e, O, arguments);
}

JSValue Call__Construct(
  JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments
) {
  return Construct(e, O, arguments);
}

// 13.2.1 [[Call]]
JSValue Call__Function(
  JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments
) {
  ProgramOrFunctionBody* code = function_object::Code(O);
  TEST_LOG("enter function_object::Call ", code->source().substr(0, 100));
  EnterFunctionCode(e, O, code, this_arg, arguments, function_object::strict(O));
  if (unlikely(!error::IsOk(e))) return JSValue();

  Completion result;
  if (code != nullptr) {
    result = EvalProgram(code);
  }
  Runtime::Global()->PopContext();   // 3
  TEST_LOG("exit function_object::Call", code->source().substr(0, 100));
  switch (result.type()) {
    case Completion::RETURN:
      if (unlikely(log::Debugger::On()))
        log::PrintSource("exit function_object::Call RETURN");
      return result.value();
    case Completion::THROW: {
      if (unlikely(log::Debugger::On()))
        log::PrintSource("exit function_object::Call THROW");
      JSValue throw_value = result.value();
      if (throw_value.IsError()) {
        e = throw_value;
        if (unlikely(log::Debugger::On()))
          log::PrintSource("message: " + JSValue::ToString(e));
        return JSValue();
      }
      if (unlikely(log::Debugger::On()))
        log::PrintSource("message: " + JSValue::ToString(throw_value));
      e = error::NativeError(throw_value);
      return JSValue();
    }
    default:
      if (unlikely(log::Debugger::On()))
        log::PrintSource("exit function_object::Call NORMAL");
      ASSERT(result.type() == Completion::NORMAL);
      return undefined::New();
  }
}

JSValue Call__BindFunction(
  JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> extra_args
) {
  TEST_LOG("enter bind_function_object::Call");
  JSValue bound_args = bind_function_object::BoundArgs(O);
  JSValue target_function = bind_function_object::TargetFunction(O);

  std::vector<JSValue> args;
  for (size_t i = 0; i < fixed_array::size(bound_args); i++) {
    args.emplace_back(fixed_array::Get(bound_args, i));
  }
  args.insert(args.end(), extra_args.begin(), extra_args.end());
  return Call(e, target_function, bind_function_object::BoundThis(O), args);
}

// 15.3.4 The Function prototype object is itself a Function object (its [[Class]] is "Function") that,
// when invoked, accepts any arguments and returns undefined.
JSValue Call__FunctionProto(
  JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments
) {
  return undefined::New();
}

// 15.6.1.1 Boolean (value)
JSValue Call__BoolConstructor(
  JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments
) {
  bool b;
  if (arguments.size() == 0)
    b = ToBoolean(undefined::New());
  else
    b = ToBoolean(arguments[0]);
  return boolean::New(b);
}

// 15.7.1.1 Number ( [ value ] )
JSValue Call__NumberConstructor(
  JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments
) {
  if (arguments.size() == 0) {
    return number::Zero();
  } else {
    double num = ToNumber(e, arguments[0]);
    if (unlikely(!error::IsOk(e))) return JSValue();
    return number::New(num);
  }
}

// 15.2.1.1 Object ( [ value ] )
JSValue Call__ObjectConstructor(
  JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments
) {
  if (arguments.size() == 0 || arguments[0].IsNull() || arguments[0].IsUndefined())
    return Construct(e, O, arguments);
  return ToObject(e, arguments[0]);
}

// 15.10.4.1 new RegExp(pattern, flags)
JSValue Call__RegExpConstructor(
  JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments
) {
  if (arguments.size() == 0) {
    e = error::TypeError(u"RegExp called with 0 parameters");
    return JSValue();
  }
  if ((arguments.size() == 1 || arguments[1].IsUndefined()) && arguments[0].IsRegExpObject()) {
      return arguments[0];
  }
  return Construct(e, O, arguments);
}

// 15.5.1.1 String ( [ value ] )
JSValue Call__StringConstructor(
  JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments
) {
  if (arguments.size() == 0)
    return string::Empty();
  return ::es::ToString(e, arguments[0]);
}

JSValue Call__GetterSetter(
  JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments
) {
  ASSERT(arguments.size() < 2);
  JSValue ref = getter_setter::ref(O);
  if (arguments.size() == 0) {
    return GetValue(e, ref);
  } else {
    PutValue(e, ref, arguments[0]);
    return JSValue();
  }
}

}  // namespace es

#endif  // ES_TYPES_CALL_IMPL_H
