#ifndef ES_TEST_HELPER_H
#define ES_TEST_HELPER_H

#include <string.h>

#include <iostream>
#include <string>
#include <string_view>
#include <codecvt>

namespace es {
namespace log {

void PrintSource(std::string comment, std::u16string_view str = u"") {
  std::cout << comment;
  for (const auto& c: str)
    std::cout << static_cast<char>(c);
  std::cout << std::endl;
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

}  // namespace es



#endif  // ES_TEST_HELPER_H