#ifndef ES_ENTER_CODE_H
#define ES_ENTER_CODE_H

#include <es/error.h>
#include <es/types/object.h>
#include <es/types/builtin/object_object.h>
#include <es/types/builtin/function_object.h>
#include <es/types/builtin/number_object.h>
#include <es/execution_context.h>

namespace es {

enum CodeType {
  CODE_GLOBAL = 0,
  CODE_FUNC,
  CODE_EVAL,
};

JSObject* CreateArgumentsObject() {
  // TODO(zhuzilin)
  return nullptr;
}

// 10.5
void DeclarationBindingInstantiation(
  Error* e, ExecutionContext context, AST* code, CodeType code_type, bool strict,
  FunctionObject* f = nullptr, std::vector<JSValue*> args = {}
) {
  log::PrintSource("enter DeclarationBindingInstantiation");
  auto env = context.variable_env()->env_rec();  // 1
  bool configurable_bindings = false;
  if (code_type == CODE_EVAL) {
    configurable_bindings = true;  // 2
  }
  if (code_type == CODE_FUNC) {  // 4
    assert(f != nullptr);
    auto names = f->FormalParameters();  // 4.a
    Function* func_ast = static_cast<Function*>(code);
    size_t arg_count = args.size();  // 4.b
    size_t n = 0;  // 4.c
    for (auto arg_name : names) {  // 4.d
      log::PrintSource("preparing arg ", arg_name);
      JSValue* v = Undefined::Instance();
      if (n < arg_count)  // 4.d.i & 4.d.ii
        v = args[n++];
      bool arg_already_declared = env->HasBinding(arg_name);  // 4.d.iii
      if (!arg_already_declared) {  // 4.d.iv
        // NOTE(zhuzlin) I'm not sure if this should be false.
        env->CreateMutableBinding(e, arg_name, false);
        if (e != nullptr)
          return;
      }
      env->SetMutableBinding(e, arg_name, v, strict);  // 4.d.v
    }
  }
  ProgramOrFunctionBody* body = static_cast<ProgramOrFunctionBody*>(code);
  // 5
  for (Function* func_decl : body->func_decls()) {
    assert(func_decl->is_named());
    std::u16string_view fn = func_decl->name();
    FunctionObject* fo = InstantiateFunctionDeclaration(func_decl);
    bool func_already_declared = env->HasBinding(fn);
    if (!func_already_declared) {  // 5.d
      env->CreateMutableBinding(e, fn, configurable_bindings);
      if (e != nullptr)
        return;
    } else {  // 5.e
      auto go = GlobalObject::Instance();
      auto existing_prop = go->GetProperty(fn);
      assert(!existing_prop->IsUndefined());
      auto existing_prop_desc = static_cast<PropertyDescriptor*>(existing_prop);
      if (existing_prop_desc->Configurable()) {  // 5.e.iii
        auto new_desc = new PropertyDescriptor();
        new_desc->SetDataDescriptor(Undefined::Instance(), true, true, configurable_bindings);
        go->DefineOwnProperty(e, fn, new_desc, true);
      } else {  // 5.e.iv
        if (existing_prop_desc->IsAccessorDescriptor() ||
            !(existing_prop_desc->HasConfigurable() && existing_prop_desc->Configurable() &&
              existing_prop_desc->HasEnumerable() && existing_prop_desc->Enumerable())) {
          e = Error::TypeError();
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
    // auto args_obj = CreateArgumentsObject();
    // if (strict) {  // 7.b
    //   DeclarativeEnvironmentRecord* decl_env = static_cast<DeclarativeEnvironmentRecord*>(env);
    //   decl_env->CreateImmutableBinding(u"arguments");
    //   decl_env->InitializeImmutableBinding(u"arguments", args_obj);
    // } else {  // 7.c
    //   // NOTE(zhuzlin) I'm not sure if this should be false.
    //   env->CreateMutableBinding(e, u"arguments", false);
    //   env->SetMutableBinding(e, u"arguments", args_obj, false);
    // }
  }
  // 8
  // TODO(zhuzilin) initialize VariableDeclaration
}

// 10.4.1
void EnterGlobalCode(Error* e, AST* ast) {
  ProgramOrFunctionBody* program;
  if (ast->type() == AST::AST_PROGRAM) {
    program = static_cast<ProgramOrFunctionBody*>(ast);
  } else {
    // TODO(zhuzilin) This is for test. Add test label like #ifdefine TEST
    program = new ProgramOrFunctionBody(AST::AST_PROGRAM);
    program->AddStatement(ast);
  }
  LexicalEnvironment* global_env = new LexicalEnvironment(
    Null::Instance(), new ObjectEnvironmentRecord(GlobalObject::Instance()));
  // 1 10.4.1.1
  ExecutionContext context(global_env, global_env, GlobalObject::Instance());
  ExecutionContextStack::Global()->AddContext(context);
  // 2
  // TODO(zhuzilin) strict
  DeclarationBindingInstantiation(e, context, program, CODE_GLOBAL, false);
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
  ExecutionContext context(local_env, local_env, this_binding);  // 8
  ExecutionContextStack::Global()->AddContext(context);
  // 9
  DeclarationBindingInstantiation(e, context, body, CODE_FUNC, false, func, args);
}

void InitGlobalObject() {
  auto global_obj = GlobalObject::Instance();
  // 15.1.1 Value Properties of the Global Object
  global_obj->AddValueProperty(u"NaN", Number::NaN(), false, false, false);
  global_obj->AddValueProperty(u"Infinity", new Number(0, 1), false, false, false);
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
  constructor->AddValueProperty(u"NEGATIVE_INFINITY", new Number(0, 1), false, false, false);
  constructor->AddValueProperty(u"POSITIVE_INFINITY", new Number(0, -1), false, false, false);

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
  proto->AddFuncProperty(u"toFixed", NumberProto::toFixed, false, false, false);
  proto->AddFuncProperty(u"toFixed", NumberProto::toFixed, false, false, false);
  proto->AddFuncProperty(u"toFixed", NumberProto::toFixed, false, false, false);
}

void Init() {
  InitGlobalObject();
  InitObject();
  InitFunction();
  InitNumber();
}

}  // namespace es

#endif  // ES_ENTER_CODE_H