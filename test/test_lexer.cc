#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include <gtest/gtest.h>

#include <es/lexer.h>
#include <test/helper.h>

TEST(TestLexer, Basic) {
  {
    std::u16string source(u"{}()[].;,?:");
    es::Lexer lexer(source);

    es::Token token = lexer.Next();
    EXPECT_EQ(es::Token::Type::TK_LBRACE, token.type());
    EXPECT_EQ(u"{", token.source());

    token = lexer.Next();
    EXPECT_EQ(es::Token::Type::TK_RBRACE, token.type());
    EXPECT_EQ(u"}", token.source());

    token = lexer.Next();
    EXPECT_EQ(es::Token::Type::TK_LPAREN, token.type());
    EXPECT_EQ(u"(", token.source());

    token = lexer.Next();
    EXPECT_EQ(es::Token::Type::TK_RPAREN, token.type());
    EXPECT_EQ(u")", token.source());
  }
}

TEST(TestLexer, String) {
  {
    std::u16string source(u"\"abc\"'def'");
    es::Lexer lexer(source);
    es::Token token = lexer.Next();
    EXPECT_EQ(es::Token::Type::TK_STRING, token.type());
    EXPECT_EQ(u"\"abc\"", token.source());

    token = lexer.Next();
    EXPECT_EQ(es::Token::Type::TK_STRING, token.type());
    EXPECT_EQ(u"'def'", token.source());
  }

  {
    std::vector<std::u16string> sources = {
      u"''", u"'\\n\\b\\u1234\\x12'", u"'😊'",
    };
    for (auto source : sources) {
      es::Lexer lexer(source);
      es::Token token = lexer.Next();
      EXPECT_EQ(es::Token::Type::TK_STRING, token.type());
      EXPECT_EQ(source, token.source());
    }
  }

  {
    std::vector<std::pair<std::u16string, std::u16string>> sources = {
      {u"'\\u12'", u"'\\u12'"}, {u"'\\uGH'", u"'\\uG"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      es::Lexer lexer(source);
      es::Token token = lexer.Next();
      EXPECT_EQ(es::Token::Type::TK_ILLEGAL, token.type());
      EXPECT_EQ(error, token.source());
    }
  }
}

TEST(TestLexer, Number) {
  {
    std::vector<std::u16string> sources = {
      u"0", u"101", u"0.01", u"12.05", u".8" ,u"0xAbC09",
    };
    for (auto source : sources) {
      es::Lexer lexer(source);
      es::Token token = lexer.Next();
      EXPECT_EQ(es::Token::Type::TK_NUMBER, token.type());
      EXPECT_EQ(source, token.source());
    }
  }

  {
    std::vector<std::u16string> sources = {
      u"0e10", u"101E02", u"0.01E5", u".8E5" ,u"12.05e05",
    };
    for (auto source : sources) {
      es::Lexer lexer(source);
      es::Token token = lexer.Next();
      EXPECT_EQ(es::Token::Type::TK_NUMBER, token.type());
      EXPECT_EQ(source, token.source());
    }
  }

  {
    std::vector<std::pair<std::u16string, std::u16string>> sources = {
      {u"01", u"01"}, {u"0..", u"0.."}, {u"3in", u"3i"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      es::Lexer lexer(source);
      es::Token token = lexer.Next();
      EXPECT_EQ(es::Token::Type::TK_ILLEGAL, token.type());
      EXPECT_EQ(error, token.source());
    }
  }
}

TEST(TestLexer, Identifier) {
  {
    std::vector<std::u16string> sources = {
      u"a", u"$abc", u"你好", u"\u12AbC",
    };
    for (auto source : sources) {
      es::Lexer lexer(source);
      es::Token token = lexer.Next();
      EXPECT_EQ(es::Token::Type::TK_IDENT, token.type());
      EXPECT_EQ(source, token.source());
    }
  }

  {
    std::vector<std::u16string> sources = {
      u"break", u"for", u"in",
    };
    for (auto source : sources) {
      es::Lexer lexer(source);
      es::Token token = lexer.Next();
      EXPECT_EQ(es::Token::Type::TK_KEYWORD, token.type());
      EXPECT_EQ(source, token.source());
    }
  }

  {
    std::vector<std::u16string> sources = {
      u"class", u"import", u"export",
    };
    for (auto source : sources) {
      es::Lexer lexer(source);
      es::Token token = lexer.Next();
      EXPECT_EQ(es::Token::Type::TK_FUTURE, token.type());
      EXPECT_EQ(source, token.source());
    }
  }

  {
    std::vector<std::u16string> sources = {
      u"true", u"false",
    };
    for (auto source : sources) {
      es::Lexer lexer(source);
      es::Token token = lexer.Next();
      EXPECT_EQ(es::Token::Type::TK_BOOL, token.type());
      EXPECT_EQ(source, token.source());
    }
  }

  {
    std::vector<std::pair<std::u16string, std::u16string>> sources = {
      {u"\\u12", u"\\u12"}, {u"😊", u"\xD83D"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      es::Lexer lexer(source);
      es::Token token = lexer.Next();
      EXPECT_EQ(es::Token::Type::TK_ILLEGAL, token.type());
      EXPECT_EQ(error, token.source());
    }
  }
}

TEST(TestLexer, Regex) {
  {
    std::vector<std::u16string> sources = {
      u"/a/", u"/[a-z]*?/", u"/[012]/gba", u"/[012]/$", u"/你好/",
    };
    for (auto source : sources) {
      es::Lexer lexer(source);
      es::Token token = lexer.ScanRegexLiteral();
      EXPECT_EQ(es::Token::Type::TK_REGEX, token.type());
      EXPECT_EQ(source, token.source());
    }
  }

  {
    std::vector<std::pair<std::u16string, std::u16string>> sources = {
      {u"//", u"//"}, {u"/*/", u"/*"}, {u"/a ", u"/a "}, {u"/[a /", u"/[a /"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      es::Lexer lexer(source);
      es::Token token = lexer.ScanRegexLiteral();
      EXPECT_EQ(es::Token::Type::TK_ILLEGAL, token.type());
      EXPECT_EQ(error, token.source());
    }
  }
}
