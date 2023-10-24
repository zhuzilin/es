#include <math.h>

#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include <gtest/gtest.h>

#include <es/parser/parser.h>
#include <es/types/conversion.h>
#include <es/types/property_descriptor_object_conversion.h>
#include <es/utils/helper.h>
#include <es/gc/heap.h>
#include <es/impl.h>
#include <es/eval.h>
#include <es/enter_code.h>

using namespace es;

TEST(TestSameValue, Number) {
  {
    std::vector<std::pair<double, double>> pairs = {
      {0, 0}, {nan(""), nan("")}, {1.23, 1.23}
    };
    for (auto pair : pairs) {
      EXPECT_EQ(true, SameValue(number::New(pair.first), number::New(pair.second)));
    }
  }
  
  {
    std::vector<std::pair<double, double>> pairs = {
      {0.0, -0.0},
    };
    for (auto pair : pairs) {
      EXPECT_EQ(false, SameValue(number::New(pair.first), number::New(pair.second)));
    }
  }
}
