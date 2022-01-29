#ifndef ES_UTILS_HELPER_H
#define ES_UTILS_HELPER_H

#include <string.h>

#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <codecvt>
#include <locale>

namespace es {
namespace log {

class Debugger {
 public:
  static Debugger* Instance() {
    static Debugger debug;
    return &debug;
  }

  static void Turn() { Debugger::Instance()->on_ = !Debugger::Instance()->on_; }
  static bool On() { return Debugger::Instance()->on_; }

 private:
#ifdef TEST
  bool on_ = true;
#else
  bool on_ = false;
#endif
};

void PrintSource(std::string comment, std::u16string str = u"", std::string postfix = "") {
  if (Debugger::On()) {
    std::cout << comment;
    for (const auto& c: str)
      std::cout << static_cast<char>(c);
    std::cout << postfix << std::endl;
  }
}

std::string ToString(std::u16string str) {
  static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
  return convert.to_bytes(str);
}

std::string ToString(bool b) {
  return b ? "true" : "false";
}

std::string ToString(const void *ptr) {
  std::stringstream ss;
  ss << ptr;  
  return ss.str();
}

}  // namespace test

std::u16string StrCat(std::vector<std::u16string> vals) {
  size_t size = 0;
  for (auto val : vals) {
    size += val.size();
  }
  std::u16string res(size, 0);
  size_t offset = 0;
  for (auto val : vals) {
    memcpy((void*)(res.c_str() + offset), (void*)(val.data()), val.size() * 2);
    offset += val.size();
  }
  return res;
}

bool HaveDuplicate(std::vector<std::u16string> vals) {
  for (size_t i = 0; i < vals.size(); i++) {
    for (size_t j = 0; j < vals.size(); j++) {
      if (i != j && vals[i] == vals[j])
        return true;
    }
  }
  return false;
}

}  // namespace es



#endif  // ES_UTILS_HELPER_H