#ifndef ES_EVALUATOR_H
#define ES_EVALUATOR_H

#include <math.h>

#include <es/types/completion.h>
#include <es/parser/character.h>
#include <es/parser/ast.h>
#include <es/helper.h>

namespace es {

class Evaluator {
 public:
  Evaluator() = default;

  Completion EvalStatement() {

  }

  Completion EvalExpression(AST* ast) {
    JSValue* val;
    switch (ast->type()) {
      case AST::AST_EXPR_NULL:
        val = Null::Instance();
        break;
      case AST::AST_EXPR_BOOL:
        val = ast->source() == u"true" ? Bool::True() : Bool::False();
        break;
      case AST::AST_EXPR_NUMBER:
        val = EvalNumber(ast);
        break;
    }
    return Completion(Completion::NORMAL, val, nullptr);
  }

  Number* EvalNumber(AST* num) {
    assert(num->type() == AST::AST_EXPR_NUMBER);
    auto source = num->source();
    double val = 0;
    double frac = 1;
    size_t pos = 0;
    bool dot = false;
    while (pos < source.size()) {
      char16_t c = source[pos];
      switch (c) {
        case u'.':
          dot = true;
          break;
        case u'e':
        case u'E': {
          double exp = 0;
          bool sign = true;
          pos++;  // skip e/E
          if (c == u'-') {
            sign = false;
            pos++;  // skip -
          } else if (c == u'+') {
            sign = true;
            pos++; // skip +;
          }
          while (pos < source.size()) {
            exp *= 10;
            exp += character::Digit(c);
          }
          if (!sign)
            exp = -exp;
          return new Number(val * pow(10.0, exp));
        }
        case u'x':
        case u'X': {
          assert(val == 0);
          pos++;
          while (pos < source.size()) {
            c = source[pos];
            val *= 16;
            val += character::Digit(c);
            pos++;
          }
          return new Number(val);
        }
        default:
          if (dot) {
            frac /= 10;
            val += character::Digit(c) * frac;
          } else {
            val *= 10;
            val += character::Digit(c);
          }
      }
      pos++;
    }
    return new Number(val);
  }

  String* EvalString(AST* ast) {
    auto source = ast->source();
    source = source.substr(1, source.size() - 2);
    size_t pos = 0;
    std::vector<std::u16string> vals;
    while (pos < source.size()) {
      char16_t c = source[pos];
      switch (c) {
        case u'\\': {
          pos++;
          c = source[pos];
          switch (c) {
            case u'b':
              pos++;
              vals.emplace_back(u"\b");
              break;
            case u't':
              pos++;
              vals.emplace_back(u"\t");
              break;
            case u'n':
              pos++;
              vals.emplace_back(u"\n");
              break;
            case u'v':
              pos++;
              vals.emplace_back(u"\v");
              break;
            case u'f':
              pos++;
              vals.emplace_back(u"\f");
              break;
            case u'r':
              pos++;
              vals.emplace_back(u"\r");
              break;
            case u'x': {
              pos++;  // skip 'x'
              char16_t hex = 0;
              for (size_t i = 0; i < 2; i++) {
                hex *= 16;
                hex += character::Digit(source[pos]);
                pos++;
              }
              vals.emplace_back(std::u16string(1, hex));
              break;
            }
            case u'u': {
              pos++;  // skip 'u'
              char16_t hex = 0;
              for (size_t i = 0; i < 4; i++) {
                hex *= 16;
                hex += character::Digit(source[pos]);
                pos++;
              }
              vals.emplace_back(std::u16string(1, hex));
              break;
            }
            default:
              c = source[pos];
              if (character::IsLineTerminator(c)) {
                pos++;
                continue;
              }
              pos++;
              vals.emplace_back(std::u16string(1, c));
          }
          break;
        }
        default: {
          size_t start = pos;
          while (true) {
            if (pos == source.size() || source[pos] == u'\\')
              break;
            pos++;
          }
          size_t end = pos;
          auto substr = source.substr(start, end - start);
          vals.emplace_back(std::u16string_view(substr.data(), substr.size()));
        }
      }
    }
    if (vals.size() == 0) {
      return new String(u"");
    } else if (vals.size() == 1) {
      return new String(vals[0]);
    }
    return new String(StrCat(vals));
  }

 private:
  
};

}  // namespace es

#endif  // ES_EVALUATOR_H