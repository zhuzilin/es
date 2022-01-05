#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include <gtest/gtest.h>

#include <es/parser.h>
#include <test/helper.h>

TEST(TestParser, PrimaryExpression) {
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

  // Array
  {
    std::vector<std::pair<std::u16string, int>> sources = {
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

  // Object
  {
    std::vector<std::u16string> sources = {
      u"{}", u"{a: 1}", u"{in: bed}", u"{1: 1}", u"{\"abc\": 1}"
    };
    for (auto source : sources) {
      es::test::PrintSource("", source);
      es::Parser parser(source);
      es::AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(es::AST::AST_EXP_OBJ, ast->type());
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

  // Illegal Array
  {
    std::vector<std::u16string> sources = {
      {u"[a,", u"[a,"}, {u"[", u"["},
    };
    for (auto source : sources) {
      es::Parser parser(source);
      es::AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(es::AST::AST_ILLEGAL, ast->type());
    }
  }

  // Illegal Object
  {
    std::vector<std::u16string> sources = {
      {u"{a,}", u"{a,"}, {u"{a 1}", u"{a 1"},
    };
    for (auto source : sources) {
      es::Parser parser(source);
      es::AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(es::AST::AST_ILLEGAL, ast->type());
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
      auto node = pair.second;
      es::Parser parser(source);
      es::AST* ast = parser.ParseBinaryAndUnaryExpression(false, 0);
      EXPECT_NE(es::AST::AST_EXP_UNARY, ast->type());
      EXPECT_EQ(node, ast->source());
    }
  }
}

