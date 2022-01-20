#ifndef ES_ENTER_CODE_H
#define ES_ENTER_CODE_H

#include <es/error.h>
#include <es/execution_context.h>
#include <es/types/object.h>
#include <es/types/builtin/object_object.h>
#include <es/types/builtin/function_object.h>
#include <es/types/builtin/number_object.h>
#include <es/types/builtin/error_object.h>
#include <es/types/builtin/bool_object.h>
#include <es/types/builtin/string_object.h>
#include <es/types/builtin/array_object.h>
#include <es/types/builtin/date_object.h>
#include <es/types/builtin/math_object.h>
#include <es/types/builtin/arguments_object.h>
#include <es/types/host/console.h>
#include <es/regex/match.h>

namespace es {

enum CodeType {
  CODE_GLOBAL = 0,
  CODE_FUNC,
  CODE_EVAL,
};

JSValue* MakeArgGetter(std::u16string name, LexicalEnvironment* env) {
  Parser parser(u"return " + name + u";");
  ProgramOrFunctionBody* body = static_cast<ProgramOrFunctionBody*>(
    parser.ParseFunctionBody(Token::TK_EOS));
  return new FunctionObject({}, body, env);
}

JSValue* MakeArgSetter(std::u16string name, LexicalEnvironment* env) {
  std::u16string param = name + u"_arg";
  Parser parser(name + u" = " + param);
  ProgramOrFunctionBody* body = static_cast<ProgramOrFunctionBody*>(
    parser.ParseFunctionBody(Token::TK_EOS));
  return new FunctionObject({param}, body, env);
}

// 10.6 Arguments Object
JSObject* CreateArgumentsObject(
  FunctionObject* func, std::vector<JSValue*>& args,
  LexicalEnvironment* env, bool strict
) {
  std::vector<std::u16string> names = func->FormalParameters();
  int len = args.size();
  Object* map = new Object();  // 8
  JSObject* obj = new ArgumentsObject(map, len);
  int indx = len - 1;  // 10
  std::set<std::u16string> mapped_names;
  while (indx >= 0) {  // 11
    JSValue* val = args[indx];  // 11.a
    obj->AddValueProperty(NumberToString(indx), val, true, true, true);  // 11.b
    if (indx < names.size()) {  // 11.c
      std::u16string name = names[indx];  // 11.c.i
      if (!strict && mapped_names.find(name) == mapped_names.end()) {  // 11.c.ii
        mapped_names.insert(name);
        JSValue* g = MakeArgGetter(name, env);
        JSValue* p = MakeArgSetter(name, env);
        PropertyDescriptor* desc = new PropertyDescriptor();
        desc->SetSet(p);
        desc->SetGet(g);
        desc->SetConfigurable(true);
        map->DefineOwnProperty(nullptr, NumberToString(indx), desc, false);
      }
    }
    indx--;  // 11.d
  }
  if (!strict) {  // 13
    obj->AddValueProperty(u"callee", func, true, false, true);
  } else {  // 14
    // TODO(zhuzilin) thrower
  }
  return obj;  // 15
}

void FindAllVarDecl(std::vector<AST*> stmts, std::vector<VarDecl*>& decls) {
  for (auto stmt : stmts) {
    switch (stmt->type()) {
      case AST::AST_STMT_BLOCK: {
        Block* block = static_cast<Block*>(stmt);
        FindAllVarDecl(block->statements(), decls);
        break;
      }
      case AST::AST_STMT_VAR: {
        VarStmt* var_stmt = static_cast<VarStmt*>(stmt);
        for (auto d : var_stmt->decls()) {
          decls.emplace_back(d);
        }
        break;
      }
      case AST::AST_STMT_IF: {
        If* if_stmt = static_cast<If*>(stmt);
        FindAllVarDecl({if_stmt->if_block()}, decls);
        if (if_stmt->else_block() != nullptr)
          FindAllVarDecl({if_stmt->else_block()}, decls);
        break;
      }
      case AST::AST_STMT_WHILE:
      case AST::AST_STMT_WITH: {
        WhileOrWith* while_stmt = static_cast<WhileOrWith*>(stmt);
        FindAllVarDecl({while_stmt->stmt()}, decls);
        break;
      }
      case AST::AST_STMT_DO_WHILE: {
        DoWhile* do_while_stmt = static_cast<DoWhile*>(stmt);
        FindAllVarDecl({do_while_stmt->stmt()}, decls);
        break;
      }
      case AST::AST_STMT_FOR: {
        For* for_stmt = static_cast<For*>(stmt);
        if (for_stmt->expr0s().size() > 0 && for_stmt->expr0s()[0]->type() == AST::AST_STMT_VAR_DECL) {
          for (AST* ast : for_stmt->expr0s()) {
            VarDecl* d = static_cast<VarDecl*>(ast);
            decls.emplace_back(d);
          }
        }
        FindAllVarDecl({for_stmt->statement()}, decls);
        break;
      }
      case AST::AST_STMT_FOR_IN: {
        ForIn* for_in_stmt = static_cast<ForIn*>(stmt);
        if (for_in_stmt->expr0()->type() == AST::AST_STMT_VAR_DECL) {
            VarDecl* d = static_cast<VarDecl*>(for_in_stmt->expr0());
            decls.emplace_back(d);
        }
        FindAllVarDecl({for_in_stmt->statement()}, decls);
        break;
      }
      case AST::AST_STMT_LABEL: {
        LabelledStmt* label = static_cast<LabelledStmt*>(stmt);
        FindAllVarDecl({label->statement()}, decls);
        break;
      }
      case AST::AST_STMT_SWITCH: {
        Switch* switch_stmt = static_cast<Switch*>(stmt);
        for (auto clause : switch_stmt->before_default_case_clauses()) {
          FindAllVarDecl(clause.stmts, decls);
        }
        if (switch_stmt->has_default_clause()) {
          FindAllVarDecl(switch_stmt->default_clause().stmts, decls);
        }
        for (auto clause : switch_stmt->after_default_case_clauses()) {
          FindAllVarDecl(clause.stmts, decls);
        }
        break;
      }
      case AST::AST_STMT_TRY: {
        Try* try_stmt = static_cast<Try*>(stmt);
        FindAllVarDecl({try_stmt->try_block()}, decls);
        if (try_stmt->catch_block() != nullptr)
          FindAllVarDecl({try_stmt->catch_block()}, decls);
        if (try_stmt->finally_block() != nullptr)
          FindAllVarDecl({try_stmt->finally_block()}, decls);
        break;
      }
      // TODO(zhuzilin) fill the other statements.
      default:
        break;
    }
  }
}

// 10.5 Declaration Binding Instantiation
void DeclarationBindingInstantiation(
  Error* e, ExecutionContext* context, AST* code, CodeType code_type,
  FunctionObject* f = nullptr, std::vector<JSValue*> args = {}
) {
  auto env = context->variable_env()->env_rec();  // 1
  bool configurable_bindings = false;
  ProgramOrFunctionBody* body = static_cast<ProgramOrFunctionBody*>(code);
  if (code_type == CODE_EVAL) {
    configurable_bindings = true;  // 2
  }
  bool strict = body->strict();  // 3
  if (code_type == CODE_FUNC) {  // 4
    assert(f != nullptr);
    auto names = f->FormalParameters();  // 4.a
    Function* func_ast = static_cast<Function*>(code);
    size_t arg_count = args.size();  // 4.b
    size_t n = 0;  // 4.c
    for (auto arg_name : names) {  // 4.d
      JSValue* v = Undefined::Instance();
      if (n < arg_count)  // 4.d.i & 4.d.ii
        v = args[n++];
      bool arg_already_declared = env->HasBinding(arg_name);  // 4.d.iii
      if (!arg_already_declared) {  // 4.d.iv
        // NOTE(zhuzlin) I'm not sure if this should be false.
        env->CreateMutableBinding(e, arg_name, false);
        if (!e->IsOk()) return;
      }
      env->SetMutableBinding(e, arg_name, v, strict);  // 4.d.v
      if (!e->IsOk()) return;
    }
  }
  // 5
  for (Function* func_decl : body->func_decls()) {
    assert(func_decl->is_named());
    std::u16string fn = func_decl->name();
    FunctionObject* fo = InstantiateFunctionDeclaration(e, func_decl);
    if (!e->IsOk()) return;
    bool func_already_declared = env->HasBinding(fn);
    if (!func_already_declared) {  // 5.d
      env->CreateMutableBinding(e, fn, configurable_bindings);
      if (!e->IsOk()) return;
    } else {  // 5.e
      auto go = GlobalObject::Instance();
      auto existing_prop = go->GetProperty(fn);
      assert(!existing_prop->IsUndefined());
      auto existing_prop_desc = static_cast<PropertyDescriptor*>(existing_prop);
      if (existing_prop_desc->Configurable()) {  // 5.e.iii
        auto new_desc = new PropertyDescriptor();
        new_desc->SetDataDescriptor(Undefined::Instance(), true, true, configurable_bindings);
        go->DefineOwnProperty(e, fn, new_desc, true);
        if (!e->IsOk()) return;
      } else {  // 5.e.iv
        if (existing_prop_desc->IsAccessorDescriptor() ||
            !(existing_prop_desc->HasConfigurable() && existing_prop_desc->Configurable() &&
              existing_prop_desc->HasEnumerable() && existing_prop_desc->Enumerable())) {
          *e = *Error::TypeError();
          return;
        }
      }
    }
    env->SetMutableBinding(e, fn, fo, strict);  // 5.f
  }
  // 6
  bool arguments_already_declared = env->HasBinding(u"arguments");
  // 7
  if (code_type == CODE_FUNC && !arguments_already_declared) {
    auto args_obj = CreateArgumentsObject(f, args, context->variable_env(), strict);
    if (strict) {  // 7.b
      DeclarativeEnvironmentRecord* decl_env = static_cast<DeclarativeEnvironmentRecord*>(env);
      decl_env->CreateImmutableBinding(u"arguments");
      decl_env->InitializeImmutableBinding(u"arguments", args_obj);
    } else {  // 7.c
      // NOTE(zhuzlin) I'm not sure if this should be false.
      env->CreateMutableBinding(e, u"arguments", false);
      env->SetMutableBinding(e, u"arguments", args_obj, false);
    }
  }
  // 8
  std::vector<VarDecl*> decls;
  FindAllVarDecl(body->statements(), decls);
  for (VarDecl* d : decls) {
    std::u16string dn = d->ident();
    bool var_already_declared = env->HasBinding(dn);
    if (!var_already_declared) {
      env->CreateMutableBinding(e, dn, configurable_bindings);
      if (!e->IsOk()) return;
      env->SetMutableBinding(e, dn, Undefined::Instance(), strict);
      if (!e->IsOk()) return;
    }
  }
}

// 10.4.1
void EnterGlobalCode(Error* e, AST* ast) {
  ProgramOrFunctionBody* program;
  if (ast->type() == AST::AST_PROGRAM) {
    program = static_cast<ProgramOrFunctionBody*>(ast);
  } else {
    // TODO(zhuzilin) This is for test. Add test label like #ifdefine TEST
    program = new ProgramOrFunctionBody(AST::AST_PROGRAM, false);
    program->AddStatement(ast);
  }
  // 1 10.4.1.1
  LexicalEnvironment* global_env = LexicalEnvironment::Global();
  ExecutionContext* context = new ExecutionContext(global_env, global_env, GlobalObject::Instance(), program->strict());
  Runtime::Global()->AddContext(context);
  // 2
  DeclarationBindingInstantiation(e, context, program, CODE_GLOBAL);
}

// 10.4.2
void EnterEvalCode(Error* e, AST* ast) {
  assert(ast->type() == AST::AST_PROGRAM);
  ProgramOrFunctionBody* program = static_cast<ProgramOrFunctionBody*>(ast);
  ExecutionContext* context;
  LexicalEnvironment* variable_env;
  LexicalEnvironment* lexical_env;
  JSValue* this_binding;
  if (!GlobalObject::Instance()->direct_eval()) {  // 1
    LexicalEnvironment* global_env = LexicalEnvironment::Global();
    variable_env = global_env;
    lexical_env = global_env;
    this_binding = GlobalObject::Instance();
  } else {  // 2
    ExecutionContext* calling_context = Runtime::TopContext();
    variable_env = calling_context->variable_env();
    lexical_env = calling_context->lexical_env();
    this_binding = calling_context->this_binding();
  }
  bool strict = Runtime::TopContext()->strict() ||
                (program->strict() && GlobalObject::Instance()->direct_eval());
  if (strict) {  // 3
    LexicalEnvironment* strict_var_env = LexicalEnvironment::NewDeclarativeEnvironment(lexical_env);
    lexical_env = strict_var_env;
    variable_env = strict_var_env;
  }
  context = new ExecutionContext(variable_env, lexical_env, this_binding, strict);
  Runtime::Global()->AddContext(context);
  // 4
  DeclarationBindingInstantiation(e, context, program, CODE_EVAL);
}

// 15.1.2.1 eval(X)
JSValue* GlobalObject::eval(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
  log::PrintSource("enter GlobalObject::eval");
  if (vals.size() == 0)
    return Undefined::Instance();
  if (!vals[0]->IsString())
    return vals[0];
  std::u16string x = static_cast<String*>(vals[0])->data();
  Parser parser(x);
  AST* program = parser.ParseProgram();
  if (program->IsIllegal()) {
    *e = *Error::SyntaxError(u"failed to parse eval");
    return nullptr;
  }
  EnterEvalCode(e, program);
  if (!e->IsOk()) return nullptr;
  Completion result = EvalProgram(program);
  Runtime::Global()->PopContext();

  switch (result.type) {
    case Completion::NORMAL:
      if (result.value != nullptr)
        return result.value;
      else
        return Undefined::Instance();
    default: {
      assert(result.type == Completion::THROW);
      std::u16string message = ::es::ToString(e, result.value);
      if (result.value->IsObject()) {
        JSObject* obj = static_cast<JSObject*>(result.value);
        if (obj->obj_type() == JSObject::OBJ_ERROR) {
          message = static_cast<ErrorObject*>(obj)->ErrorMessage();
        }
      }
      *e = *Error::NativeError(message);
      return result.value;
    }
  }
}

// 10.4.3
void EnterFunctionCode(
  Error* e, JSObject* f, ProgramOrFunctionBody* body,
  JSValue* this_arg, std::vector<JSValue*> args, bool strict
) {
  assert(f->obj_type() == JSObject::OBJ_FUNC);
  FunctionObject* func = static_cast<FunctionObject*>(f);
  JSValue* this_binding;
  if (strict) {  // 1
    this_binding = this_arg;
  } else {  // 2 & 3
    this_binding = (this_arg->IsUndefined() || this_arg->IsNull()) ?
      GlobalObject::Instance() : this_arg;
  }
  LexicalEnvironment* local_env = LexicalEnvironment::NewDeclarativeEnvironment(func->Scope());
  ExecutionContext* context = new ExecutionContext(local_env, local_env, this_binding, strict);  // 8
  Runtime::Global()->AddContext(context);
  // 9
  DeclarationBindingInstantiation(e, context, body, CODE_FUNC, func, args);
}

void InitGlobalObject() {
  auto global_obj = GlobalObject::Instance();
  // 15.1.1 Value Properties of the Global Object
  global_obj->AddValueProperty(u"NaN", Number::NaN(), false, false, false);
  global_obj->AddValueProperty(u"Infinity", Number::PositiveInfinity(), false, false, false);
  global_obj->AddValueProperty(u"undefined", Undefined::Instance(), false, false, false);
  // 15.1.2 Function Properties of the Global Object
  global_obj->AddFuncProperty(u"eval", GlobalObject::eval, true, false, true);
  global_obj->AddFuncProperty(u"parseInt", GlobalObject::parseInt, true, false, true);
  global_obj->AddFuncProperty(u"parseFloat", GlobalObject::parseFloat, true, false, true);
  global_obj->AddFuncProperty(u"isNaN", GlobalObject::isNaN, true, false, true);
  global_obj->AddFuncProperty(u"isFinite", GlobalObject::isFinite, true, false, true);
  // 15.1.3 URI Handling Function Properties
  // TODO(zhuzilin)
  // 15.1.4 Constructor Properties of the Global Object
  global_obj->AddValueProperty(u"Object", ObjectConstructor::Instance(), true, false, true);
  global_obj->AddValueProperty(u"Function", FunctionConstructor::Instance(), true, false, true);
  global_obj->AddValueProperty(u"Number", NumberConstructor::Instance(), true, false, true);
  global_obj->AddValueProperty(u"Boolean", BoolConstructor::Instance(), true, false, true);
  global_obj->AddValueProperty(u"String", StringConstructor::Instance(), true, false, true);
  global_obj->AddValueProperty(u"Array", ArrayConstructor::Instance(), true, false, true);
  global_obj->AddValueProperty(u"Date", DateConstructor::Instance(), true, false, true);
  global_obj->AddValueProperty(u"Error", ErrorConstructor::Instance(), true, false, true);
  // TODO(zhuzilin) differentiate errors.
  global_obj->AddValueProperty(u"EvalError", ErrorConstructor::Instance(), true, false, true);
  global_obj->AddValueProperty(u"RangeError", ErrorConstructor::Instance(), true, false, true);
  global_obj->AddValueProperty(u"ReferenceError", ErrorConstructor::Instance(), true, false, true);
  global_obj->AddValueProperty(u"SyntaxError", ErrorConstructor::Instance(), true, false, true);
  global_obj->AddValueProperty(u"TypeError", ErrorConstructor::Instance(), true, false, true);
  global_obj->AddValueProperty(u"URIError", ErrorConstructor::Instance(), true, false, true);

  global_obj->AddValueProperty(u"Math", Math::Instance(), true, false, true);

  global_obj->AddValueProperty(u"console", Console::Instance(), true, false, true);
}

void InitObject() {
  ObjectConstructor* constructor = ObjectConstructor::Instance();
  constructor->SetPrototype(FunctionProto::Instance());
  // 15.2.3 Properties of the Object Constructor
  constructor->AddValueProperty(u"prototype", ObjectProto::Instance(), false, false, false);
  constructor->AddFuncProperty(u"toString", ObjectConstructor::toString, true, false, false);
  constructor->AddFuncProperty(u"getPrototypeOf", ObjectConstructor::getPrototypeOf, true, false, false);
  constructor->AddFuncProperty(u"getOwnPropertyDescriptor", ObjectConstructor::getOwnPropertyDescriptor, true, false, false);
  constructor->AddFuncProperty(u"getOwnPropertyNames", ObjectConstructor::getOwnPropertyNames, true, false, false);
  constructor->AddFuncProperty(u"create", ObjectConstructor::create, true, false, false);
  constructor->AddFuncProperty(u"defineProperty", ObjectConstructor::defineProperty, true, false, false);
  constructor->AddFuncProperty(u"defineProperties", ObjectConstructor::defineProperties, true, false, false);
  constructor->AddFuncProperty(u"seal", ObjectConstructor::seal, true, false, false);
  constructor->AddFuncProperty(u"freeze", ObjectConstructor::freeze, true, false, false);
  constructor->AddFuncProperty(u"preventExtensions", ObjectConstructor::preventExtensions, true, false, false);
  constructor->AddFuncProperty(u"isSealed", ObjectConstructor::isSealed, true, false, false);
  constructor->AddFuncProperty(u"isFrozen", ObjectConstructor::isFrozen, true, false, false);
  constructor->AddFuncProperty(u"isExtensible", ObjectConstructor::isExtensible, true, false, false);
  constructor->AddFuncProperty(u"keys", ObjectConstructor::keys, true, false, false);
  // ES6
  constructor->AddFuncProperty(u"setPrototypeOf", ObjectConstructor::setPrototypeOf, true, false, false);

  ObjectProto* proto = ObjectProto::Instance();
  // 15.2.4 Properties of the Object Prototype Object
  proto->AddValueProperty(u"constructor", ObjectConstructor::Instance(), false, false, false);
  proto->AddFuncProperty(u"toString", ObjectProto::toString, true, false, false);
  proto->AddFuncProperty(u"toLocaleString", ObjectProto::toLocaleString, true, false, false);
  proto->AddFuncProperty(u"valueOf", ObjectProto::valueOf, true, false, false);
  proto->AddFuncProperty(u"hasOwnProperty", ObjectProto::hasOwnProperty, true, false, false);
  proto->AddFuncProperty(u"isPrototypeOf", ObjectProto::isPrototypeOf, true, false, false);
  proto->AddFuncProperty(u"propertyIsEnumerable", ObjectProto::propertyIsEnumerable, true, false, false);
}

void InitFunction() {
  FunctionConstructor* constructor = FunctionConstructor::Instance();
  constructor->SetPrototype(FunctionProto::Instance());
  // 15.3.3 Properties of the Function Constructor
  constructor->AddValueProperty(u"prototype", FunctionProto::Instance(), false, false, false);
  constructor->AddValueProperty(u"length", Number::One(), false, false, false);
  constructor->AddFuncProperty(u"toString", FunctionConstructor::toString, true, false, false);

  FunctionProto* proto = FunctionProto::Instance();
  proto->SetPrototype(ObjectProto::Instance());
  // 15.2.4 Properties of the Function Prototype Function
  proto->AddValueProperty(u"constructor", FunctionConstructor::Instance(), false, false, false);
  proto->AddFuncProperty(u"toString", FunctionProto::toString, true, false, false);
  proto->AddFuncProperty(u"apply", FunctionProto::apply, true, false, false);
  proto->AddFuncProperty(u"call", FunctionProto::call, true, false, false);
  proto->AddFuncProperty(u"bind", FunctionProto::bind, true, false, false);
}

void InitNumber() {
  NumberConstructor* constructor = NumberConstructor::Instance();
  constructor->SetPrototype(FunctionProto::Instance());
  // 15.3.3 Properties of the Number Constructor
  constructor->AddValueProperty(u"prototype", NumberProto::Instance(), false, false, false);
  constructor->AddValueProperty(u"length", Number::One(), false, false, false);
  constructor->AddValueProperty(u"MAX_VALUE", new Number(1.7976931348623157e308), false, false, false);
  constructor->AddValueProperty(u"MIN_VALUE", new Number(5e-324), false, false, false);
  constructor->AddValueProperty(u"NaN", Number::NaN(), false, false, false);
  constructor->AddValueProperty(u"NEGATIVE_INFINITY", Number::PositiveInfinity(), false, false, false);
  constructor->AddValueProperty(u"POSITIVE_INFINITY", Number::NegativeInfinity(), false, false, false);

  NumberProto* proto = NumberProto::Instance();
  proto->SetPrototype(ObjectProto::Instance());
  // 15.2.4 Properties of the Number Prototype Number
  proto->AddValueProperty(u"constructor", NumberConstructor::Instance(), false, false, false);
  proto->AddFuncProperty(u"toString", NumberProto::toString, true, false, false);
  proto->AddFuncProperty(u"toLocaleString", NumberProto::toLocaleString, true, false, false);
  proto->AddFuncProperty(u"valueOf", NumberProto::valueOf, true, false, false);
  proto->AddFuncProperty(u"toFixed", NumberProto::toFixed, true, false, false);
  proto->AddFuncProperty(u"toExponential", NumberProto::toExponential, true, false, false);
  proto->AddFuncProperty(u"toPrecision", NumberProto::toPrecision, true, false, false);
}

void InitError() {
  ErrorConstructor* constructor = ErrorConstructor::Instance();
  constructor->SetPrototype(FunctionProto::Instance());
  // 15.11.3 Properties of the Error Constructor
  constructor->AddValueProperty(u"prototype", ErrorProto::Instance(), false, false, false);
  constructor->AddValueProperty(u"length", Number::One(), false, false, false);
  constructor->AddFuncProperty(u"toString", NumberConstructor::toString, true, false, false);

  ErrorProto* proto = ErrorProto::Instance();
  proto->SetPrototype(ObjectProto::Instance());
  // 15.11.4 Properties of the Error Prototype Object
  proto->AddValueProperty(u"constructor", ErrorConstructor::Instance(), false, false, false);
  proto->AddValueProperty(u"name", new String(u"Error"), false, false, false);
  proto->AddValueProperty(u"message", String::Empty(), true, false, false);
  proto->AddFuncProperty(u"call", ErrorProto::toString, true, false, false);
}

void InitBool() {
  BoolConstructor* constructor = BoolConstructor::Instance();
  constructor->SetPrototype(FunctionProto::Instance());
  // 15.6.3 Properties of the Boolean Constructor
  constructor->AddValueProperty(u"prototype", BoolProto::Instance(), false, false, false);
  constructor->AddFuncProperty(u"toString", BoolConstructor::toString, true, false, false);

  BoolProto* proto = BoolProto::Instance();
  proto->SetPrototype(ObjectProto::Instance());
  // 15.6.4 Properties of the Boolean Prototype Object
  proto->AddValueProperty(u"constructor", BoolConstructor::Instance(), false, false, false);
  proto->AddFuncProperty(u"toString", BoolProto::toString, true, false, false);
  proto->AddFuncProperty(u"valueOf", BoolProto::valueOf, true, false, false);
}

void InitString() {
  StringConstructor* constructor = StringConstructor::Instance();
  constructor->SetPrototype(FunctionProto::Instance());
  // 15.3.3 Properties of the String Constructor
  constructor->AddValueProperty(u"prototype", StringProto::Instance(), false, false, false);
  constructor->AddValueProperty(u"length", Number::One(), true, false, false);
  constructor->AddFuncProperty(u"fromCharCode", StringConstructor::fromCharCode, true, false, false);
  constructor->AddFuncProperty(u"toString", StringConstructor::toString, true, false, false);

  StringProto* proto = StringProto::Instance();
  proto->SetPrototype(ObjectProto::Instance());
  // 15.2.4 Properties of the String Prototype String
  proto->AddValueProperty(u"constructor", StringConstructor::Instance(), false, false, false);
  proto->AddFuncProperty(u"toString", StringProto::toString, true, false, false);
  proto->AddFuncProperty(u"valueOf", StringProto::valueOf, true, false, false);
  proto->AddFuncProperty(u"charAt", StringProto::charAt, true, false, false);
  proto->AddFuncProperty(u"charCodeAt", StringProto::charCodeAt, true, false, false);
  proto->AddFuncProperty(u"concat", StringProto::concat, true, false, false);
  proto->AddFuncProperty(u"indexOf", StringProto::indexOf, true, false, false);
  proto->AddFuncProperty(u"lastIndexOf", StringProto::lastIndexOf, true, false, false);
  proto->AddFuncProperty(u"localeCompare", StringProto::localeCompare, true, false, false);
  proto->AddFuncProperty(u"match", StringProto::match, true, false, false);
  proto->AddFuncProperty(u"replace", StringProto::replace, true, false, false);
  proto->AddFuncProperty(u"search", StringProto::search, true, false, false);
  proto->AddFuncProperty(u"slice", StringProto::slice, true, false, false);
  proto->AddFuncProperty(u"split", StringProto::split, true, false, false);
  proto->AddFuncProperty(u"substring", StringProto::substring, true, false, false);
  proto->AddFuncProperty(u"toLowerCase", StringProto::toLowerCase, true, false, false);
  proto->AddFuncProperty(u"toLocaleLowerCase", StringProto::toLocaleLowerCase, true, false, false);
  proto->AddFuncProperty(u"toUpperCase", StringProto::toUpperCase, true, false, false);
  proto->AddFuncProperty(u"toLocaleUpperCase", StringProto::toLocaleUpperCase, true, false, false);
  proto->AddFuncProperty(u"trim", StringProto::trim, true, false, false);
}

void InitArray() {
  ArrayConstructor* constructor = ArrayConstructor::Instance();
  constructor->SetPrototype(FunctionProto::Instance());
  // 15.6.3 Properties of the Arrayean Constructor
  constructor->AddValueProperty(u"length", Number::One(), false, false, false);
  constructor->AddValueProperty(u"prototype", ArrayProto::Instance(), false, false, false);
  constructor->AddFuncProperty(u"isArray", ArrayConstructor::isArray, true, false, false);
  constructor->AddFuncProperty(u"toString", ArrayConstructor::toString, true, false, false);

  ArrayProto* proto = ArrayProto::Instance();
  proto->SetPrototype(ObjectProto::Instance());
  // 15.6.4 Properties of the Arrayean Prototype Object
  proto->AddValueProperty(u"length", Number::Zero(), false, false, false);
  proto->AddValueProperty(u"constructor", ArrayConstructor::Instance(), false, false, false);
  proto->AddFuncProperty(u"toString", ArrayProto::toString, true, false, false);
  proto->AddFuncProperty(u"toLocaleString", ArrayProto::toLocaleString, true, false, false);
  proto->AddFuncProperty(u"concat", ArrayProto::concat, true, false, false);
  proto->AddFuncProperty(u"join", ArrayProto::join, true, false, false);
  proto->AddFuncProperty(u"pop", ArrayProto::pop, true, false, false);
  proto->AddFuncProperty(u"push", ArrayProto::push, true, false, false);
  proto->AddFuncProperty(u"reverse", ArrayProto::reverse, true, false, false);
  proto->AddFuncProperty(u"shift", ArrayProto::shift, true, false, false);
  proto->AddFuncProperty(u"slice", ArrayProto::slice, true, false, false);
  proto->AddFuncProperty(u"sort", ArrayProto::sort, true, false, false);
  proto->AddFuncProperty(u"splice", ArrayProto::splice, true, false, false);
  proto->AddFuncProperty(u"unshift", ArrayProto::unshift, true, false, false);
  proto->AddFuncProperty(u"indexOf", ArrayProto::indexOf, true, false, false);
  proto->AddFuncProperty(u"lastIndexOf", ArrayProto::lastIndexOf, true, false, false);
  proto->AddFuncProperty(u"every", ArrayProto::every, true, false, false);
  proto->AddFuncProperty(u"some", ArrayProto::some, true, false, false);
  proto->AddFuncProperty(u"forEach", ArrayProto::forEach, true, false, false);
  proto->AddFuncProperty(u"map", ArrayProto::map, true, false, false);
  proto->AddFuncProperty(u"filter", ArrayProto::filter, true, false, false);
  proto->AddFuncProperty(u"reduce", ArrayProto::reduce, true, false, false);
  proto->AddFuncProperty(u"reduceRight", ArrayProto::reduceRight, true, false, false);
}

void InitDate() {
  DateConstructor* constructor = DateConstructor::Instance();
  constructor->SetPrototype(FunctionProto::Instance());
  // 15.6.3 Properties of the Dateean Constructor
  constructor->AddValueProperty(u"length", new Number(7), false, false, false);
  constructor->AddValueProperty(u"prototype", DateProto::Instance(), false, false, false);
  constructor->AddFuncProperty(u"parse", DateConstructor::parse, true, false, false);
  constructor->AddFuncProperty(u"UTC", DateConstructor::UTC, true, false, false);
  constructor->AddFuncProperty(u"now", DateConstructor::now, true, false, false);
  constructor->AddFuncProperty(u"toString", DateConstructor::toString, true, false, false);

  DateProto* proto = DateProto::Instance();
  proto->SetPrototype(ObjectProto::Instance());
  // 15.6.4 Properties of the Dateean Prototype Object
  proto->AddValueProperty(u"length", Number::Zero(), false, false, false);
  proto->AddValueProperty(u"constructor", DateConstructor::Instance(), false, false, false);
  proto->AddFuncProperty(u"toString", DateProto::toString, true, false, false);
  proto->AddFuncProperty(u"toDateString", DateProto::toDateString, true, false, false);
  proto->AddFuncProperty(u"toTimeString", DateProto::toTimeString, true, false, false);
  proto->AddFuncProperty(u"toLocaleString", DateProto::toLocaleString, true, false, false);
  proto->AddFuncProperty(u"toLocaleDateString", DateProto::toLocaleDateString, true, false, false);
  proto->AddFuncProperty(u"toLocaleTimeString", DateProto::toLocaleTimeString, true, false, false);
  proto->AddFuncProperty(u"valueOf", DateProto::valueOf, true, false, false);
  proto->AddFuncProperty(u"getTime", DateProto::getTime, true, false, false);
  proto->AddFuncProperty(u"getFullYear", DateProto::getFullYear, true, false, false);
  proto->AddFuncProperty(u"getUTCFullYear", DateProto::getUTCFullYear, true, false, false);
  proto->AddFuncProperty(u"getMonth", DateProto::getMonth, true, false, false);
  proto->AddFuncProperty(u"getUTCMonth", DateProto::getUTCMonth, true, false, false);
  proto->AddFuncProperty(u"getDate", DateProto::getDate, true, false, false);
  proto->AddFuncProperty(u"getUTCDate", DateProto::getUTCDate, true, false, false);
  proto->AddFuncProperty(u"getUTCDay", DateProto::getUTCDay, true, false, false);
  proto->AddFuncProperty(u"getHours", DateProto::getHours, true, false, false);
  proto->AddFuncProperty(u"getUTCHours", DateProto::getUTCHours, true, false, false);
  proto->AddFuncProperty(u"getMinutes", DateProto::getMinutes, true, false, false);
  proto->AddFuncProperty(u"getUTCMinutes", DateProto::getUTCMinutes, true, false, false);
  proto->AddFuncProperty(u"getSeconds", DateProto::getSeconds, true, false, false);
  proto->AddFuncProperty(u"getUTCSeconds", DateProto::getUTCSeconds, true, false, false);
  proto->AddFuncProperty(u"getMilliseconds", DateProto::getMilliseconds, true, false, false);
  proto->AddFuncProperty(u"getUTCMilliseconds", DateProto::getUTCMilliseconds, true, false, false);
  proto->AddFuncProperty(u"getTimezoneOffset", DateProto::getTimezoneOffset, true, false, false);
  proto->AddFuncProperty(u"setTime", DateProto::setTime, true, false, false);
  proto->AddFuncProperty(u"setMilliseconds", DateProto::setMilliseconds, true, false, false);
  proto->AddFuncProperty(u"setUTCMilliseconds", DateProto::setUTCMilliseconds, true, false, false);
  proto->AddFuncProperty(u"setSeconds", DateProto::setSeconds, true, false, false);
  proto->AddFuncProperty(u"setUTCSeconds", DateProto::setUTCSeconds, true, false, false);
  proto->AddFuncProperty(u"setMinutes", DateProto::setMinutes, true, false, false);
  proto->AddFuncProperty(u"setHours", DateProto::setHours, true, false, false);
  proto->AddFuncProperty(u"setUTCHours", DateProto::setUTCHours, true, false, false);
  proto->AddFuncProperty(u"setDate", DateProto::setDate, true, false, false);
  proto->AddFuncProperty(u"setUTCDate", DateProto::setUTCDate, true, false, false);
  proto->AddFuncProperty(u"setMonth", DateProto::setMonth, true, false, false);
  proto->AddFuncProperty(u"setUTCMonth", DateProto::setUTCMonth, true, false, false);
  proto->AddFuncProperty(u"setFullYear", DateProto::setFullYear, true, false, false);
  proto->AddFuncProperty(u"setUTCFullYear", DateProto::setUTCFullYear, true, false, false);
  proto->AddFuncProperty(u"toUTCString", DateProto::toUTCString, true, false, false);
  proto->AddFuncProperty(u"toISOString", DateProto::toISOString, true, false, false);
  proto->AddFuncProperty(u"toJSON", DateProto::toJSON, true, false, false);
}

void Init() {
  InitGlobalObject();
  InitObject();
  InitFunction();
  InitNumber();
  InitError();
  InitBool();
  InitString();
  InitArray();
  InitDate();
  InitMath();
}

JSValue* StringProto::split(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
  JSValue* val = Runtime::TopValue();
  val->CheckObjectCoercible(e);
  if (!e->IsOk()) return nullptr;
  std::u16string S = ::es::ToString(e, val);
  if (!e->IsOk()) return nullptr;
  ArrayObject* A = new ArrayObject(0);
  size_t length_A = 0;
  size_t lim = 4294967295.0;
  if (vals.size() >= 2 && !vals[1]->IsUndefined()) {
    lim = ToUint32(e, vals[1]);
    if (!e->IsOk()) return nullptr;
  }
  size_t s = S.size();
  size_t p = 0;
  if (vals.size() < 1 || vals[0]->IsUndefined()) {  // 10
    A->AddValueProperty(u"0", new String(S), true, true, true);
    return A;
  }
  if (vals[0]->IsRegex()) {
    assert(false);
  }
  assert(vals[0]->IsString());
  std::u16string R = ::es::ToString(e, vals[0]);
  if (s == 0) {
    regex::MatchResult z = regex::SplitMatch(S, 0, R);
    if (!z.failed) return A;
    A->AddValueProperty(u"0", new String(S), true, true, true);
    return A;
  }
  size_t q = p;
  while (q != s) {  // 13.
    regex::MatchResult z = regex::SplitMatch(S, q, R);  // 13.a
    if (z.failed) {  // 13.b
      q++;
    } else {  // 13.c
      size_t e = z.state.end_index;
      std::vector<std::u16string> cap = z.state.captures;
      if (e == p) {  // 13.c.ii
        q++;
      } else {  // 13.c.iii
        std::u16string T = S.substr(p, q - p);
        A->AddValueProperty(NumberToString(length_A), new String(T), true, true, true);
        length_A++;
        p = e;
        for (size_t i = 0; i < cap.size(); i++) {  // 13.c.iii.7
          A->AddValueProperty(NumberToString(length_A), new String(cap[i]), true, true, true);
          length_A++;
          if (length_A == lim)
            return A;
        }
        q = p;
      }
    }
  }
  std::u16string T = S.substr(p);  // 14
  A->AddValueProperty(NumberToString(length_A), new String(T), true, true, true);  // 15
  return A;
}

}  // namespace es

#endif  // ES_ENTER_CODE_H