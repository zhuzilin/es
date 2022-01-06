#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include <gtest/gtest.h>

#include <es/parser.h>
#include <test/helper.h>

TEST(TestParser, PrimaryExpressionLiteral) {
  // This
  {
    std::vector<std::u16string> sources = {
      u"\n \t this",
    };
    for (auto source : sources) {
      es::Parser parser(source);
      es::AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(es::AST::AST_EXP_THIS, ast->type());
      EXPECT_EQ(u"this", ast->source());
    }
  }

  // Identifier
  { 
    std::vector<std::u16string> sources = {
      u"你好", u"_abcDEF$", u"NULL", u"Null", u"True", u"False",
    };
    for (auto source : sources) {
      es::Parser parser(source);
      es::AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(es::AST::AST_EXP_IDENT, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // Null
  {
    std::vector<std::u16string> sources = {
      u"null",
    };
    for (auto source : sources) {
      es::Parser parser(source);
      es::AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(es::AST::AST_EXP_NULL, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // Bool
  {
    std::vector<std::u16string> sources = {
      u"true", u"false",
    };
    for (auto source : sources) {
      es::Parser parser(source);
      es::AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(es::AST::AST_EXP_BOOL, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // Number
  {
    std::vector<std::u16string> sources = {
      u"0", u"101", u"0.01", u"12.05", u".8" ,u"0xAbC09",
    };
    for (auto source : sources) {
      es::Parser parser(source);
      es::AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(es::AST::AST_EXP_NUMBER, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // String
  {
    std::vector<std::u16string> sources = {
      u"''", u"'\\n\\b\\u1234\\x12'", u"'😊'",
    };
    for (auto source : sources) {
      es::Parser parser(source);
      es::AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(es::AST::AST_EXP_STRING, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // Regex
  {
    std::vector<std::u16string> sources = {
      u"/a/", u"/[a-z]*?/", u"/[012]/g", u"/[012]/$", u"/你好/",
    };
    for (auto source : sources) {
      es::Parser parser(source);
      es::AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(es::AST::AST_EXP_REGEX, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // Illegal Literal
  {
    std::vector<std::u16string> sources = {
      {u"for", u"for"}, {u"😊", u"\xD83D"},
    };
    for (auto source : sources) {
      es::Parser parser(source);
      es::AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(es::AST::AST_ILLEGAL, ast->type());
    }
  }
}

TEST(TestParser, PrimaryExpressionArray) {
  {
    std::vector<std::pair<std::u16string, size_t>> sources = {
      {u"[]", 0}, {u"[,]", 1}, {u"[abc, 123,'string', ]", 3}, {u"[1+2*3, ++a]", 2}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      size_t length = pair.second;
      es::Parser parser(source);
      es::AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(es::AST::AST_EXP_ARRAY, ast->type());
      EXPECT_EQ(source, ast->source());
      auto array = static_cast<es::ArrayLiteral*>(ast);
      EXPECT_EQ(length, array->length());
    }
  }

  // Illegal
  {
    // TODO(zhuzilin) This error messsage is not intuitive.
    std::vector<std::pair<std::u16string,std::u16string>> sources = {
      {u"[a,", u""}, {u"[", u""},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      es::Parser parser(source);
      es::AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(es::AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(error, ast->source());
    }
  }
}

TEST(TestParser, PrimaryExpressionObject) {
  {
    std::vector<std::u16string> sources = {
      u"{}", u"{a: 1}", u"{in: bed}", u"{1: 1}", u"{\"abc\": 1}"
    };
    for (auto source : sources) {
      es::Parser parser(source);
      es::AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(es::AST::AST_EXP_OBJ, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // Illegal
  {
    std::vector<std::pair<std::u16string,std::u16string>> sources = {
      {u"{a,}", u"{a,"}, {u"{a 1}", u"{a 1"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      es::Parser parser(source);
      es::AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(es::AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(error, ast->source());
    }
  }
}

TEST(TestParser, PrimaryExpressionParentheses) {
  {
    std::vector<std::pair<std::u16string,std::u16string>> sources = {
      {u"(a)", u"a"}, {u"(a + b)", u"a + b"}, {u"(a + b, a++)", u"a + b, a++"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      es::Parser parser(source);
      es::test::PrintSource("source:", source);
      es::AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(es::AST::AST_EXP, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }

  // Illegal
  {
    // TODO(zhuzilin) This error message is not intuitive.
    std::vector<std::pair<std::u16string,std::u16string>> sources = {
      {u"()", u")"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      es::Parser parser(source);
      es::test::PrintSource("source:", source);
      es::AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(es::AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(error, ast->source());
    }
  }
}

TEST(TestParser, Binary) {
  {
    std::vector<
      std::pair<std::u16string,
      std::pair<std::u16string, std::u16string>>> sources = {
      {u"a + b * c", {u"a", u" b * c"}},
      {u"a * b + c", {u"a * b", u" c"}},
      {u"a * b + + c - d", {u"a * b + + c", u" d"}},
      {u"a++ == b && ++c != d", {u"a++ == b", u" ++c != d"}},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto lhs = pair.second.first;
      auto rhs = pair.second.second;
      es::Parser parser(source);
      es::AST* ast = parser.ParseBinaryAndUnaryExpression(false, 0);
      EXPECT_EQ(es::AST::AST_EXP_BINARY, ast->type());
      EXPECT_EQ(source, ast->source());
      auto binary = static_cast<es::Binary*>(ast);
      EXPECT_EQ(lhs, binary->lhs()->source());
      EXPECT_EQ(rhs, binary->rhs()->source());
    }
  }
}

TEST(TestParser, Unary) {
  {
    std::vector<std::pair<std::u16string, std::u16string>> sources = {
      {u"a ++", u"a"}, {u"++\na", u"\na"}, {u"++ a", u" a"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto node = pair.second;
      es::Parser parser(source);
      es::AST* ast = parser.ParseBinaryAndUnaryExpression(false, 0);
      EXPECT_EQ(es::AST::AST_EXP_UNARY, ast->type());
      EXPECT_EQ(source, ast->source());
      auto unary = static_cast<es::Unary*>(ast);
      EXPECT_EQ(node, unary->node()->source());
    }
  }

  // invalid
  {
    std::vector<std::pair<std::u16string, std::u16string>> sources = {
      {u"a\n++", u"a"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      es::Parser parser(source);
      es::AST* ast = parser.ParseBinaryAndUnaryExpression(false, 0);
      EXPECT_NE(es::AST::AST_EXP_UNARY, ast->type());
      EXPECT_EQ(error, ast->source());
    }
  }
}

TEST(TestParser, TripleCondition) {
  {
    std::vector<std::vector<std::u16string>> sources = {
      {u"a ?b:c", u"a", u"b", u"c"},
      {u"a ?c ? d : e : c", u"a", u"c ? d : e", u" c"},
    };
    for (auto vec : sources) {
      auto source = vec[0];
      es::Parser parser(vec[0]);
      es::AST* ast = parser.ParseConditionalExpression(false);
      EXPECT_EQ(es::AST::AST_EXP_TRIPLE, ast->type());
      EXPECT_EQ(source, ast->source());
      auto cond = static_cast<es::TripleCondition*>(ast);
      EXPECT_EQ(vec[1], cond->cond()->source());
      EXPECT_EQ(vec[2], cond->lhs()->source());
      EXPECT_EQ(vec[3], cond->rhs()->source());
    }
  }

  // invalid
  {
    std::vector<std::pair<std::u16string, std::u16string>> sources = {
      {u"a ?b c", u"a ?b"}, {u"a ", u"a"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      es::Parser parser(source);
      es::AST* ast = parser.ParseConditionalExpression(false);
      EXPECT_NE(es::AST::AST_EXP_TRIPLE, ast->type());
      EXPECT_EQ(error, ast->source());
    }
  }
}

TEST(TestParser, FunctionExpression) {
  // TODO(zhuzilin) Check FunctionBody
  {
    std::vector<std::pair<std::u16string,
                          std::vector<std::u16string>>> sources = {
      {u"function () {}", {u""}},
      {u"function name (a, b) {}", {u"name", u"a", u"b"}},
      {u"function (a, a, c) {}", {u"", u"a", u"a", u"c"}},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto params = pair.second;
      es::Parser parser(source);
      es::AST* ast = parser.ParseFunctionExpression();
      EXPECT_EQ(es::AST::AST_EXP_FUNC, ast->type());
      EXPECT_EQ(source, ast->source());
      auto func = static_cast<es::Function*>(ast);
      EXPECT_EQ(params[0], func->name().source());
      EXPECT_EQ(params.size() - 1, func->params().size());
      for (size_t i = 0; i < func->params().size(); i++) {
        EXPECT_EQ(params[i + 1], func->params()[i].source());
      }
    }
  }

  // invalid
  {
    std::vector<std::pair<std::u16string, std::u16string>> sources = {
      {u"function (,) {}", u"function (,"}, {u"function (a a) {}", u"function (a a"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      es::Parser parser(source);
      es::AST* ast = parser.ParseFunctionExpression();
      EXPECT_NE(es::AST::AST_EXP_FUNC, ast->type());
      EXPECT_EQ(error, ast->source());
    }
  }
}

TEST(TestParser, Arguments) {
  {
    std::vector<std::pair<std::u16string,
                          std::vector<std::u16string>>> sources = {
      {u"()", {}},
      {u"(a)", {u"a"}},
      {u"(a, 1+3, function(){})", {u"a", u" 1+3", u" function(){}"}},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto args = pair.second;
      es::Parser parser(source);
      es::AST* ast = parser.ParseArguments();
      EXPECT_EQ(es::AST::AST_EXP_ARGS, ast->type());
      EXPECT_EQ(source, ast->source());
      auto func = static_cast<es::Arguments*>(ast);
      for (size_t i = 0; i < func->args().size(); i++) {
        EXPECT_EQ(args[i], func->args()[i]->source());
      }
    }
  }
}

TEST(TestParser, LeftHandSide) {
  {
    std::vector<std::u16string> sources = {
      u"new Object()", u"function(a, b, c){}(c, d)",
      u"new new a[123 + xyz].__ABC['您好']()()"
    };
    for (auto source : sources) {
      es::Parser parser(source);
      es::test::PrintSource("source:", source);
      es::AST* ast = parser.ParseLeftHandSideExpression();
      es::test::PrintSource("ast:", ast->source());
      EXPECT_EQ(es::AST::AST_EXP_LHS, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }
}
