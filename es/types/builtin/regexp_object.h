#ifndef ES_TYPES_BUILTIN_REGEX_OBJECT
#define ES_TYPES_BUILTIN_REGEX_OBJECT

#include <es/types/object.h>
#include <es/runtime.h>

namespace es {

std::u16string ToString(Error* e, JSValue* input);

class RegExpProto : public JSObject {
 public:
  static RegExpProto* Instance() {
    static RegExpProto* singleton = new RegExpProto();
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
   RegExpProto() :
    JSObject(
      OBJ_REGEXP, u"RegExp", true, nullptr, false, false
    ) {}
};

class RegExpObject : public JSObject {
 public:
  RegExpObject(std::u16string pattern, std::u16string flag) :
    JSObject(OBJ_REGEXP, u"RegExp", true, nullptr, false, false), source_(pattern), flag_(flag) {
    SetPrototype(RegExpProto::Instance());
    for (auto c : flag) {
      switch (c) {
        case u'g':
          global_ = true;
          break;
        case u'i':
          ignore_case_ = true;
          break;
        case u'm':
          multiline_ = true;
          break;
      }
    }
    AddValueProperty(u"source", new String(source_), false, false, false);
    AddValueProperty(u"global", Bool::Wrap(global_), false, false, false);
    AddValueProperty(u"ignoreCase", Bool::Wrap(ignore_case_), false, false, false);
    AddValueProperty(u"multiline", Bool::Wrap(multiline_), false, false, false);
    // TODO(zhuzilin) Not sure if this should be initialized to 0.
    AddValueProperty(u"lastIndex", Number::Zero(), false, false, false);
  }

  std::u16string pattern() { return source_; }
  std::u16string flag() { return flag_; }
  bool global() { return global_; }
  bool ignore_case() { return ignore_case_; }
  bool multiline() { return multiline_; }

  std::string ToString() { return "/" + log::ToString(pattern()) + "/" + log::ToString(flag()); }

 private:
  std::u16string source_;
  std::u16string flag_;
  bool global_ = false;
  bool ignore_case_ = false;
  bool multiline_ = false;
};

class RegExpConstructor : public JSObject {
 public:
  static RegExpConstructor* Instance() {
    static RegExpConstructor* singleton = new RegExpConstructor();
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
    std::u16string P, F;
    if (arguments.size() == 0) {
      P = u"";
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
      F = u"";
    } else {
      F = ::es::ToString(e, arguments[1]);
      if (!e->IsOk()) return nullptr;
    }
    // Check is flag is valid
    std::unordered_map<char16_t, size_t> count;
    bool valid_flag = true;
    for (auto c : F) {
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
      *e = *Error::SyntaxError(u"invalid RegExp flag: " + F);
      return nullptr;
    }
    return new RegExpObject(P, F);
  }

  static JSValue* toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
    return new String(u"function RegExp() { [native code] }");
  }

 private:
   RegExpConstructor() :
    JSObject(OBJ_OTHER, u"RegExp", true, nullptr, true, true) {}
};

JSValue* RegExpProto::toString(Error* e, JSValue* this_arg, std::vector<JSValue*> vals) {
  JSValue* val = Runtime::TopValue();
  if (!val->IsRegExpObject()) {
    *e = *Error::TypeError(u"RegExp.prototype.toString called by non-regex");
    return nullptr;
  }
  RegExpObject* regexp = static_cast<RegExpObject*>(val);
  return new String(
    u"/" + regexp->pattern() + u"/" +
    (regexp->global() ? u"g" : u"") +
    (regexp->ignore_case() ? u"i" : u"") +
    (regexp->multiline() ? u"m" : u"")
  );
}

}  // namespace es

#endif  // ES_TYPES_BUILTIN_REGEX_OBJECT