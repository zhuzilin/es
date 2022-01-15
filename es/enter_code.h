#ifndef ES_ENTER_CODE_H
#define ES_ENTER_CODE_H

#include <es/error.h>
#include <es/types/object.h>
#include <es/types/builtin/object_object.h>
#include <es/types/builtin/function_object.h>
#include <es/types/builtin/number_object.h>
#include <es/types/builtin/error_object.h>
#include <es/types/builtin/bool_object.h>
#include <es/types/builtin/string_object.h>
#include <es/execution_context.h>

namespace es {

enum CodeType {
  CODE_GLOBAL = 0,
  CODE_FUNC,
  CODE_EVAL,
};

JSObject* CreateArgumentsObject(
  FunctionObject* func, std::vector<JSValue*>& args,
  EnvironmentRecord* env, bool strict
) {
  std::vector<std::u16string> names = func->FormalParameters();
  int len = args.size();
  JSObject* obj = new JSObject(JSObject::OBJ_OTHER, u"Arguments", true, nullptr, false, false);
  obj->SetPrototype(ObjectProto::Instance());
  obj->AddValueProperty(u"length", new Number(len), true, false, true);
  Object* map = new Object();
  int indx = len - 1;
  // TODO(zhuzilin) maps and custom methods
  while (indx >= 0) {
    JSValue* val = args[indx];
    obj->AddValueProperty(ToString(nullptr, new Number(indx)), val, true, true, true);
    indx--;
  }
  if (!strict) {
    obj->AddValueProperty(u"callee", func, true, false, true);
  } else {
    // TODO(zhuzilin) thrower
  }
  return obj;
}

void FindAllVarDecl(std::vector<AST*> stmts, std::vector<VarDecl*>& decls) {
  for (auto stmt : stmts) {
    switch (stmt->type()) {
      case AST::AST_STMT_VAR: {
        VarStmt* var_stmt = static_cast<VarStmt*>(stmt);
        for (auto d : var_stmt->decls()) {
          decls.emplace_back(d);
        }
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
      case AST::AST_STMT_BLOCK: {
        Block* block = static_cast<Block*>(stmt);
        FindAllVarDecl(block->statements(), decls);
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
  log::PrintSource("enter DeclarationBindingInstantiation");
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
      log::PrintSource("set mutable: ", arg_name, " to " + v->ToString());
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
    auto args_obj = CreateArgumentsObject(f, args, env, strict);
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
  ExecutionContextStack::Global()->AddContext(context);
  // 2
  DeclarationBindingInstantiation(e, context, program, CODE_GLOBAL);
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
  ExecutionContext* context = new ExecutionContext(local_env, local_env, this_binding, body->strict());  // 8
  ExecutionContextStack::Global()->AddContext(context);
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
  global_obj->AddValueProperty(u"Error", ErrorConstructor::Instance(), true, false, true);
  global_obj->AddValueProperty(u"Boolean", BoolConstructor::Instance(), true, false, true);
  global_obj->AddValueProperty(u"String", StringConstructor::Instance(), true, false, true);

  global_obj->AddFuncProperty(u"console_log", logger, true, false, true);
}

void InitObject() {
  ObjectConstructor* constructor = ObjectConstructor::Instance();
  constructor->SetPrototype(FunctionProto::Instance());
  // 15.2.3 Properties of the Object Constructor
  constructor->AddValueProperty(u"prototype", ObjectProto::Instance(), false, false, false);
  // TODO(zhuzilin) check if the config is correct.
  constructor->AddFuncProperty(u"getPrototypeOf", ObjectConstructor::getPrototypeOf, false, false, false);
  constructor->AddFuncProperty(u"getOwnPropertyDescriptor", ObjectConstructor::getOwnPropertyDescriptor, false, false, false);
  constructor->AddFuncProperty(u"getOwnPropertyNames", ObjectConstructor::getOwnPropertyNames, false, false, false);
  constructor->AddFuncProperty(u"create", ObjectConstructor::create, false, false, false);
  constructor->AddFuncProperty(u"defineProperty", ObjectConstructor::defineProperty, false, false, false);
  constructor->AddFuncProperty(u"defineProperties", ObjectConstructor::defineProperties, false, false, false);
  constructor->AddFuncProperty(u"seal", ObjectConstructor::seal, false, false, false);
  constructor->AddFuncProperty(u"freeze", ObjectConstructor::freeze, false, false, false);
  constructor->AddFuncProperty(u"preventExtensions", ObjectConstructor::preventExtensions, false, false, false);
  constructor->AddFuncProperty(u"isSealed", ObjectConstructor::isSealed, false, false, false);
  constructor->AddFuncProperty(u"isFrozen", ObjectConstructor::isFrozen, false, false, false);
  constructor->AddFuncProperty(u"isExtensible", ObjectConstructor::isExtensible, false, false, false);
  constructor->AddFuncProperty(u"keys", ObjectConstructor::keys, false, false, false);

  ObjectProto* proto = ObjectProto::Instance();
  // 15.2.4 Properties of the Object Prototype Object
  proto->AddValueProperty(u"constructor", ObjectConstructor::Instance(), false, false, false);
  proto->AddFuncProperty(u"toString", ObjectProto::toString, false, false, false);
  proto->AddFuncProperty(u"toLocaleString", ObjectProto::toLocaleString, false, false, false);
  proto->AddFuncProperty(u"valueOf", ObjectProto::valueOf, false, false, false);
  proto->AddFuncProperty(u"hasOwnProperty", ObjectProto::hasOwnProperty, false, false, false);
  proto->AddFuncProperty(u"isPrototypeOf", ObjectProto::isPrototypeOf, false, false, false);
  proto->AddFuncProperty(u"propertyIsEnumerable", ObjectProto::propertyIsEnumerable, false, false, false);
}

void InitFunction() {
  FunctionConstructor* constructor = FunctionConstructor::Instance();
  constructor->SetPrototype(FunctionProto::Instance());
  // 15.3.3 Properties of the Function Constructor
  constructor->AddValueProperty(u"prototype", FunctionProto::Instance(), false, false, false);
  constructor->AddValueProperty(u"length", Number::One(), false, false, false);

  FunctionProto* proto = FunctionProto::Instance();
  proto->SetPrototype(ObjectProto::Instance());
  // 15.2.4 Properties of the Function Prototype Function
  proto->AddValueProperty(u"constructor", FunctionConstructor::Instance(), false, false, false);
  proto->AddFuncProperty(u"toString", FunctionProto::toString, false, false, false);
  proto->AddFuncProperty(u"apply", FunctionProto::apply, false, false, false);
  proto->AddFuncProperty(u"call", FunctionProto::call, false, false, false);
  proto->AddFuncProperty(u"bind", FunctionProto::bind, false, false, false);
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
  proto->AddFuncProperty(u"toString", NumberProto::toString, false, false, false);
  proto->AddFuncProperty(u"toLocaleString", NumberProto::toLocaleString, false, false, false);
  proto->AddFuncProperty(u"valueOf", NumberProto::valueOf, false, false, false);
  proto->AddFuncProperty(u"toFixed", NumberProto::toFixed, false, false, false);
  proto->AddFuncProperty(u"toExponential", NumberProto::toExponential, false, false, false);
  proto->AddFuncProperty(u"toPrecision", NumberProto::toPrecision, false, false, false);
}

void InitError() {
  ErrorConstructor* constructor = ErrorConstructor::Instance();
  constructor->SetPrototype(FunctionProto::Instance());
  // 15.11.3 Properties of the Error Constructor
  constructor->AddValueProperty(u"prototype", ErrorProto::Instance(), false, false, false);
  constructor->AddValueProperty(u"length", Number::One(), false, false, false);

  ErrorProto* proto = ErrorProto::Instance();
  proto->SetPrototype(ObjectProto::Instance());
  // 15.11.4 Properties of the Error Prototype Object
  proto->AddValueProperty(u"constructor", ErrorConstructor::Instance(), false, false, false);
  proto->AddValueProperty(u"name", new String(u"Error"), false, false, false);
  proto->AddValueProperty(u"message", String::Empty(), true, false, false);
  proto->AddFuncProperty(u"call", ErrorProto::toString, false, false, false);
}

void InitBool() {
  BoolConstructor* constructor = BoolConstructor::Instance();
  constructor->SetPrototype(FunctionProto::Instance());
  // 15.6.3 Properties of the Boolean Constructor
  constructor->AddValueProperty(u"prototype", BoolProto::Instance(), false, false, false);

  BoolProto* proto = BoolProto::Instance();
  proto->SetPrototype(ObjectProto::Instance());
  // 15.6.4 Properties of the Boolean Prototype Object
  proto->AddValueProperty(u"constructor", BoolConstructor::Instance(), false, false, false);
  proto->AddFuncProperty(u"toString", BoolProto::toString, false, false, false);
  proto->AddFuncProperty(u"valueOf", BoolProto::valueOf, false, false, false);
}

void InitString() {
  StringConstructor* constructor = StringConstructor::Instance();
  constructor->SetPrototype(FunctionProto::Instance());
  // 15.3.3 Properties of the String Constructor
  constructor->AddValueProperty(u"prototype", StringProto::Instance(), false, false, false);
  constructor->AddValueProperty(u"length", Number::One(), false, false, false);
  constructor->AddFuncProperty(u"fromCharCode", StringConstructor::fromCharCode, false, false, false);

  StringProto* proto = StringProto::Instance();
  proto->SetPrototype(ObjectProto::Instance());
  // 15.2.4 Properties of the String Prototype String
  proto->AddValueProperty(u"constructor", StringConstructor::Instance(), false, false, false);
  proto->AddFuncProperty(u"toString", StringProto::toString, false, false, false);
  proto->AddFuncProperty(u"valueOf", StringProto::valueOf, false, false, false);
  proto->AddFuncProperty(u"charAt", StringProto::charAt, false, false, false);
  proto->AddFuncProperty(u"charCodeAt", StringProto::charCodeAt, false, false, false);
  proto->AddFuncProperty(u"concat", StringProto::concat, false, false, false);
  proto->AddFuncProperty(u"indexOf", StringProto::indexOf, false, false, false);
  proto->AddFuncProperty(u"lastIndexOf", StringProto::lastIndexOf, false, false, false);
  proto->AddFuncProperty(u"localeCompare", StringProto::localeCompare, false, false, false);
  proto->AddFuncProperty(u"match", StringProto::match, false, false, false);
  proto->AddFuncProperty(u"replace", StringProto::replace, false, false, false);
  proto->AddFuncProperty(u"search", StringProto::search, false, false, false);
  proto->AddFuncProperty(u"slice", StringProto::slice, false, false, false);
  proto->AddFuncProperty(u"split", StringProto::split, false, false, false);
  proto->AddFuncProperty(u"substring", StringProto::substring, false, false, false);
  proto->AddFuncProperty(u"toLowerCase", StringProto::toLowerCase, false, false, false);
  proto->AddFuncProperty(u"toLocaleLowerCase", StringProto::toLocaleLowerCase, false, false, false);
  proto->AddFuncProperty(u"toUpperCase", StringProto::toUpperCase, false, false, false);
  proto->AddFuncProperty(u"toLocaleUpperCase", StringProto::toLocaleUpperCase, false, false, false);
  proto->AddFuncProperty(u"trim", StringProto::trim, false, false, false);
}

void Init() {
  InitGlobalObject();
  InitObject();
  InitFunction();
  InitNumber();
  InitError();
  InitBool();
  InitString();
}

}  // namespace es

#endif  // ES_ENTER_CODE_H