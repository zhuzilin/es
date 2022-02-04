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

void EnterFunctionCode(
  Handle<Error>& e, Handle<JSObject> F, ProgramOrFunctionBody* body,
  Handle<JSValue> this_arg, std::vector<Handle<JSValue>> args, bool strict
);

class FunctionProto : public JSObject {
 public:
  static Handle<FunctionProto> Instance() {
    static Handle<FunctionProto> singleton = FunctionProto::New(GCFlag::SINGLE);
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
    if (!e.val()->IsOk()) return Handle<JSValue>();
    size_t n = ToNumber(e, len);
    std::vector<Handle<JSValue>> arg_list;  // 6
    size_t index = 0;  // 7
    while (index < n) {  // 8
      Handle<String> index_name = ::es::NumberToString(index);
      if (!e.val()->IsOk()) return Handle<JSValue>();
      Handle<JSValue> next_arg = Get(e, arg_array, index_name);
      if (!e.val()->IsOk()) return Handle<JSValue>();
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
  static Handle<FunctionProto> New(flag_t flag) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_FUNC_PROTO, u"Function", true, Handle<JSValue>(), false, true, nullptr, 0, flag);

    return Handle<FunctionProto>(new (jsobj.val()) FunctionProto());
  }
};

class FunctionObject : public JSObject {
 public:
  static Handle<FunctionObject> New(
    std::vector<std::u16string> names, AST* body, Handle<LexicalEnvironment> scope, bool from_bind = false, size_t size = 0
  ) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_FUNC, u"Function", true, Handle<JSValue>(), true, true, nullptr,
      kFunctionObjectOffset - kJSObjectOffset + size
    );
    std::vector<Handle<String>> name_handles(names.size());
    for (size_t i = 0; i < names.size(); i++) {
      name_handles[i] = String::New(names[i]);
    }
    Handle<FixedArray<String>> formal_parameter = FixedArray<String>::New(name_handles);

    SET_HANDLE_VALUE(jsobj.val(), kFormalParametersOffset, formal_parameter, FixedArray<String>);
    bool strict = Runtime::TopContext()->strict();
    if (body != nullptr) {
      assert(body->type() == AST::AST_FUNC_BODY);
      ProgramOrFunctionBody* func_body = static_cast<ProgramOrFunctionBody*>(body);
      strict |= func_body->strict();
      SET_VALUE(jsobj.val(), kCodeOffset, func_body, ProgramOrFunctionBody*);
    } else {
      SET_VALUE(jsobj.val(), kCodeOffset, nullptr, ProgramOrFunctionBody*);
    }
    SET_HANDLE_VALUE(jsobj.val(), kScopeOffset, scope, LexicalEnvironment);
    SET_VALUE(jsobj.val(), kFromBindOffset, from_bind, bool);
    SET_VALUE(jsobj.val(), kStrictOffset, strict, bool);

    Handle<FunctionObject> obj(new (jsobj.val()) FunctionObject());
    // 13.2 Creating Function Objects
    obj.val()->SetPrototype(FunctionProto::Instance());
    // Whether the function is made from bind.
    if (body != nullptr) {
      AddValueProperty(obj, String::Length(), Number::New(names.size()), false, false, false);  // 14 & 15
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

  std::vector<HeapObject**> Pointers() override {
    std::vector<HeapObject**> pointers = JSObject::Pointers();
    pointers.emplace_back(HEAP_PTR(kFormalParametersOffset));
    pointers.emplace_back(HEAP_PTR(kScopeOffset));
    return pointers;
  }

  virtual Handle<LexicalEnvironment> Scope() {
    return READ_HANDLE_VALUE(this, kScopeOffset, LexicalEnvironment);
  };
  virtual Handle<FixedArray<String>> FormalParameters() {
    return READ_HANDLE_VALUE(this, kFormalParametersOffset, FixedArray<String>);
  };
  virtual ProgramOrFunctionBody* Code() { return READ_VALUE(this, kCodeOffset, ProgramOrFunctionBody*); }
  virtual bool strict() { return READ_VALUE(this, kStrictOffset, bool); }
  bool from_bind() { return READ_VALUE(this, kFromBindOffset, bool); }

  virtual std::string ToString() override {
    std::string result = "Function(";
    FixedArray<String>* params = READ_VALUE(this, kFormalParametersOffset, FixedArray<String>*);
    if (params->size() > 0) {
      result += params->GetRaw(0)->ToString();
      for (size_t i = 1; i < params->size(); i++) {
        result += "," + params->GetRaw(i)->ToString();
      }
    }
    result += ")";
    return result;
  }

 protected:
  static constexpr size_t kFormalParametersOffset = kJSObjectOffset;
  static constexpr size_t kCodeOffset = kFormalParametersOffset + kPtrSize;
  static constexpr size_t kScopeOffset = kCodeOffset + kPtrSize;
  static constexpr size_t kFromBindOffset = kScopeOffset + kPtrSize;
  static constexpr size_t kStrictOffset = kFromBindOffset + kBoolSize;
  static constexpr size_t kFunctionObjectOffset = kStrictOffset + kBoolSize;
};

class BindFunctionObject : public FunctionObject {
 public:
  static Handle<BindFunctionObject> New(
    Handle<JSObject> target_function, Handle<JSValue> bound_this, std::vector<Handle<JSValue>> bound_args
  ) {
    Handle<FunctionObject> func = FunctionObject::New(
      {}, nullptr, Handle<JSValue>(), true,
      kBindFunctionObjectOffset - kFunctionObjectOffset);

    SET_HANDLE_VALUE(func.val(), kTargetFunctionOffset, target_function, JSObject);
    SET_HANDLE_VALUE(func.val(), kBoundThisOffset, bound_this, JSValue);
    SET_HANDLE_VALUE(func.val(), kBoundArgsOffset, FixedArray<JSValue>::New(bound_args), FixedArray<JSValue>);

    return Handle<BindFunctionObject>(new (func.val()) BindFunctionObject());
  }

  std::vector<HeapObject**> Pointers() override {
    std::vector<HeapObject**> pointers = JSObject::Pointers();
    pointers.emplace_back(HEAP_PTR(kTargetFunctionOffset));
    pointers.emplace_back(HEAP_PTR(kBoundThisOffset));
    pointers.emplace_back(HEAP_PTR(kBoundArgsOffset));
    return pointers;
  }

  Handle<LexicalEnvironment> Scope() override { assert(false); };
  Handle<FixedArray<String>> FormalParameters() override { assert(false); };
  ProgramOrFunctionBody* Code() override { assert(false); }
  bool strict() override { assert(false); }

  Handle<JSObject> TargetFunction() { return READ_HANDLE_VALUE(this, kTargetFunctionOffset, JSObject); }
  Handle<JSValue> BoundThis() { return READ_HANDLE_VALUE(this, kBoundThisOffset, JSValue); }
  Handle<FixedArray<JSValue>> BoundArgs() { return READ_HANDLE_VALUE(this, kBoundArgsOffset, FixedArray<JSValue>); }

  std::string ToString() override {
    return "BindFunctionObject";
  }

 private:
  static constexpr size_t kTargetFunctionOffset = kFunctionObjectOffset;
  static constexpr size_t kBoundThisOffset = kTargetFunctionOffset + kPtrSize;
  static constexpr size_t kBoundArgsOffset = kBoundThisOffset + kPtrSize;
  static constexpr size_t kBindFunctionObjectOffset = kBoundArgsOffset + kPtrSize;
};

class FunctionConstructor : public JSObject {
 public:
  static Handle<FunctionConstructor> Instance() {
    static Handle<FunctionConstructor> singleton = FunctionConstructor::New(GCFlag::SINGLE);
    return singleton;
  }

  static Handle<JSValue> toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"function Function() { [native code] }");
  }

 private:
  static Handle<FunctionConstructor> New(flag_t flag) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_FUNC_CONSTRUCTOR, u"Function", true, Handle<JSValue>(), true, true, nullptr, 0, flag);

    return Handle<FunctionConstructor>(new (jsobj.val()) FunctionConstructor());
  }
};

Handle<JSValue> FunctionProto::toString(Handle<Error>& e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
  Handle<JSValue> val = Runtime::TopValue();
  if (!val.val()->IsObject()) {
    e = Error::TypeError(u"Function.prototype.toString called on non-object");
    return Handle<JSValue>();
  }
  Handle<JSObject> obj = static_cast<Handle<JSObject>>(val);
  if (obj.val()->obj_type() != JSObject::OBJ_FUNC) {
    e = Error::TypeError(u"Function.prototype.toString called on non-function");
    return Handle<JSValue>();
  }
  Handle<FunctionObject> func = static_cast<Handle<FunctionObject>>(obj);
  std::u16string str = u"function (";
  auto params = func.val()->FormalParameters();
  if (params.val()->size() > 0) {
    str += params.val()->Get(0).val()->data();
    for (size_t i = 1; i < params.val()->size(); i++) {
      str += u"," + params.val()->Get(i).val()->data();
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
  if (target.val()->Class() == u"Function") {
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
    assert(func_ast->is_named());
    std::u16string identifier = func_ast->name();
    Handle<es::LexicalEnvironment> func_env = NewDeclarativeEnvironment(  // 1
      Runtime::TopLexicalEnv()
    );
    auto env_rec = static_cast<Handle<DeclarativeEnvironmentRecord>>(func_env.val()->env_rec());  // 2
    Handle<String> identifier_str = String::New(identifier);
    CreateImmutableBinding(env_rec, identifier_str);  // 3
    auto body = static_cast<ProgramOrFunctionBody*>(func_ast->body());
    bool strict = body->strict() || Runtime::TopContext()->strict();
    if (strict) {
      // 13.1
      if (HaveDuplicate(func_ast->params())) {
        e = Error::SyntaxError(u"have duplicate parameter name in strict mode");
        return Handle<JSValue>();
      }
      for (auto name : func_ast->params()) {
        if (name == u"eval" || name == u"arguments") {
          e = Error::SyntaxError(u"parameter name cannot be eval or arguments in strict mode");
          return Handle<JSValue>();
        }
      }
      if (func_ast->name() == u"eval" || func_ast->name() == u"arguments") {
        e = Error::SyntaxError(u"function name cannot be eval or arguments in strict mode");
        return Handle<JSValue>();
      }
    }
    Handle<FunctionObject> closure = FunctionObject::New(
      func_ast->params(), func_ast->body(), func_env);  // 4
    InitializeImmutableBinding(env_rec, identifier_str, closure);  // 5
    return closure;  // 6
}

Handle<JSValue> EvalFunction(Handle<Error>& e, AST* ast) {
  assert(ast->type() == AST::AST_FUNC);
  Function* func_ast = static_cast<Function*>(ast);

  if (func_ast->is_named()) {
    return InstantiateFunctionDeclaration(e, func_ast);
  } else {
    auto body = static_cast<ProgramOrFunctionBody*>(func_ast->body());
    bool strict = body->strict() || Runtime::TopContext()->strict();
    if (strict) {
      // 13.1
      if (HaveDuplicate(func_ast->params())) {
        e = Error::SyntaxError(u"have duplicate parameter name in strict mode");
        return Handle<JSValue>();
      }
      for (auto name : func_ast->params()) {
        if (name == u"eval" || name == u"arguments") {
          e = Error::SyntaxError(u"parameter name cannot be eval or arguments in strict mode");
          return Handle<JSValue>();
        }
      }
    }
    return FunctionObject::New(
      func_ast->params(), func_ast->body(),
      Runtime::TopLexicalEnv()
    );
  }
}

// TODO(zhuzilin) move this function to a better place
void AddFuncProperty(
  Handle<JSObject> O, std::u16string name, inner_func callable, bool writable,
  bool enumerable, bool configurable
) {
  Handle<JSObject> value = JSObject::New(
    JSObject::OBJ_INNER_FUNC, u"InternalFunc", false, Handle<JSValue>(), false, true, callable, 0);
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