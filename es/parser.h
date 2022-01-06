#ifndef ES_PARSER_H
#define ES_PARSER_H

#include <es/lexer.h>
#include <es/ast.h>

#include <test/helper.h>

#define START_POS size_t start = lexer_.Pos()
#define SOURCE_PARSED source_.substr(start, lexer_.Pos() - start)

namespace es {

class Parser {
 public:
  Parser(std::u16string_view source) : source_(source), lexer_(source) {}

  AST* ParsePrimaryExpression() {
    Token token(Token::TK_NOT_FOUND, u"");
    if (lexer_.Now() == u'/') {
      token = lexer_.ScanRegexLiteral();
      if (token.type() == Token::TK_REGEX) {
        return new AST(AST::AST_EXPR_REGEX, token.source());
      } else {
        goto error;
      }
    }
    token = lexer_.NextAndRewind();
    switch (token.type()) {
      case Token::TK_KEYWORD:
        if (token.source() == u"this") {
          return new AST(AST::AST_EXPR_THIS, token.source());
        }
        goto error;
      case Token::TK_IDENT:
        lexer_.Next();
        return new AST(AST::AST_EXPR_IDENT, token.source());
      case Token::TK_NULL:
        lexer_.Next();
        return new AST(AST::AST_EXPR_NULL, token.source());
      case Token::TK_BOOL:
        lexer_.Next();
        return new AST(AST::AST_EXPR_BOOL, token.source());
      case Token::TK_NUMBER:
        lexer_.Next();
        return new AST(AST::AST_EXPR_NUMBER, token.source());
      case Token::TK_STRING:
        lexer_.Next();
        return new AST(AST::AST_EXPR_STRING, token.source());
      case Token::TK_LBRACK:  // [
        return ParseArrayLiteral();
      case Token::TK_LBRACE:  // {
        return ParseObjectLiteral();
      case Token::TK_LPAREN: { // (
        lexer_.Next();   // skip (
        AST* value = ParseExpression(false);
        if (value->type() == AST::AST_ILLEGAL)
          return value;
        if (lexer_.Next().type() != Token::TK_RPAREN) {
          delete value;
          goto error;
        }
        return value;
      }
      default:
        goto error;
    }

error:
    return new AST(AST::AST_ILLEGAL, token.source());
  }

  AST* ParseFunctionExpression() {
    START_POS;
    assert(lexer_.Next().source() == u"function");

    Token name(Token::TK_NOT_FOUND, u"");
    std::vector<Token> params;
    AST* tmp;
    FunctionBody* body;
    Function* func;

    // Identifier_opt
    Token token = lexer_.Next();
    if (token.type() == Token::TK_IDENT) {
      name = token;
      token = lexer_.Next();
    }
    if (token.type() != Token::TK_LPAREN) {
      goto error;
    }
    token = lexer_.Next();
    if (token.type() == Token::TK_IDENT) {
      params.emplace_back(token);
      token = lexer_.Next();
    } else if (token.type() != Token::TK_RPAREN) {
      goto error;
    }
    while (token.type() != Token::TK_RPAREN) {
      if (token.type() != Token::TK_COMMA) {
        goto error;
      }
      token = lexer_.Next();
      if (token.type() != Token::TK_IDENT) {
        goto error;
      }
      params.emplace_back(token);
      token = lexer_.Next();
    }
    token = lexer_.Next();  // skip {
    if (token.type() != Token::TK_LBRACE) {
      goto error;
    }
    // tmp = ParseFunctionBody();
    // if (tmp->IsIllegal())
    //   return tmp;
    // body = static_cast<FunctionBody*>(tmp);

    token = lexer_.Next();  // skip }
    if (token.type() != Token::TK_RBRACE) {
      goto error;
    }

    func = new Function(name, params, nullptr);
    func->SetSource(SOURCE_PARSED);
    return func;
error:
    return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
  }

  AST* ParseFunctionBody() {
    return nullptr;
  }

  AST* ParseArrayLiteral() {
    START_POS;
    assert(lexer_.Next().type() == Token::TK_LBRACK);

    ArrayLiteral* array = new ArrayLiteral();
    AST* element = nullptr;

    Token token = lexer_.NextAndRewind();
    while (token.type() != Token::TK_RBRACK) {
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
    array->SetSource(SOURCE_PARSED);
    return array;
error:
    delete array;
    return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
  }

  AST* ParseObjectLiteral() {
    START_POS;
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
    obj->SetSource(SOURCE_PARSED);
    return obj;
error:
    delete obj;
    return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
  }

  AST* ParseExpression(bool no_in) {
    START_POS;
    Expression* expr = new Expression();

    AST* element = ParseAssignmentExpression(no_in);
    if (element->IsIllegal()) {
      delete expr;
      return element;
    }
    expr->AddElement(element);
    Token token = lexer_.NextAndRewind();
    while (token.type() == Token::TK_COMMA) {
      lexer_.Next();  // skip ,
      element = ParseAssignmentExpression(no_in);
      if (element->IsIllegal()) {
        delete expr;
        return element;
      }
      expr->AddElement(element);
      token = lexer_.NextAndRewind();
    }
    expr->SetSource(SOURCE_PARSED);
    return expr;
  }

  AST* ParseAssignmentExpression(bool no_in) {
    AST* lhs = ParseConditionalExpression(no_in);
    if (lhs->IsIllegal())
      return lhs;

    // Not LeftHandSideExpression
    if (lhs->type() == AST::AST_EXPR_BINARY || lhs->type() == AST::AST_EXPR_UNARY ||
        lhs->type() == AST::AST_EXPR_TRIPLE) {
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
    START_POS;
    AST* cond = ParseBinaryAndUnaryExpression(no_in, 0);
    if (cond->IsIllegal())
      return cond;
    Token token = lexer_.NextAndRewind();
    if (token.type() != Token::TK_QUESTION)
      return cond;
    lexer_.Next();
    AST* lhs = ParseAssignmentExpression(no_in);
    if (lhs->IsIllegal()) {
      delete cond;
      return lhs;
    }
    token = lexer_.NextAndRewind();
    if (token.type() != Token::TK_COLON) {
      delete cond;
      delete lhs;
      return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
    }
    lexer_.Next();
    AST* rhs = ParseAssignmentExpression(no_in);
    if (lhs->IsIllegal()) {
      delete cond;
      delete lhs;
      return rhs;
    }
    AST* triple = new TripleCondition(cond, lhs, rhs);
    triple->SetSource(SOURCE_PARSED);
    return triple;
  }

  AST* ParseBinaryAndUnaryExpression(bool no_in, int priority) {
    START_POS;
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
      if (!lexer_.LineTermAhead() && postfix_op.UnaryPostfixPriority() > priority) {
        if (lhs->type() != AST::AST_EXPR_BINARY && lhs->type() != AST::AST_EXPR_UNARY) {
          lexer_.Next();
          lhs = new Unary(lhs, postfix_op, false);
          lhs->SetSource(SOURCE_PARSED);
        } else {
          delete lhs;
          return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
        }
      }
    }
    while (true) {
      Token binary_op = lexer_.NextAndRewind();
      if (binary_op.BinaryPriority(no_in) > priority) {
        lexer_.Next();
        rhs = ParseBinaryAndUnaryExpression(no_in, binary_op.BinaryPriority(no_in));
        if (rhs->IsIllegal())
          return rhs;
        lhs = new Binary(lhs, rhs, binary_op);
        lhs->SetSource(SOURCE_PARSED);
      } else {
        break;
      }
    }
    lhs->SetSource(SOURCE_PARSED);
    return lhs;
  }

  AST* ParseLeftHandSideExpression() {
    START_POS;
    Token token = lexer_.NextAndRewind();
    AST* base;
    size_t new_count = 0;
    while (token.source() == u"new") {
      lexer_.Next();
      new_count++;
      token = lexer_.NextAndRewind();
    }
    if (token.source() == u"function") {
      base = ParseFunctionExpression();
    } else {
      base = ParsePrimaryExpression();
    }
    if (base->IsIllegal()) {
      return base;
    }
    LHS* lhs = new LHS(base, new_count);

    while (true) {
      token = lexer_.NextAndRewind();
      switch (token.type()) {
        case Token::TK_LPAREN: {  // (
          AST* ast = ParseArguments();
          if (ast->IsIllegal()) {
            delete lhs;
            return ast;
          }
          assert(ast->type() == AST::AST_EXPR_ARGS);
          Arguments* args = static_cast<Arguments*>(ast);
          lhs->AddArguments(args);
          break;
        }
        case Token::TK_LBRACK: {  // [
          lexer_.Next();  // skip [
          AST* index = ParseExpression(false);
          if (index->IsIllegal()) {
            delete lhs;
            return index;
          }
          token = lexer_.Next();  // skip ]
          if (token.type() != Token::TK_RBRACK) {
            delete lhs;
            delete index;
            goto error;
          }
          lhs->AddIndex(index);
          break;
        }
        case Token::TK_DOT: {  // .
          lexer_.Next();  // skip .
          token = lexer_.Next();  // skip IdentifierName
          if (!token.IsIdentifierName()) {
            delete lhs;
            goto error;
          }
          lhs->AddProp(token);
          break;
        }
        default:
          lhs->SetSource(SOURCE_PARSED);
          return lhs;
      }
    }
error:
    return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
  }

  AST* ParseArguments() {
    START_POS;
    assert(lexer_.Next().type() == Token::TK_LPAREN);
    std::vector<AST*> args;
    AST* arg;
    Arguments* arg_ast;
    Token token = lexer_.NextAndRewind();
    es::test::PrintSource("token:", token.source());
    if (token.type() != Token::TK_RPAREN) {
      arg = ParseAssignmentExpression(false);
      if (arg->IsIllegal())
        return arg;
      args.emplace_back(arg);
      token = lexer_.NextAndRewind();
    }
    while (token.type() != Token::TK_RPAREN) {
      if (token.type() != Token::TK_COMMA) {
        goto error;
      }
      lexer_.Next();  // skip ,
      arg = ParseAssignmentExpression(false);
      if (arg->IsIllegal()) {
        for (auto arg : args)
          delete arg;
        return arg;
      }
      args.emplace_back(arg);
      token = lexer_.NextAndRewind();
    }
    assert(lexer_.Next().type() == Token::TK_RPAREN);  // skip )
    arg_ast = new Arguments(args);
    arg_ast->SetSource(SOURCE_PARSED);
    return arg_ast;
error:
    for (auto arg : args)
      delete arg;
    return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
  }

  AST* ParseProgram() {
    START_POS;
    Program* prog = new Program();
    AST* element;

    Token token = lexer_.NextAndRewind();
    while (token.type() != Token::TK_EOS) {
      if (token.source() == u"function") {
        assert(false);
        // element = ParseFunctionDeclaration();
        //   if (element->IsIllegal()) {
        //   delete prog;
        //   return element;
        // }
        // prog->AddFunctionDecl(element);
      } else {
        element = ParseStatement();
          if (element->IsIllegal()) {
          delete prog;
          return element;
        }
        prog->AddStatement(element);
      }
      token = lexer_.NextAndRewind();
    }
    prog->SetSource(SOURCE_PARSED);
    return prog;
  }

  AST* ParseStatement() {
    START_POS;
    Token token = lexer_.NextAndRewind();

    switch (token.type()) {
      case Token::TK_LBRACE:  // {
        return ParseBlockStatement();
      case Token::TK_SEMICOLON:  // ;
        return new AST(AST::AST_STMT_EMPTY, u";");
      case Token::TK_KEYWORD: {
        if (token.source() == u"var")
          return ParseVariableStatement(false);
        else if (token.source() == u"if")
          return ParseIfStatement();
        else if (token.source() == u"do")
          return ParseDoWhileStatement();
        else if (token.source() == u"while")
          return ParseWhileStatement();
        else if (token.source() == u"for")
          return ParseForStatement();
        else if (token.source() == u"continue")
          return ParseContinueStatement();
        else if (token.source() == u"break")
          return ParseBreakStatement();
        else if (token.source() == u"return")
          return ParseReturnStatement();
        else if (token.source() == u"with")
          return ParseWithStatement();
        else if (token.source() == u"switch")
          return ParseSwitchStatement();
        else if (token.source() == u"throw")
          return ParseThrowStatement();
        else if (token.source() == u"try")
          return ParseTryStatement();
        else if (token.source() == u"debugger") {
          lexer_.Next();
          if (!lexer_.TrySkipSemiColon()) {
            goto error;
          }
          return new AST(AST::AST_STMT_DEBUG, SOURCE_PARSED);
        }
        break;
      }
      case Token::TK_IDENT: {
        size_t old_pos = lexer_.Pos();
        Token old_token = lexer_.Last();
        Token colon = lexer_.Next();
        lexer_.Rewind(old_pos, old_token);
        if (colon.type() == Token::TK_COLON)
          return ParseLabelStatement();
      }
      default:
        break;
    }
    return ParseExpressionStatement();
error:
    return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
  }

  AST* ParseBlockStatement() {
    START_POS;
    assert(lexer_.Next().type() == Token::TK_LBRACE);
    Block* block = new Block();
    Token token = lexer_.NextAndRewind();
    while (token.type() != Token::TK_RBRACE) {
      AST* stmt = ParseStatement();
      if (stmt->IsIllegal()) {
        delete block;
        return stmt;
      }
      block->AddStatement(stmt);
      token = lexer_.NextAndRewind();
    }
    assert(token.type() == Token::TK_RBRACE);
    lexer_.Next();
    block->SetSource(SOURCE_PARSED);
    return block;
  }

  AST* ParseVariableDeclaration(bool no_in) {
    START_POS;
    Token ident = lexer_.Next();
    AST* init;
    assert(ident.IsIdentifier());
    if (lexer_.Next().type() != Token::TK_ASSIGN) {
      goto error;
    }
    init = ParseAssignmentExpression(no_in);
    if (init->IsIllegal())
      return init;
    return new VarDecl(ident, init, SOURCE_PARSED);
error:
    return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
  }

  AST* ParseVariableStatement(bool no_in) {
    START_POS;
    assert(lexer_.Next().source() == u"var");
    VarStmt* var_stmt = new VarStmt();
    AST* decl;
    Token token = lexer_.NextAndRewind();
    if (!token.IsIdentifier()) {
      lexer_.Next();
      goto error;
    }
    // Similar to ParseExpression
    decl = ParseVariableDeclaration(no_in);
    if (decl->IsIllegal()) {
      delete var_stmt;
      return decl;
    }
    token = lexer_.NextAndRewind();
    while (token.type() == Token::TK_COMMA) {
      lexer_.Next();  // skip ,
      decl = ParseVariableDeclaration(no_in);
      if (decl->IsIllegal()) {
        delete var_stmt;
        return decl;
      }
      var_stmt->AddDecl(decl);
      token = lexer_.NextAndRewind();
    }

    if (!lexer_.TrySkipSemiColon())
      goto error;

    var_stmt->SetSource(SOURCE_PARSED);
    return var_stmt;
error:
    delete var_stmt;
    return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
  }

  AST* ParseExpressionStatement() {
    START_POS;
    Token token = lexer_.NextAndRewind();
    assert(token.type() != Token::TK_LBRACE && token.source() != u"function");
    AST* exp = ParseExpression(false);
    if (exp->IsIllegal())
      return exp;
    if (!lexer_.TrySkipSemiColon()) {
      delete exp;
      return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
    }
    return exp;
  }

  AST* ParseIfStatement() {

  }

  AST* ParseDoWhileStatement() {

  }

  AST* ParseWhileStatement() {

  }

  AST* ParseForStatement() {

  }

  AST* ParseContinueStatement() {
    START_POS;
    assert(lexer_.Next().source() == u"continue");
    Token ident = Token(Token::TK_NOT_FOUND, u"");
    if (!lexer_.TrySkipSemiColon()) {
      ident = lexer_.NextAndRewind();
      if (ident.IsIdentifier()) {
        lexer_.Next();  // Skip Identifier
      }
      if (!lexer_.TrySkipSemiColon()) {
        return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
      }
    }
    return new Continue(ident, SOURCE_PARSED);
  }

  // TODO(zhuzilin) Shall I merge the continue and break?
  AST* ParseBreakStatement() {
    START_POS;
    assert(lexer_.Next().source() == u"break");
    Token ident = Token(Token::TK_NOT_FOUND, u"");
    if (!lexer_.TrySkipSemiColon()) {
      ident = lexer_.NextAndRewind();
      if (ident.IsIdentifier()) {
        lexer_.Next();  // Skip Identifier
      }
      if (!lexer_.TrySkipSemiColon()) {
        return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
      }
    }
    return new Break(ident, SOURCE_PARSED);
  }

  AST* ParseReturnStatement() {
    START_POS;
    assert(lexer_.Next().source() == u"return");
    AST* expr = nullptr;
    if (!lexer_.TrySkipSemiColon()) {
      expr = ParseExpression(false);
      if (expr->IsIllegal()) {
        return expr;
      }
      if (!lexer_.TrySkipSemiColon()) {
        delete expr;
        return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
      }
    }
    return new Return(expr, SOURCE_PARSED);
  }

  AST* ParseThrowStatement() {
    START_POS;
    assert(lexer_.Next().source() == u"throw");
    AST* expr = nullptr;
    if (!lexer_.TrySkipSemiColon()) {
      expr = ParseExpression(false);
      if (expr->IsIllegal()) {
        return expr;
      }
      if (!lexer_.TrySkipSemiColon()) {
        delete expr;
        return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
      }
    }
    return new Throw(expr, SOURCE_PARSED);
  }

  AST* ParseWithStatement() {

  }

  AST* ParseSwitchStatement() {

  }

  AST* ParseTryStatement() {

  }

  AST* ParseLabelStatement() {

  }

 private:
  std::u16string_view source_;
  Lexer lexer_;
};

}  // namespace es

#endif  // ES_PARSER_H