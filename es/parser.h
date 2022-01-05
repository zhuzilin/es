#ifndef ES_PARSER_H
#define ES_PARSER_H

#include <es/lexer.h>
#include <es/ast.h>

#include <test/helper.h>

namespace es {

class Parser {
 public:
  Parser(std::u16string_view source) : source_(source), lexer_(source) {}

  AST* ParsePrimaryExpression() {
    Token token(Token::TK_NOT_FOUND, u"");
    if (lexer_.Now() == u'/') {
      token = lexer_.ScanRegexLiteral();
      if (token.type() == Token::TK_REGEX) {
        return new AST(AST::AST_EXP_REGEX, token.source());
      } else {
        goto error;
      }
    }
    token = lexer_.NextAndRewind();
    switch (token.type()) {
      case Token::TK_KEYWORD:
        if (token.source() == u"this") {
          return new AST(AST::AST_EXP_THIS, token.source());
        }
        goto error;
      case Token::TK_IDENT:
        lexer_.Next();
        return new AST(AST::AST_EXP_IDENT, token.source());
      case Token::TK_NULL:
        lexer_.Next();
        return new AST(AST::AST_EXP_NULL, token.source());
      case Token::TK_BOOL:
        lexer_.Next();
        return new AST(AST::AST_EXP_BOOL, token.source());
      case Token::TK_NUMBER:
        lexer_.Next();
        return new AST(AST::AST_EXP_NUMBER, token.source());
      case Token::TK_STRING:
        lexer_.Next();
        return new AST(AST::AST_EXP_STRING, token.source());
      case Token::TK_LBRACK:  // [
        return ParseArrayLiteral();
      case Token::TK_LBRACE:  // {
        return ParseObjectLiteral();
      case Token::TK_LPAREN: { // (
        AST* value = ParseExpression(false);
        if (value->type() == AST::AST_ILLEGAL)
          goto error;
      }
      default:
        goto error;
    }

error:
    return new AST(AST::AST_ILLEGAL, token.source());
  }

  AST* ParseArrayLiteral() {
    size_t start = lexer_.Pos();
    assert(lexer_.Next().type() == Token::TK_LBRACK);

    ArrayLiteral* array = new ArrayLiteral();
    AST* element = nullptr;

    Token token = lexer_.NextAndRewind();
    while (token.type() != Token::TK_RBRACK) {
      test::PrintSource("", token.source());
      switch (token.type()) {
        case Token::TK_COMMA:
          lexer_.Next();
          array->AddElement(element);
          element = nullptr;
          break;
        default:
          element = ParseAssignmentExpression(false);
          if (element->type() == AST::AST_ILLEGAL) {
            return element;
          }
      }
      token = lexer_.NextAndRewind();
    }
    if (element != nullptr) {
      array->AddElement(element);
    }
    assert(token.type() == Token::TK_RBRACK);
    assert(lexer_.Next().type() == Token::TK_RBRACK);
    array->SetSource(source_.substr(start, lexer_.Pos() - start));
    return array;
error:
    delete array;
    return new AST(AST::AST_ILLEGAL, source_.substr(start, lexer_.Pos() - start));
  }

  AST* ParseObjectLiteral() {
    size_t start = lexer_.Pos();
    assert(lexer_.Next().type() == Token::TK_LBRACE);

    ObjectLiteral* obj = new ObjectLiteral();
    Token token = lexer_.NextAndRewind();
    while (token.type() != Token::TK_RBRACE) {
      switch (token.type()) {
        case Token::TK_IDENT:
          if (token.source() == u"get" || token.source() == u"set") {
            assert("false");
            break;
          }
        case Token::TK_KEYWORD:
        case Token::TK_FUTURE:
        case Token::TK_STRING:
        case Token::TK_NUMBER: {
          lexer_.Next();
          if (lexer_.Next().type() != Token::TK_COLON)
            goto error;
          AST* value = ParseAssignmentExpression(false);
          if (value->type() == AST::AST_ILLEGAL)
            goto error;
          obj->AddProperty(ObjectLiteral::Property(token, value, ObjectLiteral::Property::NORMAL));
          break;
        }
        default:
          goto error;
      }
      token = lexer_.NextAndRewind();
    }
    assert(token.type() == Token::TK_RBRACE);
    assert(lexer_.Next().type() == Token::TK_RBRACE);
    obj->SetSource(source_.substr(start, lexer_.Pos() - start));
    return obj;
error:
    delete obj;
    return new AST(AST::AST_ILLEGAL, source_.substr(start, lexer_.Pos() - start));
  }

  AST* ParseExpression(bool no_in) {
    std::cout << "ParseExpression" << std::endl;
    Expression* expr = new Expression();
    size_t start = lexer_.Pos();

    AST* element = ParseAssignmentExpression(no_in);
    if (element->type() == AST::AST_ILLEGAL) {
      return element;
    }
    expr->AddElement(element);
    Token token = lexer_.Next();
    while (token.type() == Token::TK_COMMA) {
      lexer_.Next();
      element = ParseAssignmentExpression(no_in);
      if (element->type() == AST::AST_ILLEGAL) {
        return element;
      }
      expr->AddElement(element);
    }
    return expr;
error:
    delete expr;
    return new AST(AST::AST_ILLEGAL, source_.substr(start, lexer_.Pos() - start));
  }

  AST* ParseAssignmentExpression(bool no_in) {
    std::cout << "ParseAssignmentExpression" << std::endl;
    AST* lhs = ParseConditionalExpression(no_in);
    if (lhs->IsIllegal())
      return lhs;

    // Not LeftHandSideExpression
    if (lhs->type() == AST::AST_EXP_BINARY || lhs->type() == AST::AST_EXP_UNARY ||
        lhs->type() == AST::AST_EXP_TRIPLE) {
      return lhs;
    }
    Token op = lexer_.NextAndRewind();
    if (!op.IsAssignmentOperator())
      return lhs;

    lexer_.Next();
    AST* rhs = ParseAssignmentExpression(no_in);
    if (rhs->IsIllegal()) {
      delete lhs;
      return rhs;
    }

    return new Binary(lhs, rhs, op);
  }

  AST* ParseConditionalExpression(bool no_in) {
    size_t start = lexer_.Pos();
    AST* cond = ParseBinaryAndUnaryExpression(no_in, 0);
    if (cond->IsIllegal())
      return cond;
    Token token = lexer_.NextAndRewind();
    if (token.type() != Token::TK_QUESTION)
      return cond;
    AST* lhs = ParseAssignmentExpression(no_in);
    if (lhs->IsIllegal()) {
      delete cond;
      return lhs;
    }
    AST* rhs = ParseAssignmentExpression(no_in);
    if (lhs->IsIllegal()) {
      delete cond;
      delete lhs;
      return rhs;
    }
    AST* triple = new TripleCondition(cond, lhs, rhs);
    triple->SetSource(source_.substr(start, lexer_.Pos() - start));
    return triple;
  }

  AST* ParseBinaryAndUnaryExpression(bool no_in, int priority) {
    std::cout << "ParseBinaryAndUnaryExpression " << priority << std::endl;
    size_t start = lexer_.Pos();
    AST* lhs = nullptr;
    AST* rhs = nullptr;
    // Prefix Operators.
    Token prefix_op = lexer_.NextAndRewind();
    if (prefix_op.UnaryPrefixPriority() > priority) {
      lexer_.Next();
      lhs = ParseBinaryAndUnaryExpression(no_in, prefix_op.UnaryPrefixPriority());
      if (lhs->IsIllegal())
        return lhs;
      lhs = new Unary(lhs, prefix_op, true);
    } else {
      lhs = ParseLeftHandSideExpression();
      if (lhs->IsIllegal())
        return lhs;
      // Postfix Operators.
      //
      // Because the priority of postfix operators are higher than prefix ones,
      // they won't be parsed at the same time.
      Token postfix_op = lexer_.NextAndRewind();
      bool is_line_terminator = lexer_.NextAndRewind(true).IsLineTerminator();
      if (!is_line_terminator && postfix_op.UnaryPostfixPriority() > priority) {
        if (lhs->type() != AST::AST_EXP_BINARY && lhs->type() != AST::AST_EXP_UNARY) {
          lexer_.Next();
          test::PrintSource("Find postfix op: ", postfix_op.source());
          lhs = new Unary(lhs, postfix_op, false);
          lhs->SetSource(source_.substr(start, lexer_.Pos() - start));
          test::PrintSource("lhs:", lhs->source());
        } else {
          delete lhs;
          return new AST(AST::AST_ILLEGAL, source_.substr(start, lexer_.Pos() - start));
        }
      }
    }
    while (true) {
      Token binary_op = lexer_.NextAndRewind();
      if (binary_op.BinaryPriority(no_in) > priority) {
        test::PrintSource("Find binary op: ", binary_op.source());
        lexer_.Next();
        rhs = ParseBinaryAndUnaryExpression(no_in, binary_op.BinaryPriority(no_in));
        if (rhs->IsIllegal())
          return rhs;
        test::PrintSource("lhs:", lhs->source());
        test::PrintSource("rhs:", rhs->source());
        lhs = new Binary(lhs, rhs, binary_op);
        lhs->SetSource(source_.substr(start, lexer_.Pos() - start));
      } else {
        break;
      }
    }
    lhs->SetSource(source_.substr(start, lexer_.Pos() - start));
    return lhs;
  }

  AST* ParseLeftHandSideExpression() {
    return ParsePrimaryExpression();
  }

 private:
  std::u16string_view source_;
  Lexer lexer_;
};

}  // namespace es

#endif  // ES_PARSER_H