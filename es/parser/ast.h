#ifndef ES_PARSER_AST_H
#define ES_PARSER_AST_H

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <utility>

#include <es/parser/token.h>
#include <es/utils/macros.h>
#include <es/types/base.h>

namespace es {

class AST {
 public:
  enum Type {
    AST_EXPR_THIS,
    AST_EXPR_IDENT,
    AST_EXPR_STRICT_FUTURE,

    AST_EXPR_NULL,
    AST_EXPR_BOOL,
    AST_EXPR_NUMBER,
    AST_EXPR_STRING,
    AST_EXPR_REGEXP,

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

  AST(Type type) : type_(type) {}
  AST(Type type, std::u16string source, size_t start, size_t end) :
    type_(type), source_(source), start_(start), end_(end) {
    switch (type) {
      case AST::AST_EXPR_IDENT:
      case AST::AST_EXPR_STRICT_FUTURE:
        jsval_ = String::New(source, GCFlag::CONST);
        break;
      case AST::AST_EXPR_BOOL:
        jsval_ = Bool::Wrap(source == u"true");
        break;
      case AST::AST_EXPR_STRING:
        jsval_ = String::Eval(source, GCFlag::CONST);
        break;
      case AST::AST_EXPR_NUMBER:
        jsval_ = Number::Eval(source, GCFlag::CONST);
        break;
      default:
        jsval_ = Handle<JSValue>();
    }
  }
  virtual ~AST() {};

  Type type() { return type_; }
  std::u16string source() { return source_; }
  const std::u16string& source_ref() { return source_; }
  size_t start() { return start_; }
  size_t end() { return end_; }
  Handle<JSValue> jsval() { return jsval_; }

  void SetSource(std::u16string source, size_t start, size_t end) {
    source_ = source;
    start_ = start;
    end_ = end;
  }

  bool IsIllegal() { return type_ == AST_ILLEGAL; }

  std::u16string label() { return label_; }
  void SetLabel(std::u16string label) { label_ = label; }

 private:
  Type type_;
  std::u16string source_;
  size_t start_;
  size_t end_;
  std::u16string label_;
  Handle<JSValue> jsval_;
};

class RegExpLiteral : public AST {
 public:
  RegExpLiteral(std::u16string pattern, std::u16string flag,
                std::u16string source, size_t start, size_t end) :
    AST(AST_EXPR_REGEXP, source, start, end), pattern_(pattern), flag_(flag) {}

  std::u16string pattern() { return pattern_; }
  std::u16string flag() { return flag_; }

 private:
  std::u16string pattern_;
  std::u16string flag_;
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
  std::vector<std::pair<size_t, AST*>> elements() { return elements_; }

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

Handle<String> NumberToStringConst(double m);
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

    Property(Token k, AST* v, Type t) : value(v), type(t) {
      key = EvalPropertyName(k);
    }

    Handle<String> key;
    AST* value;
    Type type;
  };

  void AddProperty(Property p) {
    properties_.emplace_back(p);
  }

  static Handle<String> EvalPropertyName(Token token) {
    switch (token.type()) {
      case Token::TK_STRICT_FUTURE:
      case Token::TK_IDENT:
      case Token::TK_KEYWORD:
      case Token::TK_FUTURE:
      case Token::TK_NULL:
      case Token::TK_BOOL:
        return String::New(token.source(), GCFlag::CONST);
      case Token::TK_NUMBER:
        return NumberToStringConst(Number::Eval(token.source_ref(), GCFlag::CONST).val()->data());
      case Token::TK_STRING: {
        return String::Eval(token.source_ref(), GCFlag::CONST);
      }
      default:
        assert(false);
    }
  }

  std::vector<Property> properties() { return properties_; }

  size_t length() { return properties_.size(); }

 private:
  std::vector<Property> properties_;
};

class Paren : public AST {
 public:
  Paren(AST* expr, std::u16string source, size_t start, size_t end) :
    AST(AST_EXPR_PAREN, source, start, end), expr_(expr) {}

  AST* expr() { return expr_; }

 private:
  AST* expr_;
};

class Binary : public AST {
 public:
  Binary(AST* lhs, AST* rhs, Token op, std::u16string source, size_t start, size_t end) :
    AST(AST_EXPR_BINARY, source, start, end), lhs_(lhs), rhs_(rhs), op_(op) {}

  ~Binary() override {
    delete lhs_;
    delete rhs_;
  }

  AST* lhs() { return lhs_; }
  AST* rhs() { return rhs_; }
  Token& op() { return op_; }

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
  Token& op() { return op_; }
  bool prefix() { return prefix_; }

 private:
  AST* node_;
  Token op_;
  bool prefix_;
};

class TripleCondition : public AST {
 public:
  TripleCondition(AST* cond, AST* true_expr, AST* false_expr) :
    AST(AST_EXPR_TRIPLE), cond_(cond), true_expr_(true_expr), false_expr_(false_expr) {}

  ~TripleCondition() override {
    delete cond_;
    delete true_expr_;
    delete false_expr_;
  }

  AST* cond() { return cond_; }
  AST* true_expr() { return true_expr_; }
  AST* false_expr() { return false_expr_; }

 private:
  AST* cond_;
  AST* true_expr_;
  AST* false_expr_;
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
    AST(AST_EXPR_LHS), base_(base), total_count_(new_count), new_count_(new_count) {}

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
    total_count_++;
  }

  void AddIndex(AST* index) {
    order_.emplace_back(std::make_pair(index_list_.size(), INDEX));
    index_list_.emplace_back(index);
    total_count_++;
  }

  void AddProp(Token prop_name) {
    order_.emplace_back(std::make_pair(prop_name_list_.size(), PROP));
    prop_name_list_.emplace_back(String::New(prop_name.source(), GCFlag::CONST));
    total_count_++;
  }

  AST* base() { return base_; }
  size_t total_count() { return total_count_; }
  size_t new_count() { return new_count_; }
  std::vector<std::pair<size_t, PostfixType>> order() { return order_; }
  std::vector<Arguments*> args_list() { return args_list_; }
  std::vector<AST*> index_list() { return index_list_; }
  std::vector<Handle<String>> prop_name_list() { return prop_name_list_; }

 private:
  AST* base_;
  size_t total_count_;
  size_t new_count_;

  std::vector<std::pair<size_t, PostfixType>> order_;
  std::vector<Arguments*> args_list_;
  std::vector<AST*> index_list_;
  std::vector<Handle<String>> prop_name_list_;
};

class Function : public AST {
 public:
  Function(std::vector<std::u16string> params, AST* body,
           std::u16string source, size_t start, size_t end) :
    Function(Token(Token::TK_NOT_FOUND, u"", 0, 0), params, body, source, start, end) {}

  Function(Token name, std::vector<std::u16string> params, AST* body,
           std::u16string source, size_t start, size_t end) :
    AST(AST_FUNC, source, start, end) {
      ASSERT(body->type() == AST::AST_FUNC_BODY);
      body_ = body;
      if (name.type() != Token::TK_NOT_FOUND) {
        name_ = String::New(name.source(), GCFlag::CONST);
        name_is_eval_or_arguments_ = name.source() == u"eval" || name.source() == u"arguments";
      } else {
        name_ = Handle<String>();
        name_is_eval_or_arguments_ = false;
      }
      params_have_eval_or_arguments_ = false;
      params_have_duplicated_ = false;
      for (size_t i = 0; i < params.size(); ++i) {
        params_.emplace_back(String::New(params[i], GCFlag::CONST));
        if (params[i] == u"eval" || params[i] == u"arguments") {
          params_have_eval_or_arguments_ = true;
        }
        for (size_t j = 0; j < i; ++j) {
          if (params[i] == params[j]) {
            params_have_duplicated_ = true;
          }
        }
      }
    }

  ~Function() override {
    delete body_;
  }

  bool is_named() { return !name_.IsNullptr(); }
  Handle<String> name() { return name_; }
  bool name_is_eval_or_arguments() { return name_is_eval_or_arguments_; }
  std::vector<Handle<String>> params() { return params_; }
  bool params_have_eval_or_arguments() { return params_have_eval_or_arguments_; }
  bool params_have_duplicated() { return params_have_duplicated_; }
  AST* body() { return body_; }

 private:
  bool name_is_eval_or_arguments_;
  bool params_have_eval_or_arguments_;
  bool params_have_duplicated_;
  Handle<String> name_;
  std::vector<Handle<String>> params_;
  AST* body_;
};

class VarDecl;
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
    ASSERT(func->type() == AST_FUNC);
    func_decls_.emplace_back(static_cast<Function*>(func));
  }
  void AddStatement(AST* stmt) {
    stmts_.emplace_back(stmt);
  }

  bool strict() { return strict_; }
  std::vector<Function*> func_decls() { return func_decls_; }
  std::vector<AST*> statements() { return stmts_; }

  std::vector<VarDecl*>& var_decls() { return var_decls_; }
  void SetVarDecls(std::vector<VarDecl*>&& var_decls) { var_decls_ = var_decls; }

  size_t num_decls() { return func_decls_.size() + var_decls_.size(); }

 private:
  bool strict_;
  std::vector<Function*> func_decls_;
  std::vector<AST*> stmts_;

  std::vector<VarDecl*> var_decls_;
};

class LabelledStmt : public AST {
 public:
  LabelledStmt(Token label, AST* stmt, std::u16string source, size_t start, size_t end) :
    AST(AST_STMT_LABEL, source, start, end), label_(label), stmt_(stmt) {}
  ~LabelledStmt() {
    delete stmt_;
  }

  std::u16string label() { return label_.source(); }
  AST* statement() { return stmt_; }

 private:
  Token label_;
  AST* stmt_;
};

class ContinueOrBreak : public AST {
 public:
  ContinueOrBreak(Type type, std::u16string source, size_t start, size_t end) :
    ContinueOrBreak(type, Token(Token::TK_NOT_FOUND, u"", 0, 0), source, start, end) {}

  ContinueOrBreak(Type type, Token ident, std::u16string source, size_t start, size_t end) :
    AST(type, source, start, end), ident_(ident) {}

  std::u16string ident() { return ident_.source(); }

 private:
  Token ident_;
};

class Return : public AST {
 public:
  Return(AST* expr, std::u16string source, size_t start, size_t end) :
    AST(AST_STMT_RETURN, source, start, end), expr_(expr) {}
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
  Throw(AST* expr, std::u16string source, size_t start, size_t end) :
    AST(AST_STMT_THROW, source, start, end), expr_(expr) {}
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
  VarDecl(Token ident, std::u16string source, size_t start, size_t end) :
    VarDecl(ident, nullptr, source, start, end) {}

  VarDecl(Token ident, AST* init, std::u16string source, size_t start, size_t end) :
    AST(AST_STMT_VAR_DECL, source, start, end), init_(init) {
    ident_ = String::New(ident.source(), GCFlag::CONST);
    is_strict_future_ = ident.type() == Token::TK_STRICT_FUTURE;
    is_eval_or_arguments_ = ident.source() == u"eval" or ident.source() == u"arguments";
  }
  ~VarDecl() { delete init_; }

  Handle<String> ident() { return ident_; }
  AST* init() { return init_; }
  bool is_strict_future() { return is_strict_future_; }
  bool is_eval_or_arguments() { return is_eval_or_arguments_; }

  Handle<String> ident_;
  bool is_strict_future_;
  bool is_eval_or_arguments_;
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
    ASSERT(decl->type() == AST_STMT_VAR_DECL);
    decls_.emplace_back(static_cast<VarDecl*>(decl));
  }

  std::vector<VarDecl*> decls() { return decls_; }

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
  Try(AST* try_block, Token catch_ident, AST* catch_block,
      std::u16string source, size_t start, size_t end) :
    Try(try_block, catch_ident, catch_block, nullptr, source, start, end) {}

  Try(AST* try_block, AST* finally_block,
      std::u16string source, size_t start, size_t end) :
    Try(try_block, Token(Token::TK_NOT_FOUND, u"", 0, 0), nullptr, finally_block, source, start, end) {}

  Try(AST* try_block, Token catch_ident, AST* catch_block, AST* finally_block,
      std::u16string source, size_t start, size_t end)
    : AST(AST_STMT_TRY, source, start, end), try_block_(try_block),
      catch_ident_(String::New(catch_ident.source(), GCFlag::CONST)),
      catch_ident_is_eval_or_arguments_(catch_ident.source() == u"eval" || catch_ident.source() == u"arguments"),
      catch_block_(catch_block), finally_block_(finally_block) {}

  ~Try() {
    delete try_block_;
    if (catch_block_ != nullptr)
      delete catch_block_;
    if (finally_block_ != nullptr)
      delete finally_block_;
  }

  AST* try_block() { return try_block_; }
  Handle<String> catch_ident() { return catch_ident_; };
  bool catch_ident_is_eval_or_arguments() { return catch_ident_is_eval_or_arguments_; }
  AST* catch_block() { return catch_block_; }
  AST* finally_block() { return finally_block_; }

 public:
  AST* try_block_;
  Handle<String> catch_ident_;
  bool catch_ident_is_eval_or_arguments_;
  AST* catch_block_;
  AST* finally_block_;
};

class If : public AST {
 public:
  If(AST* cond, AST* if_block, std::u16string source, size_t start, size_t end) :
    If(cond, if_block, nullptr, source, start, end) {}

  If(AST* cond, AST* if_block, AST* else_block, std::u16string source, size_t start, size_t end) :
    AST(AST_STMT_IF, source, start, end), cond_(cond), if_block_(if_block), else_block_(else_block) {}
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
  WhileOrWith(Type type, AST* expr, AST* stmt,
              std::u16string source, size_t start, size_t end) :
    AST(type, source, start, end), expr_(expr), stmt_(stmt) {}
  ~WhileOrWith() {
    delete expr_;
    delete stmt_;
  }

  AST* expr() { return expr_; }
  AST* stmt() { return stmt_; }

 public:
  AST* expr_;
  AST* stmt_;
};

class DoWhile : public AST {
 public:
  DoWhile(AST* expr, AST* stmt, std::u16string source, size_t start, size_t end) :
    AST(AST_STMT_DO_WHILE, source, start, end), expr_(expr), stmt_(stmt) {}
  ~DoWhile() {
    delete expr_;
    delete stmt_;
  }

  AST* expr() { return expr_; }
  AST* stmt() { return stmt_; }

 public:
  AST* expr_;
  AST* stmt_;
};

class Switch : public AST {
 public:
  Switch() : AST(AST_STMT_SWITCH) {}

  ~Switch() override {
    for (CaseClause clause : before_default_case_clauses_) {
      delete clause.expr;
      for (auto stmt : clause.stmts) {
        delete stmt;
      }
    }
    for (CaseClause clause : after_default_case_clauses_) {
      delete clause.expr;
      for (auto stmt : clause.stmts) {
        delete stmt;
      }
    }
    for (auto stmt : default_clause_.stmts) {
      delete  stmt;
    }
  }

  void SetExpr(AST* expr) {
    expr_ = expr;
  }

  struct DefaultClause {
    std::vector<AST*> stmts;
  };

  struct CaseClause {
    CaseClause(AST* expr, std::vector<AST*> stmts) : expr(expr), stmts(stmts) {}
    AST* expr;
    std::vector<AST*> stmts;
  };

  void SetDefaultClause(std::vector<AST*> stmts) {
    ASSERT(!has_default_clause());
    has_default_clause_ = true;
    default_clause_.stmts = stmts;
  }

  void AddBeforeDefaultCaseClause(CaseClause c) {
    before_default_case_clauses_.emplace_back(c);
  }

  void AddAfterDefaultCaseClause(CaseClause c) {
    after_default_case_clauses_.emplace_back(c);
  }

  AST* expr() { return expr_; }
  std::vector<CaseClause> before_default_case_clauses() { return before_default_case_clauses_; }
  bool has_default_clause() { return has_default_clause_; }
  DefaultClause default_clause() {
    ASSERT(has_default_clause());
    return default_clause_;
  }
  std::vector<CaseClause> after_default_case_clauses() { return after_default_case_clauses_; }

 private:
  AST* expr_;
  bool has_default_clause_ = false;
  DefaultClause default_clause_;
  std::vector<CaseClause> before_default_case_clauses_;
  std::vector<CaseClause> after_default_case_clauses_;
};

class For : public AST {
 public:
  For(std::vector<AST*> expr0s, AST* expr1, AST* expr2, AST* stmt,
      std::u16string source, size_t start, size_t end) :
    AST(AST_STMT_FOR, source, start, end), expr0s_(expr0s), expr1_(expr1), expr2_(expr2), stmt_(stmt) {}

  std::vector<AST*> expr0s() { return expr0s_; }
  AST* expr1() { return expr1_; }
  AST* expr2() { return expr2_; }
  AST* statement() { return stmt_; }

 private:
  std::vector<AST*> expr0s_;
  AST* expr1_;
  AST* expr2_;

  AST* stmt_;
};

class ForIn : public AST {
 public:
  ForIn(AST* expr0, AST* expr1, AST* stmt,
        std::u16string source, size_t start, size_t end) :
    AST(AST_STMT_FOR_IN, source, start, end), expr0_(expr0), expr1_(expr1), stmt_(stmt) {}

  AST* expr0() { return expr0_; }
  AST* expr1() { return expr1_; }
  AST* statement() { return stmt_; }

 private:
  AST* expr0_;
  AST* expr1_;

  AST* stmt_;
};

}  // namespace es

#endif  // ES_PARSER_AST_H