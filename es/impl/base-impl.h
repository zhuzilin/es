#ifndef ES_IMPL_BASE_IMPL_H
#define ES_IMPL_BASE_IMPL_H

#include <es/types/base.h>
#include <es/error.h>
#include <es/utils/macros.h>

namespace es {

void CheckObjectCoercible(Handle<Error>& e, Handle<JSValue> val) {
  if (val.val()->IsUndefined() || val.val()->IsNull()) {
    e = Error::TypeError(u"undefined or null is not coercible");
  }
}

bool JSValue::IsCallable() {
  return (IsObject() && h_.is_callable) || IsGetterSetter();
}

bool JSValue::IsConstructor() {
  return IsObject() && h_.is_constructor;
}

std::string JSValue::ToString(JSValue* jsval) {
  switch (jsval->type()) {
    case JS_UNINIT:
      return "Unitialized HeapObject";
    case JS_UNDEFINED:
      return "Undefined";
    case JS_NULL:
      return "Null";
    case JS_BOOL:
      return static_cast<Bool*>(jsval)->data() ? "true" : "false";
    case JS_LONG_STRING:
    case JS_STRING:
      return log::ToString(static_cast<String*>(jsval)->data());
    case JS_NUMBER:
      return NumberToStdString(static_cast<Number*>(jsval)->data());
    case JS_REF: {
      return "ref(" + std::to_string(static_cast<Reference*>(jsval)->id()) + ")";
    }
    case JS_GET_SET: {
      String* name = READ_VALUE(jsval, GetterSetter::kReferenceNameOffset, String*);
      return "GetterSetter(" + ToString(name) + ")";
    }
    case JS_PROP_DESC: {
      PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(jsval);
      std::string res = "PropertyDescriptor{";
      if (desc->HasValue()) res += "v: " + desc->Value().ToString() + ", ";
      if (desc->HasWritable()) res += "w: " + log::ToString(desc->Writable()) + ", ";
      if (desc->HasGet()) res += "get: " + desc->Get().ToString() + ", ";
      if (desc->HasSet()) res += "set: " + desc->Set().ToString() + ", ";
      if (desc->HasEnumerable()) res += "e: " + log::ToString(desc->Enumerable()) + ", ";
      if (desc->HasConfigurable()) res += "c: " + log::ToString(desc->Configurable());
      res += '}';
      return res;
    }
    case JS_ENV_REC_DECL:
      return "DeclarativeEnvRec(" + log::ToString(jsval) + "," +
              ToString(static_cast<DeclarativeEnvironmentRecord*>(jsval)->bindings()) + "," +
              static_cast<EnvironmentRecord*>(jsval)->outer().ToString() + ")";
    case JS_ENV_REC_OBJ:
      return "ObjectEnvRec(" + log::ToString(jsval) + "," +
             static_cast<ObjectEnvironmentRecord*>(jsval)->bindings().ToString() + "," +
             static_cast<EnvironmentRecord*>(jsval)->outer().ToString() + ")";
    case ERROR:
      return static_cast<Error*>(jsval)->IsOk() ?
        "ok" :
        ToString(READ_VALUE(jsval, Error::kValueOffset, JSValue*));
    case FIXED_ARRAY:
      return "FixedArray(" + std::to_string(READ_VALUE(jsval, FixedArray::kSizeOffset, size_t)) + ")";
    case HASHMAP_V2:
      return "HashMap(" + std::to_string(READ_VALUE(jsval, HashMapV2::kOccupancyOffset, size_t)) + ")";
    case HASHMAP:
      return "HashMap(" + std::to_string(READ_VALUE(jsval, HashMap::kSizeOffset, size_t)) + ")";
    case PROPERTY_MAP: {
      PropertyMap* map = static_cast<PropertyMap*>(jsval);
      return "PropertyMap(" + std::to_string(map->num_fixed_slots()) + "," + map->hashmap().ToString() + ")";
    }
    case LIST_NODE:
      return "ListNode(" + ToString(READ_VALUE(jsval, ListNode::kKeyOffset, String*)) + ")";
    case OBJ_ARRAY: {
      size_t num = ToNumber(
        Error::Empty(),
        Get(Error::Empty(), Handle<JSObject>(static_cast<JSObject*>(jsval)), String::Length())
      );
      return "Array(" + std::to_string(num) + ")";
    }
    case OBJ_FUNC: {
      FunctionObject* func = static_cast<FunctionObject*>(jsval);
      std::string result = "Function(";
      std::vector<Handle<String>> params = func->FormalParameters();
      if (params.size() > 0) {
        result += params[0].ToString();
        for (size_t i = 1; i < params.size(); i++) {
          result += "," + params[i].ToString();
        }
      }
      result += ")";
      return result;
    }
    case OBJ_BIND_FUNC: {
      return "BindFunctionObject";
    }
    case OBJ_REGEXP: {
      return "/" + ToString(READ_VALUE(jsval, RegExpObject::kPatternOffset, String*)) +
             "/" + ToString(READ_VALUE(jsval, RegExpObject::kFlagOffset, String*));
    }
    case OBJ_ERROR: {
      return ToString(READ_VALUE(jsval, ErrorObject::kErrorOffset, Error*));
    }
    default:
      if (jsval->IsObject()) {
        return log::ToString(ClassToString(jsval->h_.klass));
      }
      std::cout << "\033[1;31m" << HeapObject::ToString(jsval->type()) << "\033[0m" << std::endl;
      assert(false);
  }
}

template<flag_t flag>
Handle<JSValue> String::Eval(const std::u16string& source) {
#ifdef PARSER_TEST
  std::cout << "String::Eval [" << log::ToString(source) << "]" << std::endl;
#endif
  size_t pos = 1;
  std::vector<std::u16string> vals;
  while (pos < source.size() - 1) {
    char16_t c = source[pos];
    switch (c) {
      case u'\\': {
        pos++;
        c = source[pos];
        switch (c) {
          case u'b':
            pos++;
            vals.emplace_back(u"\b");
            break;
          case u't':
            pos++;
            vals.emplace_back(u"\t");
            break;
          case u'n':
            pos++;
            vals.emplace_back(u"\n");
            break;
          case u'v':
            pos++;
            vals.emplace_back(u"\v");
            break;
          case u'f':
            pos++;
            vals.emplace_back(u"\f");
            break;
          case u'r':
            pos++;
            vals.emplace_back(u"\r");
            break;
          case u'0': {
            pos++;
            if (pos < source.size() && character::IsDecimalDigit(source[pos])) {
              return Error::SyntaxErrorConst(u"decimal digit after \\0");
            }
            vals.emplace_back(std::u16string(1, 0));
            break;
          }
          case u'x': {
            pos++;  // skip 'x'
            char16_t hex = 0;
            for (size_t i = 0; i < 2; i++) {
              hex *= 16;
              hex += character::Digit(source[pos]);
              pos++;
            }
            vals.emplace_back(std::u16string(1, hex));
            break;
          }
          case u'u': {
            pos++;  // skip 'u'
            char16_t hex = 0;
            for (size_t i = 0; i < 4; i++) {
              hex *= 16;
              hex += character::Digit(source[pos]);
              pos++;
            }
            vals.emplace_back(std::u16string(1, hex));
            break;
          }
          default:
            c = source[pos];
            if (character::IsLineTerminator(c)) {
              pos++;
              continue;
            }
            pos++;
            vals.emplace_back(std::u16string(1, c));
        }
        break;
      }
      default: {
        size_t start = pos;
        while (true) {
          if (pos == source.size() - 1 || source[pos] == u'\\')
            break;
          pos++;
        }
        size_t end = pos;
        if (end == source.size() - 1 && vals.size() == 0)
          return String::New<flag>(source.substr(start, end - start));
        vals.emplace_back(source.substr(start, end - start));
      }
    }
  }
  if (vals.size() == 0) {
    return String::Empty();
  } else if (vals.size() == 1) {
    return String::New<flag>(vals[0]);
  }
  return String::New<flag>(StrCat(vals));
}

// This verson of string to number assumes the string is valid.
template<flag_t flag>
Handle<Number> Number::Eval(const std::u16string& source) {
#ifdef PARSER_TEST
  std::cout << "Number::Eval [" << log::ToString(source) << "]" << std::endl;
#endif
  double val = 0;
  double frac = 1;
  size_t pos = 0;
  bool dot = false;
  while (pos < source.size()) {
    char16_t c = source[pos];
    switch (c) {
      case u'.':
        dot = true;
        break;
      case u'e':
      case u'E': {
        double exp = 0;
        bool sign = true;
        pos++;  // skip e/E
        c = source[pos];
        if (c == u'-') {
          sign = false;
          pos++;  // skip -
        } else if (c == u'+') {
          sign = true;
          pos++; // skip +;
        }
        while (pos < source.size()) {
          c = source[pos];
          exp *= 10;
          exp += character::Digit(c);
          pos++;
        }
        if (!sign)
          exp = -exp;
        return Number::New<flag>(val * pow(10.0, exp));
      }
      case u'x':
      case u'X': {
        ASSERT(val == 0);
        pos++;
        while (pos < source.size()) {
          c = source[pos];
          val *= 16;
          val += character::Digit(c);
          pos++;
        }
        return Number::New<flag>(val);
      }
      default:
        if (dot) {
          frac /= 10;
          val += character::Digit(c) * frac;
        } else {
          val *= 10;
          val += character::Digit(c);
        }
    }
    pos++;
  }
  return Number::New<flag>(val);
}

}  // namespace es

#endif  // ES_IMPL_BASE_IMPL_H