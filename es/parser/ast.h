#ifndef ES_PARSER_AST_H
#define ES_PARSER_AST_H

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <utility>

#include <es/parser/token.h>

namespace es {

class AST {
 public:
  enum Type {
    AST_EXPR_THIS,
    AST_EXPR_IDENT,

    AST_EXPR_NULL,
    AST_EXPR_BOOL,
    AST_EXPR_NUMBER,
    AST_EXPR_STRING,
    AST_EXPR_REGEX,

    AST_EXPR_ARRAY,
    AST_EXPR_OBJ,

    AST_EXPR_PAREN,  // ( Expression )

    AST_EXPR_BINARY,
    AST_EXPR_UNARY,
    AST_EXPR_TRIPLE,

    AST_EXPR_ARGS,
    AST_EXPR_LHS,

    AST_EXPR,

    AST_FUNC,

    AST_STMT_EMPTY,
    AST_STMT_BLOCK,
    AST_STMT_IF,
    AST_STMT_WHILE,
    AST_STMT_FOR,
    AST_STMT_FOR_IN,
    AST_STMT_WITH,
    AST_STMT_DO_WHILE,
    AST_STMT_TRY,

    AST_STMT_VAR,
    AST_STMT_VAR_DECL,

    AST_STMT_CONTINUE,
    AST_STMT_BREAK,
    AST_STMT_RETURN,
    AST_STMT_THROW,

    AST_STMT_SWITCH,

    AST_STMT_LABEL,
    AST_STMT_DEBUG,

    AST_PROGRAM,
    AST_FUNC_BODY,

    AST_ILLEGAL,
  };

  AST(Type type, std::u16string source = u"") : type_(type), source_(source) {}
  virtual ~AST() {};

  Type type() { return type_; }
  std::u16string source() { return source_; }

  void SetSource(std::u16string source) { source_ = source; }

  bool IsIllegal() { return type_ == AST_ILLEGAL; }

 private:
  Type type_;
  std::u16string source_;
};

class ArrayLiteral : public AST {
 public:
  ArrayLiteral() : AST(AST_EXPR_ARRAY), len_(0) {}

  ~ArrayLiteral() override {
    for (auto pair : elements_) {
      delete pair.second;
    }
  }

  size_t length() { return len_; }

  void AddElement(AST* element) {
    if (element != nullptr) {
      elements_.emplace_back(len_, element);
    }
    len_++;
  }

 private:
  std::vector<std::pair<size_t, AST*>> elements_;
  size_t len_;
};

class ObjectLiteral : public AST {
 public:
  ObjectLiteral() : AST(AST_EXPR_OBJ) {}

  ~ObjectLiteral() override {
    for (auto property : properties_) {
      delete property.value;
    }
  }

  struct Property {
    enum Type {
      NORMAL = 0,
      GET,
      SET,
    };

    Property(Token k, AST* v, Type t) : key(k), type(t), value(v) {}

    Token key;
    AST* value;
    Type type;
  };

  void AddProperty(Property p) {
    properties_.emplace_back(p);
  }

  std::vector<Property> properties() { return properties_; }

  size_t length() { return properties_.size(); }

 private:
  std::vector<Property> properties_;
};

class Paren : public AST {
 public:
  Paren(AST* expr, std::u16string source) :
    AST(AST_EXPR_PAREN, source), expr_(expr) {}

  AST* expr() { return expr_; }

 private:
  AST* expr_;
};

class Binary : public AST {
 public:
  Binary(AST* lhs, AST* rhs, Token op) :
    AST(AST_EXPR_BINARY), lhs_(lhs), rhs_(rhs), op_(op) {}

  ~Binary() override {
    delete lhs_;
    delete rhs_;
  }

  AST* lhs() { return lhs_; }
  AST* rhs() { return rhs_; }
  Token op() { return op_; }

 private:
  AST* lhs_;
  AST* rhs_;
  Token op_;
};

class Unary : public AST {
 public:
  Unary(AST* node, Token op, bool prefix) :
    AST(AST_EXPR_UNARY), node_(node), op_(op), prefix_(prefix) {}

  ~Unary() override {
    delete node_;
  }

  AST* node() { return node_; }
  Token op() { return op_; }
  bool prefix() { return prefix_; }

 private:
  AST* node_;
  Token op_;
  bool prefix_;
};

class TripleCondition : public AST {
 public:
  TripleCondition(AST* cond, AST* lhs, AST* rhs) :
    AST(AST_EXPR_TRIPLE), cond_(cond), lhs_(lhs), rhs_(rhs) {}

  ~TripleCondition() override {
    delete cond_;
    delete lhs_;
    delete rhs_;
  }

  AST* cond() { return cond_; }
  AST* lhs() { return lhs_; }
  AST* rhs() { return rhs_; }

 private:
  AST* cond_;
  AST* lhs_;
  AST* rhs_;
};

class Expression : public AST {
 public:
  Expression() : AST(AST_EXPR) {}
  ~Expression() override {
    for (auto element : elements_) {
      delete element;
    }
  }

  void AddElement(AST* element) { elements_.push_back(element); }

  std::vector<AST*> elements() { return elements_; }

 private:
  std::vector<AST*> elements_;
};

class Arguments : public AST {
 public:
  Arguments(std::vector<AST*> args) : AST(AST_EXPR_ARGS), args_(args) {}

  ~Arguments() override {
    for (auto arg : args_)
      delete arg;
  }

  std::vector<AST*> args() { return args_; }

 private:
  std::vector<AST*> args_;
};

class LHS : public AST {
 public:
  LHS(AST* base, size_t new_count) :
    AST(AST_EXPR_LHS), base_(base), new_count_(new_count) {}

  ~LHS() override {
    for (auto args : args_list_)
      delete args;
    for (auto index : index_list_)
      delete index;
  }

  enum PostfixType{
    CALL,
    INDEX,
    PROP,
  };

  void AddArguments(Arguments* args) {
    order_.emplace_back(std::make_pair(args_list_.size(), CALL));
    args_list_.emplace_back(args);
  }

  void AddIndex(AST* index) {
    order_.emplace_back(std::make_pair(index_list_.size(), INDEX));
    index_list_.emplace_back(index);
  }

  void AddProp(Token prop_name) {
    order_.emplace_back(std::make_pair(prop_name_list_.size(), PROP));
    prop_name_list_.emplace_back(prop_name.source());
  }

  AST* base() { return base_; }
  size_t new_count() { return new_count_; }
  std::vector<std::pair<size_t, PostfixType>> order() { return order_; }
  std::vector<Arguments*> args_list() { return args_list_; }
  std::vector<AST*> index_list() { return index_list_; }
  std::vector<std::u16string> prop_name_list() { return prop_name_list_; }

 private:
  AST* base_;
  size_t new_count_;

  std::vector<std::pair<size_t, PostfixType>> order_;
  std::vector<Arguments*> args_list_;
  std::vector<AST*> index_list_;
  std::vector<std::u16string> prop_name_list_;
};

class Function : public AST {
 public:
  Function(std::vector<std::u16string> params, AST* body, std::u16string source) :
    Function(Token(Token::TK_NOT_FOUND, u""), params, body, source) {}

  Function(Token name, std::vector<std::u16string> params, AST* body, std::u16string source) :
    AST(AST_FUNC, source), name_(name), params_(params) {
      assert(body->type() == AST::AST_FUNC_BODY);
      body_ = body;
    }

  ~Function() override {
    delete body_;
  }

  bool is_named() { return name_.type() != Token::TK_NOT_FOUND; }
  std::u16string name() { return name_.source(); }
  std::vector<std::u16string> params() { return params_; }
  AST* body() { return body_; }

 private:
  Token name_;
  std::vector<std::u16string> params_;
  AST* body_;
};

class ProgramOrFunctionBody : public AST {
 public:
  ProgramOrFunctionBody(Type type, bool strict) : AST(type), strict_(strict) {}
  ~ProgramOrFunctionBody() override {
    for (auto func_decl : func_decls_)
      delete func_decl;
    for (auto stmt : stmts_)
      delete stmt;
  }

  void AddFunctionDecl(AST* func) {
    assert(func->type() == AST_FUNC);
    func_decls_.emplace_back(static_cast<Function*>(func));
  }
  void AddStatement(AST* stmt) {
    stmts_.emplace_back(stmt);
  }

  bool strict() { return strict_; }
  std::vector<Function*> func_decls() { return func_decls_; }
  std::vector<AST*> statements() { return stmts_; }

 private:
  bool strict_;
  std::vector<Function*> func_decls_;
  std::vector<AST*> stmts_;
};

class LabelledStmt : public AST {
 public:
  LabelledStmt(Token ident, AST* stmt, std::u16string source) :
    AST(AST_STMT_LABEL, source), ident_(ident), stmt_(stmt) {}
  ~LabelledStmt() {
    delete stmt_;
  }

 private:
  Token ident_;
  AST* stmt_;
};

class ContinueOrBreak : public AST {
 public:
  ContinueOrBreak(Type type, std::u16string source) :
    ContinueOrBreak(type, Token(Token::TK_NOT_FOUND, u""), source) {}

  ContinueOrBreak(Type type, Token ident, std::u16string source) :
    AST(type, source), ident_(ident) {}

  Token ident() { return ident_; }

 private:
  Token ident_;
};

class Return : public AST {
 public:
  Return(AST* expr, std::u16string source) :
    AST(AST_STMT_RETURN, source), expr_(expr) {}
  ~Return() {
    if (expr_ != nullptr)
      delete expr_;
  }

  AST* expr() { return expr_; }

 private:
  AST* expr_;
};

class Throw : public AST {
 public:
  Throw(AST* expr, std::u16string source) :
    AST(AST_STMT_THROW, source), expr_(expr) {}
  ~Throw() {
    if (expr_ != nullptr)
      delete expr_;
  }

  AST* expr() { return expr_; }

 private:
  AST* expr_;
};

class VarDecl : public AST {
 public:
  VarDecl(Token ident, std::u16string source) :
    VarDecl(ident, nullptr, source) {}

  VarDecl(Token ident, AST* init, std::u16string source) :
    AST(AST_STMT_VAR_DECL, source), ident_(ident), init_(init) {}
  ~VarDecl() { delete init_; }

 private:
  Token ident_;
  AST* init_;
};

class VarStmt : public AST {
 public:
  VarStmt() : AST(AST_STMT_VAR) {}
  ~VarStmt() {
    for (auto decl : decls_)
      delete decl;
  }

  void AddDecl(AST* decl) {
    assert(decl->type() == AST_STMT_VAR_DECL);
    decls_.emplace_back(static_cast<VarDecl*>(decl));
  }

 public:
  std::vector<VarDecl*> decls_;
};

class Block : public AST {
 public:
  Block() : AST(AST_STMT_BLOCK) {}
  ~Block() {
    for (auto stmt : stmts_)
      delete stmt;
  }

  void AddStatement(AST* stmt) {
    stmts_.emplace_back(stmt);
  }

  std::vector<AST*> statements() { return stmts_; }

 public:
  std::vector<AST*> stmts_;
};

class Try : public AST {
 public:
  Try(AST* try_block, Token catch_ident, AST* catch_block, std::u16string source) :
    Try(try_block, catch_ident, catch_block, nullptr, source) {}

  Try(AST* try_block, AST* finally_block, std::u16string source) :
    Try(try_block, Token(Token::TK_NOT_FOUND, u""), nullptr, finally_block, source) {}

  Try(AST* try_block, Token catch_ident, AST* catch_block, AST* finally_block, std::u16string source)
    : AST(AST_STMT_TRY, source), try_block_(try_block), catch_ident_(catch_ident),
      catch_block_(catch_block), finally_block_(finally_block) {}

  ~Try() {
    delete try_block_;
    if (catch_block_ != nullptr)
      delete catch_block_;
    if (finally_block_ != nullptr)
      delete finally_block_;
  }

 public:
  AST* try_block_;
  Token catch_ident_;
  AST* catch_block_;
  AST* finally_block_;
};

class If : public AST {
 public:
  If(AST* cond, AST* if_block, std::u16string source) :
    If(cond, if_block, nullptr, source) {}

  If(AST* cond, AST* if_block, AST* else_block, std::u16string source) :
    AST(AST_STMT_IF, source), cond_(cond), if_block_(if_block), else_block_(else_block) {}
  ~If() {
    delete cond_;
    delete if_block_;
    if (else_block_ != nullptr)
      delete else_block_;
  }

  AST* cond() { return cond_; }
  AST* if_block() { return if_block_; }
  AST* else_block() { return else_block_; }

 public:
  AST* cond_;
  AST* if_block_;
  AST* else_block_;
};

class WhileOrWith : public AST {
 public:
  WhileOrWith(Type type, AST* expr, AST* stmt, std::u16string source) :
    AST(type, source), expr_(expr), stmt_(stmt) {}
  ~WhileOrWith() {
    delete expr_;
    delete stmt_;
  }

 public:
  AST* expr_;
  AST* stmt_;
};

class DoWhile : public AST {
 public:
  DoWhile(AST* cond, AST* loop_block, std::u16string source) :
    AST(AST_STMT_DO_WHILE, source), cond_(cond), loop_block_(loop_block) {}
  ~DoWhile() {
    delete cond_;
    delete loop_block_;
  }

 public:
  AST* cond_;
  AST* loop_block_;
};

class Switch : public AST {
 public:
  Switch() : AST(AST_STMT_SWITCH) {}

  ~Switch() override {
    for (CaseClause clause : case_clauses_) {
      if (clause.type == CaseClause::CASE) {
        delete clause.expr;
      }
      for (auto stmt : clause.stmts) {
        delete stmt;
      }
    }
  }

  void SetExpr(AST* expr) {
    expr_ = expr;
  }

  struct CaseClause {
    enum Type {
      CASE = 0,
      DEFAULT,
    };

    CaseClause(Type t, std::vector<AST*> ss) : CaseClause(t, nullptr, ss) {
      assert(type == DEFAULT);
    }
    CaseClause(Type t, AST* e, std::vector<AST*> ss) :
      type(t), expr(e), stmts(ss) {}

    Type type;
    AST* expr;
    std::vector<AST*> stmts;
  };

  void AddCaseClause(CaseClause c) {
    // When having same key, will keep the last one.
    // but {1.0: 1, 1.00: 2} will have 2 key-val pair.
    case_clauses_.emplace_back(c);
  }

  std::vector<CaseClause> case_clauses() { return case_clauses_; }

 private:
  AST* expr_;
  std::vector<CaseClause> case_clauses_;
};

class For : public AST {
 public:
  For(std::vector<AST*> expr0s, AST* expr1, AST* expr2, AST* stmt, std::u16string source) :
    AST(AST_STMT_FOR, source), expr0s_(expr0s), expr1_(expr1), expr2_(expr2), stmt_(stmt) {}

 private:
  std::vector<AST*> expr0s_;
  AST* expr1_;
  AST* expr2_;

  AST* stmt_;
};

class ForIn : public AST {
 public:
  ForIn(AST* expr0, AST* expr1, AST* stmt, std::u16string source) :
    AST(AST_STMT_FOR_IN, source), expr0_(expr0), expr1_(expr1), stmt_(stmt) {}

 private:
  AST* expr0_;
  AST* expr1_;

  AST* stmt_;
};

}  // namespace es

#endif  // ES_PARSER_AST_H