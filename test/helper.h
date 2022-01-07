#ifndef ES_TEST_HELPER_H
#define ES_TEST_HELPER_H

#include <iostream>
#include <string>
#include <string_view>
#include <codecvt>

namespace es {
namespace test {

void PrintSource(std::string comment, std::u16string_view str = u"") {
  std::cout << comment;
  for (const auto& c: str)
    std::cout << static_cast<char>(c);
  std::cout << std::endl;
}

}  // namespace test
}  // namespace es



#endif  // ES_TEST_HELPER_H