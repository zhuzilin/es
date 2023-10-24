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

double ToNumber(JSValue& e, JSValue input);
JSValue NumberToString(double m);
Completion EvalProgram(AST* ast);

void EnterFunctionCode(
  JSValue& e, JSValue F, ProgramOrFunctionBody* body,
  JSValue this_arg, std::vector<JSValue> args, bool strict
);

namespace function_proto {

inline JSValue New(flag_t flag) {
  JSValue jsobj = js_object::New(
    u"Function", true, JSValue(), false, true, nullptr, 0, flag);

  jsobj.SetType(OBJ_FUNC_PROTO);
  return jsobj;
}

inline JSValue Instance() {
  static JSValue singleton = function_proto::New(GCFlag::SINGLE);
  return singleton;
}

inline JSValue toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals);

// 15.3.4.3 Function.prototype.apply (thisArg, argArray)
inline JSValue apply(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue val = Runtime::TopValue();
  if (!val.IsObject()) {
    e = error::TypeError(u"Function.prototype.apply called on non-object");
    return JSValue();
  }
  if (!val.IsCallable()) {
    e = error::TypeError(u"Function.prototype.apply called on non-callable");
    return JSValue();
  }
  if (vals.size() == 0) {
    return Call(e, val, undefined::New(), {});
  }
  if (vals.size() < 2 || vals[1].IsNull() || vals[1].IsUndefined()) {  // 2
    return Call(e, val, vals[0], {});
  }
  if (!vals[1].IsObject()) {  // 3
    e = error::TypeError(u"Function.prototype.apply's argument is non-object");
    return JSValue();
  }
  JSValue arg_array = vals[1];
  JSValue len = Get(e, arg_array, string::Length());
  if (unlikely(!error::IsOk(e))) return JSValue();
  size_t n = ToNumber(e, len);
  std::vector<JSValue> arg_list;  // 6
  size_t index = 0;  // 7
  while (index < n) {  // 8
    JSValue index_name = ::es::NumberToString(index);
    if (unlikely(!error::IsOk(e))) return JSValue();
    JSValue next_arg = Get(e, arg_array, index_name);
    if (unlikely(!error::IsOk(e))) return JSValue();
    arg_list.emplace_back(next_arg);
    index++;
  }
  return Call(e, val, vals[0], arg_list);
}

inline JSValue call(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue val = Runtime::TopValue();
  if (!val.IsObject()) {
    e = error::TypeError(u"Function.prototype.call called on non-object");
    return JSValue();
  }
  if (!val.IsCallable()) {
    e = error::TypeError(u"Function.prototype.call called on non-callable");
    return JSValue();
  }
  if (vals.size()) {
    JSValue this_arg = vals[0];
    return Call(e, val, this_arg, std::vector<JSValue>(vals.begin() + 1, vals.end()));
  } else {
    return Call(e, val, undefined::New(), {});
  }
}

inline JSValue bind(JSValue& e, JSValue this_arg, std::vector<JSValue> vals);
}  // namespace function_proto

namespace function_object {

constexpr size_t kFormalParametersOffset = js_object::kJSObjectOffset;
constexpr size_t kCodeOffset = kFormalParametersOffset + sizeof(JSValue);
// this should be kPtrSize use 16 for alignment
constexpr size_t kScopeOffset = kCodeOffset + sizeof(JSValue);
constexpr size_t kStrictOffset = kScopeOffset + sizeof(JSValue);
constexpr size_t kFunctionObjectOffset = kStrictOffset + kBoolSize;

inline JSValue New(
  std::vector<std::u16string> names, AST* body, JSValue scope,
  bool strict, size_t size = 0
) {
  JSValue jsobj = js_object::New(
    u"Function", true, JSValue(), true, true, nullptr,
    kFunctionObjectOffset - js_object::kJSObjectOffset + size
  );
  std::vector<JSValue> name_handles(names.size());
  for (size_t i = 0; i < names.size(); i++) {
    name_handles[i] = string::New(names[i]);
  }
  JSValue formal_parameter = fixed_array::New(name_handles);

  SET_JSVALUE(jsobj.handle().val(), kFormalParametersOffset, formal_parameter);
  if (body != nullptr) {
    ASSERT(body->type() == AST::AST_FUNC_BODY);
    ProgramOrFunctionBody* func_body = static_cast<ProgramOrFunctionBody*>(body);
    strict |= func_body->strict();
    SET_VALUE(jsobj.handle().val(), kCodeOffset, func_body, ProgramOrFunctionBody*);
  } else {
    SET_VALUE(jsobj.handle().val(), kCodeOffset, nullptr, ProgramOrFunctionBody*);
  }
  SET_JSVALUE(jsobj.handle().val(), kScopeOffset, scope);
  SET_VALUE(jsobj.handle().val(), kStrictOffset, strict, bool);
  jsobj.SetType(OBJ_FUNC);

  // 13.2 Creating Function Objects
  js_object::SetPrototype(jsobj, function_proto::Instance());
  // Whether the function is made from bind.
  if (body != nullptr) {
    AddValueProperty(jsobj, string::Length(), number::New(names.size()), false, false, false);  // 14 & 15
    JSValue proto = object_object::New();  // 16
    AddValueProperty(proto, string::Constructor(), jsobj, true, false, true);
    // 15.3.5.2 prototype
    AddValueProperty(jsobj, string::Prototype(), proto, true, false, false);
    if (strict) {
      // TODO(zhuzilin) thrower
    }
  }
  return jsobj;
}

inline bool from_bind(JSValue jsval) { return jsval.type() == OBJ_BIND_FUNC; }

inline JSValue Scope(JSValue jsval) {
  ASSERT(!from_bind(jsval));
  return GET_JSVALUE(jsval.handle().val(), kScopeOffset);
};
inline JSValue FormalParameters(JSValue jsval) {
  ASSERT(!from_bind(jsval));
  return GET_JSVALUE(jsval.handle().val(), kFormalParametersOffset);
};
inline ProgramOrFunctionBody* Code(JSValue jsval) {
  ASSERT(!from_bind(jsval));
  return READ_VALUE(jsval.handle().val(), kCodeOffset, ProgramOrFunctionBody*);
}
inline bool strict(JSValue jsval) {
  ASSERT(!from_bind(jsval));
  return READ_VALUE(jsval.handle().val(), kStrictOffset, bool);
}
}  // function_object

namespace bind_function_object {

constexpr size_t kTargetFunctionOffset = ::es::function_object::kFunctionObjectOffset;
constexpr size_t kBoundThisOffset = kTargetFunctionOffset + sizeof(JSValue);
constexpr size_t kBoundArgsOffset = kBoundThisOffset + sizeof(JSValue);
constexpr size_t kBindFunctionObjectOffset = kBoundArgsOffset + sizeof(JSValue);

inline JSValue New(
  JSValue target_function, JSValue bound_this, std::vector<JSValue> bound_args
) {
  JSValue func = function_object::New(
    {}, nullptr, JSValue(), Runtime::TopContext().strict(),
    kBindFunctionObjectOffset - ::es::function_object::kFunctionObjectOffset);

  SET_JSVALUE(func.handle().val(), kTargetFunctionOffset, target_function);
  SET_JSVALUE(func.handle().val(), kBoundThisOffset, bound_this);
  SET_JSVALUE(func.handle().val(), kBoundArgsOffset, ::es::fixed_array::New(bound_args));
  func.SetType(OBJ_BIND_FUNC);
  return func;
}

inline JSValue TargetFunction(JSValue func) { return GET_JSVALUE(func.handle().val(), kTargetFunctionOffset); }
inline JSValue BoundThis(JSValue func) { return GET_JSVALUE(func.handle().val(), kBoundThisOffset); }
inline JSValue BoundArgs(JSValue func) { return GET_JSVALUE(func.handle().val(), kBoundArgsOffset); }

}  // namespace bind_function_object

namespace function_constructor {

inline JSValue New(flag_t flag) {
  std::cout << "function_constructor::New" << std::endl;
  JSValue jsobj = js_object::New(
    u"Function", true, JSValue(), true, true, nullptr, 0, flag);

  jsobj.SetType(OBJ_FUNC_CONSTRUCTOR);
  return jsobj;
}

inline JSValue Instance() {
  static JSValue singleton = function_constructor::New(GCFlag::SINGLE);
  return singleton;
}

inline JSValue toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  return string::New(u"function Function() { [native code] }");
}

}  // namespace function_constructor

JSValue function_proto::toString(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue func = Runtime::TopValue();
  if (!func.IsObject()) {
    e = error::TypeError(u"Function.prototype.toString called on non-object");
    return JSValue();
  }
  if (!func.IsFunctionObject()) {
    e = error::TypeError(u"Function.prototype.toString called on non-function");
    return JSValue();
  }
  std::u16string str = u"function (";
  JSValue params = function_object::FormalParameters(func);
  size_t params_size = fixed_array::size(params);
  if (params_size > 0) {
    for (size_t i = 0; i < params_size; i++) {
      if (i != 0) str += u",";
      str += string::data(fixed_array::Get(params, i));
    }
  }
  str += u")";
  return string::New(str);
}

// 15.3.4.5 Function.prototype.bind (thisArg [, arg1 [, arg2, …]])
JSValue function_proto::bind(JSValue& e, JSValue this_arg, std::vector<JSValue> vals) {
  JSValue target = Runtime::TopValue();
  if (!target.IsCallable()) {
    e = error::TypeError(u"Function.prototype.call called on non-callable");
    return JSValue();
  }
  JSValue this_arg_for_F = undefined::New();
  if (vals.size() > 0)
    this_arg_for_F = vals[0];
  std::vector<JSValue> A;
  if (vals.size() > 1) {
    A = std::vector<JSValue>(vals.begin() + 1, vals.end());
  }
  JSValue F = bind_function_object::New(target, this_arg_for_F, A);
  size_t len = 0;
  if (js_object::Class(target) == u"Function") {
    size_t L = ToNumber(e, Get(e, target, string::Length()));
    if (L - A.size() > 0)
      len = L - A.size();
  }
  AddValueProperty(F, string::Length(), number::New(len), false, false, false);
  // 19
  // TODO(zhuzilin) thrower
  return F;
}

JSValue InstantiateFunctionDeclaration(JSValue& e, Function* func_ast) {
    ASSERT(func_ast->is_named());
    std::u16string identifier = func_ast->name();
    JSValue func_env = NewDeclarativeEnvironment(  // 1
      Runtime::TopLexicalEnv()
    );
    JSValue env_rec = lexical_env::env_rec(func_env);  // 2
    JSValue identifier_str = string::New(identifier);
    ProgramOrFunctionBody* body = static_cast<ProgramOrFunctionBody*>(func_ast->body());
    bool strict = body->strict() || Runtime::TopContext().strict();
    if (strict) {
      // 13.1
      if (HaveDuplicate(func_ast->params())) {
        e = error::SyntaxError(u"have duplicate parameter name in strict mode");
        return JSValue();
      }
      for (auto name : func_ast->params()) {
        if (name == u"eval" || name == u"arguments") {
          e = error::SyntaxError(u"parameter name cannot be eval or arguments in strict mode");
          return JSValue();
        }
      }
      if (func_ast->name() == u"eval" || func_ast->name() == u"arguments") {
        e = error::SyntaxError(u"function name cannot be eval or arguments in strict mode");
        return JSValue();
      }
      for (VarDecl* d : body->var_decls()) {
        if (d->ident().type() == Token::TK_STRICT_FUTURE) {
          e = error::SyntaxError(u"Unexpected future reserved word " + d->ident().source_ref() + u" in strict mode");
          return JSValue();
        }
        if (d->ident().source_ref() == u"eval" || d->ident().source_ref() == u"arguments") {
          e = error::SyntaxError(u"Unexpected eval or arguments in strict mode");
          return JSValue();
        }
      }
    }
    JSValue closure = function_object::New(
      func_ast->params(), func_ast->body(), func_env, strict);  // 4
    CreateAndInitializeImmutableBinding(env_rec, identifier_str, closure);  // 5
    return closure;  // 6
}

JSValue EvalFunction(JSValue& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_FUNC);
  Function* func_ast = static_cast<Function*>(ast);

  if (func_ast->is_named()) {
    return InstantiateFunctionDeclaration(e, func_ast);
  } else {
    auto body = static_cast<ProgramOrFunctionBody*>(func_ast->body());
    bool strict = body->strict() || Runtime::TopContext().strict();
    if (strict) {
      // 13.1
      if (HaveDuplicate(func_ast->params())) {
        e = error::SyntaxError(u"have duplicate parameter name in strict mode");
        return JSValue();
      }
      for (auto name : func_ast->params()) {
        if (name == u"eval" || name == u"arguments") {
          e = error::SyntaxError(u"parameter name cannot be eval or arguments in strict mode");
          return JSValue();
        }
      }
    }
    return function_object::New(
      func_ast->params(), func_ast->body(),
      Runtime::TopLexicalEnv(), strict
    );
  }
}

// TODO(zhuzilin) move this function to a better place
void AddFuncProperty(
  JSValue O, std::u16string name, inner_func callable, bool writable,
  bool enumerable, bool configurable
) {
  JSValue value = js_object::New(
    u"InternalFunc", false, JSValue(), false, true, callable, 0);
  value.SetType(Type::OBJ_INNER_FUNC);
  js_object::SetPrototype(value, function_proto::Instance());
  AddValueProperty(O, name, value, writable, enumerable, configurable);
}

JSValue Get__Function(JSValue& e, JSValue O, JSValue P);
bool HasInstance__Function(JSValue& e, JSValue O, JSValue V);
bool HasInstance__BindFunction(JSValue& e, JSValue O, JSValue V);

JSValue Call__Function(JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments);
JSValue Call__BindFunction(JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> extra_args);
JSValue Call__FunctionProto(JSValue& e, JSValue O, JSValue this_arg, std::vector<JSValue> arguments);

JSValue Construct__Function(JSValue& e, JSValue O,std::vector<JSValue> arguments);
JSValue Construct__BindFunction(JSValue& e, JSValue O, std::vector<JSValue> extra_args);
JSValue Construct__FunctionConstructor(JSValue& e, JSValue O, std::vector<JSValue> arguments);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_FUNCTION_OBJECT