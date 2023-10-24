#ifndef ES_IMPL_BASE_IMPL_H
#define ES_IMPL_BASE_IMPL_H

#include <es/types/base.h>
#include <es/types/error.h>
#include <es/utils/macros.h>

namespace es {

void CheckObjectCoercible(JSValue& e, JSValue val) {
  if (val.IsUndefined() || val.IsNull()) {
    e = error::TypeError(u"undefined or null is not coercible");
  }
}

bool JSValue::IsCallable() {
  return (IsObject() && READ_VALUE(handle().val(), js_object::kIsCallableOffset, bool)) || IsGetterSetter();
}

bool JSValue::IsConstructor() {
  return IsObject() && READ_VALUE(handle().val(), js_object::kIsConstructorOffset, bool);
}

std::string JSValue::ToString(Type type) {
  switch (type) {
    case JS_UNINIT:
      return "JS_UNINIT";
    case JS_UNDEFINED:
      return "JS_UNDEFINED";
    case JS_NULL:
      return "JS_NULL";
    case JS_BOOL:
      return "JS_BOOL";
    case JS_STRING:
      return "JS_STRING";
    case JS_NUMBER:
      return "JS_NUMBER";

    case JS_OBJECT:
      return "JS_OBJECT";

    case OBJ_GLOBAL:
      return "OBJ_GLOBAL";
    case OBJ_OBJECT:
      return "OBJ_OBJECT";
    case OBJ_FUNC:
      return "OBJ_FUNC";
    case OBJ_BIND_FUNC:
      return "OBJ_BIND_FUNC";
    case OBJ_ARRAY:
      return "OBJ_ARRAY";
    case OBJ_STRING:
      return "OBJ_STRING";
    case OBJ_BOOL:
      return "OBJ_BOOL";
    case OBJ_NUMBER:
      return "OBJ_NUMBER";
    case OBJ_MATH:
      return "OBJ_MATH";
    case OBJ_DATE:
      return "OBJ_DATE";
    case OBJ_REGEXP:
      return "OBJ_REGEXP";
    case OBJ_JSON:
      return "OBJ_JSON";
    case OBJ_ERROR:
      return "OBJ_ERROR";

    case OBJ_FUNC_PROTO:
      return "OBJ_FUNC_PROTO";

    case OBJ_ARRAY_CONSTRUCTOR:
      return "OBJ_ARRAY_CONSTRUCTOR";
    case OBJ_BOOL_CONSTRUCTOR:
      return "OBJ_BOOL_CONSTRUCTOR";
    case OBJ_DATE_CONSTRUCTOR:
      return "OBJ_DATE_CONSTRUCTOR";
    case OBJ_ERROR_CONSTRUCTOR:
      return "OBJ_ERROR_CONSTRUCTOR";
    case OBJ_FUNC_CONSTRUCTOR:
      return "OBJ_FUNC_CONSTRUCTOR";
    case OBJ_NUMBER_CONSTRUCTOR:
      return "OBJ_NUMBER_CONSTRUCTOR";
    case OBJ_OBJECT_CONSTRUCTOR:
      return "OBJ_OBJECT_CONSTRUCTOR";
    case OBJ_REGEXP_CONSTRUCTOR:
      return "OBJ_REGEXP_CONSTRUCTOR";
    case OBJ_STRING_CONSTRUCTOR:
      return "OBJ_STRING_CONSTRUCTOR";

    case OBJ_ARGUMENTS:
      return "OBJ_ARGUMENTS";

    case OBJ_INNER_FUNC:
      return "OBJ_INNER_FUNC";
    case OBJ_HOST:
      return "OBJ_HOST";
    case OBJ_OTHER:
      return "OBJ_OTHER";

    case SPEC_TYPE:
      return "SPEC_TYPE";

    case JS_REF:
      return "JS_REF";
    case JS_PROP_DESC:
      return "JS_PROP_DESC";
    case JS_ENV_REC_DECL:
      return "JS_ENV_REC_DECL";
    case JS_ENV_REC_OBJ:
      return "JS_ENV_REC_OBJ";
    case JS_LEX_ENV:
      return "JS_LEX_ENV";
    case JS_GET_SET:
      return "JS_GET_SET";

    case NON_JSVALUE:
      return "NON_JSVALUE";

    case ERROR:
      return "ERROR";
    case FIXED_ARRAY:
      return "FIXED_ARRAY";
    case HASHMAP:
      return "HASHMAP";
    case BINDING:
      return "BINDING";
    case LIST_NODE:
      return "LIST_NODE";
  }
}

std::string JSValue::ToString(JSValue jsval) {
  switch (jsval.type()) {
    case JS_UNINIT:
      return "Unitialized HeapObject";
    case JS_UNDEFINED:
      return "Undefined";
    case JS_NULL:
      return "Null";
    case JS_BOOL:
      return boolean::data(jsval) ? "true" : "false";
    case JS_STRING:
      return log::ToString(string::data(jsval));
    case JS_NUMBER:
      return NumberToStdString(number::data(jsval));
    case JS_REF: {
      std::u16string name = string::data(reference::GetReferencedName(jsval));
      return "ref(" + log::ToString(name) + ")";
    }
    case JS_PROP_DESC: {
      std::string res = "PropertyDescriptor{";
      res += '}';
      return res;
    }
    case JS_ENV_REC_DECL:
      return "DeclarativeEnvRec()";
    case JS_ENV_REC_OBJ:
      return "ObjectEnvRec()";
    case JS_LEX_ENV:
      return "LexicalEnvironment";
    case JS_GET_SET:
      return "GetterSetter()";
    case ERROR:
      return error::IsOk(jsval) ?
        "ok" :
        "Error(" + JSValue::ToString(GET_JSVALUE(jsval.handle().val(), error::kValueOffset)) + ")";
    case FIXED_ARRAY:
      return "FixedArray(" + std::to_string(READ_VALUE(jsval.handle().val(), fixed_array::kSizeOffset, size_t)) + ")";
    case HASHMAP:
      return "HashMap(" + std::to_string(READ_VALUE(jsval.handle().val(), hash_map::kSizeOffset, size_t)) + ")";
    case BINDING:
      return "Binding(" + JSValue::ToString(GET_JSVALUE(jsval.handle().val(), binding::kValueOffset)) + ")";
    case LIST_NODE:
      return "ListNode(" + JSValue::ToString(GET_JSVALUE(jsval.handle().val(), list_node::kKeyOffset)) + ")";
    case OBJ_ARRAY: {
      JSValue e = error::Empty();
      size_t num = ToNumber(
        e, Get(e, jsval, string::Length())
      );
      return "Array(" + std::to_string(num) + ")";
    }
    case OBJ_FUNC: {
      std::string result = "Function(";
      result += ")";
      return result;
    }
    case OBJ_BIND_FUNC: {
      return "BindFunctionObject";
    }
    case OBJ_REGEXP: {
      return "/" + JSValue::ToString(GET_JSVALUE(jsval.handle().val(), regex_object::kPatternOffset)) +
             "/" + JSValue::ToString(GET_JSVALUE(jsval.handle().val(), regex_object::kFlagOffset));
    }
    case OBJ_ERROR: {
      return JSValue::ToString(GET_JSVALUE(jsval.handle().val(), error_object::kErrorOffset));
    }
    default:
      if (jsval.IsObject()) {
        std::cout << "jsval.IsObject" << std::endl;
        return JSValue::ToString(GET_JSVALUE(jsval.handle().val(), js_object::kClassOffset));
      }
      std::cout << "jsval.type(): " << jsval.type() << std::endl;
      assert(false);
  }
}

std::vector<JSValue> JSValue::RelevantValues(JSValue jsval) {
  switch (jsval.type()) {
    case JS_UNINIT:
    case JS_UNDEFINED:
    case JS_NULL:
    case JS_BOOL:
    case JS_STRING:
    case JS_NUMBER:
      return {};
    case JS_REF:
      return {
        GET_JSVALUE(jsval.handle().val(), reference::kBaseOffset),
        GET_JSVALUE(jsval.handle().val(), reference::kReferenceNameOffset)
      };
    case JS_PROP_DESC:
      return {
        GET_JSVALUE(jsval.handle().val(), property_descriptor::kValueOffset),
        GET_JSVALUE(jsval.handle().val(), property_descriptor::kGetOffset),
        GET_JSVALUE(jsval.handle().val(), property_descriptor::kSetOffset)
      };
    case JS_ENV_REC_DECL:
      return {
        GET_JSVALUE(jsval.handle().val(), decl_env_rec::kBindingsOffset)
      };
    case JS_ENV_REC_OBJ:
      return {
        GET_JSVALUE(jsval.handle().val(), obj_env_rec::kBindingsOffset)
      };
    case JS_LEX_ENV:
      return {
        GET_JSVALUE(jsval.handle().val(), lexical_env::kOuterOffset),
        GET_JSVALUE(jsval.handle().val(), lexical_env::kEnvRecOffset)
      };
    case JS_GET_SET:
      return {
        GET_JSVALUE(jsval.handle().val(), getter_setter::kReferenceOffset)
      };
    case ERROR:
      return {
        GET_JSVALUE(jsval.handle().val(), error::kValueOffset)
      };
    case FIXED_ARRAY: {
      size_t n = READ_VALUE(jsval.handle().val(), fixed_array::kSizeOffset, size_t);
      std::vector<JSValue> values(n);
      for (size_t i = 0; i < n; i++) {
        values[i] = GET_JSVALUE(jsval.handle().val(), fixed_array::kElementOffset + i * sizeof(JSValue));
      }
      return values;
    }
    case HASHMAP: {
      size_t n = READ_VALUE(jsval.handle().val(), hash_map::kNumBucketOffset, size_t);
      std::vector<JSValue> values(n);
      for (size_t i = 0; i < n; i++) {
        values[i] = GET_JSVALUE(jsval.handle().val(), hash_map::kElementOffset + i * sizeof(JSValue));
      }
      values.emplace_back(GET_JSVALUE(jsval.handle().val(), hash_map::kInlineCacheOffset));
      return values;
    }
    case BINDING: {
      return {GET_JSVALUE(jsval.handle().val(), binding::kValueOffset)};
    }
    case LIST_NODE: {
      return {
        GET_JSVALUE(jsval.handle().val(), list_node::kKeyOffset),
        GET_JSVALUE(jsval.handle().val(), list_node::kValOffset),
        GET_JSVALUE(jsval.handle().val(), list_node::kNextOffset)
      };
    }
    default:
      if (jsval.IsObject()) {
        std::vector<JSValue> values {
          GET_JSVALUE(jsval.handle().val(), js_object::kClassOffset),
          GET_JSVALUE(jsval.handle().val(), js_object::kPrimitiveValueOffset),
          GET_JSVALUE(jsval.handle().val(), js_object::kPrototypeOffset),
          GET_JSVALUE(jsval.handle().val(), js_object::kNamedPropertiesOffset) 
        };
        switch (jsval.type()) {
          case OBJ_FUNC: {
            values.emplace_back(GET_JSVALUE(jsval.handle().val(), function_object::kFormalParametersOffset));
            values.emplace_back(GET_JSVALUE(jsval.handle().val(), function_object::kScopeOffset));
            break;
          }
          case OBJ_BIND_FUNC: {
            values.emplace_back(GET_JSVALUE(jsval.handle().val(), bind_function_object::kTargetFunctionOffset));
            values.emplace_back(GET_JSVALUE(jsval.handle().val(), bind_function_object::kBoundThisOffset));
            values.emplace_back(GET_JSVALUE(jsval.handle().val(), bind_function_object::kBoundArgsOffset));
            break;
          }
          case OBJ_REGEXP: {
            values.emplace_back(GET_JSVALUE(jsval.handle().val(), regex_object::kPatternOffset));
            values.emplace_back(GET_JSVALUE(jsval.handle().val(), regex_object::kFlagOffset));
            break;
           }
           default:
            break;
        }
        return values;
      }
      assert(false);
  }
}

}  // namespace es

#endif  // ES_IMPL_BASE_IMPL_H