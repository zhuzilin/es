#ifndef ES_PARSER_PARSER_H
#define ES_PARSER_PARSER_H

#include <stack>

#include <es/parser/lexer.h>
#include <es/parser/ast.h>

#include <es/utils/helper.h>

#define START_POS size_t start = lexer_.Pos()
#define SOURCE_PARSED source_.substr(start, lexer_.Pos() - start), start, lexer_.Pos()
#define TOKEN_SOURCE token.source(), token.start(), token.end()

namespace es {

class Parser {
 public:
  Parser(std::u16string source) : source_(source), lexer_(source) {
    EnterFunctionScope();
  }

  AST* ParsePrimaryExpression() {
    Token token = lexer_.NextAndRewind();
    switch (token.type()) {
      case Token::TK_KEYWORD:
        if (token.source() == u"this") {
          lexer_.Next();
          return new AST(AST::AST_EXPR_THIS, TOKEN_SOURCE);
        }
        goto error;
      case Token::TK_STRICT_FUTURE:
        lexer_.Next();
        return new AST(AST::AST_EXPR_STRICT_FUTURE, TOKEN_SOURCE);
      case Token::TK_IDENT:
        lexer_.Next();
        return new AST(AST::AST_EXPR_IDENT, TOKEN_SOURCE);
      case Token::TK_NULL:
        lexer_.Next();
        return new AST(AST::AST_EXPR_NULL, TOKEN_SOURCE);
      case Token::TK_BOOL:
        lexer_.Next();
        return new AST(AST::AST_EXPR_BOOL, TOKEN_SOURCE);
      case Token::TK_NUMBER:
        lexer_.Next();
        return new AST(AST::AST_EXPR_NUMBER, TOKEN_SOURCE);
      case Token::TK_STRING:
        lexer_.Next();
        return new AST(AST::AST_EXPR_STRING, TOKEN_SOURCE);
      case Token::TK_LBRACK:  // [
        return ParseArrayLiteral();
      case Token::TK_LBRACE:  // {
        return ParseObjectLiteral();
      case Token::TK_LPAREN: { // (
        lexer_.Next();   // skip (
        AST* value = ParseExpression(false);
        if (value->IsIllegal())
          return value;
        if (lexer_.Next().type() != Token::TK_RPAREN) {
          delete value;
          goto error;
        }
        return new Paren(value, value->source(), value->start(), value->end());
      }
      case Token::TK_DIV_ASSIGN:
      case Token::TK_DIV: {  // /
        lexer_.Next(); // skip /
        lexer_.Back(); // back to /
        if (token.type() == Token::TK_DIV_ASSIGN)
          lexer_.Back();
        std::u16string pattern, flag;
        token = lexer_.ScanRegExpLiteral(pattern, flag);
        if (token.type() == Token::TK_REGEX) {
          return new RegExpLiteral(pattern, flag, TOKEN_SOURCE);
        } else {
          goto error;
        }
        break;
      }
      default:
        goto error;
    }

error:
    return new AST(AST::AST_ILLEGAL, TOKEN_SOURCE);
  }

  template<flag_t flag = 0>
  bool ParseFormalParameterList(std::vector<Handle<String>>& params) {
    if (!lexer_.NextAndRewind().IsIdentifier()) {
      // This only happens in new Function(...)
      params = {};
      return lexer_.Next().type() == Token::TK_EOS;
    }
    params.emplace_back(String::New<flag>(lexer_.Next().source()));
    Token token = lexer_.NextAndRewind();
    // NOTE(zhuzilin) the EOS is for new Function("a,b,c", "")
    while (token.type() != Token::TK_RPAREN && token.type() != Token::TK_EOS) {
      if (token.type() != Token::TK_COMMA) {
        params = {};
        return false;
      }
      lexer_.Next();  // skip ,
      token = lexer_.Next();
      if (!token.IsIdentifier()) {
        params = {};
        return false;
      }
      params.emplace_back(String::New<flag>(token.source()));
      token = lexer_.NextAndRewind();
    }
    return true;
  }

  AST* ParseFunction(bool must_be_named) {
    START_POS;
    assert(lexer_.Next().source() == u"function");

    Handle<String> name;
    std::vector<Handle<String>> params;
    AST* body;
    Function* func;

    // Identifier_opt
    Token token = lexer_.Next();
    if (token.IsIdentifier()) {
      name = String::New<GCFlag::CONST>(token.source_ref());
      token = lexer_.Next();  // skip "("
    } else if (must_be_named) {
      goto error;
    }
    if (token.type() != Token::TK_LPAREN) {
      goto error;
    }
    token = lexer_.NextAndRewind();
    if (token.IsIdentifier()) {
      if (!ParseFormalParameterList<GCFlag::CONST>(params)) {
        goto error;
      }
    }
    if (lexer_.Next().type() != Token::TK_RPAREN) {  // skip )
      goto error;
    }
    token = lexer_.Next();  // skip {
    if (token.type() != Token::TK_LBRACE) {
      goto error;
    }
    body = ParseFunctionBody();
    if (body->IsIllegal())
      return body;

    token = lexer_.Next();  // skip }
    if (token.type() != Token::TK_RBRACE) {
      goto error;
    }

    func = new Function(name, std::move(params), body, SOURCE_PARSED);

    return func;
error:
    return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
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
            delete array;
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
  }

  AST* ParseObjectLiteral() {
    START_POS;
    assert(lexer_.Next().type() == Token::TK_LBRACE);

    ObjectLiteral* obj = new ObjectLiteral();
    Token token = lexer_.NextAndRewind();
    while (token.type() != Token::TK_RBRACE) {
      if (token.IsPropertyName()) {
        lexer_.Next();
        if ((token.source() == u"get" || token.source() == u"set") &&
            lexer_.NextAndRewind().IsPropertyName()) {
          START_POS;
          ObjectLiteral::Property::Type type;
          if (token.source() == u"get")
            type = ObjectLiteral::Property::GET;
          else
            type = ObjectLiteral::Property::SET;
          Token key = lexer_.Next();  // skip property name
          if (!key.IsPropertyName()) {
            goto error;
          }
          if (lexer_.Next().type() != Token::TK_LPAREN) {
            goto error;
          }
          std::vector<Handle<String>> params;
          if (type == ObjectLiteral::Property::SET) {
            Token param = lexer_.Next();
            if (!param.IsIdentifier()) {
              goto error;
            }
            params.emplace_back(String::New<GCFlag::CONST>(param.source_ref()));
          }
          if (lexer_.Next().type() != Token::TK_RPAREN) { // Skip )
            goto error;
          }
          if (lexer_.Next().type() != Token::TK_LBRACE) { // Skip {
            goto error;
          }
          AST* body = ParseFunctionBody();
          if (body->IsIllegal()) {
            delete obj;
            return body;
          }
          if (lexer_.Next().type() != Token::TK_RBRACE) { // Skip }
            delete body;
            goto error;
          }
          Function* value = new Function(
            Handle<String>(), std::move(params), body, SOURCE_PARSED);
          obj->AddProperty(ObjectLiteral::Property(key, value, type));
        } else {
          if (lexer_.Next().type() != Token::TK_COLON)
            goto error;
          AST* value = ParseAssignmentExpression(false);
          if (value->type() == AST::AST_ILLEGAL)
            goto error;
          obj->AddProperty(ObjectLiteral::Property(token, value, ObjectLiteral::Property::NORMAL));
        }
      } else {
        lexer_.Next();
        goto error;
      }
      token = lexer_.NextAndRewind();
      if (token.type() == Token::TK_COMMA) {
        lexer_.Next();  // Skip ,
        token = lexer_.NextAndRewind();
      }
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

    AST* element = ParseAssignmentExpression(no_in);
    if (element->IsIllegal()) {
      return element;
    }
    // NOTE(zhuzilin) If expr has only one element, then just return the element.
    Token token = lexer_.NextAndRewind();
    if (token.type() != Token::TK_COMMA) {
      return element;
    }

    Expression* expr = new Expression();
    expr->AddElement(element);
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
    START_POS;

    AST* lhs = ParseConditionalExpression(no_in);
    if (lhs->IsIllegal())
      return lhs;

    // Not LeftHandSideExpression
    if (lhs->type() != AST::AST_EXPR_LHS) {
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

    return new Binary(lhs, rhs, op, SOURCE_PARSED);
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
    // NOTE(zhuzilin) !!a = !(!a)
    if (prefix_op.UnaryPrefixPriority() >= priority) {
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
        lhs = new Binary(lhs, rhs, binary_op, SOURCE_PARSED);
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
      base = ParseFunction(false);
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

  AST* ParseFunctionBody(Token::Type ending_token_type = Token::TK_RBRACE) {
    return ParseProgramOrFunctionBody(ending_token_type, AST::AST_FUNC_BODY);
  }

  AST* ParseProgram() {
    return ParseProgramOrFunctionBody(Token::TK_EOS, AST::AST_PROGRAM);
  }

  AST* ParseProgramOrFunctionBody(Token::Type ending_token_type, AST::Type program_or_function) {
    START_POS;
    // 14.1
    bool strict = false;
    size_t old_pos = lexer_.Pos();
    Token old_token = lexer_.Last();
    Token token = lexer_.NextAndRewind();
    if (token.source() == u"\"use strict\"" || token.source() == u"'use strict'") {
      lexer_.Next();
      if (lexer_.TrySkipSemiColon()) {
        strict = true;
      } else {
        lexer_.Rewind(old_pos, old_token);
      }
    }

    ProgramOrFunctionBody* prog = new ProgramOrFunctionBody(program_or_function, strict);
    EnterFunctionScope();

    AST* element;
    token = lexer_.NextAndRewind();
    while (token.type() != ending_token_type) {
      if (token.source() == u"function") {
        element = ParseFunction(true);
          if (element->IsIllegal()) {
          delete prog;
          return element;
        }
        prog->AddFunctionDecl(element);
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
    assert(token.type() == ending_token_type);
    prog->SetUseArguments(lexer_.meet_arguments_ident_);
    auto info = ExitFunctionScope();
    prog->SetVarDecls(std::move(info.var_decls_));
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
        lexer_.Next();
        return new AST(AST::AST_STMT_EMPTY, TOKEN_SOURCE);
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
            lexer_.Next();
            goto error;
          }
          return new AST(AST::AST_STMT_DEBUG, SOURCE_PARSED);
        }
        break;
      }
      case Token::TK_STRICT_FUTURE:
      case Token::TK_IDENT: {
        size_t old_pos = lexer_.Pos();
        Token old_token = lexer_.Last();
        lexer_.Next();
        Token colon = lexer_.Next();
        lexer_.Rewind(old_pos, old_token);
        if (colon.type() == Token::TK_COLON)
          return ParseLabelledStatement();
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
    if (lexer_.NextAndRewind().type() != Token::TK_ASSIGN) {
      VarDecl* var_decl = new VarDecl(ident, SOURCE_PARSED);
      function_scope_stack_.top().var_decls_.emplace_back(var_decl);
      return var_decl;
    }
    lexer_.Next();  // skip =
    init = ParseAssignmentExpression(no_in);
    if (init->IsIllegal())
      return init;
    VarDecl* var_decl =  new VarDecl(ident, init, SOURCE_PARSED);
    function_scope_stack_.top().var_decls_.emplace_back(var_decl);
    return var_decl;
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
    var_stmt->AddDecl(decl);
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
    if (!lexer_.TrySkipSemiColon()) {
      lexer_.Next();
      goto error;
    }

    var_stmt->SetSource(SOURCE_PARSED);
    return var_stmt;
error:
    delete var_stmt;
    return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
  }

  AST* ParseExpressionStatement() {
    START_POS;
    Token token = lexer_.NextAndRewind();
    if (token.source() == u"function") {
      lexer_.Next();
      return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
    }
    assert(token.type() != Token::TK_LBRACE);
    AST* exp = ParseExpression(false);
    if (exp->IsIllegal())
      return exp;
    if (!lexer_.TrySkipSemiColon()) {
      lexer_.Next();
      delete exp;
      return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
    }
    return exp;
  }

  AST* ParseIfStatement() {
    START_POS;
    AST* cond;
    AST* if_block;

    assert(lexer_.Next().source() == u"if");
    lexer_.Next();   // skip (
    cond = ParseExpression(false);
    if (cond->IsIllegal())
      return cond;
    if (lexer_.Next().type() != Token::TK_RPAREN) {  // skip )
      delete cond;
      goto error;
    }
    if_block = ParseStatement();
    if (if_block->IsIllegal()) {
      delete cond;
      return if_block;
    }
    if (lexer_.NextAndRewind().source() == u"else") {
      lexer_.Next();  // skip else
      AST* else_block = ParseStatement();
      if (else_block->IsIllegal()) {
        delete cond;
        delete if_block;
        return else_block;
      }
      return new If(cond, if_block, else_block, SOURCE_PARSED);
    }
    return new If(cond, if_block, SOURCE_PARSED);
    
error:
    return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
  }

  AST* ParseDoWhileStatement() {
    START_POS;
    assert(lexer_.Next().source() == u"do");
    AST* cond;
    AST* loop_block;
    loop_block = ParseStatement();
    if (loop_block->IsIllegal()) {
      return loop_block;
    }
    if (lexer_.Next().source() != u"while") {  // skip while
      delete loop_block;
      goto error;
    }
    if (lexer_.Next().type() != Token::TK_LPAREN) {  // skip (
      delete loop_block;
      goto error;
    }
    cond = ParseExpression(false);
    if (cond->IsIllegal()) {
      delete loop_block;
      return cond;
    }
    if (lexer_.Next().type() != Token::TK_RPAREN) {  // skip )
      delete cond;
      goto error;
    }
    if (!lexer_.TrySkipSemiColon()) {
      lexer_.Next();
      delete cond;
      delete loop_block;
      goto error;
    }
    return new DoWhile(cond, loop_block, SOURCE_PARSED);
error:
    return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
  }

  AST* ParseWhileStatement() {
    return ParseWhileOrWithStatement(u"while", AST::AST_STMT_WHILE);
  }

  AST* ParseWithStatement() {
    return ParseWhileOrWithStatement(u"with", AST::AST_STMT_WITH);
  }

  AST* ParseWhileOrWithStatement(std::u16string keyword, AST::Type type) {
    START_POS;
    assert(lexer_.Next().source() == keyword);
    AST* expr;
    AST* stmt;
    if (lexer_.Next().type() != Token::TK_LPAREN) { // skip (
      goto error;
    }
    expr = ParseExpression(false);
    if (expr->IsIllegal())
      return expr;
    if (lexer_.Next().type() != Token::TK_RPAREN) {  // skip )
      delete expr;
      goto error;
    }
    stmt = ParseStatement();
    if (stmt->IsIllegal()) {
      delete expr;
      return stmt;
    }
    return new WhileOrWith(type, expr, stmt, SOURCE_PARSED);
error:
    return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
  }

  AST* ParseForStatement() {
    START_POS;
    assert(lexer_.Next().source() == u"for");
    Token token = lexer_.Next();  // skip (
    AST* expr0;
    if (token.type() != Token::TK_LPAREN)
      goto error;
    token = lexer_.NextAndRewind();
    if (token.IsSemiColon()) {
      return ParseForStatement({}, start);  // for (;
    } else if (token.source() == u"var") {
      lexer_.Next();  // skip var
      std::vector<AST*> expr0s;

      // NOTE(zhuzilin) the starting token for ParseVariableDeclaration
      // must be identifier. This is for better error code.
      if (!lexer_.NextAndRewind().IsIdentifier()) {
        goto error;
      }
      expr0 = ParseVariableDeclaration(true);
      if (expr0->IsIllegal())
        return expr0;

      token = lexer_.NextAndRewind();
      if (token.source() == u"in")  // var VariableDeclarationNoIn in
        return ParseForInStatement(expr0, start);

      expr0s.emplace_back(expr0);
      while (!token.IsSemiColon()) {
        // NOTE(zhuzilin) the starting token for ParseVariableDeclaration
        // must be identifier. This is for better error code.
        if (lexer_.Next().type() != Token::TK_COMMA ||  // skip ,
            !lexer_.NextAndRewind().IsIdentifier()) {
          for (auto expr : expr0s)
            delete expr;
          goto error;
        }

        expr0 = ParseVariableDeclaration(true);
        if (expr0->IsIllegal()) {
          for (auto expr : expr0s)
            delete expr;
          return expr0;
        }
        expr0s.emplace_back(expr0);
        token = lexer_.NextAndRewind();
      }
      return ParseForStatement(expr0s, start);  // var VariableDeclarationListNoIn;
    } else {
      expr0 = ParseExpression(true);
      if (expr0->IsIllegal()) {
        return expr0;
      }
      token = lexer_.NextAndRewind();
      if (token.IsSemiColon()) {
        return ParseForStatement({expr0}, start);  // for ( ExpressionNoIn;
      } else if (token.source() == u"in" &&
                 expr0->type() == AST::AST_EXPR_LHS) {  // for ( LeftHandSideExpression in
        return ParseForInStatement(expr0, start);  
      } else {
        delete expr0;
        goto error;
      }
    }
error:
    return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
  }

  AST* ParseForStatement(std::vector<AST*> expr0s, size_t start) {
    assert(lexer_.Next().IsSemiColon());
    AST* expr1 = nullptr;
    AST* expr2 = nullptr;
    AST* stmt;
    Token token = lexer_.NextAndRewind();
    if (!token.IsSemiColon()) {
      expr1 = ParseExpression(false);  // for (xxx; Expression
      if (expr1->IsIllegal()) {
        for (auto expr : expr0s) {
          delete expr;
        }
        return expr1;
      }
    }

    if (!lexer_.Next().IsSemiColon()) {  // skip ;
      lexer_.Next();
      goto error;
    }

    token = lexer_.NextAndRewind();
    if (token.type() != Token::TK_RPAREN) {
      expr2 = ParseExpression(false);  // for (xxx; xxx; Expression
      if (expr2->IsIllegal()) {
        for (auto expr : expr0s) {
          delete expr;
        }
        if (expr1 != nullptr)
          delete expr1;
        return expr2;
      }
    }

    if (lexer_.Next().type() != Token::TK_RPAREN) {  // skip )
      lexer_.Next();
      goto error;
    }

    stmt = ParseStatement();
    if (stmt->IsIllegal()) {
      for (auto expr : expr0s) {
        delete expr;
      }
      if (expr1 != nullptr)
        delete expr1;
      if (expr2 != nullptr)
        delete expr2;
      return stmt;
    }

    return new For(expr0s, expr1, expr2, stmt, SOURCE_PARSED);
error:
    for (auto expr : expr0s) {
      delete expr;
    }
    if (expr1 != nullptr)
      delete expr1;
    if (expr2 != nullptr)
      delete expr2;
    return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
  }

  AST* ParseForInStatement(AST* expr0, size_t start) {
    assert(lexer_.Next().source() == u"in");
    AST* expr1 = ParseExpression(false);  // for ( xxx in Expression
    AST* stmt;
    if (expr1->IsIllegal()) {
      delete expr0;
      return expr1;
    }

    if (lexer_.Next().type() != Token::TK_RPAREN) {  // skip )
      lexer_.Next();
      goto error;
    }

    stmt = ParseStatement();
    if (stmt->IsIllegal()) {
      delete expr0;
      delete expr1;
      return stmt;
    }
    return new ForIn(expr0, expr1, stmt, SOURCE_PARSED);
error:
    delete expr0;
    delete expr1;
    return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
  }

  AST* ParseContinueStatement() {
    return ParseContinueOrBreakStatement(u"continue", AST::AST_STMT_CONTINUE);
  }

  AST* ParseBreakStatement() {
    return ParseContinueOrBreakStatement(u"break", AST::AST_STMT_BREAK);
  }

  AST* ParseContinueOrBreakStatement(std::u16string keyword, AST::Type type) {
    START_POS;
    assert(lexer_.Next().source() == keyword);
    if (!lexer_.TrySkipSemiColon()) {
      Token ident = lexer_.NextAndRewind();
      if (ident.IsIdentifier()) {
        lexer_.Next();  // Skip Identifier
      }
      if (!lexer_.TrySkipSemiColon()) {
        lexer_.Next();
        return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
      }
      return new ContinueOrBreak(type, ident, SOURCE_PARSED);
    }
    return new ContinueOrBreak(type, SOURCE_PARSED);
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
        lexer_.Next();
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
        lexer_.Next();
        delete expr;
        return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
      }
    }
    return new Throw(expr, SOURCE_PARSED);
  }

  AST* ParseSwitchStatement() {
    START_POS;
    Switch* switch_stmt = new Switch();
    AST* expr;
    Token token = lexer_.Last();
    assert(lexer_.Next().source() == u"switch");
    if (lexer_.Next().type() != Token::TK_LPAREN) { // skip (
      goto error;
    }
    expr = ParseExpression(false);
    if (expr->IsIllegal()) {
      delete switch_stmt;
      return expr;
    }
    if (lexer_.Next().type() != Token::TK_RPAREN) {  // skip )
      delete expr;
      goto error;
    }
    switch_stmt->SetExpr(expr);
    if (lexer_.Next().type() != Token::TK_LBRACE) { // skip {
      goto error;
    }
    // Loop for parsing CaseClause
    token = lexer_.NextAndRewind();
    while (token.type() != Token::TK_RBRACE) {
      AST* case_expr = nullptr;
      std::vector<AST*> stmts;
      std::u16string type = token.source();
      if (type == u"case") {
        lexer_.Next();  // skip case
        case_expr = ParseExpression(false);
        if (case_expr->IsIllegal()) {
          delete switch_stmt;
          return case_expr;
        }
      } else if (type == u"default") {
        lexer_.Next();  // skip default
        // can only have one default.
        if (switch_stmt->has_default_clause())
          goto error;
      } else {
        lexer_.Next();
        goto error;
      }
      if (lexer_.Next().type() != Token::TK_COLON) { // skip :
        delete case_expr;
        goto error;
      }
      // parse StatementList
      token = lexer_.NextAndRewind();
      while (token.source() != u"case" && token.source() != u"default" &&
             token.type() != Token::TK_RBRACE) {
        AST* stmt = ParseStatement();
        if (stmt->IsIllegal()) {
          for (auto s : stmts) {
            delete s;
          }
          delete switch_stmt;
          return stmt;
        }
        stmts.emplace_back(stmt);
        token = lexer_.NextAndRewind();
      }
      if (type == u"case") {
        if (switch_stmt->has_default_clause()) {
          switch_stmt->AddAfterDefaultCaseClause(Switch::CaseClause(case_expr, stmts));
        } else {
          switch_stmt->AddBeforeDefaultCaseClause(Switch::CaseClause(case_expr, stmts));
        }
      } else {
        switch_stmt->SetDefaultClause(stmts);
      }
      token = lexer_.NextAndRewind();
    }
    assert(token.type() == Token::TK_RBRACE);
    assert(lexer_.Next().type() == Token::TK_RBRACE);
    switch_stmt->SetSource(SOURCE_PARSED);
    return switch_stmt;
error:
    delete switch_stmt;
    return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
  }

  AST* ParseTryStatement() {
    START_POS;
    assert(lexer_.Next().source() == u"try");

    AST* try_block = nullptr;
    Token catch_ident(Token::TK_NOT_FOUND, source_, 0, 0);
    AST* catch_block = nullptr;
    AST* finally_block = nullptr;

    if (lexer_.NextAndRewind().type() != Token::TK_LBRACE) {
      lexer_.Next();
      goto error;
    }
    try_block = ParseBlockStatement();
    if (try_block->IsIllegal())
      return try_block;
    if (lexer_.NextAndRewind().source() == u"catch") {
      lexer_.Next();  // skip catch
      if (lexer_.Next().type() != Token::TK_LPAREN) {  // skip (
        delete try_block;
        goto error;
      }
      catch_ident = lexer_.Next();  // skip identifier
      if (!catch_ident.IsIdentifier()) {
        goto error;
      }
      if (lexer_.Next().type() != Token::TK_RPAREN) {  // skip )
        delete try_block;
        goto error;
      }
      catch_block = ParseBlockStatement();
      if (catch_block->IsIllegal()) {
        delete try_block;
        return catch_block;
      }
    }
    if (lexer_.NextAndRewind().source() == u"finally") {
      lexer_.Next();  // skip finally
      if (lexer_.NextAndRewind().type() != Token::TK_LBRACE) {
        lexer_.Next();
        goto error;
      }
      finally_block = ParseBlockStatement();
      if (finally_block->IsIllegal()) {
        delete try_block;
        if (catch_block != nullptr)
          delete catch_block;
        return finally_block;
      }
    }
    if (catch_block == nullptr && finally_block == nullptr) {
      goto error;
    } else if (finally_block == nullptr) {
      assert(catch_block != nullptr && catch_ident.IsIdentifier());
      return new Try(try_block, catch_ident, catch_block, SOURCE_PARSED);
    } else if (catch_block == nullptr) {
      assert(finally_block != nullptr);
      return new Try(try_block, finally_block, SOURCE_PARSED);
    }
    assert(catch_block != nullptr && catch_ident.IsIdentifier());
    assert(finally_block != nullptr);
    return new Try(try_block, catch_ident, catch_block, finally_block, SOURCE_PARSED);
error:
    if (try_block != nullptr)
      delete try_block;
    if (catch_block != nullptr)
      delete try_block;
    if (finally_block != nullptr)
      delete finally_block;
    return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
  }

  AST* ParseLabelledStatement() {
    START_POS;
    Token ident = lexer_.Next();  // skip identifier
    assert(lexer_.Next().type() == Token::TK_COLON);  // skip colon
    AST* stmt = ParseStatement();
    if (stmt->IsIllegal()) {
      return stmt;
    }
    return new LabelledStmt(ident, stmt, SOURCE_PARSED);
  }

  struct FunctionScopeInfo {
    std::vector<VarDecl*> var_decls_;
    bool use_arguments_;
  };

  void EnterFunctionScope() {
    if (function_scope_stack_.size()) {
      function_scope_stack_.top().use_arguments_ = lexer_.meet_arguments_ident_;
    }
    FunctionScopeInfo info;
    function_scope_stack_.push(info);
  }

  FunctionScopeInfo ExitFunctionScope() {
    FunctionScopeInfo info = function_scope_stack_.top();
    function_scope_stack_.pop();
    lexer_.meet_arguments_ident_ = function_scope_stack_.top().use_arguments_;
    return info;
  }

 private:
  std::u16string source_;
  Lexer lexer_;

  std::stack<FunctionScopeInfo> function_scope_stack_;
};

}  // namespace es

#endif  // ES_PARSER_PARSER_H