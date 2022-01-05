#ifndef ES_AST_H
#define ES_AST_H

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <utility>

#include <es/token.h>

namespace es {

class AST {
 public:
  enum Type {
    AST_EXP_THIS,
    AST_EXP_IDENT,

    AST_EXP_NULL,
    AST_EXP_BOOL,
    AST_EXP_NUMBER,
    AST_EXP_STRING,
    AST_EXP_REGEX,

    AST_EXP_ARRAY,
    AST_EXP_OBJ,

    AST_EXP_PAREN,  // ( Expression )

    AST_EXP_BINARY,
    AST_EXP_UNARY,
    AST_EXP_TRIPLE,

    AST_EXP,

    AST_ILLEGAL,
  };

  AST(Type type, std::u16string_view source = u"") : type_(type), source_(source) {}
  Type type() { return type_; }
  std::u16string_view source() { return source_; }

  void SetSource(std::u16string_view source) { source_ = source; }

  bool IsIllegal() { return type_ == AST_ILLEGAL; }

 private:
  Type type_;
  std::u16string_view source_;
};

class ArrayLiteral : public AST {
 public:
  ArrayLiteral() : AST(AST_EXP_ARRAY), len_(0) {}

  ~ArrayLiteral() {
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
  ObjectLiteral() : AST(AST_EXP_OBJ) {}

  ~ObjectLiteral() {
    for (auto pair : properties_) {
      delete pair.second.value;
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
    // When having same key, will keep the last one.
    // but {1.0: 1, 1.00: 2} will have 2 key-val pair.
    properties_.emplace(p.key.source(), p);
  }

 private:
  std::unordered_map<std::u16string_view, Property> properties_;
};

class Binary : public AST {
 public:
  Binary(AST* lhs, AST* rhs, Token op) :
    AST(AST_EXP_BINARY), lhs_(lhs), rhs_(rhs), op_(op) {}

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
    AST(AST_EXP_UNARY), node_(node), op_(op), prefix_(prefix) {}

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
    AST(AST_EXP_TRIPLE), cond_(cond), lhs_(lhs), rhs_(rhs) {}

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
  Expression() : AST(AST_EXP) {}

  void AddElement(AST* element) { elements_.push_back(element); }

 private:
  std::vector<AST*> elements_;
};

}  // namespace es

#endif