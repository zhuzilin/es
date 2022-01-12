#ifndef ES_TEST_HELPER_H
#define ES_TEST_HELPER_H

#include <string.h>

#include <iostream>
#include <string>
#include <string_view>
#include <codecvt>

namespace es {
namespace log {

void PrintSource(std::string comment, std::u16string str = u"", std::string postfix = "") {
  std::cout << comment;
  for (const auto& c: str)
    std::cout << static_cast<char>(c);
  std::cout << postfix << std::endl;
}

std::string ToString(std::u16string str) {
  std::string result(str.size(), ' ');
  for (size_t i = 0; i < str.size(); i++)
    result[i] = str[i];
  return result;
}

std::string ToString(bool b) {
  return b ? "true" : "false";
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



#endif  // ES_TEST_HELPER_H