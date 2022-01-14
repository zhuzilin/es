#ifndef ES_TYPES_BUILTIN_FUNCTION_OBJECT
#define ES_TYPES_BUILTIN_FUNCTION_OBJECT

#include <es/parser/ast.h>
#include <es/types/object.h>
#include <es/types/builtin/object_object.h>
#include <es/types/lexical_environment.h>
#include <es/execution_context.h>
#include <es/types/completion.h>

namespace es {

class FunctionProto : public JSObject {
 public:
  static FunctionProto* Instance() {
    static FunctionProto singleton;
    return &singleton;
  }

  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    return Undefined::Instance();
  }

  static JSValue* toString(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* apply(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* call(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* bind(Error* e, std::vector<JSValue*> vals) {
    assert(false);
  }

 private:
  FunctionProto() :
    JSObject(OBJ_OTHER, u"Function", true, nullptr, false, true) {}
};

void EnterFunctionCode(
  Error* e, JSObject* f, ProgramOrFunctionBody* body,
  JSValue* this_arg, std::vector<JSValue*> args, bool strict
);

Completion EvalProgram(Error* e, AST* ast);

class FunctionObject : public JSObject {
 public:
  FunctionObject(
    std::vector<std::u16string> names, AST* body,
    LexicalEnvironment* scope, bool from_bind = false
  ) : JSObject(OBJ_FUNC, u"Function", true, nullptr, true, true),
      formal_params_(names), scope_(scope), from_bind_(from_bind) {
    assert(body->type() == AST::AST_FUNC_BODY);
    body_ = static_cast<ProgramOrFunctionBody*>(body);
    strict_ = body_->strict() || ExecutionContextStack::TopContext()->strict();
    // 13.2 Creating Function Objects
    SetPrototype(FunctionProto::Instance());
    AddValueProperty(u"length", new Number(names.size()), false, false, false);
    JSValue* proto = new Object();
    if (!from_bind_) {
      // 15.3.5.2 prototype
      AddValueProperty(u"prototype", proto, true, false, false);
    }
    AddValueProperty(u"constructor", proto, true, false, true);
    if (strict_) {
      // TODO(zhuzilin) thrower
    }
  }

  LexicalEnvironment* Scope() { return scope_; };
  std::vector<std::u16string> FormalParameters() { return formal_params_; };
  AST* Code() { return body_; }
  bool strict() { return strict_; }

  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments) override {
    log::PrintSource("enter FunctionObject::Call ", body_->source());
    EnterFunctionCode(e, this, body_, this_arg, arguments, strict_);

    Completion comp;
    if (body_ != nullptr) {
      comp = EvalProgram(e, body_);
    }
    ExecutionContextStack::Global()->Pop();   // 3

    switch (comp.type) {
      case Completion::RETURN:
        return comp.value;
      case Completion::THROW:
        // TODO(zhuzilin) prevent nested ErrorObject.
        e = Error::NativeError(comp.value);
        return nullptr;
      default:
        assert(comp.type == Completion::NORMAL);
        return Undefined::Instance();
    }
  }

  // 13.2.2 [[Construct]]
  JSObject* Construct(Error* e, std::vector<JSValue*> arguments) override {
    log::PrintSource("enter FunctionObject::Construct");
    JSObject* obj = new JSObject(OBJ_OTHER, u"Object", true, nullptr, false, false);
    JSValue* proto = Get(e, u"prototype");
    if (proto->IsObject()) {  // 6
      obj->SetPrototype(proto);
    } else {  // 7
      obj->SetPrototype(ObjectProto::Instance());
    }
    JSValue* result = Call(e, obj, arguments);  // 8
    if (result->IsObject())  // 9
      return static_cast<JSObject*>(result);
    return obj;  // 10
  }

  // 15.3.5.3 [[HasInstance]] (V)
  bool HasInstance(Error* e, JSValue* V) override {
    if (!V->IsObject())
      return false;
    JSValue* O = Get(e, u"prototype");
    if (!e->IsOk()) return false;
    if (!O->IsObject()) {
      *e = *Error::TypeError();
      return false;
    }
    while (!V->IsNull()) {
      if (V == O)
        return true;
      V = static_cast<JSObject*>(V)->Get(e, u"prototype");
      if (!e->IsOk()) return false;
    }
    return false;
  }

  // 15.3.5.4 [[Get]] (P)
  JSValue* Get(Error* e, std::u16string P) override {
    JSValue* v = JSObject::Get(e, P);
    if (!e->IsOk()) return nullptr;
    if (P == u"caller") {  // 2
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

  std::string ToString() override { return "Function"; }

 private:
  std::vector<std::u16string> formal_params_;
  LexicalEnvironment* scope_;
  ProgramOrFunctionBody* body_;
  bool strict_;
  bool from_bind_;
};

class FunctionConstructor : public JSObject {
 public:
  static FunctionConstructor* Instance() {
    static FunctionConstructor singleton;
    return &singleton;
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
      body = ::es::ToString(e, arguments[0]);
    } else if (arg_count > 1) {
      P += ::es::ToString(e, arguments[0]);
      for (size_t i = 1; i < arg_count - 1; i++) {
        P += u"," + ::es::ToString(e, arguments[i]);
      }
      body = ::es::ToString(e, arguments[arg_count - 1]);
    }
    std::vector<std::u16string> names;
    AST* body_ast;
    if (P.size() > 0) {
      Parser parser(P);
      names = parser.ParseFormalParameterList();
      if (names.size() == 0) {
        *e = *Error::SyntaxError();
        return nullptr;
      }
    }
    {
      Parser parser(body);
      body_ast = parser.ParseFunctionBody(Token::TK_EOS);
      if (body_ast->IsIllegal()) {
        *e = *Error::SyntaxError();
        return nullptr;
      }
    }
    LexicalEnvironment* scope = LexicalEnvironment::Global();
    bool strict = static_cast<ProgramOrFunctionBody*>(body_ast)->strict();
    if (strict) {
      // 13.1
      if (HaveDuplicate(names)) {
        *e = *Error::SyntaxError();
        return nullptr;
      }
      for (auto name : names) {
        if (name == u"eval" || name == u"arguments") {
          *e = *Error::SyntaxError();
          return nullptr;
        }
      }
    }
    return new FunctionObject(names, body_ast, scope);
  }

 private:
  FunctionConstructor() :
    JSObject(OBJ_OTHER, u"Function", true, nullptr, true, true) {}
};

FunctionObject* InstantiateFunctionDeclaration(Error* e, Function* func_ast) {
    assert(func_ast->is_named());
    std::u16string identifier = func_ast->name();
    auto func_env = LexicalEnvironment::NewDeclarativeEnvironment(  // 1
      ExecutionContextStack::TopLexicalEnv()
    );
    auto env_rec = static_cast<DeclarativeEnvironmentRecord*>(func_env->env_rec());  // 2
    env_rec->CreateImmutableBinding(identifier);  // 3
    auto body = static_cast<ProgramOrFunctionBody*>(func_ast->body());
    bool strict = body->strict() || ExecutionContextStack::TopContext()->strict();
    if (strict) {
      // 13.1
      if (HaveDuplicate(func_ast->params())) {
        *e = *Error::SyntaxError();
        return nullptr;
      }
      for (auto name : func_ast->params()) {
        if (name == u"eval" || name == u"arguments") {
          *e = *Error::SyntaxError();
          return nullptr;
        }
      }
      if (func_ast->name() == u"eval" || func_ast->name() == u"arguments") {
        *e = *Error::SyntaxError();
        return nullptr;
      }
    }
    FunctionObject* closure = new FunctionObject(
      func_ast->params(), func_ast->body(), func_env);  // 4
    env_rec->InitializeImmutableBinding(identifier, closure);  // 5
    return closure;  // 6
}

JSValue* EvalFunction(Error* e, AST* ast) {
  assert(ast->type() == AST::AST_FUNC);
  Function* func_ast = static_cast<Function*>(ast);

  if (func_ast->is_named()) {
    return InstantiateFunctionDeclaration(e, func_ast);
  } else {
    auto body = static_cast<ProgramOrFunctionBody*>(func_ast->body());
    bool strict = body->strict() || ExecutionContextStack::TopContext()->strict();
    if (strict) {
      // 13.1
      if (HaveDuplicate(func_ast->params())) {
        *e = *Error::SyntaxError();
        return nullptr;
      }
      for (auto name : func_ast->params()) {
        if (name == u"eval" || name == u"arguments") {
          *e = *Error::SyntaxError();
          return nullptr;
        }
      }
    }
    return new FunctionObject(
      func_ast->params(), func_ast->body(),
      ExecutionContextStack::TopLexicalEnv()
    );
  }
}

}  // namespace es

#endif  // ES_TYPES_BUILTIN_FUNCTION_OBJECT