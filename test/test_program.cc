#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include <gtest/gtest.h>

#include <es/parser/parser.h>
#include <es/enter_code.h>
#include <es/eval.h>
#include <es/helper.h>

using namespace es;

typedef std::u16string_view string;
typedef std::vector<string> vec_string;
typedef std::pair<string,string> pair_string;
typedef std::vector<std::pair<string,string>> vec_pair_string;

TEST(TestProgram, SimpleAssign) {
  {
    Error* e = nullptr;
    Parser parser(u"a = 1;a");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(JSValue::JS_REF, res.value->type());
    Number* num = static_cast<Number*>(GetValue(e, static_cast<Reference*>(res.value)));
    EXPECT_EQ(1, num->data());
    EXPECT_EQ(nullptr, e);
  }
}

TEST(TestProgram, SimpleAssign1) {
  {
    Error* e = nullptr;
    Parser parser(u"a = 1;a=2;a");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(e, ast);
    EXPECT_EQ(JSValue::JS_REF, res.value->type());
    Reference* ref = static_cast<Reference*>(res.value);
    Number* num = static_cast<Number*>(GetValue(e, static_cast<Reference*>(res.value)));
    EXPECT_EQ(2, num->data());
    EXPECT_EQ(nullptr, e);
  }
}
