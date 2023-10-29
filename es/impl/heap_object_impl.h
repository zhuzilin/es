#ifndef ES_IMPL_HEAP_OBJECT_IMPL_H
#define ES_IMPL_HEAP_OBJECT_IMPL_H

#include <es/types.h>

namespace es {

template<typename T>
std::string Handle<T>::ToString() {
  if (IsNullptr())
    return "nullptr";
  return JSValue::ToString(val());
}

std::vector<HeapObject**> HeapObject::Pointers(HeapObject* heap_obj) {
  switch (reinterpret_cast<JSValue*>(heap_obj)->type()) {
    case JS_UNINIT:
    case JS_UNDEFINED:
    case JS_NULL:
    case JS_BOOL:
    case JS_LONG_STRING:
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
      return pointers;
    }
    case PROPERTY_MAP: {
      return {
        HEAP_PTR(heap_obj, PropertyMap::kFixedArrayOffset),
        HEAP_PTR(heap_obj, PropertyMap::kHashMapOffset),
      };
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
      if (reinterpret_cast<JSValue*>(heap_obj)->IsObject()) {
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
    case JS_LONG_STRING:
      return "JS_LONG_STRING";
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

    case NON_JSVALUE:
      return "NON_JSVALUE";

    case ERROR:
      return "ERROR";
    case FIXED_ARRAY:
      return "FIXED_ARRAY";
    case HASHMAP:
      return "HASHMAP";
    case PROPERTY_MAP:
      return "PROPERTY_MAP";
    case BINDING:
      return "BINDING";
    case LIST_NODE:
      return "LIST_NODE";
  }
}

}  // namespace es

#endif  // ES_IMPL_HEAP_OBJECT_IMPL_H
