#ifndef ES_ENTER_CODE_H
#define ES_ENTER_CODE_H

#include <es/types/error.h>
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
JSValue CreateArgumentsObject(
  JSValue func, std::vector<JSValue>& args,
  JSValue env, bool strict
) {
  JSValue names = function_object::FormalParameters(func);
  int len = args.size();
  JSValue obj = arguments_object::New(len);
  int indx = len - 1;  // 10
  std::set<std::u16string> mapped_names;
  while (indx >= 0) {  // 11
    bool is_accessor_desc = false;
    if ((size_t)indx < fixed_array::size(names)) {  // 11.c
      JSValue name = fixed_array::Get(names, indx);
      ASSERT(name.IsString());
      std::u16string name_str = string::data(name);  // 11.c.i
      if (!strict && mapped_names.find(name_str) == mapped_names.end()) {  // 11.c.ii
        mapped_names.insert(name_str);
        is_accessor_desc = true;
        JSValue ref = reference::New(lexical_env::env_rec(env), name, true);
        JSValue gs = getter_setter::New(ref);
        JSValue desc = property_descriptor::New();
        property_descriptor::SetSet(desc, gs);
        property_descriptor::SetGet(desc, gs);
        property_descriptor::SetConfigurable(desc, true);
        JSValue e = error::Empty();
        DefineOwnProperty(e, obj, NumberToString(indx), desc, false);
      }
    }
    if (!is_accessor_desc) {
      JSValue val = args[indx];  // 11.a
      AddValueProperty(obj, NumberToString(indx), val, true, true, true);  // 11.b
    }
    indx--;  // 11.d
  }
  if (!strict) {  // 13
    AddValueProperty(obj, u"callee", func, true, false, true);
  } else {  // 14
    JSValue desc = property_descriptor::New();
    // TODO(zhuzilin) thrower
    JSValue thrower = undefined::New();
    property_descriptor::SetAccessorDescriptor(desc, thrower, thrower, false, false);
    JSValue e = error::Empty();
    DefineOwnProperty(e, obj, string::New(u"caller"), desc, false);
    DefineOwnProperty(e, obj, string::New(u"callee"), desc, false);
  }
  return obj;  // 15
}

// 10.5 Declaration Binding Instantiation
void DeclarationBindingInstantiation(
  JSValue& e, AST* code, CodeType code_type,
  JSValue f = null::New(), std::vector<JSValue> args = {}
) {
  TEST_LOG("enter DeclarationBindingInstantiation");
  JSValue env = lexical_env::env_rec(Runtime::TopContext().variable_env());  // 1
  bool configurable_bindings = false;
  ProgramOrFunctionBody* body = static_cast<ProgramOrFunctionBody*>(code);
  if (code_type == CODE_EVAL) {
    configurable_bindings = true;  // 2
  }
  bool strict = body->strict() || Runtime::TopContext().strict();  // 3
  if (code_type == CODE_FUNC) {  // 4
    ASSERT(!f.IsNull());
    JSValue names = function_object::FormalParameters(f);  // 4.a
    size_t arg_count = args.size();  // 4.b
    size_t n = 0;  // 4.c
    for (size_t i = 0; i < fixed_array::size(names); i++) {
      JSValue arg_name = fixed_array::Get(names, i);  // 4.d
      JSValue v = undefined::New();
      if (n < arg_count)  // 4.d.i & 4.d.ii
        v = args[n++];
      bool arg_already_declared = HasBinding(env, arg_name);  // 4.d.iii
      if (!arg_already_declared) {  // 4.d.iv
        // NOTE(zhuzlin) I'm not sure if this should be false.
        CreateAndSetMutableBinding(e, env, arg_name, false, v, strict);
        if (unlikely(!error::IsOk(e))) return;
      } else {
        SetMutableBinding(e, env, arg_name, v, strict);  // 4.d.v
        if (unlikely(!error::IsOk(e))) return;
      }
    }
  }
  // 5
  for (Function* func_decl : body->func_decls()) {
    ASSERT(func_decl->is_named());
    JSValue fn = string::New(func_decl->name());
    JSValue fo = InstantiateFunctionDeclaration(e, func_decl);
    if (unlikely(!error::IsOk(e))) return;
    bool func_already_declared = HasBinding(env, fn);
    if (!func_already_declared) {  // 5.d
      CreateAndSetMutableBinding(e, env, fn, configurable_bindings, fo, strict);
      if (unlikely(!error::IsOk(e))) return;
    } else {
      if (env == lexical_env::env_rec(lexical_env::Global())) {  // 5.e
        auto go = global_object::Instance();
        auto existing_prop_desc = GetProperty(go, fn);
        ASSERT(existing_prop_desc.IsPropertyDescriptor());
        if (property_descriptor::Configurable(existing_prop_desc)) {  // 5.e.iii
          auto new_desc = property_descriptor::New();
          property_descriptor::SetDataDescriptor(new_desc, undefined::New(), true, true, configurable_bindings);
          DefineOwnProperty(e, go, fn, new_desc, true);
          if (unlikely(!error::IsOk(e))) return;
        } else {  // 5.e.iv
          if (property_descriptor::IsAccessorDescriptor(existing_prop_desc) ||
              !(property_descriptor::HasWritable(existing_prop_desc) &&
                property_descriptor::Writable(existing_prop_desc) &&
                property_descriptor::HasEnumerable(existing_prop_desc) &&
                property_descriptor::Enumerable(existing_prop_desc))) {
            e = error::TypeError(
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
    bool arguments_already_declared = HasBinding(env, string::Arguments());
    if (!arguments_already_declared) {
      auto args_obj = CreateArgumentsObject(f, args, Runtime::TopContext().variable_env(), strict);
      if (strict) {  // 7.b
        CreateAndInitializeImmutableBinding(env, string::Arguments(), args_obj);
      } else {  // 7.c
        // NOTE(zhuzlin) I'm not sure if this should be false.
        CreateAndSetMutableBinding(e, env, string::Arguments(), false, args_obj, false);
      }
    }
  }
  // 8
  std::vector<VarDecl*>& decls = body->var_decls();
  for (VarDecl* d : decls) {
    JSValue dn = string::New(d->ident().source());
    bool var_already_declared = HasBinding(env, dn);
    if (!var_already_declared) {
      CreateAndSetMutableBinding(e, env, dn, configurable_bindings, undefined::New(), strict);
      if (unlikely(!error::IsOk(e))) return;
    }
  }
}

// 10.4.1
void EnterGlobalCode(JSValue& e, AST* ast) {
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
        e = error::SyntaxError(u"Unexpected future reserved word " + d->ident().source_ref() + u" in strict mode");
        return;
      }
      if (d->ident().source_ref() == u"eval" || d->ident().source_ref() == u"arguments") {
        e = error::SyntaxError(u"Unexpected eval or arguments in strict mode");
        return;
      }
    }
  }
  // 1 10.4.1.1
  JSValue global_env = lexical_env::Global();
  Runtime::Global()->AddContext(ExecutionContext(global_env, global_env, global_object::Instance(), program->strict()));
  // 2
  DeclarationBindingInstantiation(e, program, CODE_GLOBAL);
}

// 10.4.2
void EnterEvalCode(JSValue& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_PROGRAM);
  ProgramOrFunctionBody* program = static_cast<ProgramOrFunctionBody*>(ast);  
  JSValue variable_env;
  JSValue lexical_env;
  JSValue this_binding;
  if (!global_object::direct_eval(global_object::Instance())) {  // 1
    JSValue global_env = lexical_env::Global();
    variable_env = global_env;
    lexical_env = global_env;
    this_binding = global_object::Instance();
  } else {  // 2
    ExecutionContext& calling_context = Runtime::TopContext();
    variable_env = calling_context.variable_env();
    lexical_env = calling_context.lexical_env();
    this_binding = calling_context.this_binding();
  }
  bool strict = Runtime::TopContext().strict() ||
                (program->strict() &&
                 global_object::direct_eval(global_object::Instance()));
  if (strict) {  // 3
    JSValue strict_var_env = NewDeclarativeEnvironment(lexical_env);
    lexical_env = strict_var_env;
    variable_env = strict_var_env;

    for (VarDecl* d : program->var_decls()) {
      if (d->ident().type() == Token::TK_STRICT_FUTURE) {
        e = error::SyntaxError(u"Unexpected future reserved word " + d->ident().source_ref() + u" in strict mode");
        return;
      }
      if (d->ident().source_ref() == u"eval" || d->ident().source_ref() == u"arguments") {
        e = error::SyntaxError(u"Unexpected eval or arguments in strict mode");
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
  JSValue& e, JSValue F, ProgramOrFunctionBody* body,
  JSValue this_arg, std::vector<JSValue> args, bool strict
) {
  ASSERT(F.type() == Type::OBJ_FUNC);
  JSValue this_binding;
  if (strict) {  // 1
    this_binding = this_arg;
  } else if (this_arg.IsUndefined() || this_arg.IsNull()) {  // 2 
    this_binding = global_object::Instance();
  } else if (!this_arg.IsObject()) {  // 3
    this_binding = ToObject(e, this_arg);
  } else {
    this_binding = this_arg;
  }
  JSValue local_env = NewDeclarativeEnvironment(function_object::Scope(F));
  Runtime::Global()->AddContext(ExecutionContext(local_env, local_env, this_binding, strict));  // 8
  // 9
  DeclarationBindingInstantiation(e, body, CODE_FUNC, F, args);
}

void InitGlobalObject() {
  auto global_obj = global_object::Instance();
  // 15.1.1 Value Properties of the Global Object
  AddValueProperty(global_obj, u"NaN", number::NaN(), false, false, false);
  AddValueProperty(global_obj, u"Infinity", number::Infinity(), false, false, false);
  AddValueProperty(global_obj, u"undefined", undefined::New(), false, false, false);
  // 15.1.2 Function Properties of the Global Object
  AddFuncProperty(global_obj, u"eval", global_object::eval, true, false, true);
  AddFuncProperty(global_obj, u"parseInt", global_object::parseInt, true, false, true);
  AddFuncProperty(global_obj, u"parseFloat", global_object::parseFloat, true, false, true);
  AddFuncProperty(global_obj, u"isNaN", global_object::isNaN, true, false, true);
  AddFuncProperty(global_obj, u"isFinite", global_object::isFinite, true, false, true);
  // 15.1.3 URI Handling Function Properties
  AddFuncProperty(global_obj, u"decodeURI", global_object::decodeURI, true, false, true);
  AddFuncProperty(global_obj, u"decodeURIComponent", global_object::decodeURIComponent, true, false, true);
  AddFuncProperty(global_obj, u"encodeURI", global_object::encodeURI, true, false, true);
  AddFuncProperty(global_obj, u"encodeURIComponent", global_object::encodeURIComponent, true, false, true);
  // 15.1.4 Constructor Properties of the Global Object
  AddValueProperty(global_obj, u"Object", object_constructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"Function", function_constructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"Number", number_constructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"Boolean", bool_constructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"String", string_constructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"Array", array_constructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"Date", date_constructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"RegExp", regex_constructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"Error", error_constructor::Instance(), true, false, true);
  // TODO(zhuzilin) differentiate errors.
  AddValueProperty(global_obj, u"EvalError", error_constructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"RangeError", error_constructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"ReferenceError", error_constructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"SyntaxError", error_constructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"TypeError", error_constructor::Instance(), true, false, true);
  AddValueProperty(global_obj, u"URIError", error_constructor::Instance(), true, false, true);

  AddValueProperty(global_obj, u"Math", math::Instance(), true, false, true);

  AddValueProperty(global_obj, u"console", console::Instance(), true, false, true);
  // B.2.1 escape
  AddFuncProperty(global_obj, u"escape", global_object::escape, true, false, true);
  AddFuncProperty(global_obj, u"unescape", global_object::unescape, true, false, true);
}

void InitObject() {
  JSValue constructor = object_constructor::Instance();
  js_object::SetPrototype(constructor, function_proto::Instance());
  // 15.2.3 Properties of the Object Constructor
  AddValueProperty(constructor, string::Prototype(), object_proto::Instance(), false, false, false);
  AddFuncProperty(constructor, u"toString", object_constructor::toString, true, false, false);
  AddFuncProperty(constructor, u"getPrototypeOf", object_constructor::getPrototypeOf, true, false, false);
  AddFuncProperty(constructor, u"getOwnPropertyDescriptor", object_constructor::getOwnPropertyDescriptor, true, false, false);
  AddFuncProperty(constructor, u"getOwnPropertyNames", object_constructor::getOwnPropertyNames, true, false, false);
  AddFuncProperty(constructor, u"create", object_constructor::create, true, false, false);
  AddFuncProperty(constructor, u"defineProperty", object_constructor::defineProperty, true, false, false);
  AddFuncProperty(constructor, u"defineProperties", object_constructor::defineProperties, true, false, false);
  AddFuncProperty(constructor, u"seal", object_constructor::seal, true, false, false);
  AddFuncProperty(constructor, u"freeze", object_constructor::freeze, true, false, false);
  AddFuncProperty(constructor, u"preventExtensions", object_constructor::preventExtensions, true, false, false);
  AddFuncProperty(constructor, u"isSealed", object_constructor::isSealed, true, false, false);
  AddFuncProperty(constructor, u"isFrozen", object_constructor::isFrozen, true, false, false);
  AddFuncProperty(constructor, u"isExtensible", object_constructor::isExtensible, true, false, false);
  AddFuncProperty(constructor, u"keys", object_constructor::keys, true, false, false);
  // ES6
  AddFuncProperty(constructor, u"setPrototypeOf", object_constructor::setPrototypeOf, true, false, false);

  JSValue proto = object_proto::Instance();
  // 15.2.4 Properties of the Object Prototype Object
  AddValueProperty(proto, string::Constructor(), object_constructor::Instance(), false, false, false);
  AddFuncProperty(proto, u"toString", object_proto::toString, true, false, false);
  AddFuncProperty(proto, u"toLocaleString", object_proto::toLocaleString, true, false, false);
  AddFuncProperty(proto, u"valueOf", object_proto::valueOf, true, false, false);
  AddFuncProperty(proto, u"hasOwnProperty", object_proto::hasOwnProperty, true, false, false);
  AddFuncProperty(proto, u"isPrototypeOf", object_proto::isPrototypeOf, true, false, false);
  AddFuncProperty(proto, u"propertyIsEnumerable", object_proto::propertyIsEnumerable, true, false, false);
}

void InitFunction() {
  JSValue constructor = function_constructor::Instance();
  js_object::SetPrototype(constructor, function_proto::Instance());
  // 15.3.3 Properties of the Function Constructor
  AddValueProperty(constructor, string::Prototype(), function_proto::Instance(), false, false, false);
  AddValueProperty(constructor, string::Length(), number::One(), false, false, false);
  AddFuncProperty(constructor, u"toString", function_constructor::toString, true, false, false);

  JSValue proto = function_proto::Instance();
  js_object::SetPrototype(proto, object_proto::Instance());
  // 15.2.4 Properties of the Function Prototype Function
  AddValueProperty(proto, string::Constructor(), function_constructor::Instance(), false, false, false);
  AddValueProperty(proto, string::Length(), number::Zero(), true, false, false);
  AddFuncProperty(proto, u"toString", function_proto::toString, true, false, false);
  AddFuncProperty(proto, u"apply", function_proto::apply, true, false, false);
  AddFuncProperty(proto, u"call", function_proto::call, true, false, false);
  AddFuncProperty(proto, u"bind", function_proto::bind, true, false, false);
}

void InitNumber() {
  JSValue constructor = number_constructor::Instance();
  js_object::SetPrototype(constructor, function_proto::Instance());
  // 15.3.3 Properties of the Number Constructor
  AddValueProperty(constructor, string::Prototype(), number_proto::Instance(), false, false, false);
  AddValueProperty(constructor, string::Length(), number::One(), false, false, false);
  AddValueProperty(constructor, u"MAX_VALUE", number::New(1.7976931348623157e308), false, false, false);
  AddValueProperty(constructor, u"MIN_VALUE", number::New(5e-324), false, false, false);
  AddValueProperty(constructor, u"NaN", number::NaN(), false, false, false);
  AddValueProperty(constructor, u"NEGATIVE_INFINITY", number::NegativeInfinity(), false, false, false);
  AddValueProperty(constructor, u"POSITIVE_INFINITY", number::Infinity(), false, false, false);

  JSValue proto = number_proto::Instance();
  js_object::SetPrototype(proto, object_proto::Instance());
  // 15.2.4 Properties of the Number Prototype Number
  AddValueProperty(proto, string::Constructor(), number_constructor::Instance(), false, false, false);
  AddFuncProperty(proto, u"toString", number_proto::toString, true, false, false);
  AddFuncProperty(proto, u"toLocaleString", number_proto::toLocaleString, true, false, false);
  AddFuncProperty(proto, u"valueOf", number_proto::valueOf, true, false, false);
  AddFuncProperty(proto, u"toFixed", number_proto::toFixed, true, false, false);
  AddFuncProperty(proto, u"toExponential", number_proto::toExponential, true, false, false);
  AddFuncProperty(proto, u"toPrecision", number_proto::toPrecision, true, false, false);
}

void InitError() {
  JSValue constructor = error_constructor::Instance();
  js_object::SetPrototype(constructor, function_proto::Instance());
  // 15.11.3 Properties of the Error Constructor
  AddValueProperty(constructor, string::Prototype(), error_proto::Instance(), false, false, false);

  JSValue proto = error_proto::Instance();
  js_object::SetPrototype(proto, object_proto::Instance());
  // 15.11.4 Properties of the Error Prototype Object
  AddValueProperty(proto, string::Constructor(), error_constructor::Instance(), false, false, false);
  AddValueProperty(proto, u"name", string::New(u"Error"), false, false, false);
  AddValueProperty(proto, u"message", string::Empty(), true, false, false);
  AddFuncProperty(proto, u"toString", error_proto::toString, true, false, false);
}

void InitBool() {
  JSValue constructor = bool_constructor::Instance();
  js_object::SetPrototype(constructor, function_proto::Instance());
  // 15.6.3 Properties of the Boolean Constructor
  AddValueProperty(constructor, string::Prototype(), bool_proto::Instance(), false, false, false);
  AddFuncProperty(constructor, u"toString", bool_constructor::toString, true, false, false);

  JSValue proto = bool_proto::Instance();
  js_object::SetPrototype(proto, object_proto::Instance());
  // 15.6.4 Properties of the Boolean Prototype Object
  AddValueProperty(proto, string::Constructor(), bool_constructor::Instance(), false, false, false);
  AddFuncProperty(proto, u"toString", bool_proto::toString, true, false, false);
  AddFuncProperty(proto, u"valueOf", bool_proto::valueOf, true, false, false);
}

void InitString() {
  JSValue constructor = string_constructor::Instance();
  js_object::SetPrototype(constructor, function_proto::Instance());
  // 15.3.3 Properties of the String Constructor
  AddValueProperty(constructor, string::Prototype(), string_proto::Instance(), false, false, false);
  AddValueProperty(constructor, string::Length(), number::One(), true, false, false);
  AddFuncProperty(constructor, u"fromCharCode", string_constructor::fromCharCode, true, false, false);
  AddFuncProperty(constructor, u"toString", string_constructor::toString, true, false, false);

  JSValue proto = string_proto::Instance();
  js_object::SetPrototype(proto, object_proto::Instance());
  // 15.2.4 Properties of the String Prototype String
  AddValueProperty(proto, string::Constructor(), string_constructor::Instance(), false, false, false);
  AddValueProperty(proto, string::Length(), number::Zero(), true, false, false);
  AddFuncProperty(proto, u"toString", string_proto::toString, true, false, false);
  AddFuncProperty(proto, u"valueOf", string_proto::valueOf, true, false, false);
  AddFuncProperty(proto, u"charAt", string_proto::charAt, true, false, false);
  AddFuncProperty(proto, u"charCodeAt", string_proto::charCodeAt, true, false, false);
  AddFuncProperty(proto, u"concat", string_proto::concat, true, false, false);
  AddFuncProperty(proto, u"indexOf", string_proto::indexOf, true, false, false);
  AddFuncProperty(proto, u"lastIndexOf", string_proto::lastIndexOf, true, false, false);
  AddFuncProperty(proto, u"localeCompare", string_proto::localeCompare, true, false, false);
  AddFuncProperty(proto, u"match", string_proto::match, true, false, false);
  AddFuncProperty(proto, u"replace", string_proto::replace, true, false, false);
  AddFuncProperty(proto, u"search", string_proto::search, true, false, false);
  AddFuncProperty(proto, u"slice", string_proto::slice, true, false, false);
  AddFuncProperty(proto, u"split", string_proto::split, true, false, false);
  AddFuncProperty(proto, u"substring", string_proto::substring, true, false, false);
  AddFuncProperty(proto, u"toLowerCase", string_proto::toLowerCase, true, false, false);
  AddFuncProperty(proto, u"toLocaleLowerCase", string_proto::toLocaleLowerCase, true, false, false);
  AddFuncProperty(proto, u"toUpperCase", string_proto::toUpperCase, true, false, false);
  AddFuncProperty(proto, u"toLocaleUpperCase", string_proto::toLocaleUpperCase, true, false, false);
  AddFuncProperty(proto, u"trim", string_proto::trim, true, false, false);

  AddFuncProperty(proto, u"substr", string_proto::substr, true, false, true);
}

void InitArray() {
  JSValue constructor = array_constructor::Instance();
  js_object::SetPrototype(constructor, function_proto::Instance());
  // 15.6.3 Properties of the Arrayean Constructor
  AddValueProperty(constructor, string::Length(), number::One(), false, false, false);
  AddValueProperty(constructor, string::Prototype(), array_proto::Instance(), false, false, false);
  AddFuncProperty(constructor, u"isArray", array_constructor::isArray, true, false, false);
  AddFuncProperty(constructor, u"toString", array_constructor::toString, true, false, false);

  JSValue proto = array_proto::Instance();
  js_object::SetPrototype(proto, object_proto::Instance());
  // 15.6.4 Properties of the Arrayean Prototype Object
  AddValueProperty(proto, string::Length(), number::Zero(), false, false, false);
  AddValueProperty(proto, string::Constructor(), array_constructor::Instance(), false, false, false);
  AddFuncProperty(proto, u"toString", array_proto::toString, true, false, false);
  AddFuncProperty(proto, u"toLocaleString", array_proto::toLocaleString, true, false, false);
  AddFuncProperty(proto, u"concat", array_proto::concat, true, false, false);
  AddFuncProperty(proto, u"join", array_proto::join, true, false, false);
  AddFuncProperty(proto, u"pop", array_proto::pop, true, false, false);
  AddFuncProperty(proto, u"push", array_proto::push, true, false, false);
  AddFuncProperty(proto, u"reverse", array_proto::reverse, true, false, false);
  AddFuncProperty(proto, u"shift", array_proto::shift, true, false, false);
  AddFuncProperty(proto, u"slice", array_proto::slice, true, false, false);
  AddFuncProperty(proto, u"sort", array_proto::sort, true, false, false);
  AddFuncProperty(proto, u"splice", array_proto::splice, true, false, false);
  AddFuncProperty(proto, u"unshift", array_proto::unshift, true, false, false);
  AddFuncProperty(proto, u"indexOf", array_proto::indexOf, true, false, false);
  AddFuncProperty(proto, u"lastIndexOf", array_proto::lastIndexOf, true, false, false);
  AddFuncProperty(proto, u"every", array_proto::every, true, false, false);
  AddFuncProperty(proto, u"some", array_proto::some, true, false, false);
  AddFuncProperty(proto, u"forEach", array_proto::forEach, true, false, false);
  AddFuncProperty(proto, u"map", array_proto::map, true, false, false);
  AddFuncProperty(proto, u"filter", array_proto::filter, true, false, false);
  AddFuncProperty(proto, u"reduce", array_proto::reduce, true, false, false);
  AddFuncProperty(proto, u"reduceRight", array_proto::reduceRight, true, false, false);
}

void InitDate() {
  JSValue constructor = date_constructor::Instance();
  js_object::SetPrototype(constructor, function_proto::Instance());
  // 15.6.3 Properties of the Dateean Constructor
  AddValueProperty(constructor, string::Length(), number::New(7), false, false, false);
  AddValueProperty(constructor, string::Prototype(), date_proto::Instance(), false, false, false);
  AddFuncProperty(constructor, u"parse", date_constructor::parse, true, false, false);
  AddFuncProperty(constructor, u"UTC", date_constructor::UTC, true, false, false);
  AddFuncProperty(constructor, u"now", date_constructor::now, true, false, false);
  AddFuncProperty(constructor, u"toString", date_constructor::toString, true, false, false);

  JSValue proto = date_proto::Instance();
  js_object::SetPrototype(proto, object_proto::Instance());
  // 15.6.4 Properties of the Dateean Prototype Object
  AddValueProperty(proto, string::Length(), number::Zero(), false, false, false);
  AddValueProperty(proto, string::Constructor(), date_constructor::Instance(), false, false, false);
  AddFuncProperty(proto, u"toString", date_proto::toString, true, false, false);
  AddFuncProperty(proto, u"toDateString", date_proto::toDateString, true, false, false);
  AddFuncProperty(proto, u"toTimeString", date_proto::toTimeString, true, false, false);
  AddFuncProperty(proto, u"toLocaleString", date_proto::toLocaleString, true, false, false);
  AddFuncProperty(proto, u"toLocaleDateString", date_proto::toLocaleDateString, true, false, false);
  AddFuncProperty(proto, u"toLocaleTimeString", date_proto::toLocaleTimeString, true, false, false);
  AddFuncProperty(proto, u"valueOf", date_proto::valueOf, true, false, false);
  AddFuncProperty(proto, u"getTime", date_proto::getTime, true, false, false);
  AddFuncProperty(proto, u"getFullYear", date_proto::getFullYear, true, false, false);
  AddFuncProperty(proto, u"getUTCFullYear", date_proto::getUTCFullYear, true, false, false);
  AddFuncProperty(proto, u"getMonth", date_proto::getMonth, true, false, false);
  AddFuncProperty(proto, u"getUTCMonth", date_proto::getUTCMonth, true, false, false);
  AddFuncProperty(proto, u"getDate", date_proto::getDate, true, false, false);
  AddFuncProperty(proto, u"getUTCDate", date_proto::getUTCDate, true, false, false);
  AddFuncProperty(proto, u"getUTCDay", date_proto::getUTCDay, true, false, false);
  AddFuncProperty(proto, u"getHours", date_proto::getHours, true, false, false);
  AddFuncProperty(proto, u"getUTCHours", date_proto::getUTCHours, true, false, false);
  AddFuncProperty(proto, u"getMinutes", date_proto::getMinutes, true, false, false);
  AddFuncProperty(proto, u"getUTCMinutes", date_proto::getUTCMinutes, true, false, false);
  AddFuncProperty(proto, u"getSeconds", date_proto::getSeconds, true, false, false);
  AddFuncProperty(proto, u"getUTCSeconds", date_proto::getUTCSeconds, true, false, false);
  AddFuncProperty(proto, u"getMilliseconds", date_proto::getMilliseconds, true, false, false);
  AddFuncProperty(proto, u"getUTCMilliseconds", date_proto::getUTCMilliseconds, true, false, false);
  AddFuncProperty(proto, u"getTimezoneOffset", date_proto::getTimezoneOffset, true, false, false);
  AddFuncProperty(proto, u"setTime", date_proto::setTime, true, false, false);
  AddFuncProperty(proto, u"setMilliseconds", date_proto::setMilliseconds, true, false, false);
  AddFuncProperty(proto, u"setUTCMilliseconds", date_proto::setUTCMilliseconds, true, false, false);
  AddFuncProperty(proto, u"setSeconds", date_proto::setSeconds, true, false, false);
  AddFuncProperty(proto, u"setUTCSeconds", date_proto::setUTCSeconds, true, false, false);
  AddFuncProperty(proto, u"setMinutes", date_proto::setMinutes, true, false, false);
  AddFuncProperty(proto, u"setHours", date_proto::setHours, true, false, false);
  AddFuncProperty(proto, u"setUTCHours", date_proto::setUTCHours, true, false, false);
  AddFuncProperty(proto, u"setDate", date_proto::setDate, true, false, false);
  AddFuncProperty(proto, u"setUTCDate", date_proto::setUTCDate, true, false, false);
  AddFuncProperty(proto, u"setMonth", date_proto::setMonth, true, false, false);
  AddFuncProperty(proto, u"setUTCMonth", date_proto::setUTCMonth, true, false, false);
  AddFuncProperty(proto, u"setFullYear", date_proto::setFullYear, true, false, false);
  AddFuncProperty(proto, u"setUTCFullYear", date_proto::setUTCFullYear, true, false, false);
  AddFuncProperty(proto, u"toUTCString", date_proto::toUTCString, true, false, false);
  AddFuncProperty(proto, u"toISOString", date_proto::toISOString, true, false, false);
  AddFuncProperty(proto, u"toJSON", date_proto::toJSON, true, false, false);

  AddFuncProperty(proto, u"getYear", date_proto::getYear, true, false, true);
  AddFuncProperty(proto, u"setYear", date_proto::setYear, true, false, true);
  AddFuncProperty(proto, u"toGMTString", date_proto::toGMTString, true, false, true);
}

void InitMath() {
  JSValue math = math::Instance();
  // 15.8.1 Value Properties of the Math Object
  AddValueProperty(math, string::New(u"E"), number::New(2.7182818284590452354), false, false, false);
  AddValueProperty(math, string::New(u"LN10"), number::New(2.302585092994046), false, false, false);
  AddValueProperty(math, string::New(u"LN2"), number::New(0.6931471805599453), false, false, false);
  AddValueProperty(math, string::New(u"LOG2E"), number::New(1.4426950408889634), false, false, false);
  AddValueProperty(math, string::New(u"LOG10E"), number::New(0.4342944819032518), false, false, false);
  AddValueProperty(math, string::New(u"PI"), number::New(3.1415926535897932), false, false, false);
  AddValueProperty(math, string::New(u"SQRT1_2"), number::New(0.7071067811865476), false, false, false);
  AddValueProperty(math, string::New(u"SQRT2"), number::New(1.4142135623730951), false, false, false);
  // 15.8.2 Function Properties of the Math Object
  AddFuncProperty(math, u"abs", math::abs, false, false, false);
  AddFuncProperty(math, u"ceil", math::ceil, false, false, false);
  AddFuncProperty(math, u"cos", math::cos, false, false, false);
  AddFuncProperty(math, u"exp", math::exp, false, false, false);
  AddFuncProperty(math, u"floor", math::floor, false, false, false);
  AddFuncProperty(math, u"max", math::max, false, false, false);
  AddFuncProperty(math, u"pow", math::pow, false, false, false);
  AddFuncProperty(math, u"round", math::round, false, false, false);
  AddFuncProperty(math, u"sin", math::sin, false, false, false);
  AddFuncProperty(math, u"sqrt", math::sqrt, false, false, false);
}

void InitRegExp() {
  JSValue constructor = regex_constructor::Instance();
  js_object::SetPrototype(constructor, function_proto::Instance());
  // 15.6.3 Properties of the RegExp Constructor
  AddValueProperty(constructor, string::Prototype(), regex_proto::Instance(), false, false, false);

  JSValue proto = regex_proto::Instance();
  js_object::SetPrototype(proto, object_proto::Instance());
  // 15.6.4 Properties of the RegExp Prototype Object
  AddValueProperty(proto, string::Constructor(), regex_constructor::Instance(), false, false, false);
  AddFuncProperty(proto, u"exec", regex_proto::exec, true, false, false);
  AddFuncProperty(proto, u"test", regex_proto::test, true, false, false);
  AddFuncProperty(proto, u"toString", regex_proto::toString, true, false, false);

  AddFuncProperty(proto, u"compile", regex_proto::compile, true, false, true);
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