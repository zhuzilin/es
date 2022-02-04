#ifndef ES_TYPES_CALL_IMPL_H
#define ES_TYPES_CALL_IMPL_H

#include <es/types/object.h>
#include <es/types/builtin/arguments_object.h>
#include <es/types/builtin/array_object.h>
#include <es/types/builtin/bool_object.h>
#include <es/types/builtin/date_object.h>
#include <es/types/builtin/error_object.h>
#include <es/types/builtin/function_object.h>
#include <es/types/builtin/math_object.h>
#include <es/types/builtin/number_object.h>
#include <es/types/builtin/object_object.h>
#include <es/types/builtin/regexp_object.h>
#include <es/types/builtin/string_object.h>

namespace es {

Handle<JSValue> Call(
  Error* e, Handle<JSObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  if (O.val()->IsFunctionObject()) {
    Handle<FunctionObject> F = static_cast<Handle<FunctionObject>>(O);
    if (!F.val()->from_bind()) {
      return Call__Function(e, F, this_arg, arguments);
    } else {
      return Call__BindFunction(e, static_cast<Handle<BindFunctionObject>>(O), this_arg, arguments);
    }
  } else if (O.val()->IsConstructor()) {
    switch (O.val()->obj_type()) {
      case JSObject::OBJ_BOOL_CONSTRUCTOR:
        return Call__BoolConstructor(e, static_cast<Handle<BoolConstructor>>(O), this_arg, arguments);
      case JSObject::OBJ_NUMBER_CONSTRUCTOR:
        return Call__NumberConstructor(e, static_cast<Handle<NumberConstructor>>(O), this_arg, arguments);
      case JSObject::OBJ_OBJECT_CONSTRUCTOR:
        return Call__ObjectConstructor(e, static_cast<Handle<ObjectConstructor>>(O), this_arg, arguments);
      case JSObject::OBJ_REGEXP_CONSTRUCTOR:
        return Call__RegExpConstructor(e, static_cast<Handle<RegExpConstructor>>(O), this_arg, arguments);
      case JSObject::OBJ_STRING_CONSTRUCTOR:
        return Call__StringConstructor(e, static_cast<Handle<StringConstructor>>(O), this_arg, arguments);
      case JSObject::OBJ_FUNC_CONSTRUCTOR:
      case JSObject::OBJ_ARRAY_CONSTRUCTOR:
      case JSObject::OBJ_DATE_CONSTRUCTOR:
      case JSObject::OBJ_ERROR_CONSTRUCTOR:
        // 15.3.1.1 Function (p1, p2, … , pn, body)
        // 15.5.1.1 Array ( [ value ] )
        // 15.9.2.1 Date ( [ year [, month [, date [, hours [, minutes [, seconds [, ms ] ] ] ] ] ] ] )
        // 15.11.1.1 Error (message)
        return Call__Construct(e, O, this_arg, arguments);
      default:
        assert(false);
    }
  } else if (O.val()->IsFunctionProto()) {
    return Call__FunctionProto(e, static_cast<Handle<FunctionProto>>(O), this_arg, arguments);
  } else {
    return Call__Base(e, O, this_arg, arguments);
  }
}

Handle<JSValue> Call__Base(
  Error* e, Handle<JSObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  inner_func callable = O.val()->callable();
  assert(O.val()->IsCallable() && callable != nullptr);
  return callable(e, O, arguments);
}

Handle<JSValue> Call__Construct(
  Error* e, Handle<JSObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  return Construct(e, O, arguments);
}

// 13.2.1 [[Call]]
Handle<JSValue> Call__Function(
  Error* e, Handle<FunctionObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  ProgramOrFunctionBody* code = O.val()->Code();
  log::PrintSource("enter FunctionObject::Call ", code->source().substr(0, 100));
  EnterFunctionCode(e, O, code, this_arg, arguments, O.val()->strict());
  if (!e->IsOk()) return Handle<JSValue>();

  Completion result;
  if (code != nullptr) {
    result = EvalProgram(code);
  }
  Runtime::Global()->PopContext();   // 3

  log::PrintSource("exit FunctionObject::Call", code->source().substr(0, 100));
  switch (result.type()) {
    case Completion::RETURN:
      log::PrintSource("exit FunctionObject::Call RETURN");
      return result.value();
    case Completion::THROW: {
      log::PrintSource("exit FunctionObject::Call THROW");
      Handle<JSValue> throw_value = result.value();
      if (throw_value.val()->IsErrorObject()) {
        *e = *(static_cast<Handle<ErrorObject>>(throw_value).val()->e());
        log::PrintSource("message: ", e->message());
        return Handle<JSValue>();
      }
      std::u16string message = ToU16String(e, throw_value);
      log::PrintSource("message: ", message);
      *e = *Error::NativeError(message);
      return Handle<JSValue>();
    }
    default:
      log::PrintSource("exit FunctionObject::Call NORMAL");
      assert(result.type() == Completion::NORMAL);
      return Undefined::Instance();
  }
}

Handle<JSValue> Call__BindFunction(
  Error* e, Handle<BindFunctionObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> extra_args
) {
  log::PrintSource("enter BindFunctionObject::Call");
  Handle<FixedArray<JSValue>> bound_args = O.val()->BoundArgs();
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
  Error* e, Handle<FunctionProto> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  return Undefined::Instance();
}

// 15.6.1.1 Boolean (value)
Handle<JSValue> Call__BoolConstructor(
  Error* e, Handle<BoolConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
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
  Error* e, Handle<NumberConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  Handle<Number> js_num;
  if (arguments.size() == 0) {
    js_num = Number::Zero();
  } else {
    double num = ToNumber(e, arguments[0]);
    if (!e->IsOk()) return Handle<JSValue>();
    js_num = Number::New(num);
  }
  return js_num;
}

// 15.2.1.1 Object ( [ value ] )
Handle<JSValue> Call__ObjectConstructor(
  Error* e, Handle<ObjectConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  if (arguments.size() == 0 || arguments[0].val()->IsNull() || arguments[0].val()->IsUndefined())
    return Construct(e, O, arguments);
  return ToObject(e, arguments[0]);
}

// 15.10.4.1 new RegExp(pattern, flags)
Handle<JSValue> Call__RegExpConstructor(
  Error* e, Handle<RegExpConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  if (arguments.size() == 0) {
    *e = *Error::TypeError(u"RegExp called with 0 parameters");
    return Handle<JSValue>();
  }
  if ((arguments.size() == 1 || arguments[1].val()->IsUndefined()) && arguments[0].val()->IsRegExpObject()) {
      return arguments[0];
  }
  return Construct(e, O, arguments);
}

// 15.5.1.1 String ( [ value ] )
Handle<JSValue> Call__StringConstructor(
  Error* e, Handle<StringConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments
) {
  if (arguments.size() == 0)
    return String::Empty();
  return ::es::ToString(e, arguments[0]);
}

}  // namespace es

#endif  // ES_TYPES_CALL_IMPL_H