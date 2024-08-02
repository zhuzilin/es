#ifndef ES_PARSER_TOKEN_H
#define ES_PARSER_TOKEN_H

#include <array>
#include <string>
#include <string_view>

namespace es {

class Token {
 public:
  // Order by section 7.5, 7.6, 7.7, 7.8
  enum Type {
    // Identifier
    TK_IDENT = 0,

    // Keywords
    TK_KEYWORD_DELETE,
    TK_KEYWORD_VOID,
    TK_KEYWORD_TYPEOF,
    TK_KEYWORD_IN,
    TK_KEYWORD_INSTANCE_OF,
    TK_KEYWORD_THIS,
    TK_KEYWORD,

    // Future Reserved Words
    TK_FUTURE,
    TK_STRICT_FUTURE,

    // Punctuator
    TK_LBRACE,  // {
    TK_RBRACE,  // }
    TK_LPAREN,  // (
    TK_RPAREN,  // )
    TK_LBRACK,  // [
    TK_RBRACK,  // ]

    TK_DOT,         // .
    TK_SEMICOLON,   // ;
    TK_COMMA,       // ,
    TK_QUESTION,    // ?
    TK_COLON,       // :

    TK_LT,   // <
    TK_GT,   // >
    TK_LE,   // <=
    TK_GE,   // >=
    TK_EQ,   // ==
    TK_NE,   // !=
    TK_EQ3,  // ===
    TK_NE3,  // !==

    TK_INC,  // ++
    TK_DEC,  // --

    TK_ADD,  // +
    TK_SUB,  // -
    TK_MUL,  // *
    TK_DIV,  // /
    TK_MOD,  // %

    TK_BIT_LSH,   // <<
    TK_BIT_RSH,   // >>
    TK_BIT_URSH,  // >>>, unsigned right shift
    TK_BIT_AND,  // &
    TK_BIT_OR,   // |
    TK_BIT_XOR,  // ^

    TK_BIT_NOT,  // ~

    TK_LOGICAL_AND,  // &&
    TK_LOGICAL_OR,   // ||
    TK_LOGICAL_NOT,  // !

    TK_ASSIGN,      // =
    // The compound assign order should be the same as their 
    // calculate op.
    TK_ADD_ASSIGN,  // +=
    TK_SUB_ASSIGN,  // -=
    TK_MUL_ASSIGN,  // *=
    TK_DIV_ASSIGN,  // /=
    TK_MOD_ASSIGN,  // %=

    TK_BIT_LSH_ASSIGN,   // <<=
    TK_BIT_RSH_ASSIGN,   // >>=
    TK_BIT_URSH_ASSIGN,  // >>>=
    TK_BIT_AND_ASSIGN,   // &=
    TK_BIT_OR_ASSIGN,    // |=
    TK_BIT_XOR_ASSIGN,   // ^=

    // Null Literal
    TK_NULL,  // null

    // Bool Literal
    TK_BOOL,   // true & false

    // Number Literal
    TK_NUMBER,

    // String Literal
    TK_STRING,

    // Regular Expression Literal
    TK_REGEX,

    TK_LINE_TERM,

    TK_EOS,
    TK_NOT_FOUND,
    TK_ILLEGAL,
  };

  Token(Type type, std::u16string source, size_t start, size_t end) :
    type_(type), source_(source), start_(start), end_(end) {}

  inline bool IsAssignmentOperator() {
    switch(type_) {
      case TK_ASSIGN:      // =
      case TK_ADD_ASSIGN:  // +=
      case TK_SUB_ASSIGN:  // -=
      case TK_MUL_ASSIGN:  // *=
      case TK_MOD_ASSIGN:  // %=
      case TK_DIV_ASSIGN:  // /=

      case TK_BIT_LSH_ASSIGN:   // <<=
      case TK_BIT_RSH_ASSIGN:   // >>=
      case TK_BIT_URSH_ASSIGN:  // >>>=
      case TK_BIT_AND_ASSIGN:   // &=
      case TK_BIT_OR_ASSIGN:    // |=
      case TK_BIT_XOR_ASSIGN:   // ^=
        return true;
      default:
        return false;
    }
  }

  inline bool IsLineTerminator() { return type_ == TK_LINE_TERM; }

  inline bool IsIdentifierName() {
    return type_ == TK_IDENT || type_ == TK_KEYWORD ||
           type_ == TK_FUTURE || type_ == TK_STRICT_FUTURE ||
           type_ == TK_NULL || type_ == TK_BOOL;
  }

  inline bool IsPropertyName() {
    return IsIdentifierName() || type_ == TK_STRING || type_ == TK_NUMBER;
  }

  inline bool IsSemiColon() { return type_ == TK_SEMICOLON; }

  inline bool IsIdentifier() { return type_ == TK_IDENT || type_ == TK_STRICT_FUTURE; }

  inline bool IsCompoundAssign() { return TK_ADD_ASSIGN <= type_ && type_ <= TK_BIT_XOR_ASSIGN; }

  inline int BinaryPriority(bool no_in) {
    switch (type_) {
      // Binary
      case TK_LOGICAL_OR:  // ||
        return 2;
      case TK_LOGICAL_AND:  // &&
        return 3;
      case TK_BIT_OR:  // |
        return 4;
      case TK_BIT_XOR:  // ^
        return 5;
      case TK_BIT_AND:  // &
        return 6;
      case TK_EQ:   // ==
      case TK_NE:   // !=
      case TK_EQ3:  // ===
      case TK_NE3:  // !==
        return 7;
      case TK_LT:   // <
      case TK_GT:   // >
      case TK_LE:   // <=
      case TK_GE:   // >=
        return 8;
      case TK_BIT_LSH:   // <<
      case TK_BIT_RSH:   // >>
      case TK_BIT_URSH:  // >>>
        return 9;
      case TK_ADD:
      case TK_SUB:
        return 10;
      case TK_MUL:
      case TK_DIV:
      case TK_MOD:
        return 11;

      case TK_KEYWORD_INSTANCE_OF:
        return 8;
      case TK_KEYWORD_IN:
        // To prevent parsing for(a in b).
        if (!no_in) {
          return 8;
        }
        [[fallthrough]];
      default:
        return -1;
    }
  }

  inline int UnaryPrefixPriority() {
    switch (type_) {
      // Prefix
      case TK_INC:
      case TK_DEC:
      case TK_ADD:
      case TK_SUB:
      case TK_BIT_NOT:
      case TK_LOGICAL_NOT:
        return 100;  // UnaryExpresion always have higher priority.

      case TK_KEYWORD_DELETE:
      case TK_KEYWORD_VOID:
      case TK_KEYWORD_TYPEOF:
        return 100;
      default:
        return -1;
    }
  }

  inline int UnaryPostfixPriority() {
    switch (type_) {
      // Prefix
      case TK_INC:
      case TK_DEC:
        return 200;  // UnaryExpresion always have higher priority.
      default:
        return -1;
    }
  }

  Token ToCalc() {
    ASSERT(IsCompoundAssign());
    return Token((Type)(type_ - TK_ADD_ASSIGN + TK_ADD), source_.substr(0, source_.size()-1), start_, end_ - 1);
  }

  inline Type type() { return type_; }
  inline std::u16string source() { return source_; }
  inline const std::u16string& source_ref() { return source_; }
  inline size_t start() { return start_; }
  inline size_t end() { return end_; }

 private:
  Type type_;
  std::u16string source_;
  size_t start_;
  size_t end_;
};

const std::array<std::u16string, 26> kKeywords = {
  u"break",    u"do",
  u"case",     u"else",     u"new",        u"var",
  u"catch",    u"finally",  u"return",     u"void",
  u"continue", u"for",      u"switch",     u"while",
  u"debugger", u"function", u"with",
  u"default",  u"if",       u"throw",
  u"try",
};

const std::array<std::u16string, 7> kFutureReservedWords = {
  u"class", u"enum",   u"extends", u"super",
  u"const", u"export", u"import",
};

const std::array<std::u16string, 9> kStrictModeFutureReservedWords = {
  u"implements", u"let",     u"private",   u"public", u"yield",
  u"interface",  u"package", u"protected", u"static"
};

}  // namespace es

#endif  // ES_PARSER_TOKEN_H