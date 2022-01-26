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

double ToNumber(Error* e, JSValue* input);
String* NumberToString(double m);
Completion EvalProgram(AST* ast);

void EnterFunctionCode(
  Error* e, JSObject* f, ProgramOrFunctionBody* body,
  JSValue* this_arg, std::vector<JSValue*> args, bool strict
);

class FunctionProto : public JSObject {
 public:
  static FunctionProto* Instance() {
    static FunctionProto* singleton = FunctionProto::New();
    return singleton;
  }

  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    return Undefined::Instance();
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals);

  // 15.3.4.3 Function.prototype.apply (thisArg, argArray)
  static JSValue* apply(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSValue* val = Runtime::TopValue();
    if (!val->IsObject()) {
      *e = *Error::TypeError(u"Function.prototype.apply called on non-object");
      return nullptr;
    }
    JSObject* func = static_cast<JSObject*>(val);
    if (!func->IsCallable()) {
      *e = *Error::TypeError(u"Function.prototype.apply called on non-callable");
      return nullptr;
    }
    if (vals.size() == 0) {
      return func->Call(e, Undefined::Instance(), {});
    }
    if (vals.size() < 2 || vals[1]->IsNull() || vals[1]->IsUndefined()) {  // 2
      return func->Call(e, vals[0], {});
    }
    if (!vals[1]->IsObject()) {  // 3
      *e = *Error::TypeError(u"Function.prototype.apply's argument is non-object");
      return nullptr;
    }
    JSObject* arg_array = static_cast<JSObject*>(vals[1]);
    JSValue* len = arg_array->Get(e, String::Length());
    if (!e->IsOk()) return nullptr;
    size_t n = ToNumber(e, len);
    std::vector<JSValue*> arg_list;  // 6
    size_t index = 0;  // 7
    while (index < n) {  // 8
      String* index_name = ::es::NumberToString(index);
      if (!e->IsOk()) return nullptr;
      JSValue* next_arg = arg_array->Get(e, index_name);
      if (!e->IsOk()) return nullptr;
      arg_list.emplace_back(next_arg);
      index++;
    }
    return func->Call(e, vals[0], arg_list);
  }

  static JSValue* call(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    JSValue* val = Runtime::TopValue();
    if (!val->IsObject()) {
      *e = *Error::TypeError(u"Function.prototype.call called on non-object");
      return nullptr;
    }
    JSObject* func = static_cast<JSObject*>(val);
    if (!func->IsCallable()) {
      *e = *Error::TypeError(u"Function.prototype.call called on non-callable");
      return nullptr;
    }
    if (vals.size()) {
      JSValue* this_arg = vals[0];
      return func->Call(e, this_arg, std::vector<JSValue*>(vals.begin() + 1, vals.end()));
    } else {
      return func->Call(e, Undefined::Instance(), {});
    }
  }

  static JSValue* bind(Error* e, JSValue* this_arg, std::vector<JSValue*> vals);

 private:
  static FunctionProto* New() {
    JSObject* jsobj = JSObject::New(
      OBJ_FUNC, u"Function", true, nullptr, false, true, nullptr, 0);
    return new (jsobj) FunctionProto();
  }
};

class FunctionObject : public JSObject {
 public:
  static FunctionObject* New(
    std::vector<std::u16string> names, AST* body, LexicalEnvironment* scope, bool from_bind = false, size_t size = 0
  ) {
    JSObject* jsobj = JSObject::New(
      OBJ_FUNC, u"Function", true, nullptr, true, true, nullptr,
      kFunctionObjectOffset - kJSObjectOffset + size
    );
    FixedArray<String>* formal_parameter = FixedArray<String>::New(names.size());
    for (size_t i = 0; i < names.size(); i++) {
      formal_parameter->Set(i, String::New(names[i]));
    }
    SET_VALUE(jsobj, kFormalParametersOffset, formal_parameter, FixedArray<String>*);
    bool strict = Runtime::TopContext()->strict();
    if (body != nullptr) {
      assert(body->type() == AST::AST_FUNC_BODY);
      ProgramOrFunctionBody* func_body = static_cast<ProgramOrFunctionBody*>(body);
      strict |= func_body->strict();
      SET_VALUE(jsobj, kCodeOffset, func_body, ProgramOrFunctionBody*);
    } else {
      SET_VALUE(jsobj, kCodeOffset, nullptr, ProgramOrFunctionBody*);
    }
    
    SET_VALUE(jsobj, kScopeOffset, scope, LexicalEnvironment*);
    SET_VALUE(jsobj, kFromBindOffset, from_bind, bool);
    SET_VALUE(jsobj, kStrictOffset, strict, bool);

    FunctionObject* obj = new (jsobj) FunctionObject();
    // 13.2 Creating Function Objects
    obj->SetPrototype(FunctionProto::Instance());
    // Whether the function is made from bind.
    if (body != nullptr) {
      obj->AddValueProperty(String::Length(), Number::New(names.size()), false, false, false);  // 14 & 15
      JSObject* proto = Object::New();  // 16
      proto->AddValueProperty(u"constructor", obj, true, false, true);
      // 15.3.5.2 prototype
      obj->AddValueProperty(String::Prototype(), proto, true, false, false);
      if (strict) {
        // TODO(zhuzilin) thrower
      }
    }
    return obj;
  }

  virtual LexicalEnvironment* Scope() { return READ_VALUE(this, kScopeOffset, LexicalEnvironment*); };
  virtual FixedArray<String>* FormalParameters() { return READ_VALUE(this, kFormalParametersOffset, FixedArray<String>*); };
  virtual ProgramOrFunctionBody* Code() { return READ_VALUE(this, kCodeOffset, ProgramOrFunctionBody*); }
  virtual bool strict() { return READ_VALUE(this, kStrictOffset, bool); }
  bool from_bind() { return READ_VALUE(this, kFromBindOffset, bool); }

  // 13.2.1 [[Call]]
  virtual JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments) override {
    log::PrintSource("enter FunctionObject::Call ", Code()->source().substr(0, 100));
    EnterFunctionCode(e, this, Code(), this_arg, arguments, strict());
    if (!e->IsOk()) return nullptr;

    Completion result;
    if (Code() != nullptr) {
      result = EvalProgram(Code());
    }
    Runtime::Global()->PopContext();   // 3

    log::PrintSource("exit FunctionObject::Call", Code()->source().substr(0, 100));
    switch (result.type) {
      case Completion::RETURN:
        log::PrintSource("exit FunctionObject::Call RETURN");
        return result.value;
      case Completion::THROW: {
        log::PrintSource("exit FunctionObject::Call THROW");
        if (result.value->IsErrorObject()) {
          *e = *(static_cast<ErrorObject*>(result.value)->e());
          log::PrintSource("message: ", e->message());
          return nullptr;
        }
        std::u16string message = ToU16String(e, result.value);
        log::PrintSource("message: ", message);
        *e = *Error::NativeError(message);
        return nullptr;
      }
      default:
        log::PrintSource("exit FunctionObject::Call NORMAL");
        assert(result.type == Completion::NORMAL);
        return Undefined::Instance();
    }
  }

  // 13.2.2 [[Construct]]
  virtual JSObject* Construct(Error* e, std::vector<JSValue*> arguments) override {
    log::PrintSource("enter FunctionObject::Construct");
    JSObject* obj = JSObject::New(OBJ_OTHER, u"Object", true, nullptr, false, false, nullptr, 0);
    JSValue* proto = Get(e, String::Prototype());
    if (!e->IsOk()) return nullptr;
    if (proto->IsObject()) {  // 6
      obj->SetPrototype(proto);
    } else {  // 7
      obj->SetPrototype(ObjectProto::Instance());
    }
    JSValue* result = Call(e, obj, arguments);  // 8
    if (!e->IsOk()) return nullptr;
    if (result->IsObject())  // 9
      return static_cast<JSObject*>(result);
    return obj;  // 10
  }

  // 15.3.5.3 [[HasInstance]] (V)
  virtual bool HasInstance(Error* e, JSValue* V) override {
    if (!V->IsObject())
      return false;
    JSValue* O = Get(e, String::Prototype());
    if (!e->IsOk()) return false;
    if (!O->IsObject()) {
      *e = *Error::TypeError();
      return false;
    }
    while (!V->IsNull()) {
      if (V == O)
        return true;
      V = static_cast<JSObject*>(V)->Get(e, String::Prototype());
      if (!e->IsOk()) return false;
    }
    return false;
  }

  // 15.3.5.4 [[Get]] (P)
  JSValue* Get(Error* e, String* P) override {
    JSValue* v = JSObject::Get(e, P);
    if (!e->IsOk()) return nullptr;
    if (P->data() == u"caller") {  // 2
      if (v->IsObject()) {
        JSObject* v_obj = static_cast<JSObject*>(v);
        if (v_obj->IsFunction()) {
          FunctionObject* v_func = static_cast<FunctionObject*>(v);
          if (v_func->strict()) {
            *e = *Error::TypeError();
            return nullptr;
          }
        }
      }
    }
    return v;
  }

  virtual std::string ToString() override {
    std::string result = "Function(";
    if (FormalParameters()->size() > 0) {
      result += log::ToString(FormalParameters()->Get(0));
      for (size_t i = 1; i < FormalParameters()->size(); i++) {
        result += "," + log::ToString(FormalParameters()->Get(i));
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
  static BindFunctionObject* New(
    JSObject* target_function, JSValue* bound_this, std::vector<JSValue*> bound_args
  ) {
    FunctionObject* func = FunctionObject::New(
      {}, nullptr, nullptr, true,
      kBindFunctionObjectOffset - kFunctionObjectOffset);
    SET_VALUE(func, kTargetFunctionOffset, target_function, JSObject*);
    SET_VALUE(func, kBoundThisOffset, bound_this, JSValue*);
    SET_VALUE(func, kBoundArgsOffset, FixedArray<JSValue>::New(bound_args), FixedArray<JSValue>*);
    return new (func) BindFunctionObject();
  }

  LexicalEnvironment* Scope() override { assert(false); };
  FixedArray<String>* FormalParameters() override { assert(false); };
  ProgramOrFunctionBody* Code() override { assert(false); }
  bool strict() override { assert(false); }

  JSObject* TargetFunction() { return READ_VALUE(this, kTargetFunctionOffset, JSObject*); }
  JSValue* BoundThis() { return READ_VALUE(this, kBoundThisOffset, JSValue*); }
  FixedArray<JSValue>* BoundArgs() { return READ_VALUE(this, kBoundArgsOffset, FixedArray<JSValue>*); }

  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> extra_args) override {
    log::PrintSource("enter BindFunctionObject::Call");
    std::vector<JSValue*> args;
    for (size_t i = 0; i < BoundArgs()->size(); i++) {
      args.emplace_back(BoundArgs()->Get(i));
    }
    args.insert(args.end(), extra_args.begin(), extra_args.end());
    return TargetFunction()->Call(e, BoundThis(), args);
  }

  // 13.2.2 [[Construct]]
  JSObject* Construct(Error* e, std::vector<JSValue*> extra_args) override {
    if (!TargetFunction()->IsConstructor()) {
      *e = *Error::TypeError(u"target function has no [[Construct]] internal method");
      return nullptr;
    }
    std::vector<JSValue*> args;
    for (size_t i = 0; i < BoundArgs()->size(); i++) {
      args.emplace_back(BoundArgs()->Get(i));
    }
    args.insert(args.end(), extra_args.begin(), extra_args.end());
    return TargetFunction()->Construct(e, args);
  }

  // 15.3.4.5.3 [[HasInstance]] (V)
  bool HasInstance(Error* e, JSValue* V) override {
    return TargetFunction()->HasInstance(e, V);
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
  static FunctionConstructor* Instance() {
    static FunctionConstructor* singleton = FunctionConstructor::New();
    return singleton;
  }

  // 15.3.1 The Function Constructor Called as a Function
  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    return Construct(e, arguments);
  }

  // 15.3.2.1 new Function (p1, p2, … , pn, body)
  JSObject* Construct(Error* e, std::vector<JSValue*> arguments) override {
    log::PrintSource("enter FunctionConstructor::Construct");
    size_t arg_count = arguments.size();
    std::u16string P = u"";
    std::u16string body = u"";
    if (arg_count == 1) {
      body = ToU16String(e, arguments[0]);
      if (!e->IsOk()) return nullptr;
    } else if (arg_count > 1) {
      P += ToU16String(e, arguments[0]);
      if (!e->IsOk()) return nullptr;
      for (size_t i = 1; i < arg_count - 1; i++) {
        P += u"," + ToU16String(e, arguments[i]);
        if (!e->IsOk()) return nullptr;
      }
      body = ToU16String(e, arguments[arg_count - 1]);
      if (!e->IsOk()) return nullptr;
    }
    std::u16string P_view = Runtime::Global()->AddSource(std::move(P));
    std::vector<std::u16string> names;
    AST* body_ast;
    if (P_view.size() > 0) {
      Parser parser(P_view);
      names = parser.ParseFormalParameterList();
      if (names.size() == 0) {
        *e = *Error::SyntaxError(u"invalid parameter name");
        return nullptr;
      }
    }
    std::u16string body_view = Runtime::Global()->AddSource(std::move(body));
    {
      Parser parser(body_view);
      body_ast = parser.ParseFunctionBody(Token::TK_EOS);
      if (body_ast->IsIllegal()) {
        *e = *Error::SyntaxError(u"failed to parse function body: " + body_ast->source());
        return nullptr;
      }
    }
    LexicalEnvironment* scope = LexicalEnvironment::Global();
    bool strict = static_cast<ProgramOrFunctionBody*>(body_ast)->strict();
    if (strict) {
      // 13.1
      if (HaveDuplicate(names)) {
        *e = *Error::SyntaxError(u"have duplicate parameter name in strict mode");
        return nullptr;
      }
      for (auto name : names) {
        if (name == u"eval" || name == u"arguments") {
          *e = *Error::SyntaxError(u"param name cannot be eval or arguments in strict mode");
          return nullptr;
        }
      }
    }
    return FunctionObject::New(names, body_ast, scope);
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    return String::New(u"function Function() { [native code] }");
  }

 private:
  static FunctionConstructor* New() {
    JSObject* jsobj = JSObject::New(
      OBJ_OTHER, u"Function", true, nullptr, true, true, nullptr, 0);
    return new (jsobj) FunctionConstructor();
  }
};

JSValue* FunctionProto::toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
  JSValue* val = Runtime::TopValue();
  if (!val->IsObject()) {
    *e = *Error::TypeError(u"Function.prototype.toString called on non-object");
    return nullptr;
  }
  JSObject* obj = static_cast<JSObject*>(val);
  if (obj->obj_type() != JSObject::OBJ_FUNC) {
    *e = *Error::TypeError(u"Function.prototype.toString called on non-function");
    return nullptr;
  }
  FunctionObject* func = static_cast<FunctionObject*>(obj);
  std::u16string str = u"function (";
  auto params = func->FormalParameters();
  if (params->size() > 0) {
    str += params->Get(0)->data();
    for (size_t i = 1; i < params->size(); i++) {
      str += u"," + params->Get(i)->data();
    }
  }
  str += u")";
  return String::New(str);
}

// 15.3.4.5 Function.prototype.bind (thisArg [, arg1 [, arg2, …]])
JSValue* FunctionProto::bind(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
  JSValue* val = Runtime::TopValue();
  if (!val->IsCallable()) {
    *e = *Error::TypeError(u"Function.prototype.call called on non-callable");
    return nullptr;
  }
  JSObject* target = static_cast<JSObject*>(val);
  JSValue* this_arg_for_F = Undefined::Instance();
  if (vals.size() > 0)
    this_arg_for_F = vals[0];
  std::vector<JSValue*> A;
  if (vals.size() > 1) {
    A = std::vector<JSValue*>(vals.begin() + 1, vals.end());
  }
  BindFunctionObject* F = BindFunctionObject::New(target, this_arg_for_F, A);
  size_t len = 0;
  if (target->Class()->data() == u"Function") {
    size_t L = ToNumber(e, target->Get(e, String::Length()));
    if (L - A.size() > 0)
      len = L - A.size();
  }
  F->AddValueProperty(String::Length(), Number::New(len), false, false, false);
  // 19
  // TODO(zhuzilin) thrower
  return F;
}

FunctionObject* InstantiateFunctionDeclaration(Error* e, Function* func_ast) {
    assert(func_ast->is_named());
    std::u16string identifier = func_ast->name();
    auto func_env = LexicalEnvironment::NewDeclarativeEnvironment(  // 1
      Runtime::TopLexicalEnv()
    );
    auto env_rec = static_cast<DeclarativeEnvironmentRecord*>(func_env->env_rec());  // 2
    String* identifier_str = String::New(identifier);
    env_rec->CreateImmutableBinding(identifier_str);  // 3
    auto body = static_cast<ProgramOrFunctionBody*>(func_ast->body());
    bool strict = body->strict() || Runtime::TopContext()->strict();
    if (strict) {
      // 13.1
      if (HaveDuplicate(func_ast->params())) {
        *e = *Error::SyntaxError(u"have duplicate parameter name in strict mode");
        return nullptr;
      }
      for (auto name : func_ast->params()) {
        if (name == u"eval" || name == u"arguments") {
          *e = *Error::SyntaxError(u"parameter name cannot be eval or arguments in strict mode");
          return nullptr;
        }
      }
      if (func_ast->name() == u"eval" || func_ast->name() == u"arguments") {
        *e = *Error::SyntaxError(u"function name cannot be eval or arguments in strict mode");
        return nullptr;
      }
    }
    FunctionObject* closure = FunctionObject::New(
      func_ast->params(), func_ast->body(), func_env);  // 4
    env_rec->InitializeImmutableBinding(identifier_str, closure);  // 5
    return closure;  // 6
}

JSValue* EvalFunction(Error* e, AST* ast) {
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
        return nullptr;
      }
      for (auto name : func_ast->params()) {
        if (name == u"eval" || name == u"arguments") {
          *e = *Error::SyntaxError(u"parameter name cannot be eval or arguments in strict mode");
          return nullptr;
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
void JSObject::AddFuncProperty(
  std::u16string name, inner_func callable, bool writable,
  bool enumerable, bool configurable
) {
  JSObject* value = JSObject::New(
    OBJ_INNER_FUNC, u"InternalFunc", false, nullptr, false, true, callable, 0);
  value->SetPrototype(FunctionProto::Instance());
  AddValueProperty(name, value, writable, enumerable, configurable);
}

}  // namespace es

#endif  // ES_TYPES_BUILTIN_FUNCTION_OBJECT