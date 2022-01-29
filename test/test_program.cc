#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include <gtest/gtest.h>

#include <es/parser/parser.h>
#include <es/enter_code.h>
#include <es/eval.h>
#include <es/types/property_descriptor_object_conversion.h>
#include <es/utils/helper.h>
#include <es/gc/heap.h>
#include <es/impl.h>
#include <es/eval.h>
#include <es/enter_code.h>

using namespace es;

typedef std::u16string string;
typedef std::vector<string> vec_string;
typedef std::pair<string,string> pair_string;
typedef std::vector<std::pair<string,string>> vec_pair_string;

TEST(TestProgram, SimpleAssign0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"a = 1;a");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(JSValue::JS_REF, res.value.val()->type());
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, static_cast<Handle<Reference>>(res.value)));
    EXPECT_EQ(1, num.val()->data());
    EXPECT_EQ(true, e->IsOk());
  }
}

TEST(TestProgram, SimpleAssign1) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"a = 1;a=2;a");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(JSValue::JS_REF, res.value.val()->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(2, num.val()->data());
    EXPECT_EQ(true, e->IsOk());
  }
}

TEST(TestProgram, CompoundAssign0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"a = 1; a+=1; a");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(JSValue::JS_REF, res.value.val()->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(2, num.val()->data());
    EXPECT_EQ(true, e->IsOk());
  }
}

TEST(TestProgram, Call0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"a = function(b){return b;}; a(3)");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(JSValue::JS_NUMBER, res.value.val()->type());
    Handle<Number> num = static_cast<Handle<Number>>(res.value);
    EXPECT_EQ(3, num.val()->data());
    EXPECT_EQ(true, e->IsOk());
  }
}

TEST(TestProgram, Call1) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"function a(b){return b;}; a(3)");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(JSValue::JS_NUMBER, res.value.val()->type());
    Handle<Number> num = static_cast<Handle<Number>>(res.value);
    EXPECT_EQ(3, num.val()->data());
    EXPECT_EQ(true, e->IsOk());
  }
}

TEST(TestProgram, Call2) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"a = 1; function b(){return a;}; b()");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(JSValue::JS_NUMBER, res.value.val()->type());
    Handle<Number> num = static_cast<Handle<Number>>(res.value);
    EXPECT_EQ(1, num.val()->data());
    EXPECT_EQ(true, e->IsOk());
  }
}

TEST(TestProgram, Call3) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"function c(){return function() { return 10};}; c()()");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(JSValue::JS_NUMBER, res.value.val()->type());
    Handle<Number> num = static_cast<Handle<Number>>(res.value);
    EXPECT_EQ(10, num.val()->data());
    EXPECT_EQ(true, e->IsOk());
  }
}

TEST(TestProgram, CallFunctionContructor) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"a = Function('return 5'); a()");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(true, e->IsOk());
    EXPECT_EQ(JSValue::JS_NUMBER, res.value.val()->type());
    Handle<Number> num = static_cast<Handle<Number>>(res.value);
    EXPECT_EQ(5, num.val()->data());
  }
}

TEST(TestProgram, Object0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"a = {a: 1}; a.a");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(true, e->IsOk());
    EXPECT_EQ(JSValue::JS_REF, res.value.val()->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(1, num.val()->data());
  }
}

TEST(TestProgram, Object1) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"a = {a: {0: 10}}; a.a[0]");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(true, e->IsOk());
    EXPECT_EQ(JSValue::JS_REF, res.value.val()->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(10, num.val()->data());
  }
}

TEST(TestProgram, Object2) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"a = {a: 136}; a.a = 5; a.a");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(true, e->IsOk());
    EXPECT_EQ(JSValue::JS_REF, res.value.val()->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(5, num.val()->data());
  }
}

TEST(TestProgram, Object3) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(u"a = {get b() {return this.c}, set b(x) {this.c = x}}; a.b = 5; a.b");
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(true, e->IsOk());
    EXPECT_EQ(JSValue::JS_REF, res.value.val()->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(5, num.val()->data());
  }
}

TEST(TestProgram, New0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"a = new new function() {\n"
      u"  this.a = 12345;\n"
      u"  return function () {this.b=23456}\n"
      u"}\n"
      u"a.b\n"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(true, e->IsOk());
    EXPECT_EQ(JSValue::JS_REF, res.value.val()->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(23456, num.val()->data());
  }
}

TEST(TestProgram, New1) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"a = new String('abc').toString()\n"
      u"a\n"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(true, e->IsOk());
    EXPECT_EQ(JSValue::JS_REF, res.value.val()->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<String> str = static_cast<Handle<String>>(GetValue(e, ref));
    EXPECT_EQ(u"abc", str.val()->data());
  }
}

TEST(TestProgram, If) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"a = 1\n"
      u"if (false)\n"
      u"  a = 4\n"
      u"else {a = 2}\n"
      u"a"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(true, e->IsOk());
    EXPECT_EQ(JSValue::JS_REF, res.value.val()->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(2, num.val()->data());
  }
}

TEST(TestProgram, Strict0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"a = 1"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(Completion::THROW, res.type);
    EXPECT_EQ(Error::E_REFERENCE, static_cast<Handle<ErrorObject>>(res.value).val()->ErrorType());
  }
}

TEST(TestProgram, Strict1) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"a = 235\n"
      u"var a; a"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(235, num.val()->data());
  }
}

TEST(TestProgram, Var0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"var a = 147; a"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(147, num.val()->data());
  }
}

TEST(TestProgram, While0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"var a = 1, n = 5\n"
      u"while (a < n) {\n"
      u" a *= 2\n"
      u"}\n"
      u"a"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(8, num.val()->data());
  }
}

TEST(TestProgram, While1) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"var a = 1, n = 5\n"
      u"while (a < n) {\n"
      u" if (a % 2 == 0) break\n"
      u" a *= 2\n"
      u"}\n"
      u"a"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(2, num.val()->data());
  }
}

TEST(TestProgram, While2) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"var a = 0, n = 4, sum = 0\n"
      u"while (a < n) {\n"
      u" a += 1\n"
      u" if (a == 2) continue\n"
      u" sum += a\n"
      u"}\n"
      u"sum"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(8, num.val()->data());
  }
}

TEST(TestProgram, DoWhile0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"var a = 10, n = 5\n"
      u"do {\n"
      u" a *= 2\n"
      u"} while (a < n)\n"
      u"a"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(20, num.val()->data());
  }
}

TEST(TestProgram, DoWhile1) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"var a = 1, n = 5\n"
      u"do {\n"
      u" if (a % 2 == 0) break\n"
      u" a *= 2\n"
      u"} while (a < n);\n"
      u"a"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(2, num.val()->data());
  }
}

TEST(TestProgram, DoWhile2) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"var a = 0, n = 4, sum = 0\n"
      u"do {"
      u" a += 1\n"
      u" if (a == 2) continue\n"
      u" sum += a\n"
      u"} while (a < n)\n"
      u"sum"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(8, num.val()->data());
  }
}

TEST(TestProgram, For0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"var sum = 0, i;"
      u"for (i = 0; i < 4; i++) {\n"
      u" sum += i }"
      u"sum"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(6, num.val()->data());
  }
}

TEST(TestProgram, For1) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"for (var i = 0, sum = 0; i < 4; i++) {\n"
      u" if (i > 2) break;"
      u" sum += i }"
      u"sum"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(3, num.val()->data());
  }
}

TEST(TestProgram, ForIn0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"var sum = 0, i;"
      u"var obj = {a: 1, b: 2, c: 3};"
      u"for (i in obj) {\n"
      u" sum += obj[i] }"
      u"sum"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(6, num.val()->data());
  }
}

TEST(TestProgram, ForIn1) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"var sum = 0;"
      u"var obj = {a: 1, b: 2, c: 3};"
      u"for (var i in obj) {\n"
      u" sum += obj[i] }"
      u"sum"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(6, num.val()->data());
  }
}

TEST(TestProgram, Try0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"try {\n"
      u"  var a = b;\n"
      u"} catch (e) {\n"
      u"  a = 10;"
      u"}\n"
      u"a"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Completion res = EvalProgram(ast);
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(10, num.val()->data());
  }
}

TEST(TestProgram, Try1) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"try {\n"
      u"  throw 2;\n"
      u"} catch (e) {\n"
      u"  var x = e;"
      u"} finally { x *= 2}\n"
      u"x"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Completion res = EvalProgram(ast);
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(4, num.val()->data());
  }
}

TEST(TestProgram, Switch0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"var a = 0;"
      u"switch (a) {\n"
      u"  case 0:"
      u"    a++; break;\n"
      u"  default: \n"
      u"   a++;"
      u"}\n"
      u"a"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Completion res = EvalProgram(ast);
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(1, num.val()->data());
  }
}

TEST(TestProgram, Switch1) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"var a = 0;"
      u"switch (a) {\n"
      u"  case 100:"
      u"    a++;\n"
      u"  default: \n"
      u"   a++;"
      u"  case 1: a++;"
      u"  case 0: a++;"
      u"}\n"
      u"a"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Completion res = EvalProgram(ast);
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(3, num.val()->data());
  }
}

TEST(TestProgram, Label0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"var i, c = 0;\n"
      u"L1: for(i = 0; i < 3; i++) {\n"
      u"  c++;\n"
      u"  while (1) {\n"
      u"    c++\n"
      u"    break L1;\n"
      u"  }\n"
      u"}\n"
      u"c"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Completion res = EvalProgram(ast);
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(2, num.val()->data());
  }
}

TEST(TestProgram, Fib0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"function fib(n) {\n"
      u"  if (n <= 0)\n"
      u"    return 0;\n"
      u"  else if (n == 1)\n"
      u"    return 1;\n"
      u"  else\n"
      u"    return fib(n - 1) + fib(n - 2);\n"
      u"}\n"
      u"fib(10)"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(55, num.val()->data());
  }
}

TEST(TestProgram, Fib1) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u"function fib(n) {\n"
      u"  if (n <= 0) return 0;\n"
      u"  var a = 0, b = 1;\n"
      u"  for (var i = 2; i <= n; i++) {\n"
      u"    var c = a + b\n"
      u"    a = b; b = c;\n"
      u"  }\n"
      u"  return b;\n"
      u"}\n"
      u"fib(10)"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(55, num.val()->data());
  }
}

TEST(TestProgram, Eval0) {
  Init();
  {
    Error* e = Error::Ok();
    Parser parser(
      u"'use strict';\n"
      u" var a = 4;\n"
      u"eval(a += 2);\n"
      u"a"
    );
    AST* ast = parser.ParseProgram();
    EnterGlobalCode(e, ast);
    Completion res = EvalProgram(ast);
    EXPECT_EQ(Error::E_OK, e->type());
    Handle<Reference> ref = static_cast<Handle<Reference>>(res.value);
    Handle<Number> num = static_cast<Handle<Number>>(GetValue(e, ref));
    EXPECT_EQ(6, num.val()->data());
  }
}
