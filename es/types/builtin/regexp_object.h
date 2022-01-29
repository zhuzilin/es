#ifndef ES_TYPES_BUILTIN_REGEX_OBJECT
#define ES_TYPES_BUILTIN_REGEX_OBJECT

#include <es/types/object.h>
#include <es/runtime.h>

namespace es {

Handle<String> ToString(Error* e, Handle<JSValue> input);

class RegExpProto : public JSObject {
 public:
  static Handle<RegExpProto> Instance() {
    static Handle<RegExpProto> singleton = RegExpProto::New(GCFlag::SINGLE);
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
  static Handle<RegExpProto> New(flag_t flag) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_REGEXP, u"RegExp", true, Handle<JSValue>(), false, false, nullptr, 0, flag);
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
    AddValueProperty(obj, u"source", pattern, false, false, false);
    AddValueProperty(obj, u"global", Bool::Wrap(global), false, false, false);
    AddValueProperty(obj, u"ignoreCase", Bool::Wrap(ignore_case), false, false, false);
    AddValueProperty(obj, u"multiline", Bool::Wrap(multiline), false, false, false);
    // TODO(zhuzilin) Not sure if this should be initialized to 0.
    AddValueProperty(obj, u"lastIndex", Number::Zero(), false, false, false);
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
    static Handle<RegExpConstructor> singleton = RegExpConstructor::New(GCFlag::SINGLE);
    return singleton;
  }

  static Handle<JSValue> toString(Error* e, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> vals) {
    return String::New(u"function RegExp() { [native code] }");
  }

 private:
  static Handle<RegExpConstructor> New(flag_t flag) {
    Handle<JSObject> jsobj = JSObject::New(
      OBJ_REGEXP_CONSTRUCTOR, u"RegExp", true, Handle<JSValue>(), true, true, nullptr, 0, flag);
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

Handle<JSValue> Call__RegExpConstructor(Error* e, Handle<RegExpConstructor> O, Handle<JSValue> this_arg, std::vector<Handle<JSValue>> arguments = {});
Handle<JSObject> Construct__RegExpConstructor(Error* e, Handle<RegExpConstructor> O, std::vector<Handle<JSValue>> arguments);

}  // namespace es

#endif  // ES_TYPES_BUILTIN_REGEX_OBJECT