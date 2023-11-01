#ifndef ES_UTILS_HELPER_H
#define ES_UTILS_HELPER_H

#include <math.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <codecvt>
#include <locale>

#include <es/utils/macros.h>

namespace es {
namespace log {

class Debugger {
 public:
  static Debugger* Instance() {
    static Debugger debug;
    return &debug;
  }

  static void Turn() { Debugger::Instance()->on_ = !Debugger::Instance()->on_; }
  static void TurnOff() { Debugger::Instance()->on_ = false; }
  static bool On() { return Debugger::Instance()->on_; }

 private:
#ifdef TEST
  bool on_ = true;
#else
  bool on_ = false;
#endif
};

// For exact debugging
class Tracker {
 public:
  static Tracker* Instance() {
    static Tracker tracker;
    return &tracker;
  }

  static void TurnOn() { Tracker::Instance()->on_ = true; }
  static bool On() { return Tracker::Instance()->on_; }

 private:
  bool on_ = false;
};

std::string ToString(std::u16string str) {
  static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
  return convert.to_bytes(str);
}

void PrintSource(std::string comment, std::u16string str = u"", std::string postfix = "", size_t limit = 100) {
  std::cout << comment;
  std::string s = ToString(str);
  if (s.size() > limit) {
    std::istringstream inputs(s);
    s = "";
    for (std::string line; std::getline(inputs, line, '\n');) {
      if (s.size() >= limit ||
          (s.size() > 0 && s.size() + line.size() + s.size() > 2 * limit))
        break;
      if (s.size() > 0)
        s += '\n';
      s += line;
    }
  }
  for (const auto& c: s)
    std::cout << static_cast<char>(c);
  std::cout << postfix << "\n";
}

std::string ToString(bool b) {
  return b ? "true" : "false";
}

std::string ToString(const void *ptr) {
  std::stringstream ss;
  ss << ptr;  
  return ss.str();
}

}  // namespace log

std::u16string StrCat(std::vector<std::u16string> vals) {
  size_t size = 0;
  for (auto val : vals) {
    size += val.size();
  }
  std::u16string res(size, 0);
  size_t offset = 0;
  for (auto val : vals) {
    memcpy((void*)(res.c_str() + offset), (void*)(val.data()), val.size() * kChar16Size);
    offset += val.size();
  }
  return res;
}

std::string StrCat(std::vector<std::string> vals) {
  size_t size = 0;
  for (auto val : vals) {
    size += val.size();
  }
  std::string res(size, 0);
  size_t offset = 0;
  for (auto val : vals) {
    memcpy((void*)(res.c_str() + offset), (void*)(val.data()), val.size() * kCharSize);
    offset += val.size();
  }
  return res;
}

// From Knuth https://stackoverflow.com/a/253874/5163915
static constexpr double kEpsilon = 1e-15;
bool ApproximatelyEqual(double a, double b) {
  return fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * kEpsilon);
}

bool EssentiallyEqual(double a, double b) {
  return fabs(a - b) <= ((fabs(a) > fabs(b) ? fabs(b) : fabs(a)) * kEpsilon);
}

bool DefinitelyGreaterThan(double a, double b) {
  return (a - b) > ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * kEpsilon);
}

bool DefinitelyLessThan(double a, double b) {
  return (b - a) > ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * kEpsilon);
}

union Double2Uint64 {
  double double_;
  uint64_t uint64_;
};

}  // namespace es



#endif  // ES_UTILS_HELPER_H