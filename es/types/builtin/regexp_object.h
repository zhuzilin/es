#ifndef ES_TYPES_BUILTIN_REGEX_OBJECT
#define ES_TYPES_BUILTIN_REGEX_OBJECT

#include <es/types/object.h>
#include <es/runtime.h>

namespace es {

String* ToString(Error* e, JSValue* input);

class RegExpProto : public JSObject {
 public:
  static RegExpProto* Instance() {
    static RegExpProto* singleton = RegExpProto::New();
    return singleton;
  }

  static JSValue* exec(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* test(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    assert(false);
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals);

 private:
  static RegExpProto* New() {
    JSObject* jsobj = JSObject::New(
      OBJ_REGEXP, u"RegExp", true, nullptr, false, false, nullptr, 0);
    return new (jsobj) RegExpProto();
  }
};

class RegExpObject : public JSObject {
 public:
  static RegExpObject* New(String* pattern, String* flag) {
    JSObject* jsobj = JSObject::New(
      OBJ_REGEXP, u"RegExp", true, nullptr, false, false, nullptr,
      kRegExpObjectOffset - kJSObjectOffset
    );
    SET_VALUE(jsobj, kPatternOffset, pattern, String*);
    SET_VALUE(jsobj, kFlagOffset, flag, String*);
    bool global = false, ignore_case = false, multiline = false;
    for (auto c : flag->data()) {
      switch (c) {
        case u'g':
          global = true;
          break;
        case u'i':
          ignore_case = true;
          break;
        case u'm':
          multiline = true;
          break;
      }
    }
    SET_VALUE(jsobj, kGlobalOffset, global, bool);
    SET_VALUE(jsobj, kIgnoreCaseOffset, ignore_case, bool);
    SET_VALUE(jsobj, kMultilineOffset, multiline, bool);

    RegExpObject* obj = new (jsobj) RegExpObject();
    obj->SetPrototype(RegExpProto::Instance());
    obj->AddValueProperty(u"source", pattern, false, false, false);
    obj->AddValueProperty(u"global", Bool::Wrap(global), false, false, false);
    obj->AddValueProperty(u"ignoreCase", Bool::Wrap(ignore_case), false, false, false);
    obj->AddValueProperty(u"multiline", Bool::Wrap(multiline), false, false, false);
    // TODO(zhuzilin) Not sure if this should be initialized to 0.
    obj->AddValueProperty(u"lastIndex", Number::Zero(), false, false, false);
    return obj;
  }

  std::vector<void*> Pointers() override {
    std::vector<void*> pointers = JSObject::Pointers();
    pointers.emplace_back(HEAP_PTR(kPatternOffset));
    pointers.emplace_back(HEAP_PTR(kFlagOffset));
    return pointers;
  }

  static constexpr size_t kPatternOffset = kJSObjectOffset;
  static constexpr size_t kFlagOffset = kPatternOffset + kPtrSize;
  static constexpr size_t kGlobalOffset = kFlagOffset + kPtrSize;
  static constexpr size_t kIgnoreCaseOffset = kGlobalOffset + kBoolSize;
  static constexpr size_t kMultilineOffset = kIgnoreCaseOffset + kBoolSize;
  static constexpr size_t kRegExpObjectOffset = kMultilineOffset + kBoolSize;

  String* pattern() { return READ_VALUE(this, kPatternOffset, String*); }
  String* flag() { return READ_VALUE(this, kFlagOffset, String*); }
  bool global() { return READ_VALUE(this, kGlobalOffset, bool); }
  bool ignore_case() { return READ_VALUE(this, kIgnoreCaseOffset, bool); }
  bool multiline() { return READ_VALUE(this, kMultilineOffset, bool); }

  std::string ToString() override { return "/" + log::ToString(pattern()) + "/" + log::ToString(flag()); }
};

class RegExpConstructor : public JSObject {
 public:
  static RegExpConstructor* Instance() {
    static RegExpConstructor* singleton = RegExpConstructor::New();
    return singleton;
  }

  JSValue* Call(Error* e, JSValue* this_arg, std::vector<JSValue*> arguments = {}) override {
    if (arguments.size() == 0) {
      *e = *Error::TypeError(u"RegExp called with 0 parameters");
      return nullptr;
    }
    if ((arguments.size() == 1 || arguments[1]->IsUndefined()) && arguments[0]->IsRegExpObject()) {
        return arguments[0];
    }
    return Construct(e, arguments);
  }

  JSObject* Construct(Error* e, std::vector<JSValue*> arguments) override {
    String* P, *F;
    if (arguments.size() == 0) {
      P = String::Empty();
    } else if (arguments[0]->IsRegExpObject()) {
      if (arguments.size() > 1 && !arguments[1]->IsUndefined()) {
        RegExpObject* R = static_cast<RegExpObject*>(arguments[0]);
        P = R->pattern();
        F = R->flag();
      } else {
        *e = *Error::TypeError(u"new RegExp called with RegExp object and flag.");
        return nullptr;
      }
    } else {
      P = ::es::ToString(e, arguments[0]);
      if (!e->IsOk()) return nullptr;
    }
    if (arguments.size() < 2 || arguments[1]->IsUndefined()) {
      F = String::Empty();
    } else {
      F = ::es::ToString(e, arguments[1]);
      if (!e->IsOk()) return nullptr;
    }
    // Check is flag is valid
    std::unordered_map<char16_t, size_t> count;
    bool valid_flag = true;
    for (auto c : F->data()) {
      if (c != u'g' && c != u'i' && c != u'm') {
        valid_flag = false;
        break;
      }
      if (count[c] > 0) {
        valid_flag = false;
        break;
      }
      count[c]++;
    }
    if (!valid_flag) {
      *e = *Error::SyntaxError(u"invalid RegExp flag: " + F->data());
      return nullptr;
    }
    return RegExpObject::New(P, F);
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    return String::New(u"function RegExp() { [native code] }");
  }

 private:
  static RegExpConstructor* New() {
    JSObject* jsobj = JSObject::New(
      OBJ_OTHER, u"RegExp", true, nullptr, true, true, nullptr, 0);
    return new (jsobj) RegExpConstructor();
  }
};

JSValue* RegExpProto::toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
  JSValue* val = Runtime::TopValue();
  if (!val->IsRegExpObject()) {
    *e = *Error::TypeError(u"RegExp.prototype.toString called by non-regex");
    return nullptr;
  }
  RegExpObject* regexp = static_cast<RegExpObject*>(val);
  return String::New(
    u"/" + regexp->pattern()->data() + u"/" +
    (regexp->global() ? u"g" : u"") +
    (regexp->ignore_case() ? u"i" : u"") +
    (regexp->multiline() ? u"m" : u"")
  );
}

}  // namespace es

#endif  // ES_TYPES_BUILTIN_REGEX_OBJECT