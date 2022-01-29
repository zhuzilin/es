#ifndef ES_ENTER_CODE_H
#define ES_ENTER_CODE_H

#include <es/error.h>
#include <es/runtime.h>
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
#include <es/types/builtin/regexp_object.h>
#include <es/types/builtin/arguments_object.h>
#include <es/types/host/console.h>
#include <es/regex/match.h>

namespace es {

enum CodeType {
  CODE_GLOBAL = 0,
  CODE_FUNC,
  CODE_EVAL,
};

Handle<JSValue> MakeArgGetter(std::u16string name, Handle<LexicalEnvironment> env) {
  Parser parser(u"return " + name + u";");
  ProgramOrFunctionBody* body = static_cast<ProgramOrFunctionBody*>(
    parser.ParseFunctionBody(Token::TK_EOS));
  return FunctionObject::New({}, body, env);
}

Handle<JSValue> MakeArgSetter(std::u16string name, Handle<LexicalEnvironment> env) {
  std::u16string param = name + u"_arg";
  Parser parser(name + u" = " + param);
  ProgramOrFunctionBody* body = static_cast<ProgramOrFunctionBody*>(
    parser.ParseFunctionBody(Token::TK_EOS));
  return FunctionObject::New({param}, body, env);
}

// 10.6 Arguments Object
Handle<ArgumentsObject> CreateArgumentsObject(
  Handle<FunctionObject> func, std::vector<Handle<JSValue>>& args,
  Handle<LexicalEnvironment> env, bool strict
) {
  Handle<es::FixedArray<es::String>> names = func.val()->FormalParameters();
  int len = args.size();
  Handle<Object> map = Object::New();  // 8
  Handle<JSObject> obj = ArgumentsObject::New(map, len);
  int indx = len - 1;  // 10
  std::set<std::u16string> mapped_names;
  while (indx >= 0) {  // 11
    Handle<JSValue> val = args[indx];  // 11.a
    AddValueProperty(obj, NumberToString(indx), val, true, true, true);  // 11.b
    if (indx < names.val()->size()) {  // 11.c
      std::u16string name = names.val()->Get(indx).val()->data();  // 11.c.i
      if (!strict && mapped_names.find(name) == mapped_names.end()) {  // 11.c.ii
        mapped_names.insert(name);
        Handle<JSValue> g = MakeArgGetter(name, env);
        Handle<JSValue> p = MakeArgSetter(name, env);
        Handle<PropertyDescriptor> desc = PropertyDescriptor::New();
        desc.val()->SetSet(p);
        desc.val()->SetGet(g);
        desc.val()->SetConfigurable(true);
        DefineOwnProperty(nullptr, map, NumberToString(indx), desc, false);
      }
    }
    indx--;  // 11.d
  }
  if (!strict) {  // 13
    AddValueProperty(obj, u"callee", func, true, false, true);
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
  Handle<FunctionObject> f = Handle<FunctionObject>(), std::vector<Handle<JSValue>> args = {}
) {
  Handle<EnvironmentRecord> env = context->variable_env().val()->env_rec();  // 1
  bool configurable_bindings = false;
  ProgramOrFunctionBody* body = static_cast<ProgramOrFunctionBody*>(code);
  if (code_type == CODE_EVAL) {
    configurable_bindings = true;  // 2
  }
  bool strict = body->strict();  // 3
  if (code_type == CODE_FUNC) {  // 4
    assert(!f.IsNullptr());
    auto names = f.val()->FormalParameters();  // 4.a
    Function* func_ast = static_cast<Function*>(code);
    size_t arg_count = args.size();  // 4.b
    size_t n = 0;  // 4.c
    for (size_t i = 0; i < names.val()->size(); i++) {
      Handle<String> arg_name = names.val()->Get(i);  // 4.d
      Handle<JSValue> v = Undefined::Instance();
      if (n < arg_count)  // 4.d.i & 4.d.ii
        v = args[n++];
      bool arg_already_declared = HasBinding(env, arg_name);  // 4.d.iii
      if (!arg_already_declared) {  // 4.d.iv
        // NOTE(zhuzlin) I'm not sure if this should be false.
        CreateMutableBinding(e, env, arg_name, false);
        if (!e->IsOk()) return;
      }
      SetMutableBinding(e, env,arg_name, v, strict);  // 4.d.v
      if (!e->IsOk()) return;
    }
  }
  // 5
  for (Function* func_decl : body->func_decls()) {
    assert(func_decl->is_named());
    Handle<String> fn = String::New(func_decl->name());
    Handle<FunctionObject> fo = InstantiateFunctionDeclaration(e, func_decl);
    if (!e->IsOk()) return;
    bool func_already_declared = HasBinding(env, fn);
    if (!func_already_declared) {  // 5.d
      CreateMutableBinding(e, env, fn, configurable_bindings);
      if (!e->IsOk()) return;
    } else {  // 5.e
      auto go = GlobalObject::Instance();
      auto existing_prop = GetProperty(go, fn);
      assert(!existing_prop.val()->IsUndefined());
      auto existing_prop_desc = static_cast<Handle<PropertyDescriptor>>(existing_prop);
      if (existing_prop_desc.val()->Configurable()) {  // 5.e.iii
        auto new_desc = PropertyDescriptor::New();
        new_desc.val()->SetDataDescriptor(Undefined::Instance(), true, true, configurable_bindings);
        DefineOwnProperty(e, go, fn, new_desc, true);
        if (!e->IsOk()) return;
      } else {  // 5.e.iv
        if (existing_prop_desc.val()->IsAccessorDescriptor() ||
            !(existing_prop_desc.val()->HasConfigurable() && existing_prop_desc.val()->Configurable() &&
              existing_prop_desc.val()->HasEnumerable() && existing_prop_desc.val()->Enumerable())) {
          *e = *Error::TypeError();
          return;
        }
      }
    }
    SetMutableBinding(e, env,fn, fo, strict);  // 5.f
  }
  // 6
  bool arguments_already_declared = HasBinding(env, String::Arguments());
  // 7
  if (code_type == CODE_FUNC && !arguments_already_declared) {
    auto args_obj = CreateArgumentsObject(f, args, context->variable_env(), strict);
    if (strict) {  // 7.b
      Handle<DeclarativeEnvironmentRecord> decl_env = static_cast<Handle<DeclarativeEnvironmentRecord>>(env);
      CreateImmutableBinding(decl_env, String::Arguments());
      InitializeImmutableBinding(decl_env, String::Arguments(), args_obj);
    } else {  // 7.c
      // NOTE(zhuzlin) I'm not sure if this should be false.
      CreateMutableBinding(e, env, String::Arguments(), false);
      SetMutableBinding(e, env,String::Arguments(), args_obj, false);
    }
  }
  // 8
  std::vector<VarDecl*> decls;
  FindAllVarDecl(body->statements(), decls);
  for (VarDecl* d : decls) {
    Handle<String> dn = String::New(d->ident());
    bool var_already_declared = HasBinding(env, dn);
    if (!var_already_declared) {
      CreateMutableBinding(e, env, dn, configurable_bindings);
      if (!e->IsOk()) return;
      SetMutableBinding(e, env,dn, Undefined::Instance(), strict);
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
  Handle<LexicalEnvironment> global_env = LexicalEnvironment::Global();
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
  Handle<LexicalEnvironment> variable_env;
  Handle<LexicalEnvironment> lexical_env;
  Handle<JSValue> this_binding;
  if (!GlobalObject::Instance().val()->direct_eval()) {  // 1
    Handle<LexicalEnvironment> global_env = LexicalEnvironment::Global();
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
                (program->strict() && GlobalObject::Instance().val()->direct_eval());
  if (strict) {  // 3
    Handle<LexicalEnvironment> strict_var_env = NewDeclarativeEnvironment(lexical_env);
    lexical_env = strict_var_env;
    variable_env = strict_var_env;
  }
  context = new ExecutionContext(variable_env, lexical_env, this_binding, strict);
  Runtime::Global()->AddContext(context);
  // 4
  DeclarationBindingInstantiation(e, context, program, CODE_EVAL);
}

// 15.1.2.1 eval(X)
Handle<JSValue> GlobalObject::eval(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
  log::PrintSource("enter GlobalObject::eval");
  if (vals.size() == 0)
    return Undefined::Instance();
  if (!vals[0].val()->IsString())
    return vals[0];
  std::u16string x = static_cast<Handle<String>>(vals[0]).val()->data();
  Parser parser(x);
  AST* program = parser.ParseProgram();
  if (program->IsIllegal()) {
    *e = *Error::SyntaxError(u"failed to parse eval");
    return Handle<JSValue>();
  }
  EnterEvalCode(e, program);
  if (!e->IsOk()) return Handle<JSValue>();
  Completion result = EvalProgram(program);
  Runtime::Global()->PopContext();

  switch (result.type()) {
    case Completion::NORMAL:
      if (!result.IsEmpty())
        return result.value();
      else
        return Undefined::Instance();
    default: {
      assert(result.type() == Completion::THROW);
      std::u16string message;
      Handle<JSValue> return_value = result.value();
      if (return_value.val()->IsErrorObject()) {
        message = static_cast<Handle<ErrorObject>>(return_value).val()->ErrorMessage();
      } else {
        message = ToU16String(e, return_value);
      }
      *e = *Error::NativeError(message);
      return return_value;
    }
  }
}

// 10.4.3
void EnterFunctionCode(
  Error* e, Handle<JSObject> F, ProgramOrFunctionBody* body,
  Handle<JSValue> this_arg, std::vector<Handle<JSValue>> args, bool strict
) {
  assert(F.val()->obj_type() == JSObject::OBJ_FUNC);
  Handle<FunctionObject> func = static_cast<Handle<FunctionObject>>(F);
  Handle<JSValue> this_binding;
  if (strict) {  // 1
    this_binding = this_arg;
  } else {  // 2 & 3
    this_binding = (this_arg.val()->IsUndefined() || this_arg.val()->IsNull()) ?
      static_cast<Handle<JSValue>>(GlobalObject::Instance()) : this_arg;
  }
  Handle<LexicalEnvironment> local_env = NewDeclarativeEnvironment(func.val()->Scope());
  ExecutionContext* context = new ExecutionContext(local_env, local_env, this_binding, strict);  // 8
  Runtime::Global()->AddContext(context);
  // 9
  DeclarationBindingInstantiation(e, context, body, CODE_FUNC, func, args);
}

void InitGlobalObject() {
  HandleScope scope;
  auto global_obj = GlobalObject::Instance();
  // 15.1.1 Value Properties of the Global Object
  AddValueProperty(global_obj, u"NaN", Number::NaN(), false, false, false);
  AddValueProperty(global_obj, u"Infinity", Number::PositiveInfinity(), false, false, false);
  AddValueProperty(global_obj, u"undefined", Undefined::Instance(), false, false, false);
  // 15.1.2 Function Properties of the Global Object
  AddFuncProperty(global_obj, u"eval", GlobalObject::eval, true, false, true);
  AddFuncProperty(global_obj, u"parseInt", GlobalObject::parseInt, true, false, true);
  AddFuncProperty(global_obj, u"parseFloat", GlobalObject::parseFloat, true, false, true);
  AddFuncProperty(global_obj, u"isNaN", GlobalObject::isNaN, true, false, true);
  AddFuncProperty(global_obj, u"isFinite", GlobalObject::isFinite, true, false, true);
  // 15.1.3 URI Handling Function Properties
  // TODO(zhuzilin)
  // 15.1.4 Constructor Properties of the Global Object
  AddValueProperty(global_obj, u"Object", ObjectConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"Function", FunctionConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"Number", NumberConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"Boolean", BoolConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"String", StringConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"Array", ArrayConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"Date", DateConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"RegExp", RegExpConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"Error", ErrorConstructor::Instance(), true, false, true);
  // TODO(zhuzilin) differentiate errors.
  AddValueProperty(global_obj, u"EvalError", ErrorConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"RangeError", ErrorConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"ReferenceError", ErrorConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"SyntaxError", ErrorConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"TypeError", ErrorConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"URIError", ErrorConstructor::Instance(), true, false, true);

  AddValueProperty(global_obj, u"Math", Math::Instance(), true, false, true);

  AddValueProperty(global_obj, u"console", Console::Instance(), true, false, true);
}

void InitObject() {
  Handle<ObjectConstructor> constructor = ObjectConstructor::Instance();
  constructor.val()->SetPrototype(FunctionProto::Instance());
  // 15.2.3 Properties of the Object Constructor
  AddValueProperty(constructor, String::Prototype(), ObjectProto::Instance(), false, false, false);
  AddFuncProperty(constructor, u"toString", ObjectConstructor::toString, true, false, false);
  AddFuncProperty(constructor, u"getPrototypeOf", ObjectConstructor::getPrototypeOf, true, false, false);
  AddFuncProperty(constructor, u"getOwnPropertyDescriptor", ObjectConstructor::getOwnPropertyDescriptor, true, false, false);
  AddFuncProperty(constructor, u"getOwnPropertyNames", ObjectConstructor::getOwnPropertyNames, true, false, false);
  AddFuncProperty(constructor, u"create", ObjectConstructor::create, true, false, false);
  AddFuncProperty(constructor, u"defineProperty", ObjectConstructor::defineProperty, true, false, false);
  AddFuncProperty(constructor, u"defineProperties", ObjectConstructor::defineProperties, true, false, false);
  AddFuncProperty(constructor, u"seal", ObjectConstructor::seal, true, false, false);
  AddFuncProperty(constructor, u"freeze", ObjectConstructor::freeze, true, false, false);
  AddFuncProperty(constructor, u"preventExtensions", ObjectConstructor::preventExtensions, true, false, false);
  AddFuncProperty(constructor, u"isSealed", ObjectConstructor::isSealed, true, false, false);
  AddFuncProperty(constructor, u"isFrozen", ObjectConstructor::isFrozen, true, false, false);
  AddFuncProperty(constructor, u"isExtensible", ObjectConstructor::isExtensible, true, false, false);
  AddFuncProperty(constructor, u"keys", ObjectConstructor::keys, true, false, false);
  // ES6
  AddFuncProperty(constructor, u"setPrototypeOf", ObjectConstructor::setPrototypeOf, true, false, false);

  Handle<ObjectProto> proto = ObjectProto::Instance();
  // 15.2.4 Properties of the Object Prototype Object
  AddValueProperty(proto, String::Constructor(), ObjectConstructor::Instance(), false, false, false);
  AddFuncProperty(proto, u"toString", ObjectProto::toString, true, false, false);
  AddFuncProperty(proto, u"toLocaleString", ObjectProto::toLocaleString, true, false, false);
  AddFuncProperty(proto, u"valueOf", ObjectProto::valueOf, true, false, false);
  AddFuncProperty(proto, u"hasOwnProperty", ObjectProto::hasOwnProperty, true, false, false);
  AddFuncProperty(proto, u"isPrototypeOf", ObjectProto::isPrototypeOf, true, false, false);
  AddFuncProperty(proto, u"propertyIsEnumerable", ObjectProto::propertyIsEnumerable, true, false, false);
}

void InitFunction() {
  HandleScope scope;
  Handle<FunctionConstructor> constructor = FunctionConstructor::Instance();
  constructor.val()->SetPrototype(FunctionProto::Instance());
  // 15.3.3 Properties of the Function Constructor
  AddValueProperty(constructor, String::Prototype(), FunctionProto::Instance(), false, false, false);
  AddValueProperty(constructor, String::Length(), Number::One(), false, false, false);
  AddFuncProperty(constructor, u"toString", FunctionConstructor::toString, true, false, false);

  Handle<FunctionProto> proto = FunctionProto::Instance();
  proto.val()->SetPrototype(ObjectProto::Instance());
  // 15.2.4 Properties of the Function Prototype Function
  AddValueProperty(proto, String::Constructor(), FunctionConstructor::Instance(), false, false, false);
  AddFuncProperty(proto, u"toString", FunctionProto::toString, true, false, false);
  AddFuncProperty(proto, u"apply", FunctionProto::apply, true, false, false);
  AddFuncProperty(proto, u"call", FunctionProto::call, true, false, false);
  AddFuncProperty(proto, u"bind", FunctionProto::bind, true, false, false);
}

void InitNumber() {
  HandleScope scope;
  Handle<NumberConstructor> constructor = NumberConstructor::Instance();
  constructor.val()->SetPrototype(FunctionProto::Instance());
  // 15.3.3 Properties of the Number Constructor
  AddValueProperty(constructor, String::Prototype(), NumberProto::Instance(), false, false, false);
  AddValueProperty(constructor, String::Length(), Number::One(), false, false, false);
  AddValueProperty(constructor, u"MAX_VALUE", Number::New(1.7976931348623157e308), false, false, false);
  AddValueProperty(constructor, u"MIN_VALUE", Number::New(5e-324), false, false, false);
  AddValueProperty(constructor, u"NaN", Number::NaN(), false, false, false);
  AddValueProperty(constructor, u"NEGATIVE_INFINITY", Number::PositiveInfinity(), false, false, false);
  AddValueProperty(constructor, u"POSITIVE_INFINITY", Number::NegativeInfinity(), false, false, false);

  Handle<NumberProto> proto = NumberProto::Instance();
  proto.val()->SetPrototype(ObjectProto::Instance());
  // 15.2.4 Properties of the Number Prototype Number
  AddValueProperty(proto, String::Constructor(), NumberConstructor::Instance(), false, false, false);
  AddFuncProperty(proto, u"toString", NumberProto::toString, true, false, false);
  AddFuncProperty(proto, u"toLocaleString", NumberProto::toLocaleString, true, false, false);
  AddFuncProperty(proto, u"valueOf", NumberProto::valueOf, true, false, false);
  AddFuncProperty(proto, u"toFixed", NumberProto::toFixed, true, false, false);
  AddFuncProperty(proto, u"toExponential", NumberProto::toExponential, true, false, false);
  AddFuncProperty(proto, u"toPrecision", NumberProto::toPrecision, true, false, false);
}

void InitError() {
  HandleScope scope;
  Handle<ErrorConstructor> constructor = ErrorConstructor::Instance();
  constructor.val()->SetPrototype(FunctionProto::Instance());
  // 15.11.3 Properties of the Error Constructor
  AddValueProperty(constructor, String::Prototype(), ErrorProto::Instance(), false, false, false);
  AddValueProperty(constructor, String::Length(), Number::One(), false, false, false);
  AddFuncProperty(constructor, u"toString", NumberConstructor::toString, true, false, false);

  Handle<ErrorProto> proto = ErrorProto::Instance();
  proto.val()->SetPrototype(ObjectProto::Instance());
  // 15.11.4 Properties of the Error Prototype Object
  AddValueProperty(proto, String::Constructor(), ErrorConstructor::Instance(), false, false, false);
  AddValueProperty(proto, u"name", String::New(u"Error"), false, false, false);
  AddValueProperty(proto, u"message", String::Empty(), true, false, false);
  AddFuncProperty(proto, u"call", ErrorProto::toString, true, false, false);
}

void InitBool() {
  HandleScope scope;
  Handle<BoolConstructor> constructor = BoolConstructor::Instance();
  constructor.val()->SetPrototype(FunctionProto::Instance());
  // 15.6.3 Properties of the Boolean Constructor
  AddValueProperty(constructor, String::Prototype(), BoolProto::Instance(), false, false, false);
  AddFuncProperty(constructor, u"toString", BoolConstructor::toString, true, false, false);

  Handle<BoolProto> proto = BoolProto::Instance();
  proto.val()->SetPrototype(ObjectProto::Instance());
  // 15.6.4 Properties of the Boolean Prototype Object
  AddValueProperty(proto, String::Constructor(), BoolConstructor::Instance(), false, false, false);
  AddFuncProperty(proto, u"toString", BoolProto::toString, true, false, false);
  AddFuncProperty(proto, u"valueOf", BoolProto::valueOf, true, false, false);
}

void InitString() {
  HandleScope scope;
  Handle<StringConstructor> constructor = StringConstructor::Instance();
  constructor.val()->SetPrototype(FunctionProto::Instance());
  // 15.3.3 Properties of the String Constructor
  AddValueProperty(constructor, String::Prototype(), StringProto::Instance(), false, false, false);
  AddValueProperty(constructor, String::Length(), Number::One(), true, false, false);
  AddFuncProperty(constructor, u"fromCharCode", StringConstructor::fromCharCode, true, false, false);
  AddFuncProperty(constructor, u"toString", StringConstructor::toString, true, false, false);

  Handle<StringProto> proto = StringProto::Instance();
  proto.val()->SetPrototype(ObjectProto::Instance());
  // 15.2.4 Properties of the String Prototype String
  AddValueProperty(proto, String::Constructor(), StringConstructor::Instance(), false, false, false);
  AddFuncProperty(proto, u"toString", StringProto::toString, true, false, false);
  AddFuncProperty(proto, u"valueOf", StringProto::valueOf, true, false, false);
  AddFuncProperty(proto, u"charAt", StringProto::charAt, true, false, false);
  AddFuncProperty(proto, u"charCodeAt", StringProto::charCodeAt, true, false, false);
  AddFuncProperty(proto, u"concat", StringProto::concat, true, false, false);
  AddFuncProperty(proto, u"indexOf", StringProto::indexOf, true, false, false);
  AddFuncProperty(proto, u"lastIndexOf", StringProto::lastIndexOf, true, false, false);
  AddFuncProperty(proto, u"localeCompare", StringProto::localeCompare, true, false, false);
  AddFuncProperty(proto, u"match", StringProto::match, true, false, false);
  AddFuncProperty(proto, u"replace", StringProto::replace, true, false, false);
  AddFuncProperty(proto, u"search", StringProto::search, true, false, false);
  AddFuncProperty(proto, u"slice", StringProto::slice, true, false, false);
  AddFuncProperty(proto, u"split", StringProto::split, true, false, false);
  AddFuncProperty(proto, u"substring", StringProto::substring, true, false, false);
  AddFuncProperty(proto, u"toLowerCase", StringProto::toLowerCase, true, false, false);
  AddFuncProperty(proto, u"toLocaleLowerCase", StringProto::toLocaleLowerCase, true, false, false);
  AddFuncProperty(proto, u"toUpperCase", StringProto::toUpperCase, true, false, false);
  AddFuncProperty(proto, u"toLocaleUpperCase", StringProto::toLocaleUpperCase, true, false, false);
  AddFuncProperty(proto, u"trim", StringProto::trim, true, false, false);
}

void InitArray() {
  HandleScope scope;
  Handle<ArrayConstructor> constructor = ArrayConstructor::Instance();
  constructor.val()->SetPrototype(FunctionProto::Instance());
  // 15.6.3 Properties of the Arrayean Constructor
  AddValueProperty(constructor, String::Length(), Number::One(), false, false, false);
  AddValueProperty(constructor, String::Prototype(), ArrayProto::Instance(), false, false, false);
  AddFuncProperty(constructor, u"isArray", ArrayConstructor::isArray, true, false, false);
  AddFuncProperty(constructor, u"toString", ArrayConstructor::toString, true, false, false);

  Handle<ArrayProto> proto = ArrayProto::Instance();
  proto.val()->SetPrototype(ObjectProto::Instance());
  // 15.6.4 Properties of the Arrayean Prototype Object
  AddValueProperty(proto, String::Length(), Number::Zero(), false, false, false);
  AddValueProperty(proto, String::Constructor(), ArrayConstructor::Instance(), false, false, false);
  AddFuncProperty(proto, u"toString", ArrayProto::toString, true, false, false);
  AddFuncProperty(proto, u"toLocaleString", ArrayProto::toLocaleString, true, false, false);
  AddFuncProperty(proto, u"concat", ArrayProto::concat, true, false, false);
  AddFuncProperty(proto, u"join", ArrayProto::join, true, false, false);
  AddFuncProperty(proto, u"pop", ArrayProto::pop, true, false, false);
  AddFuncProperty(proto, u"push", ArrayProto::push, true, false, false);
  AddFuncProperty(proto, u"reverse", ArrayProto::reverse, true, false, false);
  AddFuncProperty(proto, u"shift", ArrayProto::shift, true, false, false);
  AddFuncProperty(proto, u"slice", ArrayProto::slice, true, false, false);
  AddFuncProperty(proto, u"sort", ArrayProto::sort, true, false, false);
  AddFuncProperty(proto, u"splice", ArrayProto::splice, true, false, false);
  AddFuncProperty(proto, u"unshift", ArrayProto::unshift, true, false, false);
  AddFuncProperty(proto, u"indexOf", ArrayProto::indexOf, true, false, false);
  AddFuncProperty(proto, u"lastIndexOf", ArrayProto::lastIndexOf, true, false, false);
  AddFuncProperty(proto, u"every", ArrayProto::every, true, false, false);
  AddFuncProperty(proto, u"some", ArrayProto::some, true, false, false);
  AddFuncProperty(proto, u"forEach", ArrayProto::forEach, true, false, false);
  AddFuncProperty(proto, u"map", ArrayProto::map, true, false, false);
  AddFuncProperty(proto, u"filter", ArrayProto::filter, true, false, false);
  AddFuncProperty(proto, u"reduce", ArrayProto::reduce, true, false, false);
  AddFuncProperty(proto, u"reduceRight", ArrayProto::reduceRight, true, false, false);
}

void InitDate() {
  HandleScope scope;
  Handle<DateConstructor> constructor = DateConstructor::Instance();
  constructor.val()->SetPrototype(FunctionProto::Instance());
  // 15.6.3 Properties of the Dateean Constructor
  AddValueProperty(constructor, String::Length(), Number::New(7), false, false, false);
  AddValueProperty(constructor, String::Prototype(), DateProto::Instance(), false, false, false);
  AddFuncProperty(constructor, u"parse", DateConstructor::parse, true, false, false);
  AddFuncProperty(constructor, u"UTC", DateConstructor::UTC, true, false, false);
  AddFuncProperty(constructor, u"now", DateConstructor::now, true, false, false);
  AddFuncProperty(constructor, u"toString", DateConstructor::toString, true, false, false);

  Handle<DateProto> proto = DateProto::Instance();
  proto.val()->SetPrototype(ObjectProto::Instance());
  // 15.6.4 Properties of the Dateean Prototype Object
  AddValueProperty(proto, String::Length(), Number::Zero(), false, false, false);
  AddValueProperty(proto, String::Constructor(), DateConstructor::Instance(), false, false, false);
  AddFuncProperty(proto, u"toString", DateProto::toString, true, false, false);
  AddFuncProperty(proto, u"toDateString", DateProto::toDateString, true, false, false);
  AddFuncProperty(proto, u"toTimeString", DateProto::toTimeString, true, false, false);
  AddFuncProperty(proto, u"toLocaleString", DateProto::toLocaleString, true, false, false);
  AddFuncProperty(proto, u"toLocaleDateString", DateProto::toLocaleDateString, true, false, false);
  AddFuncProperty(proto, u"toLocaleTimeString", DateProto::toLocaleTimeString, true, false, false);
  AddFuncProperty(proto, u"valueOf", DateProto::valueOf, true, false, false);
  AddFuncProperty(proto, u"getTime", DateProto::getTime, true, false, false);
  AddFuncProperty(proto, u"getFullYear", DateProto::getFullYear, true, false, false);
  AddFuncProperty(proto, u"getUTCFullYear", DateProto::getUTCFullYear, true, false, false);
  AddFuncProperty(proto, u"getMonth", DateProto::getMonth, true, false, false);
  AddFuncProperty(proto, u"getUTCMonth", DateProto::getUTCMonth, true, false, false);
  AddFuncProperty(proto, u"getDate", DateProto::getDate, true, false, false);
  AddFuncProperty(proto, u"getUTCDate", DateProto::getUTCDate, true, false, false);
  AddFuncProperty(proto, u"getUTCDay", DateProto::getUTCDay, true, false, false);
  AddFuncProperty(proto, u"getHours", DateProto::getHours, true, false, false);
  AddFuncProperty(proto, u"getUTCHours", DateProto::getUTCHours, true, false, false);
  AddFuncProperty(proto, u"getMinutes", DateProto::getMinutes, true, false, false);
  AddFuncProperty(proto, u"getUTCMinutes", DateProto::getUTCMinutes, true, false, false);
  AddFuncProperty(proto, u"getSeconds", DateProto::getSeconds, true, false, false);
  AddFuncProperty(proto, u"getUTCSeconds", DateProto::getUTCSeconds, true, false, false);
  AddFuncProperty(proto, u"getMilliseconds", DateProto::getMilliseconds, true, false, false);
  AddFuncProperty(proto, u"getUTCMilliseconds", DateProto::getUTCMilliseconds, true, false, false);
  AddFuncProperty(proto, u"getTimezoneOffset", DateProto::getTimezoneOffset, true, false, false);
  AddFuncProperty(proto, u"setTime", DateProto::setTime, true, false, false);
  AddFuncProperty(proto, u"setMilliseconds", DateProto::setMilliseconds, true, false, false);
  AddFuncProperty(proto, u"setUTCMilliseconds", DateProto::setUTCMilliseconds, true, false, false);
  AddFuncProperty(proto, u"setSeconds", DateProto::setSeconds, true, false, false);
  AddFuncProperty(proto, u"setUTCSeconds", DateProto::setUTCSeconds, true, false, false);
  AddFuncProperty(proto, u"setMinutes", DateProto::setMinutes, true, false, false);
  AddFuncProperty(proto, u"setHours", DateProto::setHours, true, false, false);
  AddFuncProperty(proto, u"setUTCHours", DateProto::setUTCHours, true, false, false);
  AddFuncProperty(proto, u"setDate", DateProto::setDate, true, false, false);
  AddFuncProperty(proto, u"setUTCDate", DateProto::setUTCDate, true, false, false);
  AddFuncProperty(proto, u"setMonth", DateProto::setMonth, true, false, false);
  AddFuncProperty(proto, u"setUTCMonth", DateProto::setUTCMonth, true, false, false);
  AddFuncProperty(proto, u"setFullYear", DateProto::setFullYear, true, false, false);
  AddFuncProperty(proto, u"setUTCFullYear", DateProto::setUTCFullYear, true, false, false);
  AddFuncProperty(proto, u"toUTCString", DateProto::toUTCString, true, false, false);
  AddFuncProperty(proto, u"toISOString", DateProto::toISOString, true, false, false);
  AddFuncProperty(proto, u"toJSON", DateProto::toJSON, true, false, false);
}

void InitMath() {
  HandleScope scope;
  Handle<JSObject> math = Math::Instance();
  AddFuncProperty(math, u"max", Math::max, false, false, false);
}

void InitRegExp() {
  HandleScope scope;
  Handle<RegExpConstructor> constructor = RegExpConstructor::Instance();
  constructor.val()->SetPrototype(FunctionProto::Instance());
  // 15.6.3 Properties of the RegExpean Constructor
  AddValueProperty(constructor, String::Prototype(), RegExpProto::Instance(), false, false, false);

  Handle<RegExpProto> proto = RegExpProto::Instance();
  proto.val()->SetPrototype(ObjectProto::Instance());
  // 15.6.4 Properties of the RegExpean Prototype Object
  AddValueProperty(proto, String::Constructor(), RegExpConstructor::Instance(), false, false, false);
  AddFuncProperty(proto, u"exec", RegExpProto::exec, true, false, false);
  AddFuncProperty(proto, u"test", RegExpProto::test, true, false, false);
  AddFuncProperty(proto, u"toString", RegExpProto::toString, true, false, false);
};

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
  InitRegExp();
}

Handle<JSValue> StringProto::split(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
  Handle<JSValue> val = Runtime::TopValue();
  val.val()->CheckObjectCoercible(e);
  if (!e->IsOk()) return Handle<JSValue>();
  std::u16string S = ToU16String(e, val);
  if (!e->IsOk()) return Handle<JSValue>();
  Handle<ArrayObject> A = ArrayObject::New(0);
  size_t length_A = 0;
  size_t lim = 4294967295.0;
  if (vals.size() >= 2 && !vals[1].val()->IsUndefined()) {
    lim = ToUint32(e, vals[1]);
    if (!e->IsOk()) return Handle<JSValue>();
  }
  size_t s = S.size();
  size_t p = 0;
  if (lim == 0)
    return A;
  if (vals.size() < 1 || vals[0].val()->IsUndefined()) {  // 10
    AddValueProperty(A, u"0", String::New(S), true, true, true);
    return A;
  }
  if (vals[0].val()->IsRegExpObject()) {
    assert(false);
  }
  assert(vals[0].val()->IsString());
  std::u16string R = ToU16String(e, vals[0]);
  if (s == 0) {
    regex::MatchResult z = regex::SplitMatch(S, 0, R);
    if (!z.failed) return A;
    AddValueProperty(A, u"0", String::New(S), true, true, true);
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
        AddValueProperty(A, NumberToString(length_A), String::New(T), true, true, true);
        length_A++;
        if (length_A == lim)
          return A;
        p = e;
        for (size_t i = 0; i < cap.size(); i++) {  // 13.c.iii.7
          AddValueProperty(A, NumberToString(length_A), String::New(cap[i]), true, true, true);
          length_A++;
          if (length_A == lim)
            return A;
        }
        q = p;
      }
    }
  }
  std::u16string T = S.substr(p);  // 14
  AddValueProperty(A, NumberToString(length_A), String::New(T), true, true, true);  // 15
  return A;
}

}  // namespace es

#endif  // ES_ENTER_CODE_H