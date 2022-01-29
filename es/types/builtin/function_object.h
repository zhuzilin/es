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

double ToNumber(Error* e, Handle<JSValue> input);
Handle<String> NumberToString(double m);
Completion EvalProgram(AST* ast);

void EnterFunctionCode(
  Error* e, Handle<JSObject> f, ProgramOrFunctionBody* body,
  Handle<JSValue> this_arg, std::vector<Handle<JSValue>> args, bool strict
);

class FunctionProto : public JSObject {
 public:
  static Handle<FunctionProto> Instance() {
    static Handle<FunctionProto> singleton = FunctionProto::New(GCFlag::SINGLE);
    return singleton;
  }

  static Handle<JSValue> toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

  // 15.3.4.3 Function.prototype.apply (thisArg, argArray)
  static Handle<JSValue> apply(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    if (!val.val()->IsObject()) {
      *e = *Error::TypeError(u"Function.prototype.apply called on non-object");
      return Handle<JSValue>();
    }
    Handle<JSObject> func = static_cast<Handle<JSObject>>(val);
    if (!func.val()->IsCallable()) {
      *e = *Error::TypeError(u"Function.prototype.apply called on non-callable");
      return Handle<JSValue>();
    }
    if (vals.size() == 0) {
      return Call(e, func, Undefined::Instance(), {});
    }
    if (vals.size() < 2 || vals[1].val()->IsNull() || vals[1].val()->IsUndefined()) {  // 2
      return Call(e, func, vals[0], {});
    }
    if (!vals[1].val()->IsObject()) {  // 3
      *e = *Error::TypeError(u"Function.prototype.apply's argument is non-object");
      return Handle<JSValue>();
    }
    Handle<JSObject> arg_array = static_cast<Handle<JSObject>>(vals[1]);
    Handle<JSValue> len = Get(e, arg_array, String::Length());
    if (!e->IsOk()) return Handle<JSValue>();
    size_t n = ToNumber(e, len);
    std::vector<Handle<JSValue>> arg_list;  // 6
    size_t index = 0;  // 7
    while (index < n) {  // 8
      Handle<String> index_name = ::es::NumberToString(index);
      if (!e->IsOk()) return Handle<JSValue>();
      Handle<JSValue> next_arg = Get(e, arg_array, index_name);
      if (!e->IsOk()) return Handle<JSValue>();
      arg_list.emplace_back(next_arg);
      index++;
    }
    return Call(e, func, vals[0], arg_list);
  }

  static Handle<JSValue> call(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    Handle<JSValue> val = Runtime::TopValue();
    if (!val.val()->IsObject()) {
      *e = *Error::TypeError(u"Function.prototype.call called on non-object");
      return Handle<JSValue>();
    }
    Handle<JSObject> func = static_cast<Handle<JSObject>>(val);
    if (!func.val()->IsCallable()) {
      *e = *Error::TypeError(u"Function.prototype.call called on non-callable");
      return Handle<JSValue>();
    }
    if (vals.size()) {
      Handle<JSValue> this_arg = vals[0];
      return Call(e, func, this_arg, std::vector<Handle<JSValue>>(vals.begin() + 1, vals.end()));
    } else {
      return Call(e, func, Undefined::Instance(), {});
    }
  }

  static Handle<JSValue> bind(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

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
    Handle<FixedArray<String>> formal_parameter = FixedArray<String>::New(names.size());
    for (size_t i = 0; i < names.size(); i++) {
      formal_parameter.val()->Set(i, String::New(names[i]));
    }
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

  // 13.2.2 [[Construct]]
  virtual Handle<JSObject> Construct(Error* e, std::vector<Handle<JSValue>> arguments) override {
    log::PrintSource("enter FunctionObject::Construct");
    Handle<JSObject> obj = JSObject::New(OBJ_OTHER, u"Object", true, Handle<JSValue>(), false, false, nullptr, 0);
    Handle<JSValue> proto = Get(e, Handle<JSObject>(this), String::Prototype());
    if (!e->IsOk()) return Handle<JSValue>();
    if (proto.val()->IsObject()) {  // 6
      obj.val()->SetPrototype(proto);
    } else {  // 7
      obj.val()->SetPrototype(ObjectProto::Instance());
    }
    Handle<JSValue> result = Call(e, Handle<JSObject>(this), obj, arguments);  // 8
    if (!e->IsOk()) return Handle<JSValue>();
    if (result.val()->IsObject())  // 9
      return static_cast<Handle<JSObject>>(result);
    return obj;  // 10
  }

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

  // 13.2.2 [[Construct]]
  Handle<JSObject> Construct(Error* e, std::vector<Handle<JSValue>> extra_args) override {
    if (!TargetFunction().val()->IsConstructor()) {
      *e = *Error::TypeError(u"target function has no [[Construct]] internal method");
      return Handle<JSValue>();
    }
    std::vector<Handle<JSValue>> args;
    for (size_t i = 0; i < BoundArgs().val()->size(); i++) {
      args.emplace_back(BoundArgs().val()->Get(i));
    }
    args.insert(args.end(), extra_args.begin(), extra_args.end());
    return TargetFunction().val()->Construct(e, args);
  }

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

  // 15.3.2.1 new Function (p1, p2, … , pn, body)
  Handle<JSObject> Construct(Error* e, std::vector<Handle<JSValue>> arguments) override {
    log::PrintSource("enter FunctionConstructor::Construct");
    size_t arg_count = arguments.size();
    std::u16string P = u"";
    std::u16string body = u"";
    if (arg_count == 1) {
      body = ToU16String(e, arguments[0]);
      if (!e->IsOk()) return Handle<JSValue>();
    } else if (arg_count > 1) {
      P += ToU16String(e, arguments[0]);
      if (!e->IsOk()) return Handle<JSValue>();
      for (size_t i = 1; i < arg_count - 1; i++) {
        P += u"," + ToU16String(e, arguments[i]);
        if (!e->IsOk()) return Handle<JSValue>();
      }
      body = ToU16String(e, arguments[arg_count - 1]);
      if (!e->IsOk()) return Handle<JSValue>();
    }
    std::u16string P_view = Runtime::Global()->AddSource(std::move(P));
    std::vector<std::u16string> names;
    AST* body_ast;
    if (P_view.size() > 0) {
      Parser parser(P_view);
      names = parser.ParseFormalParameterList();
      if (names.size() == 0) {
        *e = *Error::SyntaxError(u"invalid parameter name");
        return Handle<JSValue>();
      }
    }
    std::u16string body_view = Runtime::Global()->AddSource(std::move(body));
    {
      Parser parser(body_view);
      body_ast = parser.ParseFunctionBody(Token::TK_EOS);
      if (body_ast->IsIllegal()) {
        *e = *Error::SyntaxError(u"failed to parse function body: " + body_ast->source());
        return Handle<JSValue>();
      }
    }
    Handle<LexicalEnvironment> scope = LexicalEnvironment::Global();
    bool strict = static_cast<ProgramOrFunctionBody*>(body_ast)->strict();
    if (strict) {
      // 13.1
      if (HaveDuplicate(names)) {
        *e = *Error::SyntaxError(u"have duplicate parameter name in strict mode");
        return Handle<JSValue>();
      }
      for (auto name : names) {
        if (name == u"eval" || name == u"arguments") {
          *e = *Error::SyntaxError(u"param name cannot be eval or arguments in strict mode");
          return Handle<JSValue>();
        }
      }
    }
    return FunctionObject::New(names, body_ast, scope);
  }

  static Handle<JSValue> toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"function Function() { [native code] }");
  }

 private:
  static Handle<FunctionConstructor> New(flag_t flag) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_OTHER, u"Function", true, Handle<JSValue>(), true, true, nullptr, 0, flag);
    return Handle<FunctionConstructor>(new (jsobj.val()) FunctionConstructor());
  }
};

Handle<JSValue> FunctionProto::toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
  Handle<JSValue> val = Runtime::TopValue();
  if (!val.val()->IsObject()) {
    *e = *Error::TypeError(u"Function.prototype.toString called on non-object");
    return Handle<JSValue>();
  }
  Handle<JSObject> obj = static_cast<Handle<JSObject>>(val);
  if (obj.val()->obj_type() != JSObject::OBJ_FUNC) {
    *e = *Error::TypeError(u"Function.prototype.toString called on non-function");
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
Handle<JSValue> FunctionProto::bind(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
  Handle<JSValue> val = Runtime::TopValue();
  if (!val.val()->IsCallable()) {
    *e = *Error::TypeError(u"Function.prototype.call called on non-callable");
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

Handle<FunctionObject> InstantiateFunctionDeclaration(Error* e, Function* func_ast) {
    assert(func_ast->is_named());
    std::u16string identifier = func_ast->name();
    Handle<es::LexicalEnvironment> func_env = LexicalEnvironment::NewDeclarativeEnvironment(  // 1
      Runtime::TopLexicalEnv()
    );
    auto env_rec = static_cast<DeclarativeEnvironmentRecord*>(func_env.val()->env_rec());  // 2
    Handle<String> identifier_str = String::New(identifier);
    env_rec->CreateImmutableBinding(identifier_str);  // 3
    auto body = static_cast<ProgramOrFunctionBody*>(func_ast->body());
    bool strict = body->strict() || Runtime::TopContext()->strict();
    if (strict) {
      // 13.1
      if (HaveDuplicate(func_ast->params())) {
        *e = *Error::SyntaxError(u"have duplicate parameter name in strict mode");
        return Handle<JSValue>();
      }
      for (auto name : func_ast->params()) {
        if (name == u"eval" || name == u"arguments") {
          *e = *Error::SyntaxError(u"parameter name cannot be eval or arguments in strict mode");
          return Handle<JSValue>();
        }
      }
      if (func_ast->name() == u"eval" || func_ast->name() == u"arguments") {
        *e = *Error::SyntaxError(u"function name cannot be eval or arguments in strict mode");
        return Handle<JSValue>();
      }
    }
    Handle<FunctionObject> closure = FunctionObject::New(
      func_ast->params(), func_ast->body(), func_env);  // 4
    env_rec->InitializeImmutableBinding(identifier_str, closure);  // 5
    return closure;  // 6
}

Handle<JSValue> EvalFunction(Error* e, AST* ast) {
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
        *e = *Error::SyntaxError(u"have duplicate parameter name in strict mode");
        return Handle<JSValue>();
      }
      for (auto name : func_ast->params()) {
        if (name == u"eval" || name == u"arguments") {
          *e = *Error::SyntaxError(u"parameter name cannot be eval or arguments in strict mode");
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

Handle<JSValue> Get__Function(Error* e, Handle<FunctionObject> O, Handle<String> P);
bool HasInstance__Function(Error* e, Handle<FunctionObject> O, Handle<JSValue> V);
bool HasInstance__BindFunction(Error* e, Handle<BindFunctionObject> O, Handle<JSValue> V);

Handle<JSValue> Call__Function(Error* e, Handle<FunctionObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments);
Handle<JSValue> Call__BindFunction(Error* e, Handle<BindFunctionObject> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments);
Handle<JSValue> Call__FunctionProto(Error* e, Handle<FunctionProto> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {});

}  // namespace es

#endif  // ES_TYPES_BUILTIN_FUNCTION_OBJECT