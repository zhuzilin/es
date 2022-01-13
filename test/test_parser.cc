#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include <gtest/gtest.h>

#include <es/parser/parser.h>
#include <es/helper.h>

using namespace es;

typedef std::u16string string;
typedef std::vector<string> vec_string;
typedef std::pair<string,string> pair_string;
typedef std::vector<std::pair<string,string>> vec_pair_string;

TEST(TestParser, PrimaryExpression_Literal) {
  // This
  {
    vec_string sources = {
      u"\n \t this",
    };
    for (auto source : sources) {
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_EXPR_THIS, ast->type());
      EXPECT_EQ(u"this", ast->source());
    }
  }

  // Identifier
  { 
    vec_string sources = {
      u"你好", u"_abcDEF$", u"NULL", u"Null", u"True", u"False",
    };
    for (auto source : sources) {
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_EXPR_IDENT, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // Null
  {
    vec_string sources = {
      u"null",
    };
    for (auto source : sources) {
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_EXPR_NULL, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // Bool
  {
    vec_string sources = {
      u"true", u"false",
    };
    for (auto source : sources) {
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_EXPR_BOOL, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // Number
  {
    vec_string sources = {
      u"0", u"101", u"0.01", u"12.05", u".8" ,u"0xAbC09",
    };
    for (auto source : sources) {
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_EXPR_NUMBER, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // String
  {
    vec_string sources = {
      u"''", u"'\\n\\b\\u1234\\x12'", u"'😊'",
    };
    for (auto source : sources) {
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_EXPR_STRING, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // Regex
  {
    vec_string sources = {
      u"/a/", u"/[a-z]*?/", u"/[012]/g", u"/[012]/$", u"/你好/",
    };
    for (auto source : sources) {
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_EXPR_REGEX, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // Illegal Literal
  {
    vec_pair_string sources = {
      {u"for", u"for"}, {u"😊", u"\xD83D"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(error, ast->source());
    }
  }
}

TEST(TestParser, PrimaryExpression_Array) {
  {
    std::vector<std::pair<string, size_t>> sources = {
      {u"[]", 0}, {u"[,]", 1}, {u"[abc, 123,'string', ]", 3}, {u"[1+2*3, ++a]", 2}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      size_t length = pair.second;
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_EXPR_ARRAY, ast->type());
      EXPECT_EQ(source, ast->source());
      auto array = static_cast<ArrayLiteral*>(ast);
      EXPECT_EQ(length, array->length());
    }
  }

  // Illegal
  {
    // TODO(zhuzilin) This error messsage is not intuitive.
    vec_pair_string sources = {
      {u"[a,", u""}, {u"[", u""},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(error, ast->source());
    }
  }
}

TEST(TestParser, PrimaryExpression_Object) {
  {
    std::vector<std::pair<string, size_t>> sources = {
      {u"{}", 0}, {u"{a: 1,}", 1}, {u"{in: bed, b: 10 + 5}", 2},
      {u"{1: 1}", 1}, {u"{\"abc\": 1}", 1},
      {u"{get name() { return 10 },}", 1},
      {u"{set name(a) { return 10 },}", 1},
      {u"{get name() { return 10 }, set name(a) { return 10 },}", 2}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      size_t length = pair.second;
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_EXPR_OBJ, ast->type());
      EXPECT_EQ(source, ast->source());
      auto obj = static_cast<ObjectLiteral*>(ast);
      EXPECT_EQ(length, obj->length());
    }
  }

  // Illegal
  {
    vec_pair_string sources = {
      {u"{,}", u"{,"}, {u"{a,}", u"{a,"}, {u"{a 1}", u"{a 1"},
      {u"{get name() return 10 }}", u"{get name() return"},
      {u"{set name() { return 10 },}", u"{set name()"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(error, ast->source());
    }
  }
}

TEST(TestParser, PrimaryExpression_Parentheses) {
  {
    vec_pair_string sources = {
      {u"(a)", u"a"}, {u"(a + b)", u"a + b"}, {u"(a + b, a++)", u"a + b, a++"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      // NOTE(zhuzilin) If expr has only one element, then just return the element.
      EXPECT_EQ(AST::AST_EXPR_PAREN, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }

  // Illegal
  {
    // TODO(zhuzilin) This error message is not intuitive.
    vec_pair_string sources = {
      {u"()", u")"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      Parser parser(source);
      AST* ast = parser.ParsePrimaryExpression();
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(error, ast->source());
    }
  }
}

TEST(TestParser, Expression_Binary) {
  {
    std::vector<
      std::pair<string,
      std::pair<string, string>>> sources = {
      {u"a + b * c", {u"a", u" b * c"}},
      {u"a * b + c", {u"a * b", u" c"}},
      {u"a * b + + c - d", {u"a * b + + c", u" d"}},
      {u"a++ == b && ++c != d", {u"a++ == b", u" ++c != d"}},
      {u"(1 + 3) * 5 - (8 + 16)", {u"(1 + 3) * 5", u" (8 + 16)"}}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto lhs = pair.second.first;
      auto rhs = pair.second.second;
      Parser parser(source);
      AST* ast = parser.ParseBinaryAndUnaryExpression(false, 0);
      EXPECT_EQ(AST::AST_EXPR_BINARY, ast->type());
      EXPECT_EQ(source, ast->source());
      auto binary = static_cast<Binary*>(ast);
      EXPECT_EQ(lhs, binary->lhs()->source());
      EXPECT_EQ(rhs, binary->rhs()->source());
    }
  }
}

TEST(TestParser, Expression_Unary) {
  {
    std::vector<std::pair<string, string>> sources = {
      {u"a ++", u"a"}, {u"++\na", u"\na"}, {u"++ a", u" a"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto node = pair.second;
      Parser parser(source);
      AST* ast = parser.ParseBinaryAndUnaryExpression(false, 0);
      EXPECT_EQ(AST::AST_EXPR_UNARY, ast->type());
      EXPECT_EQ(source, ast->source());
      auto unary = static_cast<Unary*>(ast);
      EXPECT_EQ(node, unary->node()->source());
    }
  }

  // invalid
  {
    std::vector<std::pair<string, string>> sources = {
      {u"a\n++", u"a"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      Parser parser(source);
      AST* ast = parser.ParseBinaryAndUnaryExpression(false, 0);
      EXPECT_NE(AST::AST_EXPR_UNARY, ast->type());
      EXPECT_EQ(error, ast->source());
    }
  }
}

TEST(TestParser, Expression_TripleCondition) {
  {
    std::vector<vec_string> sources = {
      {u"a ?b:c", u"a", u"b", u"c"},
      {u"a ?c ? d : e : c", u"a", u"c ? d : e", u" c"},
    };
    for (auto vec : sources) {
      auto source = vec[0];
      Parser parser(vec[0]);
      AST* ast = parser.ParseConditionalExpression(false);
      EXPECT_EQ(AST::AST_EXPR_TRIPLE, ast->type());
      EXPECT_EQ(source, ast->source());
      auto cond = static_cast<TripleCondition*>(ast);
      EXPECT_EQ(vec[1], cond->cond()->source());
      EXPECT_EQ(vec[2], cond->lhs()->source());
      EXPECT_EQ(vec[3], cond->rhs()->source());
    }
  }

  // invalid
  {
    std::vector<std::pair<string, string>> sources = {
      {u"a ?b c", u"a ?b"}, {u"a ", u"a"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      Parser parser(source);
      AST* ast = parser.ParseConditionalExpression(false);
      EXPECT_NE(AST::AST_EXPR_TRIPLE, ast->type());
      EXPECT_EQ(error, ast->source());
    }
  }
}

TEST(TestParser, Expression_Arguments) {
  {
    std::vector<std::pair<string,
                          vec_string>> sources = {
      {u"()", {}},
      {u"(a)", {u"a"}},
      {u"(a, 1+3, function(){})", {u"a", u" 1+3", u" function(){}"}},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto args = pair.second;
      Parser parser(source);
      AST* ast = parser.ParseArguments();
      EXPECT_EQ(AST::AST_EXPR_ARGS, ast->type());
      EXPECT_EQ(source, ast->source());
      auto func = static_cast<Arguments*>(ast);
      for (size_t i = 0; i < func->args().size(); i++) {
        EXPECT_EQ(args[i], func->args()[i]->source());
      }
    }
  }
}

TEST(TestParser, Expression_LeftHandSide) {
  {
    vec_string sources = {
      u"new Object()", u"function(a, b, c){}(c, d)",
      u"new new a[123 + xyz].__ABC['您好']()()"
    };
    for (auto source : sources) {
      Parser parser(source);
      AST* ast = parser.ParseLeftHandSideExpression();
      EXPECT_EQ(AST::AST_EXPR_LHS, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }
}

TEST(TestParser, Function) {
  {
    std::vector<std::pair<string,
                          vec_string>> sources = {
      {u"function () {}", {u"", u""}},
      {u"function name (a, b) {a=1}", {u"name", u"a", u"b", u"a=1"}},
      {u"function (a, a, c) {return a,b}", {u"", u"a", u"a", u"c", u"return a,b"}},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto params = pair.second;
      Parser parser(source);
      AST* ast = parser.ParseFunction(false);
      EXPECT_EQ(AST::AST_FUNC, ast->type());
      EXPECT_EQ(source, ast->source());
      auto func = static_cast<Function*>(ast);
      EXPECT_EQ(params[0], func->name());
      EXPECT_EQ(params.size() - 2, func->params().size());
      for (size_t i = 0; i < func->params().size(); i++) {
        EXPECT_EQ(params[i + 1], func->params()[i]);
      }
      EXPECT_EQ(AST::AST_FUNC_BODY, func->body()->type());
      EXPECT_EQ(params[func->params().size() + 1], func->body()->source());
    }
  }

  // invalid not named
  {
    std::vector<std::pair<string, string>> sources = {
      {u"function (,) {}", u"function (,"}, {u"function (a a) {}", u"function (a a"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      Parser parser(source);
      AST* ast = parser.ParseFunction(false);
      EXPECT_NE(AST::AST_FUNC, ast->type());
      EXPECT_EQ(error, ast->source());
    }
  }

  // invalid named
  {
    std::vector<std::pair<string, string>> sources = {
      {u"function () {}", u"function ("},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      auto error = pair.second;
      Parser parser(source);
      AST* ast = parser.ParseFunction(true);
      EXPECT_NE(AST::AST_FUNC, ast->type());
      EXPECT_EQ(error, ast->source());
    }
  }
}

TEST(TestParser, Statement_Debugger) {
  {
    vec_pair_string sources = {
      {u"\n \t debugger", u"\n \t debugger"},
      {u"debugger;", u"debugger;"},
      {u"debugger\na", u"debugger"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_STMT_DEBUG, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
  // Illegal
  {
    vec_pair_string sources = {
      {u"debugger 1", u"debugger 1"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
}

TEST(TestParser, Statement_Continue) {
  {
    vec_pair_string sources = {
      {u"continue ;", u"continue ;"}, {u"continue a ", u"continue a"},
      {u"continue a ;", u"continue a ;"}, {u"continue \n a ;", u"continue"},
      // NOTE(zhuzilin) This may be wrong...
      {u"continue a \n ;", u"continue a \n ;"},
      {u"continue a }", u"continue a"}, {u"continue }", u"continue"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_STMT_CONTINUE, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }

  // Illegal
  {
    vec_pair_string sources = {
      {u"continue 1", u"continue 1"}, {u"continue a b", u"continue a b"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
}

TEST(TestParser, Statement_Break) {
  {
    vec_pair_string sources = {
      {u"break ;", u"break ;"}, {u"break a ", u"break a"},
      {u"break a ;", u"break a ;"}, {u"break \n a ;", u"break"},
      {u"break a }", u"break a"}, {u"break }", u"break"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_STMT_BREAK, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }

  // Illegal
  {
    vec_pair_string sources = {
      {u"break 1", u"break 1"}, {u"break a b", u"break a b"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
}

TEST(TestParser, Statement_Return) {
  {
    vec_pair_string sources = {
      {u"return ;", u"return ;"}, {u"return a+6 ", u"return a+6"},
      {u"return 1,\n2 ;", u"return 1,\n2 ;"}, {u"return \n a ;", u"return"},
      {u"return this.b }", u"return this.b"}, {u"return }", u"return"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_STMT_RETURN, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }

  // Illegal
  {
    vec_pair_string sources = {
      {u"return a b", u"return a b"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
}

TEST(TestParser, Statement_Throw) {
  {
    vec_pair_string sources = {
      {u"throw ;", u"throw ;"}, {u"throw a+6 ", u"throw a+6"},
      {u"throw 1,\n2 ;", u"throw 1,\n2 ;"}, {u"throw \n a ;", u"throw"},
      {u"throw a }", u"throw a"}, {u"throw }", u"throw"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_STMT_THROW, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }

  // Illegal
  {
    vec_pair_string sources = {
      {u"throw a b", u"throw a b"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
}

TEST(TestParser, VariableDeclaration) {
  {
    vec_pair_string sources = {
      {u"a = b", u"a = b"}, {u"c123 = function() {}", u"c123 = function() {}"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseVariableDeclaration(false);
      EXPECT_EQ(AST::AST_STMT_VAR_DECL, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
}

TEST(TestParser, Statement_Variable) {
  {
    vec_pair_string sources = {
      {u"var a = b, c = 1 + 5", u"var a = b, c = 1 + 5"}, {u"var xyz", u"var xyz"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseVariableStatement(false);
      EXPECT_EQ(AST::AST_STMT_VAR, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }

  // Illegal
  {
    vec_pair_string sources = {
      {u"var a = b b", u"var a = b b"}, {u"var 1 = a", u"var 1"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseVariableStatement(false);;
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
}

TEST(TestParser, Statement_Block) {
  {
    vec_pair_string sources = {
      {u"{}", u"{}"}, {u"{var a =b\n b=c}", u"{var a =b\n b=c}"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_STMT_BLOCK, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }

  // Illegal
  {
    vec_pair_string sources = {
      {u"{", u""}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();;
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
}

TEST(TestParser, Statement_If) {
  {
    vec_pair_string sources = {
      {u"if (a == b) { a++ } else if (a > b) {d}", u"if (a == b) { a++ } else if (a > b) {d}"},
      {u"if (a == b) a++\n else {d}", u"if (a == b) a++\n else {d}"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_STMT_IF, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }

  // Illegal
  {
    vec_pair_string sources = {
      {u"if (a == b) a++ else {d}", u" a++ else"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();;
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
}

TEST(TestParser, Statement_While) {
  {
    vec_pair_string sources = {
      {u"while (true) { a++\n break }", u"while (true) { a++\n break }"},
      {u"while (a == b);", u"while (a == b);"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_STMT_WHILE, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }

  // Illegal
  {
    vec_pair_string sources = {
      {u"while (a == b a++", u"while (a == b a"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();;
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
}

TEST(TestParser, Statement_With) {
  {
    vec_pair_string sources = {
      {u"with (true) { a++\n break }", u"with (true) { a++\n break }"},
      {u"with (a == b);", u"with (a == b);"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_STMT_WITH, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }

  // Illegal
  {
    vec_pair_string sources = {
      {u"with (a == b a++", u"with (a == b a"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();;
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
}

TEST(TestParser, Statement_DoWhile) {
  {
    vec_pair_string sources = {
      {u"do ; while(true) ", u"do ; while(true)"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_STMT_DO_WHILE, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }

  // Illegal
  {
    vec_pair_string sources = {
      {u"do ; while(true) a", u"do ; while(true) a"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();;
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
}

TEST(TestParser, Statement_Try) {
  {
    vec_pair_string sources = {
      {u"try { throw error; } catch (e) {console.log(e)}", u"try { throw error; } catch (e) {console.log(e)}"},
      {u"try { throw error; } finally { return -1}", u"try { throw error; } finally { return -1}"},
      {u"try { throw error; } catch (e) {throw e;} finally { return -1; }", u"try { throw error; } catch (e) {throw e;} finally { return -1; }"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_STMT_TRY, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }

  // Illegal
  {
    vec_pair_string sources = {
      {u"try { throw error; }", u"try { throw error; }"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();;
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
}

TEST(TestParser, Statement_Labelled) {
  {
    vec_pair_string sources = {
      {u"loop1 : while (true) { while (true) { break loop1 }}", u"loop1 : while (true) { while (true) { break loop1 }}"},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_STMT_LABEL, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
}

TEST(TestParser, Statement_Switch) {
  {
    std::vector<std::pair<string, size_t>> sources = {
      {u"switch(c){case a:}", 1}, {u"switch(c){default: 10\ncase (1+2):}", 2},
      {u"switch(c){case a:{}break;\ndefault: break;}", 2},
    };
    for (auto pair : sources) {
      auto source = pair.first;
      size_t length = pair.second;
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_STMT_SWITCH, ast->type());
      EXPECT_EQ(source, ast->source());
      auto switch_stmt = static_cast<Switch*>(ast);
      EXPECT_EQ(length, switch_stmt->case_clauses().size());
    }
  }

  // Illegal
  {
    vec_pair_string sources = {
      // TODO(zhuzilin) This error message is unintuive...
      {u"switch(c){case :}", u":"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();;
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
}

TEST(TestParser, Statement_For) {
  {
    vec_string sources = {
      u"for (;;) {}", u"for (a,b,c;;) {}",
      u"for (var a=10;;) {}", u"for (var a=10,b=20;;) {}"
    };
    for (auto source : sources) {
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_STMT_FOR, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // Illegal
  {
    vec_pair_string sources = {
      {u"for (var a=10 b=20;;) {}", u"for (var a=10 b"},
      {u"for (;var a=5;) {}", u"var"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();;
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
}

TEST(TestParser, Statement_ForIn) {
  {
    vec_string sources = {
      u"for (a in 1 + 1) {}", u"for (var a = 10 in 'abc') {}",
    };
    for (auto source : sources) {
      Parser parser(source);
      AST* ast = parser.ParseStatement();
      EXPECT_EQ(AST::AST_STMT_FOR_IN, ast->type());
      EXPECT_EQ(source, ast->source());
    }
  }

  // Illegal
  {
    vec_pair_string sources = {
      {u"for (a,b in c) {}", u"for (a,b"},
      {u"for (var a=1,b=2 in d", u"for (var a=1,b=2 in"}
    };
    for (auto pair : sources) {
      auto source = pair.first;
      Parser parser(source);
      AST* ast = parser.ParseStatement();;
      EXPECT_EQ(AST::AST_ILLEGAL, ast->type());
      EXPECT_EQ(pair.second, ast->source());
    }
  }
}
