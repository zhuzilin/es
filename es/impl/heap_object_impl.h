#ifndef ES_IMPL_HEAP_OBJECT_IMPL_H
#define ES_IMPL_HEAP_OBJECT_IMPL_H

#include <es/types/object.h>
#include <es/types/builtin/arguments_object.h>
#include <es/types/builtin/array_object.h>
#include <es/types/builtin/bool_object.h>
#include <es/types/builtin/date_object.h>
#include <es/types/builtin/error_object.h>
#include <es/types/builtin/function_object.h>
#include <es/types/builtin/math_object.h>
#include <es/types/builtin/number_object.h>
#include <es/types/builtin/object_object.h>
#include <es/types/builtin/regexp_object.h>
#include <es/types/builtin/string_object.h>

namespace es {

template<typename T>
std::string Handle<T>::ToString() {
  if (IsNullptr())
    return "nullptr";
  return HeapObject::ToString(val());
}

std::string HeapObject::ToString(HeapObject* heap_obj) {
  switch (heap_obj->type()) {
    case JS_UNINIT:
      return "Unitialized HeapObject";
    case JS_UNDEFINED:
      return "Undefined";
    case JS_NULL:
      return "Null";
    case JS_BOOL:
      return static_cast<Bool*>(heap_obj)->data() ? "true" : "false";
    case JS_STRING:
      return log::ToString(static_cast<String*>(heap_obj)->data());
    case JS_NUMBER:
      return NumberToStdString(static_cast<Number*>(heap_obj)->data());
    case JS_REF: {
      String* name = READ_VALUE(heap_obj, Reference::kReferenceNameOffset, String*);
      return "ref(" + ToString(name) + ")";
    }
    case JS_PROP_DESC: {
      PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(heap_obj);
      std::string res = "PropertyDescriptor{";
      if (desc->HasValue()) res += "v: " + ToString(READ_VALUE(heap_obj, PropertyDescriptor::kValueOffset, JSValue*)) + ", ";
      if (desc->HasWritable()) res += "w: " + log::ToString(desc->Writable()) + ", ";
      if (desc->HasGet()) res += "get: " + ToString(READ_VALUE(heap_obj, PropertyDescriptor::kGetOffset, JSValue*)) + ", ";
      if (desc->HasSet()) res += "set: " + ToString(READ_VALUE(heap_obj, PropertyDescriptor::kSetOffset, JSValue*)) + ", ";
      if (desc->HasEnumerable()) res += "e: " + log::ToString(desc->Enumerable()) + ", ";
      if (desc->HasConfigurable()) res += "c: " + log::ToString(desc->Configurable());
      res += '}';
      return res;
    }
    case JS_ENV_REC_DECL:
      return "DeclarativeEnvRec(" + log::ToString(heap_obj) + ")";
    case JS_ENV_REC_OBJ:
      return "ObjectEnvRec(" + log::ToString(heap_obj) + ")";
    case JS_LEX_ENV:
      return "LexicalEnvironment";
    case JS_GET_SET:
      return "GetterSetter(" + ToString(READ_VALUE(heap_obj, GetterSetter::kReferenceOffset, Reference*)) + ")";
    case ERROR:
      return static_cast<Error*>(heap_obj)->IsOk() ?
        "ok" :
        ("error(" + ToString(READ_VALUE(heap_obj, Error::kValueOffset, JSValue*)) + ")");
    case FIXED_ARRAY:
      return "FixedArray(" + std::to_string(READ_VALUE(heap_obj, FixedArray::kSizeOffset, size_t)) + ")";
    case HASHMAP:
      return "HashMap(" + std::to_string(READ_VALUE(heap_obj, HashMap::kSizeOffset, size_t)) + ")";
    case BINDING:
      return "Binding(" + ToString(READ_VALUE(heap_obj, Binding::kValueOffset, JSValue*)) + ")";
    case LIST_NODE:
      return "ListNode(" + ToString(READ_VALUE(heap_obj, ListNode::kKeyOffset, String*)) + ")";
    case OBJ_ARRAY: {
      size_t num = ToNumber(
        Error::Empty(),
        Get(Error::Empty(), Handle<JSObject>(static_cast<JSObject*>(heap_obj)), String::Length())
      );
      return "Array(" + std::to_string(num) + ")";
    }
    case OBJ_FUNC: {
      std::string result = "Function(";
      FixedArray* params = READ_VALUE(heap_obj, FunctionObject::kFormalParametersOffset, FixedArray*);
      if (params->size() > 0) {
        result += ToString(params->GetRaw(0));
        for (size_t i = 1; i < params->size(); i++) {
          result += "," + ToString(params->GetRaw(i));
        }
      }
      result += ")";
      return result;
    }
    case OBJ_BIND_FUNC: {
      return "BindFunctionObject";
    }
    case OBJ_REGEXP: {
      return "/" + ToString(READ_VALUE(heap_obj, RegExpObject::kPatternOffset, String*)) +
             "/" + ToString(READ_VALUE(heap_obj, RegExpObject::kFlagOffset, String*));
    }
    default:
      if (heap_obj->IsObject()) {
        return ToString(READ_VALUE(heap_obj, JSObject::kClassOffset, String*));
      }
      assert(false);
  }
}

std::vector<HeapObject**> HeapObject::Pointers(HeapObject* heap_obj) {
  switch (heap_obj->type()) {
    case JS_UNINIT:
    case JS_UNDEFINED:
    case JS_NULL:
    case JS_BOOL:
    case JS_STRING:
    case JS_NUMBER:
      return {};
    case JS_REF:
      return {
        HEAP_PTR(heap_obj, Reference::kBaseOffset),
        HEAP_PTR(heap_obj, Reference::kReferenceNameOffset)
      };
    case JS_PROP_DESC:
      return {
        HEAP_PTR(heap_obj, PropertyDescriptor::kValueOffset),
        HEAP_PTR(heap_obj, PropertyDescriptor::kGetOffset),
        HEAP_PTR(heap_obj, PropertyDescriptor::kSetOffset)
      };
    case JS_ENV_REC_DECL:
      return {
        HEAP_PTR(heap_obj, DeclarativeEnvironmentRecord::kBindingsOffset)
      };
    case JS_ENV_REC_OBJ:
      return {
        HEAP_PTR(heap_obj, ObjectEnvironmentRecord::kBindingsOffset)
      };
    case JS_LEX_ENV:
      return {
        HEAP_PTR(heap_obj, LexicalEnvironment::kOuterOffset),
        HEAP_PTR(heap_obj, LexicalEnvironment::kEnvRecOffset)
      };
    case JS_GET_SET:
      return {
        HEAP_PTR(heap_obj, GetterSetter::kReferenceOffset)
      };
    case ERROR:
      return {
        HEAP_PTR(heap_obj, Error::kValueOffset)
      };
    case FIXED_ARRAY: {
      size_t n = READ_VALUE(heap_obj, FixedArray::kSizeOffset, size_t);
      std::vector<HeapObject**> pointers(n);
      for (size_t i = 0; i < n; i++) {
        pointers[i] = HEAP_PTR(heap_obj, FixedArray::kElementOffset + i * kPtrSize);
      }
      return pointers;
    }
    case HASHMAP: {
      size_t n = READ_VALUE(heap_obj, HashMap::kNumBucketOffset, size_t);
      std::vector<HeapObject**> pointers(n);
      for (size_t i = 0; i < n; i++) {
        pointers[i] = HEAP_PTR(heap_obj, HashMap::kElementOffset + i * kPtrSize);
      }
      pointers.emplace_back(HEAP_PTR(heap_obj, HashMap::kInlineCacheOffset));
      return pointers;
    }
    case BINDING: {
      return {HEAP_PTR(heap_obj, Binding::kValueOffset)};
    }
    case LIST_NODE: {
      return {
        HEAP_PTR(heap_obj, ListNode::kKeyOffset),
        HEAP_PTR(heap_obj, ListNode::kValOffset),
        HEAP_PTR(heap_obj, ListNode::kNextOffset)
      };
    }
    default:
      if (heap_obj->IsObject()) {
        std::vector<HeapObject**> pointers {
          HEAP_PTR(heap_obj, JSObject::kClassOffset),
          HEAP_PTR(heap_obj, JSObject::kPrimitiveValueOffset),
          HEAP_PTR(heap_obj, JSObject::kPrototypeOffset),
          HEAP_PTR(heap_obj, JSObject::kNamedPropertiesOffset) 
        };
        switch (heap_obj->type()) {
          case OBJ_FUNC: {
            pointers.emplace_back(HEAP_PTR(heap_obj, FunctionObject::kFormalParametersOffset));
            pointers.emplace_back(HEAP_PTR(heap_obj, FunctionObject::kScopeOffset));
            break;
          }
          case OBJ_BIND_FUNC: {
            pointers.emplace_back(HEAP_PTR(heap_obj, BindFunctionObject::kTargetFunctionOffset));
            pointers.emplace_back(HEAP_PTR(heap_obj, BindFunctionObject::kBoundThisOffset));
            pointers.emplace_back(HEAP_PTR(heap_obj, BindFunctionObject::kBoundArgsOffset));
            break;
          }
          case OBJ_REGEXP: {
            pointers.emplace_back(HEAP_PTR(heap_obj, RegExpObject::kPatternOffset));
            pointers.emplace_back(HEAP_PTR(heap_obj, RegExpObject::kFlagOffset));
            break;
           }
           default:
            break;
        }
        return pointers;
      }
      assert(false);
  }
}

std::string HeapObject::ToString(Type type) {
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

}  // namespace es

#endif  // ES_IMPL_HEAP_OBJECT_IMPL_H
