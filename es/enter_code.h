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
  Handle<FixedArray> names = func.val()->FormalParameters();
  int len = args.size();
  Handle<JSObject> obj = ArgumentsObject::New(len);
  int indx = len - 1;  // 10
  std::set<std::u16string> mapped_names;
  while (indx >= 0) {  // 11
    bool is_accessor_desc = false;
    if ((size_t)indx < names.val()->size()) {  // 11.c
      Handle<String> name = static_cast<Handle<String>>(names.val()->Get(indx));
      std::u16string name_str = name.val()->data();  // 11.c.i
      if (!strict && mapped_names.find(name_str) == mapped_names.end()) {  // 11.c.ii
        mapped_names.insert(name_str);
        is_accessor_desc = true;
        Handle<Reference> ref = Reference::New(env.val()->env_rec(), name, true);
        Handle<GetterSetter> gs = GetterSetter::New(ref);
        Handle<PropertyDescriptor> desc = PropertyDescriptor::New();
        desc.val()->SetSet(gs);
        desc.val()->SetGet(gs);
        desc.val()->SetConfigurable(true);
        DefineOwnProperty(Error::Empty(), obj, NumberToString(indx), desc, false);
      }
    }
    if (!is_accessor_desc) {
      Handle<JSValue> val = args[indx];  // 11.a
      AddValueProperty(obj, NumberToString(indx), val, true, true, true);  // 11.b
    }
    indx--;  // 11.d
  }
  if (!strict) {  // 13
    AddValueProperty(obj, u"callee", func, true, false, true);
  } else {  // 14
    Handle<PropertyDescriptor> desc = PropertyDescriptor::New();
    // TODO(zhuzilin) thrower
    Handle<JSValue> thrower = Undefined::Instance();
    desc.val()->SetAccessorDescriptor(thrower, thrower, false, false);
    DefineOwnProperty(Error::Empty(), obj, String::New(u"caller"), desc, false);
    DefineOwnProperty(Error::Empty(), obj, String::New(u"callee"), desc, false);
  }
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
    size_t n = 0;  // 4.c
    for (size_t i = 0; i < names.val()->size(); i++) {
      Handle<String> arg_name = names.val()->Get(i);  // 4.d
      Handle<JSValue> v = Undefined::Instance();
      if (n < arg_count)  // 4.d.i & 4.d.ii
        v = args[n++];
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
    Handle<String> fn = String::New(func_decl->name());
    Handle<FunctionObject> fo = InstantiateFunctionDeclaration(e, func_decl);
    if (unlikely(!e.val()->IsOk())) return;
    bool func_already_declared = HasBinding(env, fn);
    if (!func_already_declared) {  // 5.d
      CreateAndSetMutableBinding(e, env, fn, configurable_bindings, fo, strict);
      if (unlikely(!e.val()->IsOk())) return;
    } else {
      if (env.val() == LexicalEnvironment::Global().val()->env_rec().val()) {  // 5.e
        auto go = GlobalObject::Instance();
        auto existing_prop = GetProperty(go, fn);
        ASSERT(existing_prop.val()->IsPropertyDescriptor());
        auto existing_prop_desc = static_cast<Handle<PropertyDescriptor>>(existing_prop);
        if (existing_prop_desc.val()->Configurable()) {  // 5.e.iii
          auto new_desc = PropertyDescriptor::New();
          new_desc.val()->SetDataDescriptor(Undefined::Instance(), true, true, configurable_bindings);
          DefineOwnProperty(e, go, fn, new_desc, true);
          if (unlikely(!e.val()->IsOk())) return;
        } else {  // 5.e.iv
          if (existing_prop_desc.val()->IsAccessorDescriptor() ||
              !(existing_prop_desc.val()->HasWritable() && existing_prop_desc.val()->Writable() &&
                existing_prop_desc.val()->HasEnumerable() && existing_prop_desc.val()->Enumerable())) {
            e = Error::TypeError(
              u"existing desc of " + func_decl->name() + " is accessor "
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
    bool arguments_already_declared = HasBinding(env, String::Arguments());
    if (!arguments_already_declared) {
      auto args_obj = CreateArgumentsObject(f, args, Runtime::TopContext().variable_env(), strict);
      if (strict) {  // 7.b
        Handle<DeclarativeEnvironmentRecord> decl_env = static_cast<Handle<DeclarativeEnvironmentRecord>>(env);
        CreateAndInitializeImmutableBinding(decl_env, String::Arguments(), args_obj);
      } else {  // 7.c
        // NOTE(zhuzlin) I'm not sure if this should be false.
        CreateAndSetMutableBinding(e, env, String::Arguments(), false, args_obj, false);
      }
    }
  }
  // 8
  std::vector<VarDecl*>& decls = body->var_decls();
  for (VarDecl* d : decls) {
    Handle<String> dn = String::New(d->ident().source());
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
      if (d->ident().type() == Token::TK_STRICT_FUTURE) {
        e = Error::SyntaxError(u"Unexpected future reserved word " + d->ident().source_ref() + u" in strict mode");
        return;
      }
      if (d->ident().source_ref() == u"eval" || d->ident().source_ref() == u"arguments") {
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
    Handle<LexicalEnvironment> strict_var_env = NewDeclarativeEnvironment(lexical_env);
    lexical_env = strict_var_env;
    variable_env = strict_var_env;

    for (VarDecl* d : program->var_decls()) {
      if (d->ident().type() == Token::TK_STRICT_FUTURE) {
        e = Error::SyntaxError(u"Unexpected future reserved word " + d->ident().source_ref() + u" in strict mode");
        return;
      }
      if (d->ident().source_ref() == u"eval" || d->ident().source_ref() == u"arguments") {
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
  ASSERT(F.val()->type() == JSObject::OBJ_FUNC);
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
  Handle<LexicalEnvironment> local_env = NewDeclarativeEnvironment(func.val()->Scope());
  Runtime::Global()->AddContext(ExecutionContext(local_env, local_env, this_binding, strict));  // 8
  // 9
  DeclarationBindingInstantiation(e, body, CODE_FUNC, func, args);
}

void InitGlobalObject() {
  HandleScope scope;
  auto global_obj = GlobalObject::Instance();
  // 15.1.1 Value Properties of the Global Object
  AddValueProperty(global_obj, u"NaN", Number::NaN(), false, false, false);
  AddValueProperty(global_obj, u"Infinity", Number::Infinity(), false, false, false);
  AddValueProperty(global_obj, u"undefined", Undefined::Instance(), false, false, false);
  // 15.1.2 Function Properties of the Global Object
  AddFuncProperty(global_obj, u"eval", GlobalObject::eval, true, false, true);
  AddFuncProperty(global_obj, u"parseInt", GlobalObject::parseInt, true, false, true);
  AddFuncProperty(global_obj, u"parseFloat", GlobalObject::parseFloat, true, false, true);
  AddFuncProperty(global_obj, u"isNaN", GlobalObject::isNaN, true, false, true);
  AddFuncProperty(global_obj, u"isFinite", GlobalObject::isFinite, true, false, true);
  // 15.1.3 URI Handling Function Properties
  AddFuncProperty(global_obj, u"decodeURI", GlobalObject::decodeURI, true, false, true);
  AddFuncProperty(global_obj, u"decodeURIComponent", GlobalObject::decodeURIComponent, true, false, true);
  AddFuncProperty(global_obj, u"encodeURI", GlobalObject::encodeURI, true, false, true);
  AddFuncProperty(global_obj, u"encodeURIComponent", GlobalObject::encodeURIComponent, true, false, true);
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
  AddValueProperty(proto, String::Length(), Number::Zero(), true, false, false);
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
  AddValueProperty(constructor, u"NEGATIVE_INFINITY", Number::NegativeInfinity(), false, false, false);
  AddValueProperty(constructor, u"POSITIVE_INFINITY", Number::Infinity(), false, false, false);

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

  Handle<ErrorProto> proto = ErrorProto::Instance();
  proto.val()->SetPrototype(ObjectProto::Instance());
  // 15.11.4 Properties of the Error Prototype Object
  AddValueProperty(proto, String::Constructor(), ErrorConstructor::Instance(), false, false, false);
  AddValueProperty(proto, u"name", String::New(u"Error"), false, false, false);
  AddValueProperty(proto, u"message", String::Empty(), true, false, false);
  AddFuncProperty(proto, u"toString", ErrorProto::toString, true, false, false);
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
  AddValueProperty(proto, String::Length(), Number::Zero(), true, false, false);
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
  // 15.8.1 Value Properties of the Math Object
  AddValueProperty(math, String::New(u"E"), Number::New(2.7182818284590452354), false, false, false);
  AddValueProperty(math, String::New(u"LN10"), Number::New(2.302585092994046), false, false, false);
  AddValueProperty(math, String::New(u"LN2"), Number::New(0.6931471805599453), false, false, false);
  AddValueProperty(math, String::New(u"LOG2E"), Number::New(1.4426950408889634), false, false, false);
  AddValueProperty(math, String::New(u"LOG10E"), Number::New(0.4342944819032518), false, false, false);
  AddValueProperty(math, String::New(u"PI"), Number::New(3.1415926535897932), false, false, false);
  AddValueProperty(math, String::New(u"SQRT1_2"), Number::New(0.7071067811865476), false, false, false);
  AddValueProperty(math, String::New(u"SQRT2"), Number::New(1.4142135623730951), false, false, false);
  // 15.8.2 Function Properties of the Math Object
  AddFuncProperty(math, u"abs", Math::abs, false, false, false);
  AddFuncProperty(math, u"ceil", Math::ceil, false, false, false);
  AddFuncProperty(math, u"cos", Math::cos, false, false, false);
  AddFuncProperty(math, u"exp", Math::exp, false, false, false);
  AddFuncProperty(math, u"floor", Math::floor, false, false, false);
  AddFuncProperty(math, u"max", Math::max, false, false, false);
  AddFuncProperty(math, u"pow", Math::pow, false, false, false);
  AddFuncProperty(math, u"round", Math::round, false, false, false);
  AddFuncProperty(math, u"sin", Math::sin, false, false, false);
  AddFuncProperty(math, u"sqrt", Math::sqrt, false, false, false);
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

}  // namespace es

#endif  // ES_ENTER_CODE_H