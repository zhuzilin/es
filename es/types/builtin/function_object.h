#ifndef ES_TYPES_BUILTIN_FUNCTION_OBJECT
#define ES_TYPES_BUILTIN_FUNCTION_OBJECT

#include <es/parser/ast.h>
#include <es/types/object.h>
#include <es/types/builtin/object_object.h>
#include <es/types/builtin/error_object.h>
#include <es/types/lexical_environment.h>
#include <es/runtime.h>
#include <es/types/completion.h>
#include <es/utils/fixed_array.h>

namespace es {

double ToNumber(Handle<Error>& e, Handle<JSValue> input);
Handle<String> NumberToString(double m);
Completion EvalProgram(AST* ast);

class FunctionObject;
void EnterFunctionCode(
  Handle<Error>& e, Handle<FunctionObject> func, ProgramOrFunctionBody* body,
  Handle<JSValue> this_arg, std::vector<Handle<JSValue>> args, bool strict,
  Handle<EnvironmentRecord> local_env
);

class FunctionProto : public JSObject {
 public:
  static Handle<FunctionProto> Instance() {
    static Handle<FunctionProto> singleton = FunctionProto::New<GCFlag::SINGLE>();
    return singleton;
  }

  static Handle<JSValue> toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  // 15.3.4.3 Function.prototype.apply (thisArg, argArray)
  static Handle<JSValue> apply(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    if (!val.val()->IsObject()) {
      e = Error::TypeError(u"Function.prototype.apply called on non-object");
      return Handle<JSValue>();
    }
    Handle<JSObject> func = static_cast<Handle<JSObject>>(val);
    if (!func.val()->IsCallable()) {
      e = Error::TypeError(u"Function.prototype.apply called on non-callable");
      return Handle<JSValue>();
    }
    if (vals.size() == 0) {
      return Call(e, func, Undefined::Instance(), {});
    }
    if (vals.size() < 2 || vals[1].val()->IsNull() || vals[1].val()->IsUndefined()) {  // 2
      return Call(e, func, vals[0], {});
    }
    if (!vals[1].val()->IsObject()) {  // 3
      e = Error::TypeError(u"Function.prototype.apply's argument is non-object");
      return Handle<JSValue>();
    }
    Handle<JSObject> arg_array = static_cast<Handle<JSObject>>(vals[1]);
    Handle<JSValue> len = Get(e, arg_array, String::Length());
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    size_t n = ToNumber(e, len);
    std::vector<Handle<JSValue>> arg_list;  // 6
    size_t index = 0;  // 7
    while (index < n) {  // 8
      Handle<String> index_name = ::es::NumberToString(index);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
      Handle<JSValue> next_arg = Get(e, arg_array, index_name);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
      arg_list.emplace_back(next_arg);
      index++;
    }
    return Call(e, func, vals[0], arg_list);
  }

  static Handle<JSValue> call(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    if (!val.val()->IsObject()) {
      e = Error::TypeError(u"Function.prototype.call called on non-object");
      return Handle<JSValue>();
    }
    Handle<JSObject> func = static_cast<Handle<JSObject>>(val);
    if (!func.val()->IsCallable()) {
      e = Error::TypeError(u"Function.prototype.call called on non-callable");
      return Handle<JSValue>();
    }
    if (vals.size()) {
      Handle<JSValue> this_arg = vals[0];
      return Call(e, func, this_arg, std::vector<Handle<JSValue>>(vals.begin() + 1, vals.end()));
    } else {
      return Call(e, func, Undefined::Instance(), {});
    }
  }

  static Handle<JSValue> bind(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

 private:
  template<flag_t flag>
  static Handle<FunctionProto> New() {
    Handle<JSObject> jsobj = JSObject::New<0, flag>(
      CLASS_FUNCTION, true, Handle<JSValue>(), false, true, nullptr);

    jsobj.val()->SetType(OBJ_FUNC_PROTO);
    return Handle<FunctionProto>(jsobj);
  }
};

class FunctionObject : public JSObject {
 public:
  static Handle<FunctionObject> New(
    Function* func_ast, Handle<EnvironmentRecord> scope, bool strict, size_t size = 0
  ) {
    Handle<JSObject> jsobj = JSObject::New(
      CLASS_FUNCTION, true, Handle<JSValue>(), true, true, nullptr,
      kFunctionObjectOffset - kJSObjectOffset + size
    );
    if (func_ast != nullptr) {
      ASSERT(func_ast->type() == AST::AST_FUNC);
      strict |= func_ast->body()->strict();
      SET_VALUE(jsobj.val(), kCodeOffset, func_ast, Function*);
    } else {
      SET_VALUE(jsobj.val(), kCodeOffset, nullptr, ProgramOrFunctionBody*);
    }
    SET_HANDLE_VALUE(jsobj.val(), kScopeOffset, scope, EnvironmentRecord);
    jsobj.val()->h_.strict = strict;
    jsobj.val()->SetType(OBJ_FUNC);

    Handle<FunctionObject> obj(jsobj);
    // 13.2 Creating Function Objects
    obj.val()->SetPrototype(FunctionProto::Instance());
    // Whether the function is made from bind.
    if (func_ast != nullptr) {
      AddValueProperty(obj, String::Length(), Number::New(func_ast->params().size()), false, false, false);  // 14 & 15
      Handle<JSObject> proto = Object::New();  // 16
      AddValueProperty(proto, String::Constructor(), obj, true, false, true);
      // 15.3.5.2 prototype
      AddValueProperty(obj, String::Prototype(), proto, true, false, false);
      if (strict) {
        // TODO(zhuzilin) thrower
      }
    }
    return obj;
  }

  Handle<EnvironmentRecord> Scope() {
    ASSERT(!from_bind());
    return READ_HANDLE_VALUE(this, kScopeOffset, EnvironmentRecord);
  };
  const std::vector<Handle<String>>& FormalParameters() {
    ASSERT(!from_bind());
    return Code()->params();
  };
  Function* Code() {
    ASSERT(!from_bind());
    return READ_VALUE(this, kCodeOffset, Function*);
  }
  bool strict() {
    ASSERT(!from_bind());
    return h_.strict;
  }
  bool from_bind() { return type() == OBJ_BIND_FUNC; }

 public:
  static constexpr size_t kCodeOffset = kJSObjectOffset;
  static constexpr size_t kScopeOffset = kCodeOffset + kPtrSize;
  static constexpr size_t kFunctionObjectOffset = kScopeOffset + kPtrSize;
};

class BindFunctionObject : public FunctionObject {
 public:
  static Handle<BindFunctionObject> New(
    Handle<JSObject> target_function, Handle<JSValue> bound_this, std::vector<Handle<JSValue>> bound_args
  ) {
    Handle<FunctionObject> func = FunctionObject::New(
      nullptr, Handle<JSValue>(), Runtime::TopContext().strict(),
      kBindFunctionObjectOffset - kFunctionObjectOffset);

    SET_HANDLE_VALUE(func.val(), kTargetFunctionOffset, target_function, JSObject);
    SET_HANDLE_VALUE(func.val(), kBoundThisOffset, bound_this, JSValue);
    SET_HANDLE_VALUE(func.val(), kBoundArgsOffset, FixedArray::New<JSValue>(bound_args), FixedArray);
    func.val()->SetType(OBJ_BIND_FUNC);
    return Handle<BindFunctionObject>(func);
  }

  Handle<JSObject> TargetFunction() { return READ_HANDLE_VALUE(this, kTargetFunctionOffset, JSObject); }
  Handle<JSValue> BoundThis() { return READ_HANDLE_VALUE(this, kBoundThisOffset, JSValue); }
  Handle<FixedArray> BoundArgs() { return READ_HANDLE_VALUE(this, kBoundArgsOffset, FixedArray); }

 public:
  static constexpr size_t kTargetFunctionOffset = kFunctionObjectOffset;
  static constexpr size_t kBoundThisOffset = kTargetFunctionOffset + kPtrSize;
  static constexpr size_t kBoundArgsOffset = kBoundThisOffset + kPtrSize;
  static constexpr size_t kBindFunctionObjectOffset = kBoundArgsOffset + kPtrSize;
};

class FunctionConstructor : public JSObject {
 public:
  static Handle<FunctionConstructor> Instance() {
    static Handle<FunctionConstructor> singleton = FunctionConstructor::New<GCFlag::SINGLE>();
    return singleton;
  }

  static Handle<JSValue> toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"function Function() { [native code] }");
  }

 private:
  template<flag_t flag>
  static Handle<FunctionConstructor> New() {
    Handle<JSObject> jsobj = JSObject::New<0, flag>(
      CLASS_FUNCTION, true, Handle<JSValue>(), true, true, nullptr);

    jsobj.val()->SetType(OBJ_FUNC_CONSTRUCTOR);
    return Handle<FunctionConstructor>(jsobj);
  }
};

Handle<JSValue> FunctionProto::toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
  Handle<JSValue> val = Runtime::TopValue();
  if (!val.val()->IsObject()) {
    e = Error::TypeError(u"Function.prototype.toString called on non-object");
    return Handle<JSValue>();
  }
  if (!val.val()->IsFunctionObject()) {
    e = Error::TypeError(u"Function.prototype.toString called on non-function");
    return Handle<JSValue>();
  }
  Handle<FunctionObject> func = static_cast<Handle<FunctionObject>>(val);
  std::u16string str = u"function (";
  const std::vector<Handle<String>>& params = func.val()->FormalParameters();
  if (params.size() > 0) {
    str += params[0].val()->data();
    for (size_t i = 1; i < params.size(); i++) {
      str += u"," + params[i].val()->data();
    }
  }
  str += u")";
  return String::New(str);
}

// 15.3.4.5 Function.prototype.bind (thisArg [, arg1 [, arg2, …]])
Handle<JSValue> FunctionProto::bind(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
  Handle<JSValue> val = Runtime::TopValue();
  if (!val.val()->IsCallable()) {
    e = Error::TypeError(u"Function.prototype.call called on non-callable");
    return Handle<JSValue>();
  }
  Handle<JSObject> target = static_cast<Handle<JSObject>>(val);
  Handle<JSValue> this_arg_for_F = Undefined::Instance();
  if (vals.size() > 0)
    this_arg_for_F = vals[0];
  std::vector<Handle<JSValue>> A;
  if (vals.size() > 1) {
    A = std::vector<Handle<JSValue>>(vals.begin() + 1, vals.end());
  }
  Handle<BindFunctionObject> F = BindFunctionObject::New(target, this_arg_for_F, A);
  size_t len = 0;
  if (target.val()->Class() == CLASS_FUNCTION) {
    size_t L = ToNumber(e, Get(e, target, String::Length()));
    if (L - A.size() > 0)
      len = L - A.size();
  }
  AddValueProperty(F, String::Length(), Number::New(len), false, false, false);
  // 19
  // TODO(zhuzilin) thrower
  return F;
}

Handle<FunctionObject> InstantiateFunctionDeclaration(Handle<Error>& e, Function* func_ast) {
    ASSERT(func_ast->is_named());
    auto body = func_ast->body();
    Handle<EnvironmentRecord> env_rec = NewDeclarativeEnvironment(  // 1, 2
      Runtime::TopLexicalEnv(), func_ast->num_decls()
    );
    Handle<String> identifier = func_ast->name();
    bool strict = body->strict() || Runtime::TopContext().strict();
    if (strict) {
      // 13.1
      if (func_ast->params_have_duplicated()) {
        e = Error::SyntaxError(u"have duplicate parameter name in strict mode");
        return Handle<JSValue>();
      }
      if (func_ast->name_is_eval_or_arguments()) {
        e = Error::SyntaxError(u"parameter name cannot be eval or arguments in strict mode");
        return Handle<JSValue>();
      }
      if (func_ast->name_is_eval_or_arguments()) {
        e = Error::SyntaxError(u"function name cannot be eval or arguments in strict mode");
        return Handle<JSValue>();
      }
      for (VarDecl* d : body->var_decls()) {
        if (d->is_strict_future()) {
          e = Error::SyntaxError(u"Unexpected future reserved word " + d->ident().val()->data() + u" in strict mode");
          return Handle<JSValue>();
        }
        if (d->is_eval_or_arguments()) {
          e = Error::SyntaxError(u"Unexpected eval or arguments in strict mode");
          return Handle<JSValue>();
        }
      }
    }
    Handle<FunctionObject> closure = FunctionObject::New(func_ast, env_rec, strict);  // 4
    CreateAndInitializeImmutableBinding(env_rec, identifier, closure);  // 5
    return closure;  // 6
}

Handle<JSValue> EvalFunction(Handle<Error>& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_FUNC);
  Function* func_ast = static_cast<Function*>(ast);

  if (func_ast->is_named()) {
    return InstantiateFunctionDeclaration(e, func_ast);
  } else {
    auto body = func_ast->body();
    bool strict = body->strict() || Runtime::TopContext().strict();
    if (strict) {
      // 13.1
      if (func_ast->params_have_duplicated()) {
        e = Error::SyntaxError(u"have duplicate parameter name in strict mode");
        return Handle<JSValue>();
      }
      if (func_ast->params_have_eval_or_arguments()) {
        e = Error::SyntaxError(u"parameter name cannot be eval or arguments in strict mode");
        return Handle<JSValue>();
      }
    }
    return FunctionObject::New(func_ast, Runtime::TopLexicalEnv(), strict);
  }
}

// TODO(zhuzilin) move this function to a better place
void AddFuncProperty(
  Handle<JSObject> O, Handle<String> name, inner_func callable, bool writable,
  bool enumerable, bool configurable
) {
  Handle<JSObject> value = JSObject::New<0>(
    CLASS_INTERNAL_FUNCTION, false, Handle<JSValue>(), false, true, callable);
  value.val()->SetType(Type::OBJ_INNER_FUNC);
  value.val()->SetPrototype(FunctionProto::Instance());
  AddValueProperty(O, name, value, writable, enumerable, configurable);
}

Handle<JSValue> Get__Function(Handle<Error>& e, Handle<FunctionObject> O, Handle<String> P);
bool HasInstance__Function(Handle<Error>& e, Handle<FunctionObject> O, Handle<JSValue> V);
bool HasInstance__BindFunction(Handle<Error>& e, Handle<BindFunctionObject> O, Handle<JSValue> V);

Handle<JSValue> Call__Function(Handle<Error>& e, Handle<FunctionObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments);
Handle<JSValue> Call__BindFunction(Handle<Error>& e, Handle<BindFunctionObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> extra_args);
Handle<JSValue> Call__FunctionProto(Handle<Error>& e, Handle<FunctionProto> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments);

Handle<JSObject> Construct__Function(Handle<Error>& e, Handle<FunctionObject> O,std::vector<Handle<JSValue>> arguments);
Handle<JSObject> Construct__BindFunction(Handle<Error>& e, Handle<BindFunctionObject> O, std::vector<Handle<JSValue>> extra_args);
Handle<JSObject> Construct__FunctionConstructor(Handle<Error>& e, Handle<FunctionConstructor> O, std::vector<Handle<JSValue>> arguments);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_FUNCTION_OBJECT