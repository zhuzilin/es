#ifndef ES_LEXER_H
#define ES_LEXER_H

#include <string>
#include <string_view>

#include <es/character.h>
#include <es/token.h>

namespace es {

class Lexer {
 public:
  Lexer(std::u16string_view source) :
    source_(source), pos_(0), end_(source.size()) {
    UpdateC();
  }

  Token Next() {
    Token token = Token(Token::Type::TK_NOT_FOUND, source_.substr(pos_, 0));
    do {
      size_t start = pos_;
      switch (c_) {
        case character::EOS: {
          token = Token(Token::Type::TK_EOF, source_.substr(pos_, 0));
          break;
        }

        case u'{': {
          Advance();
          token = Token(Token::Type::TK_LBRACE, source_.substr(start, 1));
          break;
        }
        case u'}': {
          Advance();
          token = Token(Token::Type::TK_RBRACE, source_.substr(start, 1));
          break;
        }
        case u'(': {
          Advance();
          token = Token(Token::Type::TK_LPAREN, source_.substr(start, 1));
          break;
        }
        case u')': {
          Advance();
          token = Token(Token::Type::TK_RPAREN, source_.substr(start, 1));
          break;
        }
        case u'[': {
          Advance();
          token = Token(Token::Type::TK_LBRACK, source_.substr(start, 1));
          break;
        }
        case u']': {
          Advance();
          token = Token(Token::Type::TK_LBRACK, source_.substr(start, 1));
          break;
        }
        case u'.': {
          if (character::IsDecimalDigit(LookAhead())) {
            token = ScanNumericLiteral();
          } else {
            token = Token(Token::Type::TK_DOT, source_.substr(start, 1));
            Advance();
          }
          break;
        }
        case u';': {
          Advance();
          token = Token(Token::Type::TK_SEMICOLON, source_.substr(start, 1));
          break;
        }
        case u',': {
          Advance();
          token = Token(Token::Type::TK_COMMA, source_.substr(start, 1));
          break;
        }
        case u'?': {
          Advance();
          token = Token(Token::Type::TK_QUESTION, source_.substr(start, 1));
          break;
        }
        case u':': {
          Advance();
          token = Token(Token::Type::TK_COLON, source_.substr(start, 1));
          break;
        }

        case u'<': {
          Advance();
          switch (c_) {
            case u'<':
              Advance();
              switch (c_) {
                case u'=':  // <<=
                  Advance();
                  token = Token(Token::Type::TK_BIT_LSH_ASSIGN, source_.substr(start, 3));
                  break;
                default:  // <<
                  token = Token(Token::Type::TK_BIT_LSH, source_.substr(start, 2));
              }
              break;
            case u'=':  // <=
              Advance();
              token = Token(Token::Type::TK_LE, source_.substr(start, 2));
              break;
            default:  // <
              token = Token(Token::Type::TK_LT, source_.substr(start, 1));
          }
          break;
        }
        case u'>': {
          Advance();
          switch (c_) {
            case u'>':
              Advance();
              switch (c_) {
                case u'>':
                  Advance();
                  switch (c_) {
                    case u'=':  // >>>=
                      Advance();
                      token = Token(Token::Type::TK_BIT_URSH_ASSIGN, source_.substr(start, 4));
                      break;
                    default:  // >>>
                      token = Token(Token::Type::TK_BIT_URSH, source_.substr(start, 3));
                  }
                case u'=':  // >>=
                  token = Token(Token::Type::TK_BIT_RSH_ASSIGN, source_.substr(start, 3));
                  Advance();
                  break;
                default:  // >>
                  token = Token(Token::Type::TK_BIT_RSH, source_.substr(start, 2));
              }
              break;
            case u'=':  // >=
              Advance();
              token = Token(Token::Type::TK_GE, source_.substr(start, 2));
              break;
            default:  // >
              token = Token(Token::Type::TK_GT, source_.substr(start, 1));
          }
          break;
        }
        case u'=': {
          Advance();
          switch (c_) {
            case u'=':
              Advance();
              switch (c_) {
                case u'=':  // ===
                  Advance();
                  token = Token(Token::Type::TK_EQ3, source_.substr(start, 3));
                  break;
                default:  // ==
                  token = Token(Token::Type::TK_EQ, source_.substr(start, 2));
                  break;
              }
              break;
            default:  // =
              token = Token(Token::Type::TK_ASSIGN, source_.substr(start, 1));
          }
          break;
        }
        case u'!': {
          Advance();
          switch (c_) {
            case u'=':
              Advance();
              switch (c_) {
                case u'=':  // !==
                  Advance();
                  token = Token(Token::Type::TK_NE3, source_.substr(start, 3));
                  break;
                default:  // !=
                  token = Token(Token::Type::TK_NE, source_.substr(start, 2));
                  break;
              }
              break;
            default:  // !
              token = Token(Token::Type::TK_LOGICAL_NOT, source_.substr(start, 1));
          }
          break;
        }

        case u'+': {
          Advance();
          switch (c_) {
            case u'+':  // ++
              Advance();
              token = Token(Token::Type::TK_INC, source_.substr(start, 2));
              break;
            case u'=':  // +=
              Advance();
              token = Token(Token::Type::TK_ADD_ASSIGN, source_.substr(start, 2));
            default:  // +
              token = Token(Token::Type::TK_ADD, source_.substr(start, 1));
          }
          break;
        }
        case u'-': {
          Advance();
          switch (c_) {
            case u'-':  // --
              Advance();
              token = Token(Token::Type::TK_DEC, source_.substr(start, 2));
              break;
            case u'=':  // -=
              Advance();
              token = Token(Token::Type::TK_SUB_ASSIGN, source_.substr(start, 2));
            default:  // -
              token = Token(Token::Type::TK_SUB, source_.substr(start, 1));
          }
          break;
        }
        case u'*': {
          Advance();
          if (c_ == u'=') {  // *=
            Advance();
            token = Token(Token::Type::TK_MUL_ASSIGN, source_.substr(start, 2));
          } else {  // +
            token = Token(Token::Type::TK_MUL, source_.substr(start, 1));
          }
          break;
        }
        case u'%': {
          Advance();
          if (c_ == u'=') {  // %=
            Advance();
            token = Token(Token::Type::TK_MOD_ASSIGN, source_.substr(start, 2));
          } else {  // %
            token = Token(Token::Type::TK_MOD, source_.substr(start, 1));
          }
          break;
        }

        case u'&': {
          Advance();
          switch (c_) {
            case u'&':  // &&
              Advance();
              token = Token(Token::Type::TK_LOGICAL_AND, source_.substr(start, 2));
              break;
            case u'=':  // &=
              Advance();
              token = Token(Token::Type::TK_BIT_AND_ASSIGN, source_.substr(start, 2));
              break;
            default:  // &
              token = Token(Token::Type::TK_BIT_AND, source_.substr(start, 1));
          }
          break;
        }
        case u'|': {
          Advance();
          switch (c_) {
            case u'|':  // ||
              Advance();
              token = Token(Token::Type::TK_LOGICAL_OR, source_.substr(start, 2));
              break;
            case u'=':  // |=
              Advance();
              token = Token(Token::Type::TK_BIT_OR_ASSIGN, source_.substr(start, 2));
              break;
            default:  // |
              token = Token(Token::Type::TK_BIT_OR, source_.substr(start, 1));
          }
          break;
        }
        case u'^': {
          Advance();
          if (c_ == u'=') {  // ^=
            Advance();
            token = Token(Token::Type::TK_BIT_XOR_ASSIGN, source_.substr(start, 2));
          } else {
            token = Token(Token::Type::TK_BIT_XOR, source_.substr(start, 1));
          }
          break;
        }
        case u'~': {
          Advance();
          token = Token(Token::Type::TK_BIT_NOT, source_.substr(start, 1));
          break;
        }

        case u'/': {
          Advance();
          switch (c_) {
            case u'*':  // /*
              Advance();
              SkipMultiLineComment();
              break;
            case u'/':  // //
              Advance();
              SkipSingleLineComment();
              break;
            case u'=':  // /=
              Advance();
              token = Token(Token::Type::TK_DIV_ASSIGN, source_.substr(start, 2));
              break;
            default:  // /
              // We cannot distinguish DIV and regex in lexer level and therefore,
              // we need to check if the symbol of div operator or start of regex
              // in parser.
              token = Token(Token::Type::TK_DIV, source_.substr(start, 1));
          }
          break;
        }

        case u'\'':
        case u'"': {
          token = ScanStringLiteral();
          break;
        }

        default:
          if (character::IsWhiteSpace(c_)) {
            SkipWhiteSpace();
          } else if (character::IsLineTerminator(c_)) {
            SkipLineTerminatorSequence();
          } else if (character::IsDecimalDigit(c_)) {
            token = ScanNumericLiteral();
          } else if (character::IsIdentifierStart(c_)) {
            token = ScanIdentifier();
          }
      }
    } while(token.type() == Token::Type::TK_NOT_FOUND && c_ != character::EOS);
    return token;
  }

 private:
  inline char16_t LookAhead() {
    if (pos_ + 1 >= end_) {
      return character::EOS;
    }
    return source_[pos_ + 1];
  }

  inline void Advance() {
    if (pos_ < end_) {
      pos_++;
    }
    UpdateC();
  }

  inline void UpdateC() {
    if (pos_ < end_) {
      c_ = source_[pos_];
    } else {
      c_ = character::EOS;
    }
  }

  void SkipMultiLineComment() {
    while (c_ != character::EOS) {
      if (c_ == u'*') {
        Advance();
        if (c_ == u'/') {
          Advance();
          break;
        }
      } else {
        Advance();
      }
    }
  }

  void SkipSingleLineComment() {
    // This will not skip line terminators.
    while (c_ != character::EOS && !character::IsLineTerminator(c_)) {
      Advance();
    }
  }

  void SkipWhiteSpace() {
    while(character::IsWhiteSpace(c_)) {
      Advance();
    }
  }

  void SkipLineTerminatorSequence() {
    assert(character::IsLineTerminator(c_));
    if (c_ == character::CR && LookAhead() == character::LF) {
      Advance(); Advance();
    } else {
      Advance();
    }
  }

  Token ScanStringLiteral() {
    char16_t quote = c_;
    size_t start = pos_;
    Advance();
    while(c_ != character::EOS && c_ != quote && !character::IsLineTerminator(c_)) {
      switch (c_) {
        case u'\\': {
          Advance();
          // TODO(zhuzilin) Find out if "\1" will trigger error.
          switch (c_) {
            case u'0': {
              Advance();
              if (character::IsDecimalDigit(LookAhead())) {
                Advance();
                goto error;
              }
              break;
            }
            case u'x': {  // HexEscapeSequence
              Advance();
              for (size_t i = 0; i < 2; i++) {
                if (!character::IsHexDigit(c_)) {
                  Advance();
                  goto error;
                }
                Advance();
              }
              break;
            }
            case u'u': {  // UnicodeEscapeSequence
              if (!SkipUnicodeEscapeSequence()) {
                Advance();
                goto error;
              }
              break;
            }
            default:
              if (character::IsLineTerminator(c_)) {
                SkipLineTerminatorSequence();
              } else if (character::IsCharacterEscapeSequence(c_)) {
                Advance();
              } else {
                Advance();
                goto error;
              }
          }
          break;
        }
        default:
          Advance();
      }
    }

    if (c_ == quote) {
      Advance();
      return Token(Token::Type::TK_STRING, source_.substr(start, pos_ - start));
    }
error:
    return Token(Token::Type::TK_ILLEGAL, source_.substr(start, pos_ - start));
  }

  bool SkipAtLeastOneDecimalDigit() {
    if (!character::IsDecimalDigit(c_)) {
      return false;
    }
    while (character::IsDecimalDigit(c_)) {
      Advance();
    }
    return true;
  }

  bool SkipAtLeastOneHexDigit() {
    if (!character::IsHexDigit(c_)) {
      return false;
    }
    while (character::IsHexDigit(c_)) {
      Advance();
    }
    return true;
  }

  Token ScanNumericLiteral() {
    assert(c_ == u'.' || character::IsDecimalDigit(c_));
    size_t start = pos_;

    bool is_hex = false;
    switch (c_) {
      case u'0': {
        Advance();
        switch (c_) {
          case u'x':
          case u'X': {  // HexIntegerLiteral
            Advance();
            if (!SkipAtLeastOneHexDigit()) {
              Advance();
              goto error;
            }
            is_hex = true;
            break;
          }
          case u'.': {
            Advance();
            if (!SkipAtLeastOneDecimalDigit()) {
              Advance();
              goto error;
            }
            break;
          }
        }
        break;
      }
      case u'.': {
        Advance();
        if (!SkipAtLeastOneDecimalDigit()) {
          Advance();
          goto error;
        }
        break;
      }
      default:  // NonZeroDigit
        SkipAtLeastOneDecimalDigit();
        if (c_ == u'.') {
          Advance();
          if (!SkipAtLeastOneDecimalDigit()) {
            Advance();
            goto error;
          }
        }
    }

    if(!is_hex) {  // ExponentPart
      if (c_ == u'e' || c_ == u'E') {
        Advance();
        if (c_ == u'+' || c_ == u'-') {
          Advance();
        }
        if (!SkipAtLeastOneDecimalDigit()) {
          Advance();
          goto error;
        }
      }
    }

    // The source character immediately following a NumericLiteral must not
    // be an IdentifierStart or DecimalDigit.
    if (character::IsIdentifierStart(c_) || character::IsDecimalDigit(c_)) {
      Advance();
      goto error;
    }
    return Token(Token::Type::TK_NUMBER, source_.substr(start, pos_ - start));
error:
    return Token(Token::Type::TK_ILLEGAL, source_.substr(start, pos_ - start));
  }

  bool SkipUnicodeEscapeSequence() {
    if (c_ != u'u') {
      return false;
    }
    Advance();
    for (size_t i = 0; i < 4; i++) {
      if (!character::IsHexDigit(c_)) {
        return false;
      }
      Advance();
    }
    return true;
  }

  Token ScanIdentifier() {
    assert(character::IsIdentifierStart(c_));
    size_t start = pos_;
    std::u16string_view source;
    if (c_ == u'\\') {
      Advance();
      if (!SkipUnicodeEscapeSequence()) {
        Advance();
        goto error;
      }
    } else {
      Advance();
    }

    while (character::IsIdentifierPart(c_)) {
      if (c_ == u'\\') {
        Advance();
        if (!SkipUnicodeEscapeSequence()) {
          Advance();
          goto error;
        }
      } else {
        Advance();
      }
    }

    source = source_.substr(start, pos_ - start);
    if (source == u"null") {
      return Token(Token::Type::TK_NULL, source);
    }
    if (source == u"true") {
      return Token(Token::Type::TK_TRUE, source);
    }
    if (source == u"false") {
      return Token(Token::Type::TK_FALSE, source);
    }
    for (auto keyword : kKeywords) {
      if (source == keyword) {
        return Token(Token::Type::TK_KEYWORD, source);
      }
    }
    for (auto future : kFutureReservedWords) {
      if (source == future) {
        return Token(Token::Type::TK_FUTURE, source);
      }
      // TODO(zhuzilin) Check if source in kStrictModeFutureReservedWords
      // when stric mode code is supported.
    }
    return Token(Token::Type::TK_IDENT, source);
error:
    return Token(Token::Type::TK_ILLEGAL, source_.substr(start, pos_ - start)); 
  }

  char16_t c_;
  size_t pos_;
  size_t end_;
  std::u16string_view source_;
};

}

#endif  // ES_LEXER_H