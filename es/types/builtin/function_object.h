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
    JSObject(
      OBJ_OTHER, u"Function", true, nullptr, false, true
    ) {}
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
    LexicalEnvironment* scope, bool strict, bool from_bind = false
  ) : JSObject(OBJ_FUNC, u"Function", true, nullptr, true, true),
      formal_params_(names), scope_(scope), strict_(strict), from_bind_(from_bind) {
    assert(body->type() == AST::AST_FUNC_BODY);
    body_ = static_cast<ProgramOrFunctionBody*>(body);

    AddValueProperty(u"length", new Number(names.size()), false, false, false);
    if (!from_bind_) {
      // 15.3.5.2 prototype
      AddValueProperty(u"prototype", new Object(), true, false, false);
    }
  }

  LexicalEnvironment* Scope() { return scope_; };
  std::vector<std::u16string> FormalParameters() { return formal_params_; };
  AST* Code() { return body_; }

  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments) override {
    log::PrintSource("enter FunctionObject::Call ", body_->source());
    EnterFunctionCode(e, this, body_, this_arg, arguments, strict_);
    log::PrintSource("after EnterFunctionCode");

    Completion comp;
    if (body_ != nullptr) {
      comp = EvalProgram(e, body_);
    }
    switch (comp.type) {
      case Completion::RETURN:
        return comp.value;
      case Completion::THROW:
        // TODO(zhuzilin) how to throw value?
        assert(false);
      default:
        assert(comp.type == Completion::NORMAL);
        return Undefined::Instance();
    }
    ExecutionContextStack::Global()->Pop();   // 3
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
    bool strict = false;
    log::PrintSource("P: ", P);
    if (P.size() > 0) {
      Parser parser(P);
      names = parser.ParseFormalParameterList();
      if (names.size() == 0) {
        e = Error::SyntaxError();
        return nullptr;
      }
    }
    log::PrintSource("body: ", body);
    {
      Parser parser(body);
      body_ast = parser.ParseFunctionBody(Token::TK_EOS);
      if (body_ast->IsIllegal()) {
        e = Error::SyntaxError();
        return nullptr;
      }
    }
    LexicalEnvironment* scope = LexicalEnvironment::Global();
    return new FunctionObject(names, body_ast, scope, strict, false);
  }

 private:
  FunctionConstructor() :
    JSObject(
      OBJ_OTHER, u"Function", false, nullptr, true, true
    ) {}
};

FunctionObject* InstantiateFunctionDeclaration(Function* func_ast) {
    assert(func_ast->is_named());
    std::u16string identifier = func_ast->name();
    auto func_env = LexicalEnvironment::NewDeclarativeEnvironment(  // 1
      ExecutionContextStack::Global()->Top().lexical_env()
    );
    auto env_rec = static_cast<DeclarativeEnvironmentRecord*>(func_env->env_rec());  // 2
    env_rec->CreateImmutableBinding(identifier);  // 3
    FunctionObject* closure = new FunctionObject(
      func_ast->params(), func_ast->body(), func_env, false);  // 4
    env_rec->InitializeImmutableBinding(identifier, closure);  // 5
    return closure;  // 6
}

JSValue* EvalFunction(AST* ast) {
  assert(ast->type() == AST::AST_FUNC);
  Function* func_ast = static_cast<Function*>(ast);

  // TODO(zhuzilin) strict.
  if (func_ast->is_named()) {
    return InstantiateFunctionDeclaration(func_ast);
  } else {
    return new FunctionObject(
      func_ast->params(), func_ast->body(),
      ExecutionContextStack::Global()->Top().lexical_env(), false
    );
  }
}

}  // namespace es

#endif  // ES_TYPES_BUILTIN_FUNCTION_OBJECT