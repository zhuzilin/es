#ifndef ES_TYPES_BUILTIN_REGEX_OBJECT
#define ES_TYPES_BUILTIN_REGEX_OBJECT

#include <es/types/object.h>
#include <es/runtime.h>

namespace es {

Handle<String> ToString(Error* e, Handle<JSValue> input);

class RegExpProto : public JSObject {
 public:
  static Handle<RegExpProto> Instance() {
    static Handle<RegExpProto> singleton = RegExpProto::New();
    return singleton;
  }

  static Handle<JSValue> exec(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> test(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    assert(false);
  }

  static Handle<JSValue> toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals);

 private:
  static Handle<RegExpProto> New() {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_REGEXP, u"RegExp", true, Handle<JSValue>(), false, false, nullptr, 0);
    return Handle<RegExpProto>(new (jsobj.val()) RegExpProto());
  }
};

class RegExpObject : public JSObject {
 public:
  static Handle<RegExpObject> New(Handle<String> pattern, Handle<String> flag) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_REGEXP, u"RegExp", true, Handle<JSValue>(), false, false, nullptr,
      kRegExpObjectOffset - kJSObjectOffset
    );
    SET_HANDLE_VALUE(jsobj.val(), kPatternOffset, pattern, String);
    SET_HANDLE_VALUE(jsobj.val(), kFlagOffset, flag, String);
    bool global = false, ignore_case = false, multiline = false;
    for (auto c : flag.val()->data()) {
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
    SET_VALUE(jsobj.val(), kGlobalOffset, global, bool);
    SET_VALUE(jsobj.val(), kIgnoreCaseOffset, ignore_case, bool);
    SET_VALUE(jsobj.val(), kMultilineOffset, multiline, bool);

    Handle<RegExpObject> obj = Handle<RegExpObject>(new (jsobj.val()) RegExpObject());
    obj.val()->SetPrototype(RegExpProto::Instance());
    obj.val()->AddValueProperty(u"source", pattern, false, false, false);
    obj.val()->AddValueProperty(u"global", Bool::Wrap(global), false, false, false);
    obj.val()->AddValueProperty(u"ignoreCase", Bool::Wrap(ignore_case), false, false, false);
    obj.val()->AddValueProperty(u"multiline", Bool::Wrap(multiline), false, false, false);
    // TODO(zhuzilin) Not sure if this should be initialized to 0.
    obj.val()->AddValueProperty(u"lastIndex", Number::Zero(), false, false, false);
    return obj;
  }

  std::vector<HeapObject**> Pointers() override {
    std::vector<HeapObject**> pointers = JSObject::Pointers();
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

  Handle<String> pattern() { return READ_HANDLE_VALUE(this, kPatternOffset, String); }
  Handle<String> flag() { return READ_HANDLE_VALUE(this, kFlagOffset, String); }
  bool global() { return READ_VALUE(this, kGlobalOffset, bool); }
  bool ignore_case() { return READ_VALUE(this, kIgnoreCaseOffset, bool); }
  bool multiline() { return READ_VALUE(this, kMultilineOffset, bool); }

  std::string ToString() override { return "/" + log::ToString(pattern().val()) + "/" + log::ToString(flag().val()); }
};

class RegExpConstructor : public JSObject {
 public:
  static Handle<RegExpConstructor> Instance() {
    static Handle<RegExpConstructor> singleton = RegExpConstructor::New();
    return singleton;
  }

  Handle<JSValue> Call(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {}) override {
    if (arguments.size() == 0) {
      *e = *Error::TypeError(u"RegExp called with 0 parameters");
      return Handle<JSValue>();
    }
    if ((arguments.size() == 1 || arguments[1].val()->IsUndefined()) && arguments[0].val()->IsRegExpObject()) {
        return arguments[0];
    }
    return Construct(e, arguments);
  }

  Handle<JSObject> Construct(Error* e, std::vector<Handle<JSValue>> arguments) override {
    Handle<String> P, F;
    if (arguments.size() == 0) {
      P = String::Empty();
    } else if (arguments[0].val()->IsRegExpObject()) {
      if (arguments.size() > 1 && !arguments[1].val()->IsUndefined()) {
        Handle<RegExpObject> R = static_cast<Handle<RegExpObject>>(arguments[0]);
        P = R.val()->pattern();
        F = R.val()->flag();
      } else {
        *e = *Error::TypeError(u"new RegExp called with RegExp object and flag.");
        return Handle<JSValue>();
      }
    } else {
      P = ::es::ToString(e, arguments[0]);
      if (!e->IsOk()) return Handle<JSValue>();
    }
    if (arguments.size() < 2 || arguments[1].val()->IsUndefined()) {
      F = String::Empty();
    } else {
      F = ::es::ToString(e, arguments[1]);
      if (!e->IsOk()) return Handle<JSValue>();
    }
    // Check is flag is valid
    std::unordered_map<char16_t, size_t> count;
    bool valid_flag = true;
    for (auto c : F.val()->data()) {
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
      *e = *Error::SyntaxError(u"invalid RegExp flag: " + F.val()->data());
      return Handle<JSValue>();
    }
    return RegExpObject::New(P, F);
  }

  static Handle<JSValue> toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"function RegExp() { [native code] }");
  }

 private:
  static Handle<RegExpConstructor> New() {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_OTHER, u"RegExp", true, Handle<JSValue>(), true, true, nullptr, 0);
    return Handle<RegExpConstructor>(new (jsobj.val()) RegExpConstructor());
  }
};

Handle<JSValue> RegExpProto::toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
  Handle<JSValue> val = Runtime::TopValue();
  if (!val.val()->IsRegExpObject()) {
    *e = *Error::TypeError(u"RegExp.prototype.toString called by non-regex");
    return Handle<JSValue>();
  }
  Handle<RegExpObject> regexp = static_cast<Handle<RegExpObject>>(val);
  return String::New(
    u"/" + regexp.val()->pattern().val()->data() + u"/" +
    (regexp.val()->global() ? u"g" : u"") +
    (regexp.val()->ignore_case() ? u"i" : u"") +
    (regexp.val()->multiline() ? u"m" : u"")
  );
}

}  // namespace es

#endif  // ES_TYPES_BUILTIN_REGEX_OBJECT