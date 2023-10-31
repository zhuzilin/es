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

// 10.6 Arguments Object
Handle<ArgumentsObject> CreateArgumentsObject(
  Handle<FunctionObject> func, std::vector<Handle<JSValue>>& args,
  Handle<LexicalEnvironment> env, bool strict
) {
  TEST_LOG("enter CreateArgumentsObject");
  Handle<FixedArray> names = func.val()->FormalParameters();
  int len = args.size();
  Handle<JSObject> obj = ArgumentsObject::New(len);
  int indx = len - 1;  // 10
  std::set<std::u16string> mapped_names;
  while (indx >= 0) {  // 11
    bool is_accessor_desc = false;
    if (!strict && (size_t)indx < names.val()->size()) {  // 11.c
      Handle<String> name = static_cast<Handle<String>>(names.val()->Get(indx));
      std::u16string name_str = name.val()->data();  // 11.c.i
      if (mapped_names.find(name_str) == mapped_names.end()) {  // 11.c.ii
        mapped_names.insert(name_str);
        is_accessor_desc = true;
        Handle<GetterSetter> gs = GetterSetter::New(env.val()->env_rec(), name, true);
        Handle<PropertyDescriptor> desc = PropertyDescriptor::New();
        desc.val()->SetSet(gs);
        desc.val()->SetGet(gs);
        desc.val()->SetConfigurable(true);
        obj.val()->named_properties()->SetRawArray(indx, desc.val());
      }
    }
    if (!is_accessor_desc) {
      Handle<JSValue> val = args[indx];  // 11.a
      Handle<PropertyDescriptor> desc = PropertyDescriptor::NewDataDescriptor(
        val, true, true, true);
      obj.val()->named_properties()->SetRawArray(indx, desc.val()); // 11.b
    }
    indx--;  // 11.d
  }
  if (!strict) {  // 13
    AddValueProperty(obj, String::callee(), func, true, false, true);
  } else {  // 14
    // TODO(zhuzilin) thrower
    Handle<JSValue> thrower = Undefined::Instance();
    StackPropertyDescriptor desc;
    desc.SetAccessorDescriptor(thrower, thrower, false, false);
    DefineOwnProperty(Error::Empty(), obj, String::caller(), desc, false);
    DefineOwnProperty(Error::Empty(), obj, String::callee(), desc, false);
  }
  TEST_LOG("exit CreateArgumentsObject");
  return obj;  // 15
}

// 10.5 Declaration Binding Instantiation
void DeclarationBindingInstantiation(
  Handle<Error>& e, AST* code, CodeType code_type,
  Handle<FunctionObject> f = Handle<FunctionObject>(), std::vector<Handle<JSValue>> args = {}
) {
  TEST_LOG("enter DeclarationBindingInstantiation");
  Handle<EnvironmentRecord> env = Runtime::TopContext().variable_env().val()->env_rec();  // 1
  bool configurable_bindings = false;
  ProgramOrFunctionBody* body = static_cast<ProgramOrFunctionBody*>(code);
  if (code_type == CODE_EVAL) {
    configurable_bindings = true;  // 2
  }
  bool strict = body->strict() || Runtime::TopContext().strict();  // 3
  if (code_type == CODE_FUNC) {  // 4
    ASSERT(!f.IsNullptr());
    auto names = f.val()->FormalParameters();  // 4.a
    size_t arg_count = args.size();  // 4.b
    for (size_t i = 0; i < names.val()->size(); i++) {
      Handle<String> arg_name = names.val()->Get(i);  // 4.d
      Handle<JSValue> v = Undefined::Instance();
      if (i < arg_count)  // 4.d.i & 4.d.ii
        v = args[i];
      bool arg_already_declared = HasBinding(env, arg_name);  // 4.d.iii
      if (!arg_already_declared) {  // 4.d.iv
        // NOTE(zhuzlin) I'm not sure if this should be false.
        CreateAndSetMutableBinding(e, env, arg_name, false, v, strict);
        if (unlikely(!e.val()->IsOk())) return;
      } else {
        SetMutableBinding(e, env, arg_name, v, strict);  // 4.d.v
        if (unlikely(!e.val()->IsOk())) return;
      }
    }
  }
  // 5
  for (Function* func_decl : body->func_decls()) {
    ASSERT(func_decl->is_named());
    Handle<String> fn = func_decl->name();
    Handle<FunctionObject> fo = InstantiateFunctionDeclaration(e, func_decl);
    if (unlikely(!e.val()->IsOk())) return;
    bool func_already_declared = HasBinding(env, fn);
    if (!func_already_declared) {  // 5.d
      CreateAndSetMutableBinding(e, env, fn, configurable_bindings, fo, strict);
      if (unlikely(!e.val()->IsOk())) return;
    } else {
      if (env.val() == LexicalEnvironment::Global().val()->env_rec().val()) {  // 5.e
        auto go = GlobalObject::Instance();
        StackPropertyDescriptor existing_prop_desc = GetProperty(go, fn);
        if (existing_prop_desc.Configurable()) {  // 5.e.iii
          auto new_desc = StackPropertyDescriptor::NewDataDescriptor(Undefined::Instance(), true, true, configurable_bindings);
          DefineOwnProperty(e, go, fn, new_desc, true);
          if (unlikely(!e.val()->IsOk())) return;
        } else {  // 5.e.iv
          if (existing_prop_desc.IsAccessorDescriptor() ||
              !(existing_prop_desc.HasWritable() && existing_prop_desc.Writable() &&
                existing_prop_desc.HasEnumerable() && existing_prop_desc.Enumerable())) {
            e = Error::TypeError(
              u"existing desc of " + func_decl->name().val()->data() + " is accessor "
              u"but not both configurable and enumerable");
            return;
          }
        }
      }
      SetMutableBinding(e, env, fn, fo, strict);  // 5.f
    }
  }
  // 7
  if (code_type == CODE_FUNC) {
    // 6
    bool arguments_already_declared = HasBinding(env, String::arguments());
    if (!arguments_already_declared) {
      auto args_obj = CreateArgumentsObject(f, args, Runtime::TopContext().variable_env(), strict);
      if (strict) {  // 7.b
        Handle<DeclarativeEnvironmentRecord> decl_env = static_cast<Handle<DeclarativeEnvironmentRecord>>(env);
        CreateAndInitializeImmutableBinding(decl_env, String::arguments(), args_obj);
      } else {  // 7.c
        // NOTE(zhuzlin) I'm not sure if this should be false.
        CreateAndSetMutableBinding(e, env, String::arguments(), false, args_obj, false);
      }
    }
  }
  // 8
  std::vector<VarDecl*>& decls = body->var_decls();
  for (VarDecl* d : decls) {
    Handle<String> dn = d->ident();
    bool var_already_declared = HasBinding(env, dn);
    if (!var_already_declared) {
      CreateAndSetMutableBinding(e, env, dn, configurable_bindings, Undefined::Instance(), strict);
      if (unlikely(!e.val()->IsOk())) return;
    }
  }
}

// 10.4.1
void EnterGlobalCode(Handle<Error>& e, AST* ast) {
  ProgramOrFunctionBody* program;
  if (ast->type() == AST::AST_PROGRAM) {
    program = static_cast<ProgramOrFunctionBody*>(ast);
  } else {
    // TODO(zhuzilin) This is for test. Add test label like #ifdefine TEST
    program = new ProgramOrFunctionBody(AST::AST_PROGRAM, false);
    program->AddStatement(ast);
  }
  if (program->strict()) {
    for (VarDecl* d : program->var_decls()) {
      if (d->is_strict_future()) {
        e = Error::SyntaxError(u"Unexpected future reserved word " + d->ident().val()->data() + u" in strict mode");
        return;
      }
      if (d->is_eval_or_arguments()) {
        e = Error::SyntaxError(u"Unexpected eval or arguments in strict mode");
        return;
      }
    }
  }
  // 1 10.4.1.1
  Handle<LexicalEnvironment> global_env = LexicalEnvironment::Global();
  Runtime::Global()->AddContext(ExecutionContext(global_env, global_env, GlobalObject::Instance(), program->strict()));
  // 2
  DeclarationBindingInstantiation(e, program, CODE_GLOBAL);
}

// 10.4.2
void EnterEvalCode(Handle<Error>& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_PROGRAM);
  ProgramOrFunctionBody* program = static_cast<ProgramOrFunctionBody*>(ast);  
  Handle<LexicalEnvironment> variable_env;
  Handle<LexicalEnvironment> lexical_env;
  Handle<JSValue> this_binding;
  if (!GlobalObject::Instance().val()->direct_eval()) {  // 1
    Handle<LexicalEnvironment> global_env = LexicalEnvironment::Global();
    variable_env = global_env;
    lexical_env = global_env;
    this_binding = GlobalObject::Instance();
  } else {  // 2
    ExecutionContext& calling_context = Runtime::TopContext();
    variable_env = calling_context.variable_env();
    lexical_env = calling_context.lexical_env();
    this_binding = calling_context.this_binding();
  }
  bool strict = Runtime::TopContext().strict() ||
                (program->strict() && GlobalObject::Instance().val()->direct_eval());
  if (strict) {  // 3
    Handle<LexicalEnvironment> strict_var_env = NewDeclarativeEnvironment(lexical_env, program->num_decls());
    lexical_env = strict_var_env;
    variable_env = strict_var_env;

    for (VarDecl* d : program->var_decls()) {
      if (d->is_strict_future()) {
        e = Error::SyntaxError(u"Unexpected future reserved word " + d->ident().val()->data() + u" in strict mode");
        return;
      }
      if (d->is_eval_or_arguments()) {
        e = Error::SyntaxError(u"Unexpected eval or arguments in strict mode");
        return;
      }
    }
  }
  Runtime::Global()->AddContext(ExecutionContext(variable_env, lexical_env, this_binding, strict));
  // 4
  DeclarationBindingInstantiation(e, program, CODE_EVAL);
}

// 10.4.3
void EnterFunctionCode(
  Handle<Error>& e, Handle<JSObject> F, ProgramOrFunctionBody* body,
  Handle<JSValue> this_arg, std::vector<Handle<JSValue>> args, bool strict
) {
  ASSERT(F.val()->type() == Type::OBJ_FUNC);
  Handle<FunctionObject> func = static_cast<Handle<FunctionObject>>(F);
  Handle<JSValue> this_binding;
  if (strict) {  // 1
    this_binding = this_arg;
  } else if (this_arg.val()->IsUndefined() || this_arg.val()->IsNull()) {  // 2 
    this_binding = GlobalObject::Instance();
  } else if (!this_arg.val()->IsObject()) {  // 3
    this_binding = ToObject(e, this_arg);
  } else {
    this_binding = this_arg;
  }
  Handle<LexicalEnvironment> local_env = NewDeclarativeEnvironment(func.val()->Scope(), body->num_decls());
  Runtime::Global()->AddContext(ExecutionContext(local_env, local_env, this_binding, strict));  // 8
  // 9
  DeclarationBindingInstantiation(e, body, CODE_FUNC, func, args);
}

void InitGlobalObject() {
  HandleScope scope;
  auto global_obj = GlobalObject::Instance();
  // 15.1.1 Value Properties of the Global Object
  AddValueProperty(global_obj, String::New(u"NaN", GCFlag::CONST), Number::NaN(), false, false, false);
  AddValueProperty(global_obj, String::New(u"Infinity", GCFlag::CONST), Number::Infinity(), false, false, false);
  AddValueProperty(global_obj, String::New(u"undefined", GCFlag::CONST), Undefined::Instance(), false, false, false);
  // 15.1.2 Function Properties of the Global Object
  AddFuncProperty(global_obj, String::New(u"eval", GCFlag::CONST), GlobalObject::eval, true, false, true);
  AddFuncProperty(global_obj, String::New(u"parseInt", GCFlag::CONST), GlobalObject::parseInt, true, false, true);
  AddFuncProperty(global_obj, String::New(u"parseFloat", GCFlag::CONST), GlobalObject::parseFloat, true, false, true);
  AddFuncProperty(global_obj, String::New(u"isNaN", GCFlag::CONST), GlobalObject::isNaN, true, false, true);
  AddFuncProperty(global_obj, String::New(u"isFinite", GCFlag::CONST), GlobalObject::isFinite, true, false, true);
  // 15.1.3 URI Handling Function Properties
  AddFuncProperty(global_obj, String::New(u"decodeURI", GCFlag::CONST), GlobalObject::decodeURI, true, false, true);
  AddFuncProperty(global_obj, String::New(u"decodeURIComponent", GCFlag::CONST), GlobalObject::decodeURIComponent, true, false, true);
  AddFuncProperty(global_obj, String::New(u"encodeURI", GCFlag::CONST), GlobalObject::encodeURI, true, false, true);
  AddFuncProperty(global_obj, String::New(u"encodeURIComponent", GCFlag::CONST), GlobalObject::encodeURIComponent, true, false, true);
  // 15.1.4 Constructor Properties of the Global Object
  AddValueProperty(global_obj, String::New(u"Object", GCFlag::CONST), ObjectConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, String::New(u"Function", GCFlag::CONST), FunctionConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, String::New(u"Number", GCFlag::CONST), NumberConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, String::New(u"Boolean", GCFlag::CONST), BoolConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, String::New(u"String", GCFlag::CONST), StringConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, String::New(u"Array", GCFlag::CONST), ArrayConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, String::New(u"Date", GCFlag::CONST), DateConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, String::New(u"RegExp", GCFlag::CONST), RegExpConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, String::New(u"Error", GCFlag::CONST), ErrorConstructor::Instance(), true, false, true);
  // TODO(zhuzilin) differentiate errors.
  AddValueProperty(global_obj, String::New(u"EvalError", GCFlag::CONST), ErrorConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, String::New(u"RangeError", GCFlag::CONST), ErrorConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, String::New(u"ReferenceError", GCFlag::CONST), ErrorConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, String::New(u"SyntaxError", GCFlag::CONST), ErrorConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, String::New(u"TypeError", GCFlag::CONST), ErrorConstructor::Instance(), true, false, true);
  AddValueProperty(global_obj, String::New(u"URIError", GCFlag::CONST), ErrorConstructor::Instance(), true, false, true);

  AddValueProperty(global_obj, String::New(u"Math", GCFlag::CONST), Math::Instance(), true, false, true);

  AddValueProperty(global_obj, String::New(u"console", GCFlag::CONST), Console::Instance(), true, false, true);
}

void InitObject() {
  Handle<ObjectConstructor> constructor = ObjectConstructor::Instance();
  constructor.val()->SetPrototype(FunctionProto::Instance());
  // 15.2.3 Properties of the Object Constructor
  AddValueProperty(constructor, String::Prototype(), ObjectProto::Instance(), false, false, false);
  AddFuncProperty(constructor, String::New(u"toString", GCFlag::CONST), ObjectConstructor::toString, true, false, false);
  AddFuncProperty(constructor, String::New(u"getPrototypeOf", GCFlag::CONST), ObjectConstructor::getPrototypeOf, true, false, false);
  AddFuncProperty(constructor, String::New(u"getOwnPropertyDescriptor", GCFlag::CONST), ObjectConstructor::getOwnPropertyDescriptor, true, false, false);
  AddFuncProperty(constructor, String::New(u"getOwnPropertyNames", GCFlag::CONST), ObjectConstructor::getOwnPropertyNames, true, false, false);
  AddFuncProperty(constructor, String::New(u"create", GCFlag::CONST), ObjectConstructor::create, true, false, false);
  AddFuncProperty(constructor, String::New(u"defineProperty", GCFlag::CONST), ObjectConstructor::defineProperty, true, false, false);
  AddFuncProperty(constructor, String::New(u"defineProperties", GCFlag::CONST), ObjectConstructor::defineProperties, true, false, false);
  AddFuncProperty(constructor, String::New(u"seal", GCFlag::CONST), ObjectConstructor::seal, true, false, false);
  AddFuncProperty(constructor, String::New(u"freeze", GCFlag::CONST), ObjectConstructor::freeze, true, false, false);
  AddFuncProperty(constructor, String::New(u"preventExtensions", GCFlag::CONST), ObjectConstructor::preventExtensions, true, false, false);
  AddFuncProperty(constructor, String::New(u"isSealed", GCFlag::CONST), ObjectConstructor::isSealed, true, false, false);
  AddFuncProperty(constructor, String::New(u"isFrozen", GCFlag::CONST), ObjectConstructor::isFrozen, true, false, false);
  AddFuncProperty(constructor, String::New(u"isExtensible", GCFlag::CONST), ObjectConstructor::isExtensible, true, false, false);
  AddFuncProperty(constructor, String::New(u"keys", GCFlag::CONST), ObjectConstructor::keys, true, false, false);
  // ES6
  AddFuncProperty(constructor, String::New(u"setPrototypeOf", GCFlag::CONST), ObjectConstructor::setPrototypeOf, true, false, false);

  Handle<ObjectProto> proto = ObjectProto::Instance();
  // 15.2.4 Properties of the Object Prototype Object
  AddValueProperty(proto, String::Constructor(), ObjectConstructor::Instance(), false, false, false);
  AddFuncProperty(proto, String::New(u"toString", GCFlag::CONST), ObjectProto::toString, true, false, false);
  AddFuncProperty(proto, String::New(u"toLocaleString", GCFlag::CONST), ObjectProto::toLocaleString, true, false, false);
  AddFuncProperty(proto, String::New(u"valueOf", GCFlag::CONST), ObjectProto::valueOf, true, false, false);
  AddFuncProperty(proto, String::New(u"hasOwnProperty", GCFlag::CONST), ObjectProto::hasOwnProperty, true, false, false);
  AddFuncProperty(proto, String::New(u"isPrototypeOf", GCFlag::CONST), ObjectProto::isPrototypeOf, true, false, false);
  AddFuncProperty(proto, String::New(u"propertyIsEnumerable", GCFlag::CONST), ObjectProto::propertyIsEnumerable, true, false, false);
}

void InitFunction() {
  HandleScope scope;
  Handle<FunctionConstructor> constructor = FunctionConstructor::Instance();
  constructor.val()->SetPrototype(FunctionProto::Instance());
  // 15.3.3 Properties of the Function Constructor
  AddValueProperty(constructor, String::Prototype(), FunctionProto::Instance(), false, false, false);
  AddValueProperty(constructor, String::Length(), Number::New(1), false, false, false);
  AddFuncProperty(constructor, String::New(u"toString", GCFlag::CONST), FunctionConstructor::toString, true, false, false);

  Handle<FunctionProto> proto = FunctionProto::Instance();
  proto.val()->SetPrototype(ObjectProto::Instance());
  // 15.2.4 Properties of the Function Prototype Function
  AddValueProperty(proto, String::Constructor(), FunctionConstructor::Instance(), false, false, false);
  AddValueProperty(proto, String::Length(), Number::Zero(), true, false, false);
  AddFuncProperty(proto, String::New(u"toString", GCFlag::CONST), FunctionProto::toString, true, false, false);
  AddFuncProperty(proto, String::New(u"apply", GCFlag::CONST), FunctionProto::apply, true, false, false);
  AddFuncProperty(proto, String::New(u"call", GCFlag::CONST), FunctionProto::call, true, false, false);
  AddFuncProperty(proto, String::New(u"bind", GCFlag::CONST), FunctionProto::bind, true, false, false);
}

void InitNumber() {
  HandleScope scope;
  Handle<NumberConstructor> constructor = NumberConstructor::Instance();
  constructor.val()->SetPrototype(FunctionProto::Instance());
  // 15.3.3 Properties of the Number Constructor
  AddValueProperty(constructor, String::Prototype(), NumberProto::Instance(), false, false, false);
  AddValueProperty(constructor, String::Length(), Number::New(1), false, false, false);
  AddValueProperty(constructor, String::New(u"MAX_VALUE", GCFlag::CONST), Number::New(1.7976931348623157e308), false, false, false);
  AddValueProperty(constructor, String::New(u"MIN_VALUE", GCFlag::CONST), Number::New(5e-324), false, false, false);
  AddValueProperty(constructor, String::New(u"NaN", GCFlag::CONST), Number::NaN(), false, false, false);
  AddValueProperty(constructor, String::New(u"NEGATIVE_INFINITY", GCFlag::CONST), Number::NegativeInfinity(), false, false, false);
  AddValueProperty(constructor, String::New(u"POSITIVE_INFINITY", GCFlag::CONST), Number::Infinity(), false, false, false);

  Handle<NumberProto> proto = NumberProto::Instance();
  proto.val()->SetPrototype(ObjectProto::Instance());
  // 15.2.4 Properties of the Number Prototype Number
  AddValueProperty(proto, String::Constructor(), NumberConstructor::Instance(), false, false, false);
  AddFuncProperty(proto, String::New(u"toString", GCFlag::CONST), NumberProto::toString, true, false, false);
  AddFuncProperty(proto, String::New(u"toLocaleString", GCFlag::CONST), NumberProto::toLocaleString, true, false, false);
  AddFuncProperty(proto, String::New(u"valueOf", GCFlag::CONST), NumberProto::valueOf, true, false, false);
  AddFuncProperty(proto, String::New(u"toFixed", GCFlag::CONST), NumberProto::toFixed, true, false, false);
  AddFuncProperty(proto, String::New(u"toExponential", GCFlag::CONST), NumberProto::toExponential, true, false, false);
  AddFuncProperty(proto, String::New(u"toPrecision", GCFlag::CONST), NumberProto::toPrecision, true, false, false);
}

void InitError() {
  HandleScope scope;
  Handle<ErrorConstructor> constructor = ErrorConstructor::Instance();
  constructor.val()->SetPrototype(FunctionProto::Instance());
  // 15.11.3 Properties of the Error Constructor
  AddValueProperty(constructor, String::Prototype(), ErrorProto::Instance(), false, false, false);

  Handle<ErrorProto> proto = ErrorProto::Instance();
  proto.val()->SetPrototype(ObjectProto::Instance());
  // 15.11.4 Properties of the Error Prototype Object
  AddValueProperty(proto, String::Constructor(), ErrorConstructor::Instance(), false, false, false);
  AddValueProperty(proto, String::New(u"name", GCFlag::CONST), String::New(u"Error", GCFlag::CONST), false, false, false);
  AddValueProperty(proto, String::New(u"message", GCFlag::CONST), String::Empty(), true, false, false);
  AddFuncProperty(proto, String::New(u"toString", GCFlag::CONST), ErrorProto::toString, true, false, false);
}

void InitBool() {
  HandleScope scope;
  Handle<BoolConstructor> constructor = BoolConstructor::Instance();
  constructor.val()->SetPrototype(FunctionProto::Instance());
  // 15.6.3 Properties of the Boolean Constructor
  AddValueProperty(constructor, String::Prototype(), BoolProto::Instance(), false, false, false);
  AddFuncProperty(constructor, String::New(u"toString", GCFlag::CONST), BoolConstructor::toString, true, false, false);

  Handle<BoolProto> proto = BoolProto::Instance();
  proto.val()->SetPrototype(ObjectProto::Instance());
  // 15.6.4 Properties of the Boolean Prototype Object
  AddValueProperty(proto, String::Constructor(), BoolConstructor::Instance(), false, false, false);
  AddFuncProperty(proto, String::New(u"toString", GCFlag::CONST), BoolProto::toString, true, false, false);
  AddFuncProperty(proto, String::New(u"valueOf", GCFlag::CONST), BoolProto::valueOf, true, false, false);
}

void InitString() {
  HandleScope scope;
  Handle<StringConstructor> constructor = StringConstructor::Instance();
  constructor.val()->SetPrototype(FunctionProto::Instance());
  // 15.3.3 Properties of the String Constructor
  AddValueProperty(constructor, String::Prototype(), StringProto::Instance(), false, false, false);
  AddValueProperty(constructor, String::Length(), Number::New(1), true, false, false);
  AddFuncProperty(constructor, String::New(u"fromCharCode", GCFlag::CONST), StringConstructor::fromCharCode, true, false, false);
  AddFuncProperty(constructor, String::New(u"toString", GCFlag::CONST), StringConstructor::toString, true, false, false);

  Handle<StringProto> proto = StringProto::Instance();
  proto.val()->SetPrototype(ObjectProto::Instance());
  // 15.2.4 Properties of the String Prototype String
  AddValueProperty(proto, String::Constructor(), StringConstructor::Instance(), false, false, false);
  AddValueProperty(proto, String::Length(), Number::Zero(), true, false, false);
  AddFuncProperty(proto, String::New(u"toString", GCFlag::CONST), StringProto::toString, true, false, false);
  AddFuncProperty(proto, String::New(u"valueOf", GCFlag::CONST), StringProto::valueOf, true, false, false);
  AddFuncProperty(proto, String::New(u"charAt", GCFlag::CONST), StringProto::charAt, true, false, false);
  AddFuncProperty(proto, String::New(u"charCodeAt", GCFlag::CONST), StringProto::charCodeAt, true, false, false);
  AddFuncProperty(proto, String::New(u"concat", GCFlag::CONST), StringProto::concat, true, false, false);
  AddFuncProperty(proto, String::New(u"indexOf", GCFlag::CONST), StringProto::indexOf, true, false, false);
  AddFuncProperty(proto, String::New(u"lastIndexOf", GCFlag::CONST), StringProto::lastIndexOf, true, false, false);
  AddFuncProperty(proto, String::New(u"localeCompare", GCFlag::CONST), StringProto::localeCompare, true, false, false);
  AddFuncProperty(proto, String::New(u"match", GCFlag::CONST), StringProto::match, true, false, false);
  AddFuncProperty(proto, String::New(u"replace", GCFlag::CONST), StringProto::replace, true, false, false);
  AddFuncProperty(proto, String::New(u"search", GCFlag::CONST), StringProto::search, true, false, false);
  AddFuncProperty(proto, String::New(u"slice", GCFlag::CONST), StringProto::slice, true, false, false);
  AddFuncProperty(proto, String::New(u"split", GCFlag::CONST), StringProto::split, true, false, false);
  AddFuncProperty(proto, String::New(u"substring", GCFlag::CONST), StringProto::substring, true, false, false);
  AddFuncProperty(proto, String::New(u"toLowerCase", GCFlag::CONST), StringProto::toLowerCase, true, false, false);
  AddFuncProperty(proto, String::New(u"toLocaleLowerCase", GCFlag::CONST), StringProto::toLocaleLowerCase, true, false, false);
  AddFuncProperty(proto, String::New(u"toUpperCase", GCFlag::CONST), StringProto::toUpperCase, true, false, false);
  AddFuncProperty(proto, String::New(u"toLocaleUpperCase", GCFlag::CONST), StringProto::toLocaleUpperCase, true, false, false);
  AddFuncProperty(proto, String::New(u"trim", GCFlag::CONST), StringProto::trim, true, false, false);
}

void InitArray() {
  HandleScope scope;
  Handle<ArrayConstructor> constructor = ArrayConstructor::Instance();
  constructor.val()->SetPrototype(FunctionProto::Instance());
  // 15.6.3 Properties of the Arrayean Constructor
  AddValueProperty(constructor, String::Length(), Number::New(1), false, false, false);
  AddValueProperty(constructor, String::Prototype(), ArrayProto::Instance(), false, false, false);
  AddFuncProperty(constructor, String::New(u"isArray", GCFlag::CONST), ArrayConstructor::isArray, true, false, false);
  AddFuncProperty(constructor, String::New(u"toString", GCFlag::CONST), ArrayConstructor::toString, true, false, false);

  Handle<ArrayProto> proto = ArrayProto::Instance();
  proto.val()->SetPrototype(ObjectProto::Instance());
  // 15.6.4 Properties of the Arrayean Prototype Object
  AddValueProperty(proto, String::Length(), Number::Zero(), false, false, false);
  AddValueProperty(proto, String::Constructor(), ArrayConstructor::Instance(), false, false, false);
  AddFuncProperty(proto, String::New(u"toString", GCFlag::CONST), ArrayProto::toString, true, false, false);
  AddFuncProperty(proto, String::New(u"toLocaleString", GCFlag::CONST), ArrayProto::toLocaleString, true, false, false);
  AddFuncProperty(proto, String::New(u"concat", GCFlag::CONST), ArrayProto::concat, true, false, false);
  AddFuncProperty(proto, String::New(u"join", GCFlag::CONST), ArrayProto::join, true, false, false);
  AddFuncProperty(proto, String::New(u"pop", GCFlag::CONST), ArrayProto::pop, true, false, false);
  AddFuncProperty(proto, String::New(u"push", GCFlag::CONST), ArrayProto::push, true, false, false);
  AddFuncProperty(proto, String::New(u"reverse", GCFlag::CONST), ArrayProto::reverse, true, false, false);
  AddFuncProperty(proto, String::New(u"shift", GCFlag::CONST), ArrayProto::shift, true, false, false);
  AddFuncProperty(proto, String::New(u"slice", GCFlag::CONST), ArrayProto::slice, true, false, false);
  AddFuncProperty(proto, String::New(u"sort", GCFlag::CONST), ArrayProto::sort, true, false, false);
  AddFuncProperty(proto, String::New(u"splice", GCFlag::CONST), ArrayProto::splice, true, false, false);
  AddFuncProperty(proto, String::New(u"unshift", GCFlag::CONST), ArrayProto::unshift, true, false, false);
  AddFuncProperty(proto, String::New(u"indexOf", GCFlag::CONST), ArrayProto::indexOf, true, false, false);
  AddFuncProperty(proto, String::New(u"lastIndexOf", GCFlag::CONST), ArrayProto::lastIndexOf, true, false, false);
  AddFuncProperty(proto, String::New(u"every", GCFlag::CONST), ArrayProto::every, true, false, false);
  AddFuncProperty(proto, String::New(u"some", GCFlag::CONST), ArrayProto::some, true, false, false);
  AddFuncProperty(proto, String::New(u"forEach", GCFlag::CONST), ArrayProto::forEach, true, false, false);
  AddFuncProperty(proto, String::New(u"map", GCFlag::CONST), ArrayProto::map, true, false, false);
  AddFuncProperty(proto, String::New(u"filter", GCFlag::CONST), ArrayProto::filter, true, false, false);
  AddFuncProperty(proto, String::New(u"reduce", GCFlag::CONST), ArrayProto::reduce, true, false, false);
  AddFuncProperty(proto, String::New(u"reduceRight", GCFlag::CONST), ArrayProto::reduceRight, true, false, false);
}

void InitDate() {
  HandleScope scope;
  Handle<DateConstructor> constructor = DateConstructor::Instance();
  constructor.val()->SetPrototype(FunctionProto::Instance());
  // 15.6.3 Properties of the Dateean Constructor
  AddValueProperty(constructor, String::Length(), Number::New(7), false, false, false);
  AddValueProperty(constructor, String::Prototype(), DateProto::Instance(), false, false, false);
  AddFuncProperty(constructor, String::New(u"parse", GCFlag::CONST), DateConstructor::parse, true, false, false);
  AddFuncProperty(constructor, String::New(u"UTC", GCFlag::CONST), DateConstructor::UTC, true, false, false);
  AddFuncProperty(constructor, String::New(u"now", GCFlag::CONST), DateConstructor::now, true, false, false);
  AddFuncProperty(constructor, String::New(u"toString", GCFlag::CONST), DateConstructor::toString, true, false, false);

  Handle<DateProto> proto = DateProto::Instance();
  proto.val()->SetPrototype(ObjectProto::Instance());
  // 15.6.4 Properties of the Dateean Prototype Object
  AddValueProperty(proto, String::Length(), Number::Zero(), false, false, false);
  AddValueProperty(proto, String::Constructor(), DateConstructor::Instance(), false, false, false);
  AddFuncProperty(proto, String::New(u"toString", GCFlag::CONST), DateProto::toString, true, false, false);
  AddFuncProperty(proto, String::New(u"toDateString", GCFlag::CONST), DateProto::toDateString, true, false, false);
  AddFuncProperty(proto, String::New(u"toTimeString", GCFlag::CONST), DateProto::toTimeString, true, false, false);
  AddFuncProperty(proto, String::New(u"toLocaleString", GCFlag::CONST), DateProto::toLocaleString, true, false, false);
  AddFuncProperty(proto, String::New(u"toLocaleDateString", GCFlag::CONST), DateProto::toLocaleDateString, true, false, false);
  AddFuncProperty(proto, String::New(u"toLocaleTimeString", GCFlag::CONST), DateProto::toLocaleTimeString, true, false, false);
  AddFuncProperty(proto, String::New(u"valueOf", GCFlag::CONST), DateProto::valueOf, true, false, false);
  AddFuncProperty(proto, String::New(u"getTime", GCFlag::CONST), DateProto::getTime, true, false, false);
  AddFuncProperty(proto, String::New(u"getFullYear", GCFlag::CONST), DateProto::getFullYear, true, false, false);
  AddFuncProperty(proto, String::New(u"getUTCFullYear", GCFlag::CONST), DateProto::getUTCFullYear, true, false, false);
  AddFuncProperty(proto, String::New(u"getMonth", GCFlag::CONST), DateProto::getMonth, true, false, false);
  AddFuncProperty(proto, String::New(u"getUTCMonth", GCFlag::CONST), DateProto::getUTCMonth, true, false, false);
  AddFuncProperty(proto, String::New(u"getDate", GCFlag::CONST), DateProto::getDate, true, false, false);
  AddFuncProperty(proto, String::New(u"getUTCDate", GCFlag::CONST), DateProto::getUTCDate, true, false, false);
  AddFuncProperty(proto, String::New(u"getUTCDay", GCFlag::CONST), DateProto::getUTCDay, true, false, false);
  AddFuncProperty(proto, String::New(u"getHours", GCFlag::CONST), DateProto::getHours, true, false, false);
  AddFuncProperty(proto, String::New(u"getUTCHours", GCFlag::CONST), DateProto::getUTCHours, true, false, false);
  AddFuncProperty(proto, String::New(u"getMinutes", GCFlag::CONST), DateProto::getMinutes, true, false, false);
  AddFuncProperty(proto, String::New(u"getUTCMinutes", GCFlag::CONST), DateProto::getUTCMinutes, true, false, false);
  AddFuncProperty(proto, String::New(u"getSeconds", GCFlag::CONST), DateProto::getSeconds, true, false, false);
  AddFuncProperty(proto, String::New(u"getUTCSeconds", GCFlag::CONST), DateProto::getUTCSeconds, true, false, false);
  AddFuncProperty(proto, String::New(u"getMilliseconds", GCFlag::CONST), DateProto::getMilliseconds, true, false, false);
  AddFuncProperty(proto, String::New(u"getUTCMilliseconds", GCFlag::CONST), DateProto::getUTCMilliseconds, true, false, false);
  AddFuncProperty(proto, String::New(u"getTimezoneOffset", GCFlag::CONST), DateProto::getTimezoneOffset, true, false, false);
  AddFuncProperty(proto, String::New(u"setTime", GCFlag::CONST), DateProto::setTime, true, false, false);
  AddFuncProperty(proto, String::New(u"setMilliseconds", GCFlag::CONST), DateProto::setMilliseconds, true, false, false);
  AddFuncProperty(proto, String::New(u"setUTCMilliseconds", GCFlag::CONST), DateProto::setUTCMilliseconds, true, false, false);
  AddFuncProperty(proto, String::New(u"setSeconds", GCFlag::CONST), DateProto::setSeconds, true, false, false);
  AddFuncProperty(proto, String::New(u"setUTCSeconds", GCFlag::CONST), DateProto::setUTCSeconds, true, false, false);
  AddFuncProperty(proto, String::New(u"setMinutes", GCFlag::CONST), DateProto::setMinutes, true, false, false);
  AddFuncProperty(proto, String::New(u"setHours", GCFlag::CONST), DateProto::setHours, true, false, false);
  AddFuncProperty(proto, String::New(u"setUTCHours", GCFlag::CONST), DateProto::setUTCHours, true, false, false);
  AddFuncProperty(proto, String::New(u"setDate", GCFlag::CONST), DateProto::setDate, true, false, false);
  AddFuncProperty(proto, String::New(u"setUTCDate", GCFlag::CONST), DateProto::setUTCDate, true, false, false);
  AddFuncProperty(proto, String::New(u"setMonth", GCFlag::CONST), DateProto::setMonth, true, false, false);
  AddFuncProperty(proto, String::New(u"setUTCMonth", GCFlag::CONST), DateProto::setUTCMonth, true, false, false);
  AddFuncProperty(proto, String::New(u"setFullYear", GCFlag::CONST), DateProto::setFullYear, true, false, false);
  AddFuncProperty(proto, String::New(u"setUTCFullYear", GCFlag::CONST), DateProto::setUTCFullYear, true, false, false);
  AddFuncProperty(proto, String::New(u"toUTCString", GCFlag::CONST), DateProto::toUTCString, true, false, false);
  AddFuncProperty(proto, String::New(u"toISOString", GCFlag::CONST), DateProto::toISOString, true, false, false);
  AddFuncProperty(proto, String::New(u"toJSON", GCFlag::CONST), DateProto::toJSON, true, false, false);
}

void InitMath() {
  HandleScope scope;
  Handle<JSObject> math = Math::Instance();
  // 15.8.1 Value Properties of the Math Object
  AddValueProperty(math, String::New(u"E", GCFlag::CONST), Number::New(2.7182818284590452354), false, false, false);
  AddValueProperty(math, String::New(u"LN10", GCFlag::CONST), Number::New(2.302585092994046), false, false, false);
  AddValueProperty(math, String::New(u"LN2", GCFlag::CONST), Number::New(0.6931471805599453), false, false, false);
  AddValueProperty(math, String::New(u"LOG2E", GCFlag::CONST), Number::New(1.4426950408889634), false, false, false);
  AddValueProperty(math, String::New(u"LOG10E", GCFlag::CONST), Number::New(0.4342944819032518), false, false, false);
  AddValueProperty(math, String::New(u"PI", GCFlag::CONST), Number::New(3.1415926535897932), false, false, false);
  AddValueProperty(math, String::New(u"SQRT1_2", GCFlag::CONST), Number::New(0.7071067811865476), false, false, false);
  AddValueProperty(math, String::New(u"SQRT2", GCFlag::CONST), Number::New(1.4142135623730951), false, false, false);
  // 15.8.2 Function Properties of the Math Object
  AddFuncProperty(math, String::New(u"abs", GCFlag::CONST), Math::abs, false, false, false);
  AddFuncProperty(math, String::New(u"ceil", GCFlag::CONST), Math::ceil, false, false, false);
  AddFuncProperty(math, String::New(u"cos", GCFlag::CONST), Math::cos, false, false, false);
  AddFuncProperty(math, String::New(u"exp", GCFlag::CONST), Math::exp, false, false, false);
  AddFuncProperty(math, String::New(u"floor", GCFlag::CONST), Math::floor, false, false, false);
  AddFuncProperty(math, String::New(u"max", GCFlag::CONST), Math::max, false, false, false);
  AddFuncProperty(math, String::New(u"pow", GCFlag::CONST), Math::pow, false, false, false);
  AddFuncProperty(math, String::New(u"round", GCFlag::CONST), Math::round, false, false, false);
  AddFuncProperty(math, String::New(u"sin", GCFlag::CONST), Math::sin, false, false, false);
  AddFuncProperty(math, String::New(u"sqrt", GCFlag::CONST), Math::sqrt, false, false, false);
}

void InitRegExp() {
  HandleScope scope;
  Handle<RegExpConstructor> constructor = RegExpConstructor::Instance();
  constructor.val()->SetPrototype(FunctionProto::Instance());
  // 15.6.3 Properties of the RegExp Constructor
  AddValueProperty(constructor, String::Prototype(), RegExpProto::Instance(), false, false, false);

  Handle<RegExpProto> proto = RegExpProto::Instance();
  proto.val()->SetPrototype(ObjectProto::Instance());
  // 15.6.4 Properties of the RegExp Prototype Object
  AddValueProperty(proto, String::Constructor(), RegExpConstructor::Instance(), false, false, false);
  AddFuncProperty(proto, String::New(u"exec", GCFlag::CONST), RegExpProto::exec, true, false, false);
  AddFuncProperty(proto, String::New(u"test", GCFlag::CONST), RegExpProto::test, true, false, false);
  AddFuncProperty(proto, String::New(u"toString", GCFlag::CONST), RegExpProto::toString, true, false, false);
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

}  // namespace es

#endif  // ES_ENTER_CODE_H