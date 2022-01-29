
#pragma once

#include <utility>
#include <algorithm>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <array>
#include <vector>
#include <unordered_map>
#include <map>
#include <bitset>
#include <set>
#include <stack>
#include <codecvt>
#include <locale>
#include <math.h>
#include <string.h>
#include <assert.h>

/*
 * The code in this file is extracted directly from iv.
 */


namespace es
{
    namespace character
    {
        namespace unicode
        {
            enum Category
            {
                UNASSIGNED = 0,// Cn
                UPPERCASE_LETTER = 1,// Lu
                LOWERCASE_LETTER = 2,// Ll
                TITLECASE_LETTER = 3,// Lt
                MODIFIER_LETTER = 4,// Lm
                OTHER_LETTER = 5,// Lo
                NON_SPACING_MARK = 6,// Mn
                ENCLOSING_MARK = 7,// Me
                COMBINING_SPACING_MARK = 8,// Mc
                DECIMAL_DIGIT_NUMBER = 9,// Nd
                LETTER_NUMBER = 10,// Nl
                OTHER_NUMBER = 11,// No
                SPACE_SEPARATOR = 12,// Zs
                LINE_SEPARATOR = 13,// Zl
                PARAGRAPH_SEPARATOR = 14,// Zp
                CONTROL = 15,// Cc
                FORMAT = 16,// Cf
                PRIVATE_USE = 18,// Co
                SURROGATE = 19,// Cs
                DASH_PUNCTUATION = 20,// Pd
                START_PUNCTUATION = 21,// Ps
                END_PUNCTUATION = 22,// Pe
                CONNECTOR_PUNCTUATION = 23,// Pc
                OTHER_PUNCTUATION = 24,// Po
                MATH_SYMBOL = 25,// Sm
                CURRENCY_SYMBOL = 26,// Sc
                MODIFIER_SYMBOL = 27,// Sk
                OTHER_SYMBOL = 28,// So
                INITIAL_QUOTE_PUNCTUATION = 29,// Pi
                FINAL_QUOTE_PUNCTUATION = 30// Pf
            };

            enum CategoryFlag
            {
                Cn = 1 << UNASSIGNED,
                Lu = 1 << UPPERCASE_LETTER,
                Ll = 1 << LOWERCASE_LETTER,
                Lt = 1 << TITLECASE_LETTER,
                Lm = 1 << MODIFIER_LETTER,
                Lo = 1 << OTHER_LETTER,
                Mn = 1 << NON_SPACING_MARK,
                Me = 1 << ENCLOSING_MARK,
                Mc = 1 << COMBINING_SPACING_MARK,
                Nd = 1 << DECIMAL_DIGIT_NUMBER,
                Nl = 1 << LETTER_NUMBER,
                No = 1 << OTHER_NUMBER,
                Zs = 1 << SPACE_SEPARATOR,
                Zl = 1 << LINE_SEPARATOR,
                Zp = 1 << PARAGRAPH_SEPARATOR,
                Cc = 1 << CONTROL,
                Cf = 1 << FORMAT,
                Co = 1 << PRIVATE_USE,
                Cs = 1 << SURROGATE,
                Pd = 1 << DASH_PUNCTUATION,
                Ps = 1 << START_PUNCTUATION,
                Pe = 1 << END_PUNCTUATION,
                Pc = 1 << CONNECTOR_PUNCTUATION,
                Po = 1 << OTHER_PUNCTUATION,
                Sm = 1 << MATH_SYMBOL,
                Sc = 1 << CURRENCY_SYMBOL,
                Sk = 1 << MODIFIER_SYMBOL,
                So = 1 << OTHER_SYMBOL,
                Pi = 1 << INITIAL_QUOTE_PUNCTUATION,
                Pf = 1 << FINAL_QUOTE_PUNCTUATION
            };

            // unicode bidirectional constant
            static const int kDirectionalityUndefined = -1;
            static const int kDirectionalityLeftToRight = 0;// L
            static const int kDirectionalityRightToLeft = 1;// R
            static const int kDirectionalityRightToLeftArabic = 2;// AL
            static const int kDirectionalityEnuropeanNumber = 3;// EN
            static const int kDirectionalityEnuropeanNumberSeparator = 4;// ES
            static const int kDirectionalityEnuropeanNumberTerminator = 5;// ET
            static const int kDirectionalityEnuropeanArabicNumber = 6;// AN
            static const int kDirectionalityCommonNumberSeparator = 7;// CS
            static const int kDirectionalityNonspacingMark = 8;// NSM
            static const int kDirectionalityBoundaryNeutral = 9;// BN
            static const int kDirectionalityParagraphSeparator = 10;// B
            static const int kDirectionalitySegmentSeparator = 11;// S
            static const int kDirectionalityWhiteSpace = 12;// WS
            static const int kDirectionalityOtherNeutrals = 13;// ON
            static const int kDirectionalityLeftToRightEmbedding = 14;// LRE
            static const int kDirectionalityLeftToRightOverride = 15;// LRO
            static const int kDirectionalityRightToLeftEmbedding = 16;// RLE
            static const int kDirectionalityRightToLeftOverride = 17;// RLO
            static const int kDirectionalityPopDirectionalFormat = 18;// PDF

            static const int kSize = 16;


            Category GetCategory(char16_t c);

        }// namespace unicode
    }// namespace character

    namespace log
    {
        inline void PrintSource(const std::string& comment, const std::string& str = "", const std::string& postfix = "")
        {
            (void)comment;
            (void)str;
            (void)postfix;
#ifdef TEST
            std::cout << comment;
            for(const auto& c : str)
                std::cout << static_cast<char>(c);
            std::cout << postfix << std::endl;
#endif
        }

        inline std::string ToString(const std::string& str)
        {
            std::string result(str.size(), ' ');
            for(size_t i = 0; i < str.size(); i++)
            {
                result[i] = str[i];
            }
            return result;
        }

        inline std::string ToString(bool b)
        {
            return b ? "true" : "false";
        }

        inline std::string ToString(const void* ptr)
        {
            std::stringstream ss;
            ss << ptr;
            return ss.str();
        }

    }// namespace log

    inline std::string StrCat(const std::vector<std::string>& vals)
    {
        size_t size;
        size_t offset;
        size = 0;
        for(const auto& val : vals)
        {
            size += val.size();
        }
        std::string res(size, 0);
        offset = 0;
        for(auto val : vals)
        {
            memcpy((void*)(res.c_str() + offset), (void*)(val.data()), val.size() * 2);
            offset += val.size();
        }
        return res;
    }

    inline bool HaveDuplicate(const std::vector<std::string>& vals)
    {
        size_t i;
        size_t j;
        for(i = 0; i < vals.size(); i++)
        {
            for(j = 0; j < vals.size(); j++)
            {
                if((i != j) && (vals[i] == vals[j]))
                {
                    return true;
                }
            }
        }
        return false;
    }

    namespace character
    {
        // uninames
        using namespace unicode;

        // End of Line
        static const char16_t CH_EOS = 0x0000;

        // Format-Control Character
        static const char16_t CH_ZWNJ = 0x200C;// Zero width non-joiner
        static const char16_t CH_ZWJ = 0x200D;// Zero width joiner
        static const char16_t CH_BOM = 0xFEFF;// Byte Order Mark

        // White Space
        static const char16_t CH_TAB = 0x0009;// Tab
        static const char16_t CH_VT = 0x000B;// Vertical Tab
        static const char16_t CH_FF = 0x0020;// Form Feed
        static const char16_t CH_SP = 0x0020;// Space
        static const char16_t CH_hashx0a = 0x00A0;// No-break space
        // static const char16_t CH_BOM  = 0xFEFF;  // Byte Order Mark
        // USP includes lots of characters, therefore only included in the function.

        // Line Terminators
        static const char16_t CH_LF = 0x000A;
        static const char16_t CH_CR = 0x000D;
        static const char16_t CH_LS = 0x2028;
        static const char16_t CH_PS = 0x2029;

        inline bool IsUSP(char16_t c)
        {
            return (
                (c == 0x1680) ||
                ((c >= 0x2000) && (c <= 0x200A)) ||
                (c == 0x202F) ||
                (c == 0x205F) ||
                (c == 0x3000)
            );
        }

        inline bool IsWhiteSpace(char16_t c)
        {
            return c == CH_TAB || c == CH_VT || c == CH_FF || c == CH_FF || c == CH_SP || c == CH_hashx0a || IsUSP(c);
        }

        inline bool IsLineTerminator(char16_t c)
        {
            return (
                (c == CH_LF) ||
                (c == CH_CR) ||
                (c == CH_LS) ||
                (c == CH_PS)
            );
        }

        inline bool IsDecimalDigit(char16_t c)
        {
            return ((c >= u'0') && (c <= u'9'));
        }

        inline bool IsUnicodeLetter(char16_t c)
        {
            return ((1 << GetCategory(c)) & (Lu | Ll | Lt | Lm | Lo | Nl)) != 0;
        }

        inline bool IsUnicodeCombiningMark(char16_t c)
        {
            return ((1 << GetCategory(c)) & (Mn | Mc)) != 0;
        }

        inline bool IsUnicodeDigit(char16_t c)
        {
            return GetCategory(c) == DECIMAL_DIGIT_NUMBER;
        }

        inline bool IsUnicodeConnectorPunctuation(char16_t c)
        {
            return GetCategory(c) == CONNECTOR_PUNCTUATION;
        }

        inline bool IsHexDigit(char16_t c)
        {
            return (
                IsDecimalDigit(c) || ((u'A' <= c) && (c <= u'F')) || ((u'a' <= c) && (c <= u'f'))
            );
        }

        inline bool IsSingleEscapeCharacter(int c)
        {
            return (
                (c == u'\'') ||
                (c == u'"') ||
                (c == u'\\') ||
                (c == u'b') ||
                (c == u'f') ||
                (c == u'f') ||
                (c == u'n') ||
                (c == u'r') ||
                (c == u't') ||
                (c == u'v')
            );
        }

        inline bool IsEscapeCharacter(int c)
        {
            return IsSingleEscapeCharacter(c) || IsDecimalDigit(c) || c == u'x' || c == u'u';
        }

        inline bool IsNonEscapeCharacter(int c)
        {
            return !IsEscapeCharacter(c) && !IsLineTerminator(c);
        }

        inline bool IsCharacterEscapeSequence(char16_t c)
        {
            // NODE(zhuzilin) The chars that are not in { LineTerminator, DecimalDigit, u'x', u'u' }.
            return IsSingleEscapeCharacter(c) || IsNonEscapeCharacter(c);
        }

        inline bool IsIdentifierStart(char16_t c)
        {
            return IsUnicodeLetter(c) || c == u'$' || c == u'_' || c == u'\\';
        }

        inline bool IsIdentifierPart(char16_t c)
        {
            return IsIdentifierStart(c) || IsUnicodeCombiningMark(c) || IsUnicodeDigit(c)
                   || IsUnicodeConnectorPunctuation(c) || c == CH_ZWNJ || c == CH_ZWJ;
        }

        inline bool IsRegularExpressionChar(int c)
        {
            return (
                !IsLineTerminator(c) &&
                (c != u'\\') &&
                (c != u'/') &&
                (c != u'[')
            );
        }

        inline bool IsRegularExpressionFirstChar(int c)
        {
            return (
                !IsLineTerminator(c) &&
                (c != u'*') &&
                (c != u'/')
            );
        }

        inline bool IsRegularExpressionClassChar(int c)
        {
            return (
                !IsLineTerminator(c) &&
                (c != u']')
            );
        }

        inline double Digit(int c)
        {
            switch(c)
            {
                case u'0':
                case u'1':
                case u'2':
                case u'3':
                case u'4':
                case u'5':
                case u'6':
                case u'7':
                case u'8':
                case u'9':
                    return c - u'0';
                case u'A':
                case u'a':
                    return 10;
                case u'B':
                case u'b':
                    return 11;
                case u'C':
                case u'c':
                    return 12;
                case u'D':
                case u'd':
                    return 13;
                case u'E':
                case u'e':
                    return 14;
                case u'F':
                case u'f':
                    return 15;
                default:
                    assert(false);
            }
        }
    }// namespace character

    class Token
    {
        public:
            // Order by section 7.5, 7.6, 7.7, 7.8
            enum Type
            {
                // Identifier
                TK_IDENT = 0,

                // Keywords
                TK_KEYWORD,

                // Future Reserved Words
                TK_FUTURE,

                // Punctuator
                TK_LBRACE,// {
                TK_RBRACE,// }
                TK_LPAREN,// (
                TK_RPAREN,// )
                TK_LBRACK,// [
                TK_RBRACK,// ]

                TK_DOT,// .
                TK_SEMICOLON,// ;
                TK_COMMA,// ,
                TK_QUESTION,// ?
                TK_COLON,// :

                TK_LT,// <
                TK_GT,// >
                TK_LE,// <=
                TK_GE,// >=
                TK_EQ,// ==
                TK_NE,// !=
                TK_EQ3,// ===
                TK_NE3,// !==

                TK_ADD,// +
                TK_SUB,// -
                TK_MUL,// *
                TK_MOD,// %
                TK_INC,// ++
                TK_DEC,// --

                TK_BIT_LSH,// <<
                TK_BIT_RSH,// >>
                TK_BIT_URSH,// >>>, unsigned right shift

                TK_BIT_AND,// &
                TK_BIT_OR,// |
                TK_BIT_XOR,// ^
                TK_BIT_NOT,// ~

                TK_LOGICAL_AND,// &&
                TK_LOGICAL_OR,// ||
                TK_LOGICAL_NOT,// !

                TK_ASSIGN,// =
                TK_ADD_ASSIGN,// +=
                TK_SUB_ASSIGN,// -=
                TK_MUL_ASSIGN,// *=
                TK_MOD_ASSIGN,// %=

                TK_BIT_LSH_ASSIGN,// <<=
                TK_BIT_RSH_ASSIGN,// >>=
                TK_BIT_URSH_ASSIGN,// >>>=
                TK_BIT_AND_ASSIGN,// &=
                TK_BIT_OR_ASSIGN,// |=
                TK_BIT_XOR_ASSIGN,// ^=

                // DivPunctuator
                TK_DIV,// /
                TK_DIV_ASSIGN,// /=

                // Null Literal
                TK_NULL,// null

                // Bool Literal
                TK_BOOL,// true & false

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

        private:
            Type m_type;
            std::string m_source;

        public:
            Token(Type type, const std::string& source) : m_type(type), m_source(source)
            {
            }

            inline bool IsAssignmentOperator()
            {
                switch(m_type)
                {
                    case TK_ASSIGN:// =
                    case TK_ADD_ASSIGN:// +=
                    case TK_SUB_ASSIGN:// -=
                    case TK_MUL_ASSIGN:// *=
                    case TK_MOD_ASSIGN:// %=
                    case TK_DIV_ASSIGN:// /=

                    case TK_BIT_LSH_ASSIGN:// <<=
                    case TK_BIT_RSH_ASSIGN:// >>=
                    case TK_BIT_URSH_ASSIGN:// >>>=
                    case TK_BIT_AND_ASSIGN:// &=
                    case TK_BIT_OR_ASSIGN:// |=
                    case TK_BIT_XOR_ASSIGN:// ^=
                        return true;
                    default:
                        return false;
                }
            }

            inline bool IsLineTerminator()
            {
                return m_type == TK_LINE_TERM;
            }

            inline bool IsIdentifierName()
            {
                return m_type == TK_IDENT || m_type == TK_KEYWORD || m_type == TK_FUTURE;
            }

            inline bool IsPropertyName()
            {
                return IsIdentifierName() || m_type == TK_STRING || m_type == TK_NUMBER;
            }

            inline bool IsSemiColon()
            {
                return m_type == TK_SEMICOLON;
            }

            inline bool IsIdentifier()
            {
                return m_type == TK_IDENT;
            }

            inline int BinaryPriority(bool no_in)
            {
                switch(m_type)
                {
                    // Binary
                    case TK_LOGICAL_OR:// ||
                        return 2;
                    case TK_LOGICAL_AND:// &&
                        return 3;
                    case TK_BIT_OR:// |
                        return 4;
                    case TK_BIT_XOR:// ^
                        return 5;
                    case TK_BIT_AND:// &
                        return 6;
                    case TK_EQ:// ==
                    case TK_NE:// !=
                    case TK_EQ3:// ===
                    case TK_NE3:// !==
                        return 7;
                    case TK_LT:// <
                    case TK_GT:// >
                    case TK_LE:// <=
                    case TK_GE:// >=
                        return 8;
                    case TK_BIT_LSH:// <<
                    case TK_BIT_RSH:// >>
                    case TK_BIT_URSH:// >>>
                        return 9;
                    case TK_ADD:
                    case TK_SUB:
                        return 10;
                    case TK_MUL:
                    case TK_DIV:
                    case TK_MOD:
                        return 11;

                    case TK_KEYWORD:
                        if(m_source == "instanceof")
                        {
                            return 8;
                            // To prevent parsing for(a in b).
                        }
                        else if(!no_in && m_source == "in")
                        {
                            return 8;
                        }
                    default:
                        return -1;
                }
            }

            inline int UnaryPrefixPriority()
            {
                switch(m_type)
                {
                    // Prefix
                    case TK_INC:
                    case TK_DEC:
                    case TK_ADD:
                    case TK_SUB:
                    case TK_BIT_NOT:
                    case TK_LOGICAL_NOT:
                        return 100;// UnaryExpresion always have higher priority.

                    case TK_KEYWORD:
                        if(m_source == "delete" || m_source == "void" || m_source == "typeof")
                        {
                            return 100;
                        }
                    default:
                        return -1;
                }
            }

            inline int UnaryPostfixPriority()
            {
                switch(m_type)
                {
                    // Prefix
                    case TK_INC:
                    case TK_DEC:
                        return 200;// UnaryExpresion always have higher priority.
                    default:
                        return -1;
                }
            }

            Type type()
            {
                return m_type;
            }
            std::string source()
            {
                return m_source;
            }
    };

    static const auto kKeywords =
    std::to_array<std::string>({
        "break",   "do",     "instanceof", "typeof",   "case",  "else",   "new",   "var",      "catch",
        "finally", "return", "void",       "continue", "for",   "switch", "while", "debugger", "function",
        "this",    "with",   "default",    "if",       "throw", "delete", "in",    "try",
    });

    static const auto kFutureReservedWords = std::to_array<std::string>(
    {
        "class", "enum", "extends", "super", "const", "export", "import",
    });

    static const auto kStrictModeFutureReservedWords = std::to_array<std::string>(
    {
        "implements",
        "let",
        "private",
        "public",
        "yield"
        "interface",
        "package",
        "protected",
        "static"
    });

    class Lexer
    {
        private:
            int c_;
            std::string m_source;
            size_t pos_;
            size_t end_;
            Token token_;

        private:
            inline int LookAhead()
            {
                if(pos_ + 1 >= end_)
                {
                    return character::CH_EOS;
                }
                return m_source[pos_ + 1];
            }

            inline void Advance()
            {
                if(pos_ < end_)
                {
                    pos_++;
                }
                UpdateC();
            }

            inline void UpdateC()
            {
                if(pos_ < end_)
                {
                    c_ = m_source[pos_];
                }
                else
                {
                    c_ = character::CH_EOS;
                }
            }

            bool SkipRegularExpressionBackslashSequence()
            {
                assert(c_ == u'\\');
                Advance();
                if(character::IsLineTerminator(c_))
                {
                    return false;
                }
                Advance();
                return true;
            }

            void SkipRegularExpressionChars()
            {
                while(c_ != character::CH_EOS && character::IsRegularExpressionChar(c_))
                {
                    Advance();
                }
            }

            bool SkipRegularExpressionClass()
            {
                assert(c_ == u'[');
                Advance();
                while(c_ != character::CH_EOS && character::IsRegularExpressionClassChar(c_))
                {
                    switch(c_)
                    {
                        case u'\\':
                        {
                            if(!SkipRegularExpressionBackslashSequence())
                            {
                                return false;
                            }
                            break;
                        }
                        default:
                            Advance();
                    }
                }
                return c_ == u']';
            }

            void SkipMultiLineComment()
            {
                while(c_ != character::CH_EOS)
                {
                    if(c_ == u'*')
                    {
                        Advance();
                        if(c_ == u'/')
                        {
                            Advance();
                            break;
                        }
                    }
                    else
                    {
                        Advance();
                    }
                }
            }

            void SkipSingleLineComment()
            {
                // This will not skip line terminators.
                while(c_ != character::CH_EOS && !character::IsLineTerminator(c_))
                {
                    Advance();
                }
            }

            void SkipWhiteSpace()
            {
                while(character::IsWhiteSpace(c_))
                {
                    Advance();
                }
            }

            Token ScanLineTerminatorSequence()
            {
                assert(character::IsLineTerminator(c_));
                size_t start = pos_;
                if(c_ == character::CH_CR && LookAhead() == character::CH_LF)
                {
                    Advance();
                    Advance();
                }
                else
                {
                    Advance();
                }
                return Token(Token::TK_LINE_TERM, m_source.substr(start, pos_ - start));
            }

            void SkipLineTerminatorSequence()
            {
                assert(character::IsLineTerminator(c_));
                if(c_ == character::CH_CR && LookAhead() == character::CH_LF)
                {
                    Advance();
                    Advance();
                }
                else
                {
                    Advance();
                }
            }

            Token ScanStringLiteral()
            {
                int quote = c_;
                size_t start = pos_;
                Advance();
                while(c_ != character::CH_EOS && c_ != quote && !character::IsLineTerminator(c_))
                {
                    switch(c_)
                    {
                        case u'\\':
                        {
                            Advance();
                            // TODO(zhuzilin) Find out if "\1" will trigger error.
                            switch(c_)
                            {
                                case u'0':
                                {
                                    Advance();
                                    if(character::IsDecimalDigit(LookAhead()))
                                    {
                                        Advance();
                                        goto error;
                                    }
                                    break;
                                }
                                case u'x':
                                {// HexEscapeSequence
                                    Advance();
                                    for(size_t i = 0; i < 2; i++)
                                    {
                                        if(!character::IsHexDigit(c_))
                                        {
                                            Advance();
                                            goto error;
                                        }
                                        Advance();
                                    }
                                    break;
                                }
                                case u'u':
                                {// UnicodeEscapeSequence
                                    if(!SkipUnicodeEscapeSequence())
                                    {
                                        Advance();
                                        goto error;
                                    }
                                    break;
                                }
                                default:
                                    if(character::IsLineTerminator(c_))
                                    {
                                        SkipLineTerminatorSequence();
                                    }
                                    else if(character::IsCharacterEscapeSequence(c_))
                                    {
                                        Advance();
                                    }
                                    else
                                    {
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

                if(c_ == quote)
                {
                    Advance();
                    return Token(Token::Type::TK_STRING, m_source.substr(start, pos_ - start));
                }
            error:
                return Token(Token::Type::TK_ILLEGAL, m_source.substr(start, pos_ - start));
            }

            bool SkipAtLeastOneDecimalDigit()
            {
                if(!character::IsDecimalDigit(c_))
                {
                    return false;
                }
                while(character::IsDecimalDigit(c_))
                {
                    Advance();
                }
                return true;
            }

            bool SkipAtLeastOneHexDigit()
            {
                if(!character::IsHexDigit(c_))
                {
                    return false;
                }
                while(character::IsHexDigit(c_))
                {
                    Advance();
                }
                return true;
            }

            Token ScanNumericLiteral()
            {
                assert(c_ == u'.' || character::IsDecimalDigit(c_));
                size_t start = pos_;

                bool is_hex = false;
                switch(c_)
                {
                    case u'0':
                    {
                        Advance();
                        switch(c_)
                        {
                            case u'x':
                            case u'X':
                            {// HexIntegerLiteral
                                Advance();
                                if(!SkipAtLeastOneHexDigit())
                                {
                                    Advance();
                                    goto error;
                                }
                                is_hex = true;
                                break;
                            }
                            case u'.':
                            {
                                Advance();
                                if(!SkipAtLeastOneDecimalDigit())
                                {
                                    Advance();
                                    goto error;
                                }
                                break;
                            }
                        }
                        break;
                    }
                    case u'.':
                    {
                        Advance();
                        if(!SkipAtLeastOneDecimalDigit())
                        {
                            Advance();
                            goto error;
                        }
                        break;
                    }
                    default:// NonZeroDigit
                        SkipAtLeastOneDecimalDigit();
                        if(c_ == u'.')
                        {
                            Advance();
                            if(!SkipAtLeastOneDecimalDigit())
                            {
                                Advance();
                                goto error;
                            }
                        }
                }

                if(!is_hex)
                {// ExponentPart
                    if(c_ == u'e' || c_ == u'E')
                    {
                        Advance();
                        if(c_ == u'+' || c_ == u'-')
                        {
                            Advance();
                        }
                        if(!SkipAtLeastOneDecimalDigit())
                        {
                            Advance();
                            goto error;
                        }
                    }
                }

                // The source character immediately following a NumericLiteral must not
                // be an IdentifierStart or DecimalDigit.
                if(character::IsIdentifierStart(c_) || character::IsDecimalDigit(c_))
                {
                    Advance();
                    goto error;
                }
                return Token(Token::Type::TK_NUMBER, m_source.substr(start, pos_ - start));
            error:
                return Token(Token::Type::TK_ILLEGAL, m_source.substr(start, pos_ - start));
            }

            bool SkipUnicodeEscapeSequence()
            {
                if(c_ != u'u')
                {
                    return false;
                }
                Advance();
                for(size_t i = 0; i < 4; i++)
                {
                    if(!character::IsHexDigit(c_))
                    {
                        return false;
                    }
                    Advance();
                }
                return true;
            }

            Token ScanIdentifier()
            {
                assert(character::IsIdentifierStart(c_));
                size_t start = pos_;
                std::string source;
                if(c_ == u'\\')
                {
                    Advance();
                    if(!SkipUnicodeEscapeSequence())
                    {
                        Advance();
                        goto error;
                    }
                }
                else
                {
                    Advance();
                }

                while(character::IsIdentifierPart(c_))
                {
                    if(c_ == u'\\')
                    {
                        Advance();
                        if(!SkipUnicodeEscapeSequence())
                        {
                            Advance();
                            goto error;
                        }
                    }
                    else
                    {
                        Advance();
                    }
                }

                source = m_source.substr(start, pos_ - start);
                if(source == "null")
                {
                    return Token(Token::Type::TK_NULL, source);
                }
                if(source == "true" || source == "false")
                {
                    return Token(Token::Type::TK_BOOL, source);
                }
                for(const auto& keyword : kKeywords)
                {
                    if(source == keyword)
                    {
                        return Token(Token::Type::TK_KEYWORD, source);
                    }
                }
                for(const auto& future : kFutureReservedWords)
                {
                    if(source == future)
                    {
                        return Token(Token::Type::TK_FUTURE, source);
                    }
                    // TODO(zhuzilin) Check if source in kStrictModeFutureReservedWords
                    // when stric mode code is supported.
                }
                return Token(Token::Type::TK_IDENT, source);
            error:
                return Token(Token::Type::TK_ILLEGAL, m_source.substr(start, pos_ - start));
            }

        public:
            Lexer(const std::string& source):
                c_(0),
                m_source(source),
                pos_(0),
                end_(source.size()),
                token_(Token::Type::TK_NOT_FOUND, "")
            {
                UpdateC();
            }

            Token Next(bool line_terminator = false)
            {
                Token token = Token(Token::Type::TK_NOT_FOUND, "");
                do
                {
                    size_t start = pos_;
                    switch(c_)
                    {
                        case character::CH_EOS:
                        {
                            token = Token(Token::Type::TK_EOS, m_source.substr(pos_, 0));
                            break;
                        }

                        case u'{':
                        {
                            Advance();
                            token = Token(Token::Type::TK_LBRACE, m_source.substr(start, 1));
                            break;
                        }
                        case u'}':
                        {
                            Advance();
                            token = Token(Token::Type::TK_RBRACE, m_source.substr(start, 1));
                            break;
                        }
                        case u'(':
                        {
                            Advance();
                            token = Token(Token::Type::TK_LPAREN, m_source.substr(start, 1));
                            break;
                        }
                        case u')':
                        {
                            Advance();
                            token = Token(Token::Type::TK_RPAREN, m_source.substr(start, 1));
                            break;
                        }
                        case u'[':
                        {
                            Advance();
                            token = Token(Token::Type::TK_LBRACK, m_source.substr(start, 1));
                            break;
                        }
                        case u']':
                        {
                            Advance();
                            token = Token(Token::Type::TK_RBRACK, m_source.substr(start, 1));
                            break;
                        }
                        case u'.':
                        {
                            if(character::IsDecimalDigit(LookAhead()))
                            {
                                token = ScanNumericLiteral();
                            }
                            else
                            {
                                token = Token(Token::Type::TK_DOT, m_source.substr(start, 1));
                                Advance();
                            }
                            break;
                        }
                        case u';':
                        {
                            Advance();
                            token = Token(Token::Type::TK_SEMICOLON, m_source.substr(start, 1));
                            break;
                        }
                        case u',':
                        {
                            Advance();
                            token = Token(Token::Type::TK_COMMA, m_source.substr(start, 1));
                            break;
                        }
                        case u'?':
                        {
                            Advance();
                            token = Token(Token::Type::TK_QUESTION, m_source.substr(start, 1));
                            break;
                        }
                        case u':':
                        {
                            Advance();
                            token = Token(Token::Type::TK_COLON, m_source.substr(start, 1));
                            break;
                        }

                        case u'<':
                        {
                            Advance();
                            switch(c_)
                            {
                                case u'<':
                                    Advance();
                                    switch(c_)
                                    {
                                        case u'=':// <<=
                                            Advance();
                                            token = Token(Token::Type::TK_BIT_LSH_ASSIGN, m_source.substr(start, 3));
                                            break;
                                        default:// <<
                                            token = Token(Token::Type::TK_BIT_LSH, m_source.substr(start, 2));
                                    }
                                    break;
                                case u'=':// <=
                                    Advance();
                                    token = Token(Token::Type::TK_LE, m_source.substr(start, 2));
                                    break;
                                default:// <
                                    token = Token(Token::Type::TK_LT, m_source.substr(start, 1));
                            }
                            break;
                        }
                        case u'>':
                        {
                            Advance();
                            switch(c_)
                            {
                                case u'>':
                                    Advance();
                                    switch(c_)
                                    {
                                        case u'>':
                                            Advance();
                                            switch(c_)
                                            {
                                                case u'=':// >>>=
                                                    Advance();
                                                    token = Token(Token::Type::TK_BIT_URSH_ASSIGN, m_source.substr(start, 4));
                                                    break;
                                                default:// >>>
                                                    token = Token(Token::Type::TK_BIT_URSH, m_source.substr(start, 3));
                                            }
                                            break;
                                        case u'=':// >>=
                                            token = Token(Token::Type::TK_BIT_RSH_ASSIGN, m_source.substr(start, 3));
                                            Advance();
                                            break;
                                        default:// >>
                                            token = Token(Token::Type::TK_BIT_RSH, m_source.substr(start, 2));
                                    }
                                    break;
                                case u'=':// >=
                                    Advance();
                                    token = Token(Token::Type::TK_GE, m_source.substr(start, 2));
                                    break;
                                default:// >
                                    token = Token(Token::Type::TK_GT, m_source.substr(start, 1));
                            }
                            break;
                        }
                        case u'=':
                        {
                            Advance();
                            switch(c_)
                            {
                                case u'=':
                                    Advance();
                                    switch(c_)
                                    {
                                        case u'=':// ===
                                            Advance();
                                            token = Token(Token::Type::TK_EQ3, m_source.substr(start, 3));
                                            break;
                                        default:// ==
                                            token = Token(Token::Type::TK_EQ, m_source.substr(start, 2));
                                            break;
                                    }
                                    break;
                                default:// =
                                    token = Token(Token::Type::TK_ASSIGN, m_source.substr(start, 1));
                            }
                            break;
                        }
                        case u'!':
                        {
                            Advance();
                            switch(c_)
                            {
                                case u'=':
                                    Advance();
                                    switch(c_)
                                    {
                                        case u'=':// !==
                                            Advance();
                                            token = Token(Token::Type::TK_NE3, m_source.substr(start, 3));
                                            break;
                                        default:// !=
                                            token = Token(Token::Type::TK_NE, m_source.substr(start, 2));
                                            break;
                                    }
                                    break;
                                default:// !
                                    token = Token(Token::Type::TK_LOGICAL_NOT, m_source.substr(start, 1));
                            }
                            break;
                        }

                        case u'+':
                        {
                            Advance();
                            switch(c_)
                            {
                                case u'+':// ++
                                    Advance();
                                    token = Token(Token::Type::TK_INC, m_source.substr(start, 2));
                                    break;
                                case u'=':// +=
                                    Advance();
                                    token = Token(Token::Type::TK_ADD_ASSIGN, m_source.substr(start, 2));
                                    break;
                                default:// +
                                    token = Token(Token::Type::TK_ADD, m_source.substr(start, 1));
                            }
                            break;
                        }
                        case u'-':
                        {
                            Advance();
                            switch(c_)
                            {
                                case u'-':// --
                                    Advance();
                                    token = Token(Token::Type::TK_DEC, m_source.substr(start, 2));
                                    break;
                                case u'=':// -=
                                    Advance();
                                    token = Token(Token::Type::TK_SUB_ASSIGN, m_source.substr(start, 2));
                                default:// -
                                    token = Token(Token::Type::TK_SUB, m_source.substr(start, 1));
                            }
                            break;
                        }
                        case u'*':
                        {
                            Advance();
                            if(c_ == u'=')
                            {// *=
                                Advance();
                                token = Token(Token::Type::TK_MUL_ASSIGN, m_source.substr(start, 2));
                            }
                            else
                            {// +
                                token = Token(Token::Type::TK_MUL, m_source.substr(start, 1));
                            }
                            break;
                        }
                        case u'%':
                        {
                            Advance();
                            if(c_ == u'=')
                            {// %=
                                Advance();
                                token = Token(Token::Type::TK_MOD_ASSIGN, m_source.substr(start, 2));
                            }
                            else
                            {// %
                                token = Token(Token::Type::TK_MOD, m_source.substr(start, 1));
                            }
                            break;
                        }

                        case u'&':
                        {
                            Advance();
                            switch(c_)
                            {
                                case u'&':// &&
                                    Advance();
                                    token = Token(Token::Type::TK_LOGICAL_AND, m_source.substr(start, 2));
                                    break;
                                case u'=':// &=
                                    Advance();
                                    token = Token(Token::Type::TK_BIT_AND_ASSIGN, m_source.substr(start, 2));
                                    break;
                                default:// &
                                    token = Token(Token::Type::TK_BIT_AND, m_source.substr(start, 1));
                            }
                            break;
                        }
                        case u'|':
                        {
                            Advance();
                            switch(c_)
                            {
                                case u'|':// ||
                                    Advance();
                                    token = Token(Token::Type::TK_LOGICAL_OR, m_source.substr(start, 2));
                                    break;
                                case u'=':// |=
                                    Advance();
                                    token = Token(Token::Type::TK_BIT_OR_ASSIGN, m_source.substr(start, 2));
                                    break;
                                default:// |
                                    token = Token(Token::Type::TK_BIT_OR, m_source.substr(start, 1));
                            }
                            break;
                        }
                        case u'^':
                        {
                            Advance();
                            if(c_ == u'=')
                            {// ^=
                                Advance();
                                token = Token(Token::Type::TK_BIT_XOR_ASSIGN, m_source.substr(start, 2));
                            }
                            else
                            {
                                token = Token(Token::Type::TK_BIT_XOR, m_source.substr(start, 1));
                            }
                            break;
                        }
                        case u'~':
                        {
                            Advance();
                            token = Token(Token::Type::TK_BIT_NOT, m_source.substr(start, 1));
                            break;
                        }

                        case u'/':
                        {
                            Advance();
                            switch(c_)
                            {
                                case u'*':// /*
                                    Advance();
                                    SkipMultiLineComment();
                                    break;
                                case u'/':// //
                                    Advance();
                                    SkipSingleLineComment();
                                    break;
                                case u'=':// /=
                                    Advance();
                                    token = Token(Token::Type::TK_DIV_ASSIGN, m_source.substr(start, 2));
                                    break;
                                default:// /
                                    // We cannot distinguish DIV and regex in lexer level and therefore,
                                    // we need to check if the symbol of div operator or start of regex
                                    // in parser.
                                    token = Token(Token::Type::TK_DIV, m_source.substr(start, 1));
                            }
                            break;
                        }

                        case u'\'':
                        case u'"':
                        {
                            token = ScanStringLiteral();
                            break;
                        }

                        default:
                            if(character::IsWhiteSpace(c_))
                            {
                                SkipWhiteSpace();
                            }
                            else if(character::IsLineTerminator(c_))
                            {
                                if(line_terminator)
                                {
                                    token = ScanLineTerminatorSequence();
                                }
                                else
                                {
                                    SkipLineTerminatorSequence();
                                }
                            }
                            else if(character::IsDecimalDigit(c_))
                            {
                                token = ScanNumericLiteral();
                            }
                            else if(character::IsIdentifierStart(c_))
                            {
                                token = ScanIdentifier();
                            }
                            else
                            {
                                Advance();
                                token = Token(Token::TK_ILLEGAL, m_source.substr(start, 1));
                            }
                    }
                } while(token.type() == Token::Type::TK_NOT_FOUND);
                token_ = token;
                return token_;
            }

            inline Token Last()
            {
                return token_;
            }
            inline size_t Pos()
            {
                return pos_;
            }

            void Rewind(size_t pos, Token token)
            {
                pos_ = pos;
                token_ = std::move(token);
                UpdateC();
            }

            Token NextAndRewind(bool line_terminator = false)
            {
                size_t old_pos = Pos();
                Token old_token = Last();
                Token token = Next(line_terminator);
                Rewind(old_pos, old_token);
                return token;
            }

            bool LineTermAhead()
            {
                return NextAndRewind(true).IsLineTerminator();
            }

            bool TrySkipSemiColon()
            {
                Token token = NextAndRewind();
                if(token.IsSemiColon())
                {
                    Next();
                    return true;
                }
                // 7.9 Automatic Semicolon Insertion
                return token.type() == Token::TK_EOS || token.type() == Token::TK_RBRACE || LineTermAhead();
            }

            Token ScanRegexLiteral()
            {
                assert(c_ == u'/');
                size_t start = pos_;
                Advance();
                if(!character::IsRegularExpressionFirstChar(c_))
                {
                    Advance();
                    goto error;
                }
                while(c_ != character::CH_EOS && c_ != u'/' && !character::IsLineTerminator(c_))
                {
                    switch(c_)
                    {
                        case u'\\':
                        {// Regular Expression
                            if(!SkipRegularExpressionBackslashSequence())
                            {
                                Advance();
                                goto error;
                            }
                            break;
                        }
                        case u'[':
                        {
                            if(!SkipRegularExpressionClass())
                            {
                                Advance();
                                goto error;
                            }
                            break;
                        }
                        default:
                            SkipRegularExpressionChars();
                    }
                }

                if(c_ == u'/')
                {
                    Advance();
                    // RegularExpressionFlags
                    while(character::IsIdentifierPart(c_))
                    {
                        if(c_ == u'\\')
                        {
                            Advance();
                            if(!SkipUnicodeEscapeSequence())
                            {
                                Advance();
                                goto error;
                            }
                        }
                        else
                        {
                            Advance();
                        }
                    }
                    token_ = Token(Token::Type::TK_REGEX, m_source.substr(start, pos_ - start));
                    return token_;
                }
            error:
                token_ = Token(Token::Type::TK_ILLEGAL, m_source.substr(start, pos_ - start));
                return token_;
            }

            // For regex
            inline void Back()
            {
                if(pos_ == 0)
                {
                    return;
                }
                pos_--;
                UpdateC();
            }
    };

    class AST
    {
    public:
        enum Type
        {
            AST_EXPR_THIS,
            AST_EXPR_IDENT,

            AST_EXPR_NULL,
            AST_EXPR_BOOL,
            AST_EXPR_NUMBER,
            AST_EXPR_STRING,
            AST_EXPR_REGEX,

            AST_EXPR_ARRAY,
            AST_EXPR_OBJ,

            AST_EXPR_PAREN,// ( Expression )

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

    private:
        Type m_type;
        std::string m_source;
        std::string label_;

    public:
        AST(Type type, const std::string& source = "") : m_type(type), m_source(source)
        {
        }
        virtual ~AST(){};

        Type type()
        {
            return m_type;
        }
        std::string source()
        {
            return m_source;
        }

        void SetSource(const std::string& source)
        {
            m_source = source;
        }

        bool IsIllegal()
        {
            return m_type == AST_ILLEGAL;
        }

        std::string label()
        {
            return label_;
        }
        void SetLabel(const std::string& label)
        {
            label_ = label;
        }
    };

    class ArrayLiteral : public AST
    {
        private:
            std::vector<std::pair<size_t, AST*>> elements_;
            size_t len_;

        public:
            ArrayLiteral() : AST(AST_EXPR_ARRAY), len_(0)
            {
            }

            ~ArrayLiteral() override
            {
                for(auto pair : elements_)
                {
                    delete pair.second;
                }
            }

            size_t length()
            {
                return len_;
            }
            std::vector<std::pair<size_t, AST*>> elements()
            {
                return elements_;
            }

            void AddElement(AST* element)
            {
                if(element != nullptr)
                {
                    elements_.emplace_back(len_, element);
                }
                len_++;
            }
    };

    class ObjectLiteral : public AST
    {
        public:
            struct Property
            {
                enum Type
                {
                    NORMAL = 0,
                    GET,
                    SET,
                };

                Token key;
                AST* value;
                Type type;

                Property(Token k, AST* v, Type t) : key(std::move(k)), value(v), type(t)
                {
                }
            };

        private:
            std::vector<Property> properties_;

        public:
            ObjectLiteral() : AST(AST_EXPR_OBJ)
            {
            }

            ~ObjectLiteral() override
            {
                for(const auto& property : properties_)
                {
                    delete property.value;
                }
            }

            void AddProperty(const Property& p)
            {
                properties_.emplace_back(p);
            }

            std::vector<Property> properties()
            {
                return properties_;
            }

            size_t length()
            {
                return properties_.size();
            }


    };

    class Paren : public AST
    {
        private:
            AST* expr_;

        public:
            Paren(AST* expr, const std::string& source) : AST(AST_EXPR_PAREN, source), expr_(expr)
            {
            }

            AST* expr()
            {
                return expr_;
            }
    };

    class Binary : public AST
    {
        private:
            AST* lhs_;
            AST* rhs_;
            Token op_;

        public:
            Binary(AST* lhs, AST* rhs, Token op, const std::string& source = "")
            : AST(AST_EXPR_BINARY, source), lhs_(lhs), rhs_(rhs), op_(std::move(op))
            {
            }

            ~Binary() override
            {
                delete lhs_;
                delete rhs_;
            }

            AST* lhs()
            {
                return lhs_;
            }
            AST* rhs()
            {
                return rhs_;
            }
            std::string op()
            {
                return op_.source();
            }
    };

    class Unary : public AST
    {
        private:
            AST* node_;
            Token op_;
            bool prefix_;

        public:
            Unary(AST* node, Token op, bool prefix) : AST(AST_EXPR_UNARY), node_(node), op_(std::move(op)), prefix_(prefix)
            {
            }

            ~Unary() override
            {
                delete node_;
            }

            AST* node()
            {
                return node_;
            }
            Token op()
            {
                return op_;
            }
            bool prefix()
            {
                return prefix_;
            }
    };

    class TripleCondition : public AST
    {
        private:
            AST* cond_;
            AST* true_expr_;
            AST* false_expr_;

        public:
            TripleCondition(AST* cond, AST* true_expr, AST* false_expr)
            : AST(AST_EXPR_TRIPLE), cond_(cond), true_expr_(true_expr), false_expr_(false_expr)
            {
            }

            ~TripleCondition() override
            {
                delete cond_;
                delete true_expr_;
                delete false_expr_;
            }

            AST* cond()
            {
                return cond_;
            }
            AST* true_expr()
            {
                return true_expr_;
            }
            AST* false_expr()
            {
                return false_expr_;
            }

    };

    class Expression : public AST
    {
        private:
            std::vector<AST*> elements_;

        public:
            Expression() : AST(AST_EXPR)
            {
            }
            ~Expression() override
            {
                for(auto element : elements_)
                {
                    delete element;
                }
            }

            void AddElement(AST* element)
            {
                elements_.push_back(element);
            }

            std::vector<AST*> elements()
            {
                return elements_;
            }
    };

    class Arguments : public AST
    {
        private:
            std::vector<AST*> args_;

        public:
            Arguments(const std::vector<AST*>& args) : AST(AST_EXPR_ARGS), args_(args)
            {
            }

            ~Arguments() override
            {
                for(auto arg : args_)
                {
                    delete arg;
                }
            }

            std::vector<AST*> args()
            {
                return args_;
            }
    };

    class LHS : public AST
    {
        public:
            enum PostfixType
            {
                CALL,
                INDEX,
                PROP,
            };

        private:
            AST* base_;
            size_t new_count_;

            std::vector<std::pair<size_t, PostfixType>> order_;
            std::vector<Arguments*> args_list_;
            std::vector<AST*> index_list_;
            std::vector<std::string> prop_name_list_;

        public:
            LHS(AST* base, size_t new_count) : AST(AST_EXPR_LHS), base_(base), new_count_(new_count)
            {
            }

            ~LHS() override
            {
                for(auto args : args_list_)
                {
                    delete args;
                }
                for(auto index : index_list_)
                {
                    delete index;
                }
            }

            void AddArguments(Arguments* args)
            {
                order_.emplace_back(std::make_pair(args_list_.size(), CALL));
                args_list_.emplace_back(args);
            }

            void AddIndex(AST* index)
            {
                order_.emplace_back(std::make_pair(index_list_.size(), INDEX));
                index_list_.emplace_back(index);
            }

            void AddProp(Token prop_name)
            {
                order_.emplace_back(std::make_pair(prop_name_list_.size(), PROP));
                prop_name_list_.emplace_back(prop_name.source());
            }

            AST* base()
            {
                return base_;
            }
            size_t new_count()
            {
                return new_count_;
            }
            std::vector<std::pair<size_t, PostfixType>> order()
            {
                return order_;
            }
            std::vector<Arguments*> args_list()
            {
                return args_list_;
            }
            std::vector<AST*> index_list()
            {
                return index_list_;
            }
            std::vector<std::string> prop_name_list()
            {
                return prop_name_list_;
            }

    };

    class Function : public AST
    {
        private:
            Token name_;
            std::vector<std::string> params_;
            AST* body_;

        public:
            Function(const std::vector<std::string>& params, AST* body, const std::string& source)
            : Function(Token(Token::TK_NOT_FOUND, ""), params, body, source)
            {
            }

            Function(Token name, const std::vector<std::string>& params, AST* body, const std::string& source)
            : AST(AST_FUNC, source), name_(std::move(name)), params_(params)
            {
                assert(body->type() == AST::AST_FUNC_BODY);
                body_ = body;
            }

            ~Function() override
            {
                delete body_;
            }

            bool is_named()
            {
                return name_.type() != Token::TK_NOT_FOUND;
            }
            std::string name()
            {
                return name_.source();
            }
            std::vector<std::string> params()
            {
                return params_;
            }
            AST* body()
            {
                return body_;
            }
    };

    class ProgramOrFunctionBody : public AST
    {
        private:
            bool strict_;
            std::vector<Function*> func_decls_;
            std::vector<AST*> stmts_;

        public:
            ProgramOrFunctionBody(Type type, bool strict) : AST(type), strict_(strict)
            {
            }
            ~ProgramOrFunctionBody() override
            {
                for(auto func_decl : func_decls_)
                {
                    delete func_decl;
                }
                for(auto stmt : stmts_)
                {
                    delete stmt;
                }
            }

            void AddFunctionDecl(AST* func)
            {
                assert(func->type() == AST_FUNC);
                func_decls_.emplace_back(static_cast<Function*>(func));
            }
            void AddStatement(AST* stmt)
            {
                stmts_.emplace_back(stmt);
            }

            bool strict()
            {
                return strict_;
            }
            std::vector<Function*> func_decls()
            {
                return func_decls_;
            }
            std::vector<AST*> statements()
            {
                return stmts_;
            }
    };

    class LabelledStmt : public AST
    {
        private:
            Token label_;
            AST* stmt_;

        public:
            LabelledStmt(Token label, AST* stmt, const std::string& source)
            : AST(AST_STMT_LABEL, source), label_(std::move(label)), stmt_(stmt)
            {
            }
            ~LabelledStmt()
            {
                delete stmt_;
            }

            std::string label()
            {
                return label_.source();
            }
            AST* statement()
            {
                return stmt_;
            }
    };

    class ContinueOrBreak : public AST
    {
        private:
            Token ident_;

        public:
            ContinueOrBreak(Type type, const std::string& source)
            : ContinueOrBreak(type, Token(Token::TK_NOT_FOUND, ""), source)
            {
            }

            ContinueOrBreak(Type type, Token ident, const std::string& source)
            : AST(type, source), ident_(std::move(ident))
            {
            }

            std::string ident()
            {
                return ident_.source();
            }
    };

    class Return : public AST
    {
        private:
            AST* expr_;

        public:
            Return(AST* expr, const std::string& source) : AST(AST_STMT_RETURN, source), expr_(expr)
            {
            }
            ~Return()
            {
                {
                    delete expr_;
                }
            }

            AST* expr()
            {
                return expr_;
            }
    };

    class Throw : public AST
    {
        private:
            AST* expr_;

        public:
            Throw(AST* expr, const std::string& source) : AST(AST_STMT_THROW, source), expr_(expr)
            {
            }
            ~Throw()
            {
                {
                    delete expr_;
                }
            }

            AST* expr()
            {
                return expr_;
            }
    };

    class VarDecl : public AST
    {
        private:
            Token ident_;
            AST* init_;

        public:
            VarDecl(Token ident, const std::string& source) : VarDecl(std::move(ident), nullptr, source)
            {
            }

            VarDecl(Token ident, AST* init, const std::string& source)
            : AST(AST_STMT_VAR_DECL, source), ident_(std::move(ident)), init_(init)
            {
            }
            ~VarDecl()
            {
                delete init_;
            }

            std::string ident()
            {
                return ident_.source();
            }
            AST* init()
            {
                return init_;
            }
    };

    class VarStmt : public AST
    {
        public:
            std::vector<VarDecl*> decls_;

        public:
            VarStmt() : AST(AST_STMT_VAR)
            {
            }
            ~VarStmt()
            {
                for(auto decl : decls_)
                {
                    delete decl;
                }
            }

            void AddDecl(AST* decl)
            {
                assert(decl->type() == AST_STMT_VAR_DECL);
                decls_.emplace_back(static_cast<VarDecl*>(decl));
            }

            std::vector<VarDecl*> decls()
            {
                return decls_;
            }
    };

    class Block : public AST
    {
        public:
            std::vector<AST*> stmts_;

        public:
            Block() : AST(AST_STMT_BLOCK)
            {
            }
            ~Block()
            {
                for(auto stmt : stmts_)
                {
                    delete stmt;
                }
            }

            void AddStatement(AST* stmt)
            {
                stmts_.emplace_back(stmt);
            }

            std::vector<AST*> statements()
            {
                return stmts_;
            }
    };

    class Try : public AST
    {
        public:
            AST* try_block_;
            Token catch_ident_;
            AST* catch_block_;
            AST* finally_block_;

        public:
            Try(AST* try_block, Token catch_ident, AST* catch_block, const std::string& source)
            : Try(try_block, std::move(catch_ident), catch_block, nullptr, source)
            {
            }

            Try(AST* try_block, AST* finally_block, const std::string& source)
            : Try(try_block, Token(Token::TK_NOT_FOUND, ""), nullptr, finally_block, source)
            {
            }

            Try(AST* try_block, Token catch_ident, AST* catch_block, AST* finally_block, const std::string& source)
            : AST(AST_STMT_TRY, source), try_block_(try_block), catch_ident_(std::move(catch_ident)),
              catch_block_(catch_block), finally_block_(finally_block)
            {
            }

            ~Try()
            {
                delete try_block_;
                {
                    delete catch_block_;
                }
                {
                    delete finally_block_;
                }
            }

            AST* try_block()
            {
                return try_block_;
            }
            std::string catch_ident()
            {
                return catch_ident_.source();
            };
            AST* catch_block()
            {
                return catch_block_;
            }
            AST* finally_block()
            {
                return finally_block_;
            }
    };

    class If : public AST
    {
        public:
            AST* cond_;
            AST* if_block_;
            AST* else_block_;

        public:
            If(AST* cond, AST* if_block, const std::string& source) : If(cond, if_block, nullptr, source)
            {
            }

            If(AST* cond, AST* if_block, AST* else_block, const std::string& source)
            : AST(AST_STMT_IF, source), cond_(cond), if_block_(if_block), else_block_(else_block)
            {
            }
            ~If()
            {
                delete cond_;
                delete if_block_;
                {
                    delete else_block_;
                }
            }

            AST* cond()
            {
                return cond_;
            }
            AST* if_block()
            {
                return if_block_;
            }
            AST* else_block()
            {
                return else_block_;
            }
    };

    class WhileOrWith : public AST
    {
        public:
            AST* expr_;
            AST* stmt_;

        public:
            WhileOrWith(Type type, AST* expr, AST* stmt, const std::string& source)
            : AST(type, source), expr_(expr), stmt_(stmt)
            {
            }
            ~WhileOrWith()
            {
                delete expr_;
                delete stmt_;
            }

            AST* expr()
            {
                return expr_;
            }
            AST* stmt()
            {
                return stmt_;
            }
    };

    class DoWhile : public AST
    {
        public:
            AST* expr_;
            AST* stmt_;

        public:
            DoWhile(AST* expr, AST* stmt, const std::string& source)
            : AST(AST_STMT_DO_WHILE, source), expr_(expr), stmt_(stmt)
            {
            }
            ~DoWhile()
            {
                delete expr_;
                delete stmt_;
            }

            AST* expr()
            {
                return expr_;
            }
            AST* stmt()
            {
                return stmt_;
            }

    };

    class Switch : public AST
    {
        public:
            struct DefaultClause
            {
                std::vector<AST*> stmts;
            };

            struct CaseClause
            {
                CaseClause(AST* expr, const std::vector<AST*>& stmts) : expr(expr), stmts(stmts)
                {
                }
                AST* expr;
                std::vector<AST*> stmts;
            };

        private:
            AST* expr_;
            bool has_default_clause_ = false;
            DefaultClause default_clause_;
            std::vector<CaseClause> before_default_case_clauses_;
            std::vector<CaseClause> after_default_case_clauses_;

        public:
            Switch() : AST(AST_STMT_SWITCH)
            {
            }

            ~Switch() override
            {
                for(const CaseClause& clause : before_default_case_clauses_)
                {
                    delete clause.expr;
                    for(auto stmt : clause.stmts)
                    {
                        delete stmt;
                    }
                }
                for(const CaseClause& clause : after_default_case_clauses_)
                {
                    delete clause.expr;
                    for(auto stmt : clause.stmts)
                    {
                        delete stmt;
                    }
                }
                for(auto stmt : default_clause_.stmts)
                {
                    delete stmt;
                }
            }

            void SetExpr(AST* expr)
            {
                expr_ = expr;
            }

            void SetDefaultClause(const std::vector<AST*>& stmts)
            {
                assert(!has_default_clause());
                has_default_clause_ = true;
                default_clause_.stmts = stmts;
            }

            void AddBeforeDefaultCaseClause(const CaseClause& c)
            {
                before_default_case_clauses_.emplace_back(c);
            }

            void AddAfterDefaultCaseClause(const CaseClause& c)
            {
                after_default_case_clauses_.emplace_back(c);
            }

            AST* expr()
            {
                return expr_;
            }
            std::vector<CaseClause> before_default_case_clauses()
            {
                return before_default_case_clauses_;
            }
            bool has_default_clause()
            {
                return has_default_clause_;
            }
            DefaultClause default_clause()
            {
                assert(has_default_clause());
                return default_clause_;
            }
            std::vector<CaseClause> after_default_case_clauses()
            {
                return after_default_case_clauses_;
            }
    };

    class For : public AST
    {
        private:
            std::vector<AST*> expr0s_;
            AST* expr1_;
            AST* expr2_;

            AST* stmt_;

        public:
            For(const std::vector<AST*>& expr0s, AST* expr1, AST* expr2, AST* stmt, const std::string& source)
            : AST(AST_STMT_FOR, source), expr0s_(expr0s), expr1_(expr1), expr2_(expr2), stmt_(stmt)
            {
            }

            std::vector<AST*> expr0s()
            {
                return expr0s_;
            }
            AST* expr1()
            {
                return expr1_;
            }
            AST* expr2()
            {
                return expr2_;
            }
            AST* statement()
            {
                return stmt_;
            }
    };

    class ForIn : public AST
    {
        private:
            AST* expr0_;
            AST* expr1_;
            AST* stmt_;

        public:
            ForIn(AST* expr0, AST* expr1, AST* stmt, const std::string& source)
            : AST(AST_STMT_FOR_IN, source), expr0_(expr0), expr1_(expr1), stmt_(stmt)
            {
            }

            AST* expr0()
            {
                return expr0_;
            }
            AST* expr1()
            {
                return expr1_;
            }
            AST* statement()
            {
                return stmt_;
            }
    };

    class Parser
    {
        private:
            std::string m_source;
            Lexer lexer_;

        public:
            Parser(const std::string& source);
            AST* ParsePrimaryExpression();
            std::vector<std::string> ParseFormalParameterList();
            AST* ParseFunction(bool must_be_named);
            AST* ParseArrayLiteral();
            AST* ParseObjectLiteral();
            AST* ParseExpression(bool no_in);
            AST* ParseAssignmentExpression(bool no_in);
            AST* ParseConditionalExpression(bool no_in);
            AST* ParseBinaryAndUnaryExpression(bool no_in, int priority);
            AST* ParseLeftHandSideExpression();
            AST* ParseArguments();
            AST* ParseFunctionBody(Token::Type ending_token_type = Token::TK_RBRACE);
            AST* ParseProgram();
            AST* ParseProgramOrFunctionBody(Token::Type ending_token_type, AST::Type program_or_function);
            AST* ParseStatement();
            AST* ParseBlockStatement();
            AST* ParseVariableDeclaration(bool no_in);
            AST* ParseVariableStatement(bool no_in);
            AST* ParseExpressionStatement();
            AST* ParseIfStatement();
            AST* ParseDoWhileStatement();
            AST* ParseWhileStatement();
            AST* ParseWithStatement();
            AST* ParseWhileOrWithStatement(const std::string& keyword, AST::Type type);
            AST* ParseForStatement();
            AST* ParseForStatement(const std::vector<AST*>& expr0s, size_t start);
            AST* ParseForInStatement(AST* expr0, size_t start);
            AST* ParseContinueStatement();
            AST* ParseBreakStatement();
            AST* ParseContinueOrBreakStatement(const std::string& keyword, AST::Type type);
            AST* ParseReturnStatement();
            AST* ParseThrowStatement();
            AST* ParseSwitchStatement();
            AST* ParseTryStatement();
            AST* ParseLabelledStatement();
    };

    class JSValue;

    class Error
    {
        public:
            enum Type
            {
                E_OK = 0,
                E_EVAL,
                E_RANGE,
                E_REFERENCE,
                E_SYNTAX,
                E_TYPE,
                E_URI,
                E_NATIVE,
            };

        private:
            Type m_type;
            std::string m_message;

        public:

            // TODO(zhuzilin) Fix memory leakage here.
            static Error* Ok()
            {
                return new Error(E_OK);
            }

            static Error* EvalError()
            {
                return new Error(E_EVAL);
            }

            static Error* RangeError(const std::string& message = "")
            {
                return new Error(E_RANGE, message);
            }

            static Error* ReferenceError(const std::string& message = "")
            {
                return new Error(E_REFERENCE, message);
            }

            static Error* SyntaxError(const std::string& message = "")
            {
                return new Error(E_SYNTAX, message);
            }

            static Error* TypeError(const std::string& message = "")
            {
                return new Error(E_TYPE, message);
            }

            static Error* UriError()
            {
                return new Error(E_URI);
            }

            static Error* NativeError(const std::string& message)
            {
                return new Error(E_NATIVE, message);
            }

        private:        
            Error(Type t, const std::string& message = "") : m_type(t), m_message(message)
            {
            }

        public:
            Type type()
            {
                return m_type;
            }

            bool IsOk()
            {
                return m_type == E_OK;
            }

            std::string message()
            {
                return m_message;
            }

            std::string ToString()
            {
                return Ok() != nullptr ? "ok" : "error";
            }


    };

    class JSValue
    {
        public:
            enum Type
            {
                JS_UNDEFINED = 0,
                JS_NULL,
                JS_BOOL,
                JS_STRING,
                JS_NUMBER,
                JS_OBJECT,

                LANG_TO_SPEC,

                JS_REF,
                JS_LIST,
                JS_PROP_DESC,
                JS_PROP_IDEN,
                JS_LEX_ENV,
                JS_ENV_REC,
            };


        private:
            Type m_type;

        public:
            JSValue(Type type) : m_type(type)
            {
            }

            inline Type type()
            {
                return m_type;
            }
            inline bool IsLanguageType()
            {
                return m_type < LANG_TO_SPEC;
            }
            inline bool IsSpecificationType()
            {
                return m_type > LANG_TO_SPEC;
            }
            inline bool IsPrimitive()
            {
                return m_type < JS_OBJECT;
            }

            inline bool IsUndefined()
            {
                return m_type == JS_UNDEFINED;
            }
            inline bool IsNull()
            {
                return m_type == JS_NULL;
            }
            inline bool IsBool()
            {
                return m_type == JS_BOOL;
            }
            inline bool IsString()
            {
                return m_type == JS_STRING;
            }
            inline bool IsNumber()
            {
                return m_type == JS_NUMBER;
            }
            inline bool IsObject()
            {
                return m_type == JS_OBJECT;
            }

            inline bool IsReference()
            {
                return m_type == JS_REF;
            }
            inline bool IsPropertyDescriptor()
            {
                return m_type == JS_PROP_DESC;
            }
            inline bool IsPropertyIdentifier()
            {
                return m_type == JS_PROP_IDEN;
            }
            inline bool IsLexicalEnvironment()
            {
                return m_type == JS_LEX_ENV;
            }
            inline bool IsEnvironmentRecord()
            {
                return m_type == JS_ENV_REC;
            }

            virtual std::string ToString() = 0;

            void CheckObjectCoercible(Error* e)
            {
                if(IsUndefined() || IsNull())
                {
                    *e = *Error::TypeError("undefined or null is not coercible");
                }
            }
            virtual bool IsCallable()
            {
                // JSObject need to implement its own IsCallable
                assert(!IsObject());
                return false;
            }

            virtual bool IsConstructor()
            {
                // JSObject need to implement its own IsConstructor
                assert(!IsObject());
                return false;
            }
    };

    class Undefined : public JSValue
    {
    public:
        static Undefined* Instance()
        {
            static Undefined singleton;
            return &singleton;
        }

        inline std::string ToString() override
        {
            return "Undefined";
        }

    private:
        Undefined() : JSValue(JS_UNDEFINED)
        {
        }
    };

    class Null : public JSValue
    {
    public:
        static Null* Instance()
        {
            static Null singleton;
            return &singleton;
        }
        std::string ToString() override
        {
            return "Null";
        }

    private:
        Null() : JSValue(JS_NULL)
        {
        }
    };

    class Bool : public JSValue
    {
    public:
        static Bool* True()
        {
            static Bool singleton(true);
            return &singleton;
        }
        static Bool* False()
        {
            static Bool singleton(false);
            return &singleton;
        }

        static Bool* Wrap(bool val)
        {
            return val ? True() : False();
        }

        inline bool data()
        {
            return data_;
        }

        inline std::string ToString() override
        {
            return data_ ? "true" : "false";
        }

    private:
        Bool(bool data) : JSValue(JS_BOOL), data_(data)
        {
        }

        bool data_;
    };

    class String : public JSValue
    {
    public:
        String(const std::string& data) : JSValue(JS_STRING), data_(data)
        {
        }
        std::string data()
        {
            return data_;
        }

        static String* Empty()
        {
            static String singleton("");
            return &singleton;
        }

        static String* Undefined()
        {
            static String singleton("undefined");
            return &singleton;
        }

        static String* Null()
        {
            static String singleton("null");
            return &singleton;
        }

        static String* True()
        {
            static String singleton("true");
            return &singleton;
        }

        static String* False()
        {
            static String singleton("false");
            return &singleton;
        }

        static String* NaN()
        {
            static String singleton("NaN");
            return &singleton;
        }

        static String* Zero()
        {
            static String singleton("0");
            return &singleton;
        }

        static String* Infinity()
        {
            static String singleton("Infinity");
            return &singleton;
        }

        inline std::string ToString() override
        {
            return log::ToString(data_);
        }

    private:
        std::string data_;
    };

    class Number : public JSValue
    {
    public:
        Number(double data) : JSValue(JS_NUMBER), data_(data)
        {
        }

        static Number* NaN()
        {
            static Number singleton(nan(""));
            return &singleton;
        }

        static Number* PositiveInfinity()
        {
            static Number singleton(std::numeric_limits<double>::infinity());
            return &singleton;
        }

        static Number* NegativeInfinity()
        {
            static Number singleton(-std::numeric_limits<double>::infinity());
            return &singleton;
        }

        static Number* Zero()
        {
            static Number singleton(0.0);
            return &singleton;
        }

        static Number* NegativeZero()
        {
            static Number singleton(-0.0);
            return &singleton;
        }

        static Number* One()
        {
            static Number singleton(1.0);
            return &singleton;
        }

        inline bool IsInfinity()
        {
            return isinf(data_);
        }
        inline bool IsPositiveInfinity()
        {
            return data_ == std::numeric_limits<double>::infinity();
        }
        inline bool IsNegativeInfinity()
        {
            return data_ == -std::numeric_limits<double>::infinity();
        }
        inline bool IsNaN()
        {
            return isnan(data_);
        }

        inline double data()
        {
            return data_;
        }

        inline std::string ToString() override
        {
            return std::to_string(data_);
        }

    private:
        double data_;
    };

    class PropertyDescriptor : public JSValue
    {
        public:
            enum Field
            {
                VALUE = 1 << 0,
                WRITABLE = 1 << 1,
                GET = 1 << 2,
                SET = 1 << 3,
                ENUMERABLE = 1 << 4,
                CONFIGURABLE = 1 << 5,
            };

        private:
            char bitmask_;
            JSValue* value_;
            JSValue* getter_;
            JSValue* setter_;
            bool writable_;
            bool enumerable_;
            bool configurable_;

        public:
            PropertyDescriptor():
                JSValue(JS_PROP_DESC),
                bitmask_(0),
                value_(Undefined::Instance()),
                getter_(Undefined::Instance()),
                setter_(Undefined::Instance()),
                writable_(false),
                enumerable_(false),
                configurable_(false)
            {
            }

            inline bool IsAccessorDescriptor()
            {
                return ((bitmask_ & GET) != 0) && ((bitmask_ & SET) != 0);
            }

            inline bool IsDataDescriptor()
            {
                return ((bitmask_ & VALUE) != 0) && ((bitmask_ & WRITABLE) != 0);
            }

            inline bool IsGenericDescriptor()
            {
                return !IsAccessorDescriptor() && !IsDataDescriptor();
            }

            // TODO(zhuzilin) May be check the member variable is initialized?
            inline bool HasValue()
            {
                return (bitmask_ & VALUE) != 0;
            }
            inline JSValue* Value()
            {
                return value_;
            }
            inline void SetValue(JSValue* value)
            {
                bitmask_ |= VALUE;
                value_ = value;
            }

            inline bool HasWritable()
            {
                return (bitmask_ & WRITABLE) != 0;
            }
            inline bool Writable()
            {
                return writable_;
            }
            inline void SetWritable(bool writable)
            {
                bitmask_ |= WRITABLE;
                writable_ = writable;
            }

            inline bool HasGet()
            {
                return (bitmask_ & GET) != 0;
            }
            inline JSValue* Get()
            {
                return getter_;
            }
            inline void SetGet(JSValue* getter)
            {
                bitmask_ |= GET;
                getter_ = getter;
            }

            inline bool HasSet()
            {
                return (bitmask_ & SET) != 0;
            }
            inline JSValue* Set()
            {
                return setter_;
            }
            inline void SetSet(JSValue* setter)
            {
                bitmask_ |= SET;
                setter_ = setter;
            }

            inline bool HasEnumerable()
            {
                return (bitmask_ & ENUMERABLE) != 0;
            }
            inline bool Enumerable()
            {
                return enumerable_;
            }
            inline void SetEnumerable(bool enumerable)
            {
                bitmask_ |= ENUMERABLE;
                enumerable_ = enumerable;
            }

            inline bool HasConfigurable()
            {
                return (bitmask_ & CONFIGURABLE) != 0;
            }
            inline bool Configurable()
            {
                return configurable_;
            }
            inline void SetConfigurable(bool configurable)
            {
                bitmask_ |= CONFIGURABLE;
                configurable_ = configurable;
            }

            inline void SetDataDescriptor(JSValue* value, bool writable, bool enumerable, bool configurable)
            {
                SetValue(value);
                SetWritable(writable);
                SetEnumerable(enumerable);
                SetConfigurable(configurable);
            }

            inline void SetAccessorDescriptor(JSValue* getter, JSValue* setter, bool enumerable, bool configurable)
            {
                SetGet(getter);
                SetSet(setter);
                SetEnumerable(enumerable);
                SetConfigurable(configurable);
            }

            // Set the value to `this` if `other` has.
            inline void Set(PropertyDescriptor* other)
            {
                if(other->HasValue())
                {
                    SetValue(other->Value());
                }
                if(other->HasWritable())
                {
                    SetWritable(other->Writable());
                }
                if(other->HasGet())
                {
                    SetGet(other->Get());
                }
                if(other->HasSet())
                {
                    SetSet(other->Set());
                }
                if(other->HasConfigurable())
                {
                    SetConfigurable(other->Configurable());
                }
                if(other->HasEnumerable())
                {
                    SetEnumerable(other->Enumerable());
                }
            }

            char bitmask()
            {
                return bitmask_;
            }
            void SetBitMask(char bitmask)
            {
                bitmask_ = bitmask;
            }

            std::string ToString() override
            {
                std::string res = "PropertyDescriptor{";
                if(HasValue())
                {
                    res += "v: " + value_->ToString() + ", ";
                }
                if(HasWritable())
                {
                    res += "w: " + log::ToString(writable_) + ", ";
                }
                if(HasEnumerable())
                {
                    res += "e: " + log::ToString(enumerable_) + ", ";
                }
                if(HasConfigurable())
                {
                    res += "c: " + log::ToString(configurable_);
                }

                res += '}';
                return res;
            }


    };

    class PropertyIdentifier : public JSValue
    {
        private:
            std::string name_;
            PropertyDescriptor* descriptor_;

        public:
            PropertyIdentifier(const std::string& name, PropertyDescriptor* desciptor)
            : JSValue(JS_PROP_IDEN), name_(name), descriptor_(desciptor)
            {
                (void)descriptor_;
            }
    };

    inline bool SameValue(JSValue* x, JSValue* y)
    {
        if(x->type() != y->type())
        {
            return false;
        }
        switch(x->type())
        {
            case JSValue::JS_UNDEFINED:
                return true;
            case JSValue::JS_NULL:
                return true;
            case JSValue::JS_NUMBER:
            {
                Number* num_x = static_cast<Number*>(x);
                Number* num_y = static_cast<Number*>(y);
                if(num_x->IsNaN() && num_y->IsNaN())
                {
                    return true;
                }
                double dx = num_x->data();
                double dy = num_y->data();
                if(dx == dy && dx == 0.0)
                {
                    return signbit(dx) == signbit(dy);
                }
                return dx == dy;
            }
            case JSValue::JS_STRING:
            {
                String* str_x = static_cast<String*>(x);
                String* str_y = static_cast<String*>(y);
                return str_x->data() == str_y->data();
            }
            case JSValue::JS_BOOL:
            {
                Bool* b_x = static_cast<Bool*>(x);
                Bool* b_y = static_cast<Bool*>(y);
                return b_x->data() == b_y->data();
            }
            default:
                return x == y;
        }
    }

    typedef std::function<JSValue*(Error*, JSValue*, const std::vector<JSValue*>&)> inner_func;

    class JSObject : public JSValue
    {
        public:
            enum ObjType
            {
                OBJ_GLOBAL,
                OBJ_OBJECT,
                OBJ_FUNC,
                OBJ_ARRAY,
                OBJ_STRING,
                OBJ_BOOL,
                OBJ_NUMBER,
                OBJ_MATH,
                OBJ_DATE,
                OBJ_REGEX,
                OBJ_JSON,
                OBJ_ERROR,

                OBJ_INNER_FUNC,
                OBJ_OTHER,
            };


        private:
            ObjType obj_type_;
            std::map<std::string, PropertyDescriptor*> named_properties_;

            JSValue* prototype_;
            std::string class_;
            bool extensible_;

            JSValue* primitive_value_;

            bool is_constructor_;
            bool is_callable_;
            inner_func callable_;

        public:

            JSObject(ObjType obj_type,
                     const std::string& klass,
                     bool extensible,
                     JSValue* primitive_value,
                     bool is_constructor,
                     bool is_callable,
                     inner_func callable = nullptr)
            : JSValue(JS_OBJECT), obj_type_(obj_type), prototype_(Null::Instance()), class_(klass), extensible_(extensible),
              primitive_value_(primitive_value), is_constructor_(is_constructor), is_callable_(is_callable),
              callable_(std::move(callable))
            {
            }

            ObjType obj_type()
            {
                return obj_type_;
            }

            bool IsFunction()
            {
                return obj_type_ == OBJ_FUNC;
            }

            // Internal Preperties Common to All Objects
            JSValue* Prototype()
            {
                return prototype_;
            }
            void SetPrototype(JSValue* proto)
            {
                assert(proto->type() == JS_NULL || proto->type() == JS_OBJECT);
                prototype_ = proto;
            }

            std::string Class()
            {
                return class_;
            }

            bool Extensible()
            {
                return extensible_;
            }

            void SetExtensible(bool extensible)
            {
                extensible_ = extensible;
            }

            virtual JSValue* Get(Error* e, const std::string& P);
            virtual JSValue* GetOwnProperty(const std::string& P);
            JSValue* GetProperty(const std::string& P);
            void Put(Error* e, const std::string& P, JSValue* V, bool throw_flag);
            bool CanPut(const std::string& P);
            bool HasProperty(const std::string& P);
            virtual bool Delete(Error* e, const std::string& P, bool throw_flag);
            JSValue* DefaultValue(Error* e, const std::string& hint);
            virtual bool DefineOwnProperty(Error* e, const std::string& P, PropertyDescriptor* desc, bool throw_flag);

            // Internal Properties Only Defined for Some Objects
            // [[PrimitiveValue]]
            JSValue* PrimitiveValue()
            {
                assert(primitive_value_ != nullptr);
                return primitive_value_;
            };
            bool HasPrimitiveValue()
            {
                return obj_type_ == OBJ_BOOL || obj_type_ == OBJ_DATE || obj_type_ == OBJ_NUMBER || obj_type_ == OBJ_STRING;
            }

            // [[Construct]]
            virtual JSObject* Construct(Error* e, const std::vector<JSValue*>& arguments)
            {
                (void)e;
                (void)arguments;
                assert(false);
            }

            bool IsConstructor() override
            {
                return is_constructor_;
            }

            // [[Call]]
            virtual JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {})
            {
                (void)this_arg;
                assert(is_callable_ && callable_ != nullptr);
                return callable_(e, this, arguments);
            }

            bool IsCallable() override
            {
                return is_callable_;
            }

            // [[HasInstance]]
            // NOTE(zhuzilin) Here we use the implementation in 15.3.5.3 [[HasInstance]] (V)
            // to make sure all callables have HasInstance.
            virtual bool HasInstance(Error* e, JSValue* V)
            {
                assert(IsCallable());
                if(!V->IsObject())
                {
                    return false;
                }
                JSValue* O = Get(e, "prototype");
                if(!e->IsOk())
                {
                    return false;
                }
                if(!O->IsObject())
                {
                    *e = *Error::TypeError();
                    return false;
                }
                while(!V->IsNull())
                {
                    if(V == O)
                    {
                        return true;
                    }
                    assert(V->IsObject());
                    V = static_cast<JSObject*>(V)->Prototype();
                    if(!e->IsOk())
                    {
                        return false;
                    }
                }
                return false;
            }

            void AddValueProperty(const std::string& name, JSValue* value, bool writable, bool enumerable, bool configurable)
            {
                PropertyDescriptor* desc = new PropertyDescriptor();
                desc->SetDataDescriptor(value, writable, enumerable, configurable);
                // This should just like named_properties_[name] = desc
                DefineOwnProperty(nullptr, name, desc, false);
            }

            void AddFuncProperty(const std::string& name, inner_func callable, bool writable, bool enumerable, bool configurable);

            // This for for-in statement.
            virtual std::vector<std::pair<std::string, PropertyDescriptor*>> AllEnumerableProperties()
            {
                std::vector<std::pair<std::string, PropertyDescriptor*>> result;
                for(auto pair : named_properties_)
                {
                    if(!pair.second->HasEnumerable() || !pair.second->Enumerable())
                    {
                        continue;
                    }
                    result.emplace_back(pair);
                }
                if(!prototype_->IsNull())
                {
                    JSObject* proto = static_cast<JSObject*>(prototype_);
                    for(auto pair : proto->AllEnumerableProperties())
                    {
                        if(!pair.second->HasEnumerable() || !pair.second->Enumerable())
                        {
                            continue;
                        }
                        if(named_properties_.find(pair.first) == named_properties_.end())
                        {
                            result.emplace_back(pair);
                        }
                    }
                }
                return result;
            }

            virtual std::string ToString() override
            {
                return log::ToString(class_);
            }
    };

    // 8.12.1 [[GetOwnProperty]] (P)
    inline JSValue* JSObject::GetOwnProperty(const std::string& P)
    {
        // TODO(zhuzilin) String Object has a more elaborate impl 15.5.5.2.
        auto iter = named_properties_.find(P);
        if(iter == named_properties_.end())
        {
            return Undefined::Instance();
        }
        // NOTE(zhuzilin) In the spec, we need to create a new property descriptor D,
        // And assign the property to it. However, if we init D->value = a, and
        // set D->value = b in DefineOwnProperty, the value saved in the named_properties_ will
        // remain b and that is not what we want.
        return iter->second;
    }

    inline JSValue* JSObject::GetProperty(const std::string& P)
    {
        JSValue* own_property = GetOwnProperty(P);
        if(!own_property->IsUndefined())
        {
            return own_property;
        }
        JSValue* proto = Prototype();
        if(proto->IsNull())
        {
            return Undefined::Instance();
        }
        assert(proto->IsObject());
        JSObject* proto_obj = static_cast<JSObject*>(proto);
        return proto_obj->GetProperty(P);
    }

    inline JSValue* JSObject::Get(Error* e, const std::string& P)
    {
        JSValue* value = GetProperty(P);
        if(value->IsUndefined())
        {
            return Undefined::Instance();
        }
        PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(value);
        if(desc->IsDataDescriptor())
        {
            return desc->Value();
        }
        else
        {
            assert(desc->IsAccessorDescriptor());
            JSValue* getter = desc->Get();
            if(getter->IsUndefined())
            {
                return Undefined::Instance();
            }
            JSObject* getter_obj = static_cast<JSObject*>(getter);
            return getter_obj->Call(e, this);
        }
    }

    inline bool JSObject::CanPut(const std::string& P)
    {
        JSValue* value = GetOwnProperty(P);
        if(!value->IsUndefined())
        {
            PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(value);
            if(desc->IsAccessorDescriptor())
            {
                return !desc->Set()->IsUndefined();
            }
            else
            {
                return desc->Writable();
            }
        }

        JSValue* proto = Prototype();
        if(proto->IsNull())
        {
            return Extensible();
        }
        JSObject* proto_obj = static_cast<JSObject*>(proto);
        JSValue* inherit = proto_obj->GetProperty(P);
        if(inherit->IsUndefined())
        {
            return Extensible();
        }
        PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(inherit);
        if(desc->IsAccessorDescriptor())
        {
            return !desc->Set()->IsUndefined();
        }
        else
        {
            return Extensible() ? desc->Writable() : false;
        }
    }

    // 8.12.5 [[Put]] ( P, V, Throw )
    inline void JSObject::Put(Error* e, const std::string& P, JSValue* V, bool throw_flag)
    {
        log::PrintSource("Put ", P, " " + V->ToString());
        if(!CanPut(P))
        {// 1
            if(throw_flag)
            {// 1.a
                *e = *Error::TypeError();
            }
            return;// 1.b
        }
        JSValue* value = GetOwnProperty(P);
        if(!value->IsUndefined())
        {
            PropertyDescriptor* own_desc = static_cast<PropertyDescriptor*>(value);// 2
            if(own_desc->IsDataDescriptor())
            {// 3
                PropertyDescriptor* value_desc = new PropertyDescriptor();
                value_desc->SetValue(V);
                log::PrintSource("Overwrite the old desc with " + value_desc->ToString());
                DefineOwnProperty(e, P, value_desc, throw_flag);
                return;
            }
        }
        value = GetProperty(P);
        if(!value->IsUndefined())
        {
            PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(value);
            if(desc->IsAccessorDescriptor())
            {
                log::PrintSource("Use parent prototype's setter");
                JSValue* setter = desc->Set();
                assert(!setter->IsUndefined());
                JSObject* setter_obj = static_cast<JSObject*>(setter);
                setter_obj->Call(e, this, { V });
                return;
            }
        }
        PropertyDescriptor* new_desc = new PropertyDescriptor();
        new_desc->SetDataDescriptor(V, true, true, true);// 6.a
        DefineOwnProperty(e, P, new_desc, throw_flag);
    }

    inline bool JSObject::HasProperty(const std::string& P)
    {
        JSValue* desc = GetOwnProperty(P);
        return !desc->IsUndefined();
    }

    inline bool JSObject::Delete(Error* e, const std::string& P, bool throw_flag)
    {
        JSValue* value = GetOwnProperty(P);
        if(value->IsUndefined())
        {
            return true;
        }
        PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(value);
        if(desc->Configurable())
        {
            named_properties_.erase(P);
            return true;
        }
        else
        {
            if(throw_flag)
            {
                *e = *Error::TypeError();
            }
            return false;
        }
    }

    // 8.12.9 [[DefineOwnProperty]] (P, Desc, Throw)
    inline bool JSObject::DefineOwnProperty(Error* e, const std::string& P, PropertyDescriptor* desc, bool throw_flag)
    {
        JSValue* current = GetOwnProperty(P);
        PropertyDescriptor* current_desc;
        if(current->IsUndefined())
        {
            if(!extensible_)
            {// 3
                goto reject;
            }
            // 4.
            named_properties_[P] = desc;
            return true;
        }
        if(desc->bitmask() == 0)
        {// 5
            return true;
        }
        current_desc = static_cast<PropertyDescriptor*>(current);
        if((desc->bitmask() & current_desc->bitmask()) == desc->bitmask())
        {
            bool same = true;
            if(desc->HasValue())
            {
                same = same && SameValue(desc->Value(), current_desc->Value());
            }
            if(desc->HasWritable())
            {
                same = same && (desc->Writable() == current_desc->Writable());
            }
            if(desc->HasGet())
            {
                same = same && SameValue(desc->Get(), current_desc->Get());
            }
            if(desc->HasSet())
            {
                same = same && SameValue(desc->Set(), current_desc->Set());
            }
            if(desc->HasConfigurable())
            {
                same = same && (desc->Configurable() == current_desc->Configurable());
            }
            if(desc->HasEnumerable())
            {
                same = same && (desc->Enumerable() == current_desc->Enumerable());
            }
            if(same)
            {
                return true;// 6
            }
        }
        log::PrintSource("desc: " + desc->ToString() + ", current: " + current_desc->ToString());
        if(!current_desc->Configurable())
        {// 7
            if(desc->Configurable())
            {// 7.a
                log::PrintSource("DefineOwnProperty: ", P, " not configurable, while new value configurable");
                goto reject;
            }
            if(desc->HasEnumerable() && (desc->Enumerable() != current_desc->Enumerable()))
            {// 7.b
                log::PrintSource("DefineOwnProperty: ", P, " enumerable value differ");
                goto reject;
            }
        }
        // 8.
        if(!desc->IsGenericDescriptor())
        {
            if(current_desc->IsDataDescriptor() != desc->IsDataDescriptor())
            {// 9.
                // 9.a
                if(!current_desc->Configurable())
                {
                    goto reject;
                }
                // 9.b.i & 9.c.i
                PropertyDescriptor* old_property = named_properties_[P];
                PropertyDescriptor* new_property = new PropertyDescriptor();
                new_property->SetConfigurable(old_property->Configurable());
                new_property->SetEnumerable(old_property->Enumerable());
                new_property->SetBitMask(old_property->bitmask());
                named_properties_[P] = new_property;
            }
            else if(current_desc->IsDataDescriptor() && desc->IsDataDescriptor())
            {// 10.
                if(!current_desc->Configurable())
                {// 10.a
                    if(!current_desc->Writable())
                    {
                        if(desc->Writable())
                        {
                            goto reject;// 10.a.i
                        }
                        // 10.a.ii.1
                        if(desc->HasValue() && !SameValue(desc->Value(), current_desc->Value()))
                        {
                            goto reject;
                        }
                    }
                }
                else
                {// 10.b
                    assert(current_desc->Configurable());
                }
            }
            else
            {// 11.
                assert(current_desc->IsAccessorDescriptor() && desc->IsAccessorDescriptor());
                if(!current_desc->Configurable())
                {// 11.a
                    if(!SameValue(desc->Set(), current_desc->Set()) ||// 11.a.i
                       !SameValue(desc->Get(), current_desc->Get()))
                    {// 11.a.ii
                        goto reject;
                    }
                }
            }
        }
        log::PrintSource("DefineOwnProperty: ", P, " is set" + (desc->HasValue() ? " to " + desc->Value()->ToString() : ""));
        // 12.
        current_desc->Set(desc);
        // 13.
        return true;
    reject:
        log::PrintSource("DefineOwnProperty reject");
        if(throw_flag)
        {
            *e = *Error::TypeError();
        }
        return false;
    }

    // EnvironmentRecord is also of type JSValue
    class EnvironmentRecord : public JSValue
    {
        public:
            EnvironmentRecord() : JSValue(JS_ENV_REC)
            {
            }

            virtual bool HasBinding(const std::string& N) = 0;
            virtual void CreateMutableBinding(Error* e, const std::string& N, bool D) = 0;
            virtual void SetMutableBinding(Error* e, const std::string& N, JSValue* V, bool S) = 0;
            virtual JSValue* GetBindingValue(Error* e, const std::string& N, bool S) = 0;
            virtual bool DeleteBinding(Error* e, const std::string& N) = 0;
            virtual JSValue* ImplicitThisValue() = 0;
    };

    class DeclarativeEnvironmentRecord : public EnvironmentRecord
    {
        public:
            struct Binding
            {
                JSValue* value;
                bool can_delete;
                bool is_mutable;
            };

        private:
            std::unordered_map<std::string, Binding> bindings_;

        public:
            bool HasBinding(const std::string& N) override
            {
                return bindings_.find(N) != bindings_.end();
            }

            void CreateMutableBinding(Error* e, const std::string& N, bool D) override
            {
                (void)e;
                assert(!HasBinding(N));
                Binding b;
                b.value = Undefined::Instance();
                b.can_delete = D;
                b.is_mutable = true;
                bindings_[N] = b;
            }

            void SetMutableBinding(Error* e, const std::string& N, JSValue* V, bool S) override
            {
                log::PrintSource("enter SetMutableBinding ", N, " to " + V->ToString());
                assert(V->IsLanguageType());
                assert(HasBinding(N));
                // NOTE(zhuzilin) If we do note b = bindings_[N] and change b.value,
                // the value stored in bindings_ won't change.
                if(bindings_[N].is_mutable)
                {
                    bindings_[N].value = V;
                }
                else if(S)
                {
                    *e = *Error::TypeError();
                }
            }

            JSValue* GetBindingValue(Error* e, const std::string& N, bool S) override
            {
                assert(HasBinding(N));
                Binding b = bindings_[N];
                if(b.value->IsUndefined())
                {
                    if(S)
                    {
                        *e = *Error::ReferenceError(N + " is not defined");
                        return nullptr;
                    }
                    else
                    {
                        log::PrintSource("GetBindingValue ", N, " undefined");
                        return Undefined::Instance();
                    }
                }
                log::PrintSource("GetBindingValue ", N, " " + b.value->ToString());
                return b.value;
            }

            bool DeleteBinding(Error* e, const std::string& N) override
            {
                (void)e;
                if(!HasBinding(N))
                {
                    return true;
                }
                if(!bindings_[N].can_delete)
                {
                    return false;
                }
                bindings_.erase(N);
                return true;
            }

            JSValue* ImplicitThisValue() override
            {
                return Undefined::Instance();
            }

            void CreateImmutableBinding(const std::string& N)
            {
                assert(!HasBinding(N));
                Binding b;
                b.value = Undefined::Instance();
                b.can_delete = false;
                b.is_mutable = false;
                bindings_[N] = b;
            }

            void InitializeImmutableBinding(const std::string& N, JSValue* V)
            {
                assert(HasBinding(N));
                assert(!bindings_[N].is_mutable && bindings_[N].value->IsUndefined());
                bindings_[N].value = V;
            }

            virtual std::string ToString() override
            {
                return "DeclarativeEnvRec(" + log::ToString(this) + ")";
            }
    };

    class ObjectEnvironmentRecord : public EnvironmentRecord
    {
        private:
            JSObject* bindings_;
            bool provide_this_;

        public:
            ObjectEnvironmentRecord(JSObject* obj, bool provide_this = false) : bindings_(obj), provide_this_(provide_this)
            {
            }

            bool HasBinding(const std::string& N) override
            {
                return bindings_->HasProperty(N);
            }

            // 10.2.1.2.2 CreateMutableBinding (N, D)
            void CreateMutableBinding(Error* e, const std::string& N, bool D) override
            {
                assert(!HasBinding(N));
                PropertyDescriptor* desc = new PropertyDescriptor();
                desc->SetDataDescriptor(Undefined::Instance(), true, true, D);
                bindings_->DefineOwnProperty(e, N, desc, true);
            }

            void SetMutableBinding(Error* e, const std::string& N, JSValue* V, bool S) override
            {
                log::PrintSource("enter SetMutableBinding ", N, " to " + V->ToString());
                assert(V->IsLanguageType());
                bindings_->Put(e, N, V, S);
            }

            JSValue* GetBindingValue(Error* e, const std::string& N, bool S) override
            {
                bool value = HasBinding(N);
                if(!value)
                {
                    if(S)
                    {
                        *e = *Error::ReferenceError(N + " is not defined");
                        return nullptr;
                    }
                    else
                    {
                        return Undefined::Instance();
                    }
                }
                return bindings_->Get(e, N);
            }

            bool DeleteBinding(Error* e, const std::string& N) override
            {
                return bindings_->Delete(e, N, false);
            }

            JSValue* ImplicitThisValue() override
            {
                if(provide_this_)
                {
                    return bindings_;
                }
                return Undefined::Instance();
            }

            virtual std::string ToString() override
            {
                return "ObjectEnvRec(" + log::ToString(this) + ")";
            }


    };

    // 15.1 The Global Object
    class GlobalObject : public JSObject
    {
        private:
            bool direct_eval_;


        private:
            GlobalObject()
            : JSObject(OBJ_GLOBAL,
                       // 15.1 The values of the [[Prototype]] and [[Class]]
                       // of the global object are implementation-dependent.
                       "Global",
                       // NOTE(zhuzilin) global object need to have [[Extensible]] as true,
                       // otherwise we cannot define variable in global code, as global varaibles
                       // are the property of global object.
                       true,
                       nullptr,
                       false,
                       false),
              direct_eval_(false)
            {
            }


        public:
            static GlobalObject* Instance()
            {
                static GlobalObject singleton;
                return &singleton;
            }

            bool direct_eval()
            {
                return direct_eval_;
            }
            void SetDirectEval(bool direct_eval)
            {
                direct_eval_ = direct_eval;
            }

            // 15.1.2.1 eval(X)
            static JSValue* eval(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals);

            // 15.1.2.2 parseInt (string , radix)
            static JSValue* parseInt(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                assert(false);
            }

            // 15.1.2.3 parseFloat (string)
            static JSValue* parseFloat(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                assert(false);
            }

            // 15.1.2.4 isNaN (number)
            static JSValue* isNaN(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                assert(false);
            }

            // 15.1.2.5 isFinite (number)
            static JSValue* isFinite(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                assert(false);
            }

            inline std::string ToString() override
            {
                return "GlobalObject";
            }
    };

    class DirectEvalGuard
    {
        public:
            DirectEvalGuard()
            {
                GlobalObject::Instance()->SetDirectEval(true);
            }

            ~DirectEvalGuard()
            {
                GlobalObject::Instance()->SetDirectEval(false);
            }
    };

    JSObject* ToObject(Error* e, JSValue* input);

    class Reference : public JSValue
    {
        private:
            JSValue* base_;
            std::string reference_name_;
            bool strict_reference_;

        public:
            Reference(JSValue* base, const std::string& reference_name, bool strict_reference)
            : JSValue(JS_REF), base_(base), reference_name_(reference_name), strict_reference_(strict_reference)
            {
            }

            JSValue* GetBase()
            {
                return base_;
            }
            std::string GetReferencedName()
            {
                return reference_name_;
            }
            bool IsStrictReference()
            {
                return strict_reference_;
            }
            bool HasPrimitiveBase()
            {
                return base_->IsBool() || base_->IsString() || base_->IsNumber();
            }
            bool IsPropertyReference()
            {
                return base_->IsObject() || HasPrimitiveBase();
            }
            bool IsUnresolvableReference()
            {
                return base_->IsUndefined();
            }

            std::string ToString() override
            {
                return "ref(" + log::ToString(reference_name_) + ")";
            }

    };

    inline JSValue* GetValue(Error* e, JSValue* V)
    {
        if(!V->IsReference())
        {
            return V;
        }
        Reference* ref = static_cast<Reference*>(V);
        if(ref->IsUnresolvableReference())
        {
            *e = *Error::ReferenceError(ref->GetReferencedName() + " is not defined");
            return nullptr;
        }
        JSValue* base = ref->GetBase();
        if(ref->IsPropertyReference())
        {// 4
            // 4.a & 4.b
            if(!ref->HasPrimitiveBase())
            {
                assert(base->IsObject());
                JSObject* obj = static_cast<JSObject*>(base);
                return obj->Get(e, ref->GetReferencedName());
            }
            else
            {// special [[Get]]
                JSObject* O = ToObject(e, base);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                JSValue* tmp = O->GetProperty(ref->GetReferencedName());
                if(tmp->IsUndefined())
                {
                    return Undefined::Instance();
                }
                PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(tmp);
                if(desc->IsDataDescriptor())
                {
                    return desc->Value();
                }
                else
                {
                    assert(desc->IsAccessorDescriptor());
                    JSValue* getter = desc->Get();
                    if(getter->IsUndefined())
                    {
                        return Undefined::Instance();
                    }
                    JSObject* getter_obj = static_cast<JSObject*>(getter);
                    return getter_obj->Call(e, base, {});
                }
            }
        }
        else
        {
            assert(base->IsEnvironmentRecord());
            EnvironmentRecord* er = static_cast<EnvironmentRecord*>(base);
            return er->GetBindingValue(e, ref->GetReferencedName(), ref->IsStrictReference());
        }
    }

    inline void PutValue(Error* e, JSValue* V, JSValue* W)
    {
        log::PrintSource("PutValue V: " + V->ToString() + ", W: " + W->ToString());
        if(!V->IsReference())
        {
            *e = *Error::ReferenceError();
            return;
        }
        Reference* ref = static_cast<Reference*>(V);
        JSValue* base = ref->GetBase();
        if(ref->IsUnresolvableReference())
        {// 3
            if(ref->IsStrictReference())
            {// 3.a
                *e = *Error::ReferenceError();
                return;
            }
            GlobalObject::Instance()->Put(e, ref->GetReferencedName(), W, false);// 3.b
        }
        else if(ref->IsPropertyReference())
        {
            bool throw_flag = ref->IsStrictReference();
            std::string P = ref->GetReferencedName();
            if(!ref->HasPrimitiveBase())
            {
                assert(base->IsObject());
                JSObject* base_obj = static_cast<JSObject*>(base);
                base_obj->Put(e, P, W, throw_flag);
            }
            else
            {// special [[Put]]
                JSObject* O = ToObject(e, base);
                if(!O->CanPut(P))
                {// 2
                    if(throw_flag)
                    {
                        *e = *Error::TypeError();
                    }
                    return;
                }
                JSValue* tmp = O->GetOwnProperty(P);// 3
                if(!tmp->IsUndefined())
                {
                    PropertyDescriptor* own_desc = static_cast<PropertyDescriptor*>(tmp);
                    if(own_desc->IsDataDescriptor())
                    {// 4
                        if(throw_flag)
                        {
                            *e = *Error::TypeError();
                        }
                        return;
                    }
                }
                tmp = O->GetProperty(P);
                if(!tmp->IsUndefined())
                {
                    PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(tmp);
                    if(desc->IsAccessorDescriptor())
                    {// 4
                        JSValue* setter = desc->Set();
                        assert(!setter->IsUndefined());
                        JSObject* setter_obj = static_cast<JSObject*>(setter);
                        setter_obj->Call(e, base, { W });
                    }
                    else
                    {// 7
                        if(throw_flag)
                        {
                            *e = *Error::TypeError();
                        }
                        return;
                    }
                }
            }
        }
        else
        {
            assert(base->IsEnvironmentRecord());
            EnvironmentRecord* er = static_cast<EnvironmentRecord*>(base);
            er->SetMutableBinding(e, ref->GetReferencedName(), W, ref->IsStrictReference());
        }
    }

    class LexicalEnvironment : public JSValue
    {
        private:
            JSValue* outer_;// not owned
            EnvironmentRecord* env_rec_;

        public:
            LexicalEnvironment(JSValue* outer, EnvironmentRecord* env_rec) : JSValue(JS_LEX_ENV), env_rec_(env_rec)
            {
                assert(outer->IsNull() || outer->IsLexicalEnvironment());
                outer_ = outer;
            }

            static LexicalEnvironment* Global()
            {
                static LexicalEnvironment singleton(Null::Instance(), new ObjectEnvironmentRecord(GlobalObject::Instance()));
                return &singleton;
            }

            Reference* GetIdentifierReference(const std::string& name, bool strict)
            {
                bool exists = env_rec_->HasBinding(name);
                if(exists)
                {
                    return new Reference(env_rec_, name, strict);
                }
                if(outer_->IsNull())
                {
                    return new Reference(Undefined::Instance(), name, strict);
                }
                LexicalEnvironment* outer = static_cast<LexicalEnvironment*>(outer_);
                return outer->GetIdentifierReference(name, strict);
            }

            static LexicalEnvironment* NewDeclarativeEnvironment(JSValue* lex)
            {
                DeclarativeEnvironmentRecord* env_rec = new DeclarativeEnvironmentRecord();
                return new LexicalEnvironment(lex, env_rec);
            }

            static LexicalEnvironment* NewObjectEnvironment(JSObject* obj, JSValue* lex, bool provide_this = false)
            {
                ObjectEnvironmentRecord* env_rec = new ObjectEnvironmentRecord(obj, provide_this);
                return new LexicalEnvironment(lex, env_rec);
            }

            JSValue* outer()
            {
                return outer_;
            }
            EnvironmentRecord* env_rec()
            {
                return env_rec_;
            }

            std::string ToString() override
            {
                return "LexicalEnvironment";
            }
    };

    class ExecutionContext
    {
        private:
            LexicalEnvironment* variable_env_;
            LexicalEnvironment* lexical_env_;
            JSValue* this_binding_;
            bool strict_;
            std::stack<std::string> label_stack_;
            size_t iteration_layers_;

        public:
            ExecutionContext(LexicalEnvironment* variable_env, LexicalEnvironment* lexical_env, JSValue* this_binding, bool strict)
            : variable_env_(variable_env), lexical_env_(lexical_env), this_binding_(this_binding), strict_(strict),
              iteration_layers_(0)
            {
            }

            LexicalEnvironment* variable_env()
            {
                return variable_env_;
            }
            LexicalEnvironment* lexical_env()
            {
                return lexical_env_;
            }
            JSValue* this_binding()
            {
                return this_binding_;
            }
            bool strict()
            {
                return strict_;
            }

            void SetLexicalEnv(LexicalEnvironment* lexical_env)
            {
                lexical_env_ = lexical_env;
            }

            bool HasLabel(const std::string& label)
            {
                if(label.empty())
                {
                    return true;
                }
                return (static_cast<unsigned int>(!label_stack_.empty()) != 0u) && label_stack_.top() == label;
            }

            void AddLabel(const std::string& label)
            {
                assert(!HasLabel(label));
                label_stack_.push(label);
            }

            void RemoveLabel(const std::string& label)
            {
                if(label.empty())
                {
                    return;
                }
                assert(HasLabel(label));
                label_stack_.pop();
            }

            void EnterIteration()
            {
                iteration_layers_++;
            }
            void ExitIteration()
            {
                if(iteration_layers_ != 0)
                {
                    iteration_layers_--;
                }
            }
            bool InIteration()
            {
                return iteration_layers_ != 0;
            }


    };

    class RuntimeContext
    {
        private:
            std::stack<ExecutionContext*> context_stack_;
            ExecutionContext* global_env_;
            // This is to make sure builtin function like `array.push()`
            // can visit `array`.
            std::stack<JSValue*> value_stack_;

        private:
            RuntimeContext()
            {
                value_stack_.push(Null::Instance());
            }

        public:
            static RuntimeContext* Global()
            {
                static RuntimeContext singleton;
                return &singleton;
            }

            void AddContext(ExecutionContext* context)
            {
                context_stack_.push(context);
                if(context_stack_.size() == 1)
                {
                    global_env_ = context;
                }
            }

            static ExecutionContext* TopContext()
            {
                return RuntimeContext::Global()->context_stack_.top();
            }

            static LexicalEnvironment* TopLexicalEnv()
            {
                return RuntimeContext::TopContext()->lexical_env();
            }

            void PopContext()
            {
                ExecutionContext* top = context_stack_.top();
                context_stack_.pop();
                delete top;
            }

            static JSValue* TopValue()
            {
                return RuntimeContext::Global()->value_stack_.top();
            }

            void AddValue(JSValue* val)
            {
                value_stack_.push(val);
            }

            void PopValue()
            {
                value_stack_.pop();
            }

            ExecutionContext* global_env()
            {
                return global_env_;
            }
    };

    class ValueGuard
    {
        private:
            size_t count_;

        public:
            ValueGuard() : count_(0)
            {
            }
            ~ValueGuard()
            {
                while(count_ > 0)
                {
                    RuntimeContext::Global()->PopValue();
                    count_--;
                }
            }

            void AddValue(JSValue* val)
            {
                RuntimeContext::Global()->AddValue(val);
                count_++;
            }
    };

    // TODO(zhuzilin) move this method to a better place
    inline JSValue* JSObject::DefaultValue(Error* e, const std::string& hint)
    {
        std::string first;
        std::string second;
        if((hint == "String") || (hint.empty() && (obj_type() == OBJ_DATE)))
        {
            first = "toString";
            second = "valueOf";
        }
        else if((hint == "Number") || (hint.empty() && (obj_type() != OBJ_DATE)))
        {
            first = "valueOf";
            second = "toString";
        }
        else
        {
            assert(false);
        }

        ValueGuard guard;
        guard.AddValue(this);

        JSValue* to_string = Get(e, first);
        if(!e->IsOk())
        {
            return nullptr;
        }
        if(to_string->IsCallable())
        {
            JSObject* to_string_obj = static_cast<JSObject*>(to_string);
            JSValue* str = to_string_obj->Call(e, this);
            if(!e->IsOk())
            {
                return nullptr;
            }
            if(str->IsPrimitive())
            {
                return str;
            }
        }
        JSValue* value_of = Get(e, second);
        if(!e->IsOk())
        {
            return nullptr;
        }
        if(value_of->IsCallable())
        {
            JSObject* value_of_obj = static_cast<JSObject*>(value_of);
            JSValue* val = value_of_obj->Call(e, this);
            if(!e->IsOk())
            {
                return nullptr;
            }
            if(val->IsPrimitive())
            {
                return val;
            }
        }
        *e = *Error::TypeError("failed to get [[DefaultValue]]");
        return nullptr;
    }

    std::string ToString(Error* e, JSValue* input);
    PropertyDescriptor* ToPropertyDescriptor(Error* e, JSValue* val);

    class ObjectProto : public JSObject
    {
        private:
            ObjectProto() : JSObject(OBJ_OTHER, "Object", true, nullptr, false, false)
            {
            }

        public:
            static ObjectProto* Instance()
            {
                static ObjectProto singleton;
                return &singleton;
            }

            static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                JSValue* val;
                JSObject* obj;
                (void)this_arg;
                (void)vals;
                val = RuntimeContext::TopValue();
                if(val->IsUndefined())
                {
                    return new String("[object Undefined]");
                }
                if(val->IsNull())
                {
                    return new String("[object Null]");
                }
                obj = ToObject(e, val);
                return new String("[object " + obj->Class() + "]");
            }

            static JSValue* toLocaleString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                assert(false);
            }

            static JSValue* valueOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                JSValue* val;
                JSObject* O;
                (void)this_arg;
                (void)vals;
                val = RuntimeContext::TopValue();
                O = ToObject(e, val);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                // TODO(zhuzilin) Host object
                return O;
            }

            static JSValue* hasOwnProperty(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                assert(false);
            }

            static JSValue* isPrototypeOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                assert(false);
            }

            static JSValue* propertyIsEnumerable(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                assert(false);
            }

    };

    class Object : public JSObject
    {
        public:
            Object() : JSObject(OBJ_OTHER, "Object", true, nullptr, false, false)
            {
                SetPrototype(ObjectProto::Instance());
            }
    };

    class ObjectConstructor : public JSObject
    {
        public:
            static ObjectConstructor* Instance()
            {
                static ObjectConstructor singleton;
                return &singleton;
            }

            // 15.2.1 The Object Constructor Called as a Function
            JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {}) override
            {
                (void)this_arg;
                if(arguments.empty() || arguments[0]->IsNull() || arguments[0]->IsUndefined())
                {
                    return Construct(e, arguments);
                }
                return ToObject(e, arguments[0]);
            }

            // 15.2.2 The Object Constructor
            JSObject* Construct(Error* e, const std::vector<JSValue*>& arguments) override
            {
                JSValue* value;
                JSObject* obj;
                if(!arguments.empty())
                {// 1
                    value = arguments[0];
                    switch(value->type())
                    {
                        case JSValue::JS_OBJECT:
                            // TODO(zhuzilin) deal with host object.
                            return static_cast<JSObject*>(value);
                        case JSValue::JS_STRING:
                        case JSValue::JS_BOOL:
                        case JSValue::JS_NUMBER:
                            return ToObject(e, value);
                        default:
                            break;
                    }
                }
                assert(arguments.empty() || arguments[0]->IsNull() || arguments[0]->IsUndefined());
                obj = new Object();
                return obj;
            }

            // 15.2.3.2 Object.getPrototypeOf ( O )
            static JSValue* getPrototypeOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)this_arg;
                if(vals.empty() || !vals[0]->IsObject())
                {
                    *e = *Error::TypeError();
                    return nullptr;
                }
                return static_cast<JSObject*>(vals[0])->Prototype();
            }

            // 15.2.3.3 Object.getOwnPropertyDescriptor ( O, P )
            static JSValue* getOwnPropertyDescriptor(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                assert(false);
            }

            static JSValue* getOwnPropertyNames(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                assert(false);
            }

            static JSValue* create(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                Object* obj;
                if(vals.empty() || (!vals[0]->IsObject() && !vals[0]->IsNull()))
                {
                    *e = *Error::TypeError("Object.create called on non-object");
                    return nullptr;
                }
                obj = new Object();
                obj->SetPrototype(vals[0]);
                if(vals.size() > 1 && !vals[1]->IsUndefined())
                {
                    ObjectConstructor::defineProperties(e, this_arg, vals);
                    if(!e->IsOk())
                    {
                        return nullptr;
                    }
                }
                return obj;
            }

            static JSValue* defineProperty(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                std::string name;
                JSObject* O;
                PropertyDescriptor* desc;
                (void)this_arg;
                if(vals.empty() || !vals[0]->IsObject())
                {
                    *e = *Error::TypeError("Object.defineProperty called on non-object");
                    return nullptr;
                }
                O = static_cast<JSObject*>(vals[0]);
                if(vals.size() < 2)
                {
                    *e = *Error::TypeError("Object.defineProperty need 3 arguments");
                    return nullptr;
                }
                name = ::es::ToString(e, vals[1]);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                desc = ToPropertyDescriptor(e, vals[2]);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                O->DefineOwnProperty(e, name, desc, true);
                return O;
            }

            static JSValue* defineProperties(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                assert(false);
            }

            static JSValue* seal(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                assert(false);
            }

            static JSValue* freeze(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                assert(false);
            }

            static JSValue* preventExtensions(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                JSObject* obj;
                (void)this_arg;
                if(vals.empty() || !vals[0]->IsObject())
                {
                    *e = *Error::TypeError("Object.preventExtensions called on non-object");
                    return nullptr;
                }
                obj = static_cast<JSObject*>(vals[0]);
                obj->SetExtensible(false);
                return obj;
            }

            static JSValue* isSealed(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                assert(false);
            }

            static JSValue* isFrozen(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                assert(false);
            }

            static JSValue* isExtensible(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                JSObject* obj;
                (void)this_arg;
                if(vals.empty() || !vals[0]->IsObject())
                {
                    *e = *Error::TypeError("Object.isExtensible called on non-object");
                    return nullptr;
                }
                obj = static_cast<JSObject*>(vals[0]);
                return Bool::Wrap(obj->Extensible());
            }

            static JSValue* keys(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals);

            // ES6
            static JSValue* setPrototypeOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)this_arg;
                if(vals.size() < 2)
                {
                    *e = *Error::TypeError("Object.preventExtensions need 2 arguments");
                    return nullptr;
                }
                vals[0]->CheckObjectCoercible(e);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                if(!(vals[1]->IsNull() || vals[1]->IsObject()))
                {
                    *e = *Error::TypeError("");
                    return nullptr;
                }
                if(!vals[0]->IsObject())
                {
                    return vals[0];
                }
                static_cast<JSObject*>(vals[0])->SetPrototype(vals[1]);
                return vals[0];
            }

            static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                return new String("function Object() { [native code] }");
            }

        private:
            ObjectConstructor() : JSObject(OBJ_OTHER, "Object", true, nullptr, true, true)
            {
            }
    };

    bool ToBoolean(JSValue* input);

    inline JSValue* FromPropertyDescriptor(Error* e, JSValue* value)
    {
        if(value->IsUndefined())
        {
            return Undefined::Instance();
        }
        PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(value);
        JSObject* obj = new Object();
        if(desc->IsDataDescriptor())
        {
            PropertyDescriptor* value_desc = new PropertyDescriptor();
            value_desc->SetDataDescriptor(desc->Value(), true, true, true);
            obj->DefineOwnProperty(e, "value", value_desc, false);
            if(!e->IsOk())
            {
                return nullptr;
            }

            PropertyDescriptor* writable_desc = new PropertyDescriptor();
            writable_desc->SetDataDescriptor(Bool::Wrap(desc->Writable()), true, true, true);
            obj->DefineOwnProperty(e, "writable", writable_desc, false);
            if(!e->IsOk())
            {
                return nullptr;
            }
        }
        else
        {
            assert(desc->IsAccessorDescriptor());
            PropertyDescriptor* get_desc = new PropertyDescriptor();
            get_desc->SetDataDescriptor(desc->Get(), true, true, true);
            obj->DefineOwnProperty(e, "get", get_desc, false);
            if(!e->IsOk())
            {
                return nullptr;
            }

            PropertyDescriptor* set_desc = new PropertyDescriptor();
            set_desc->SetDataDescriptor(desc->Set(), true, true, true);
            obj->DefineOwnProperty(e, "set", set_desc, false);
            if(!e->IsOk())
            {
                return nullptr;
            }
        }

        PropertyDescriptor* enumerable_desc = new PropertyDescriptor();
        enumerable_desc->SetDataDescriptor(Bool::Wrap(desc->Enumerable()), true, true, true);
        obj->DefineOwnProperty(e, "get", enumerable_desc, false);
        if(!e->IsOk())
        {
            return nullptr;
        }

        PropertyDescriptor* configurable_desc = new PropertyDescriptor();
        configurable_desc->SetDataDescriptor(Bool::Wrap(desc->Configurable()), true, true, true);
        obj->DefineOwnProperty(e, "set", configurable_desc, false);
        if(!e->IsOk())
        {
            return nullptr;
        }

        return obj;
    }

    inline PropertyDescriptor* ToPropertyDescriptor(Error* e, JSValue* val)
    {
        if(!val->IsObject())
        {
            *e = *Error::TypeError();
            return nullptr;
        }
        JSObject* obj = static_cast<JSObject*>(val);
        PropertyDescriptor* desc = new PropertyDescriptor();
        if(obj->HasProperty("enumerable"))
        {
            JSValue* value = obj->Get(e, "enumerable");
            desc->SetEnumerable(ToBoolean(value));
        }
        if(obj->HasProperty("configurable"))
        {
            JSValue* value = obj->Get(e, "configurable");
            desc->SetConfigurable(ToBoolean(value));
        }
        if(obj->HasProperty("value"))
        {
            JSValue* value = obj->Get(e, "value");
            desc->SetValue(value);
        }
        if(obj->HasProperty("writable"))
        {
            JSValue* value = obj->Get(e, "writable");
            desc->SetWritable(ToBoolean(value));
        }
        if(obj->HasProperty("get"))
        {
            JSValue* value = obj->Get(e, "get");
            if(!value->IsCallable() && !value->IsUndefined())
            {
                *e = *Error::TypeError("getter not callable.");
            }
            desc->SetGet(value);
        }
        if(obj->HasProperty("set"))
        {
            JSValue* value = obj->Get(e, "set");
            if(!value->IsCallable() && !value->IsUndefined())
            {
                *e = *Error::TypeError("setter not callable.");
            }
            desc->SetSet(value);
        }
        if(desc->HasSet() || desc->HasGet())
        {
            if(desc->HasValue() || desc->HasWritable())
            {
                *e = *Error::TypeError("cannot have both get/set and value/writable");
                return nullptr;
            }
        }
        return desc;
    }

    class ErrorProto : public JSObject
    {
        public:
            static ErrorProto* Instance()
            {
                static ErrorProto singleton;
                return &singleton;
            }

            JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {}) override
            {
                (void)e;
                (void)this_arg;
                (void)arguments;
                return Undefined::Instance();
            }

            static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                assert(false);
            }

        private:
            ErrorProto() : JSObject(OBJ_ERROR, "Error", true, nullptr, false, true)
            {
            }
    };

    class ErrorObject : public JSObject
    {
        private:
            Error* e_;

        public:
            ErrorObject(Error* e) : JSObject(OBJ_ERROR, "Error", true, nullptr, false, false), e_(e)
            {
                SetPrototype(ErrorProto::Instance());
                AddValueProperty("message", new String(e->message()), true, false, false);
            }

            Error* e()
            {
                return e_;
            }
            Error::Type ErrorType()
            {
                return e_->type();
            }
            std::string ErrorMessage()
            {
                return e_->message();
            }

            std::string ToString()
            {
                return log::ToString(e_->message());
            }
    };

    class ErrorConstructor : public JSObject
    {
        public:
            static ErrorConstructor* Instance()
            {
                static ErrorConstructor singleton;
                return &singleton;
            }

            JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {}) override
            {
                (void)this_arg;
                return Construct(e, arguments);
            }

            JSObject* Construct(Error* e, const std::vector<JSValue*>& arguments) override
            {
                if(arguments.empty() || arguments[0]->IsUndefined())
                {
                    return new ErrorObject(Error::NativeError(::es::ToString(nullptr, Undefined::Instance())));
                }
                std::string s = ::es::ToString(e, arguments[0]);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                return new ErrorObject(Error::NativeError(s));
            }

            static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                return new String("function Error() { [native code] }");
            }

        private:
            ErrorConstructor() : JSObject(OBJ_OTHER, "Error", true, nullptr, true, true)
            {
            }
    };

    // NOTE(zhuzilin) Completion is the spec type to represent
    // the evaluation result of the statement.
    // It won't interact with other types, so does not need to
    // inherit JSValue.
    struct Completion
    {
        enum Type
        {
            NORMAL,
            BREAKING,
            CONTINUING,
            RETURNING,
            THROWING,
        };

        Type type;
        JSValue* value;
        std::string target;

        Completion() : Completion(NORMAL, nullptr, "")
        {
        }

        Completion(Type type, JSValue* value, const std::string& target) : type(type), value(value), target(target)
        {
        }

        bool IsAbruptCompletion()
        {
            return type != NORMAL;
        }
        bool IsThrow()
        {
            return type == THROWING;
        }
    };

    double ToNumber(Error* e, JSValue* input);
    std::string NumberToString(double m);
    Completion EvalProgram(AST* ast);

    class FunctionProto : public JSObject
    {
        private:
            FunctionProto() : JSObject(OBJ_FUNC, "Function", true, nullptr, false, true)
            {
            }

        public:
            static FunctionProto* Instance()
            {
                static FunctionProto singleton;
                return &singleton;
            }

            JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {}) override
            {
                (void)e;
                (void)this_arg;
                (void)arguments;
                return Undefined::Instance();
            }

            static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals);

            // 15.3.4.3 Function.prototype.apply (thisArg, argArray)
            static JSValue* apply(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                JSValue* val;
                (void)this_arg;
                val = RuntimeContext::TopValue();
                if(!val->IsObject())
                {
                    *e = *Error::TypeError("Function.prototype.apply called on non-object");
                    return nullptr;
                }
                JSObject* func = static_cast<JSObject*>(val);
                if(!func->IsCallable())
                {
                    *e = *Error::TypeError("Function.prototype.apply called on non-callable");
                    return nullptr;
                }
                if(vals.empty())
                {
                    return func->Call(e, Undefined::Instance(), {});
                }
                if(vals.size() < 2 || vals[1]->IsNull() || vals[1]->IsUndefined())
                {// 2
                    return func->Call(e, vals[0], {});
                }
                if(!vals[1]->IsObject())
                {// 3
                    *e = *Error::TypeError("Function.prototype.apply's argument is non-object");
                    return nullptr;
                }
                JSObject* arg_array = static_cast<JSObject*>(vals[1]);
                JSValue* len = arg_array->Get(e, "length");
                if(!e->IsOk())
                {
                    return nullptr;
                }
                size_t n = ToNumber(e, len);
                std::vector<JSValue*> arg_list;// 6
                size_t index = 0;// 7
                while(index < n)
                {// 8
                    std::string index_name = ::es::NumberToString(index);
                    if(!e->IsOk())
                    {
                        return nullptr;
                    }
                    JSValue* next_arg = arg_array->Get(e, index_name);
                    if(!e->IsOk())
                    {
                        return nullptr;
                    }
                    arg_list.emplace_back(next_arg);
                    index++;
                }
                return func->Call(e, vals[0], arg_list);
            }

            static JSValue* call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)this_arg;
                JSValue* val = RuntimeContext::TopValue();
                if(!val->IsObject())
                {
                    *e = *Error::TypeError("Function.prototype.call called on non-object");
                    return nullptr;
                }
                JSObject* func = static_cast<JSObject*>(val);
                if(!func->IsCallable())
                {
                    *e = *Error::TypeError("Function.prototype.call called on non-callable");
                    return nullptr;
                }
                if(static_cast<unsigned int>(!vals.empty()) != 0u)
                {
                    JSValue* this_arg = vals[0];
                    return func->Call(e, this_arg, std::vector<JSValue*>(vals.begin() + 1, vals.end()));
                }
                else
                {
                    return func->Call(e, Undefined::Instance(), {});
                }
            }

            static JSValue* bind(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals);

    };

    void EnterFunctionCode(Error* e, JSObject* f, ProgramOrFunctionBody* body, JSValue* this_arg, const std::vector<JSValue*>& args, bool strict);

    class FunctionObject : public JSObject
    {
        private:
            std::vector<std::string> formal_params_;
            LexicalEnvironment* scope_;
            ProgramOrFunctionBody* body_;
            bool strict_ = false;

        protected:
            bool from_bind_;

        public:
            FunctionObject(const std::vector<std::string>& names, AST* body, LexicalEnvironment* scope, bool from_bind_ = false):
                JSObject(OBJ_FUNC, "Function", true, nullptr, true, true),
                formal_params_(names),
                scope_(scope),
                from_bind_(from_bind_)
            {
                // 13.2 Creating Function Objects
                SetPrototype(FunctionProto::Instance());
                // Whether the function is made from bind.
                if(body != nullptr)
                {
                    assert(body->type() == AST::AST_FUNC_BODY);
                    body_ = static_cast<ProgramOrFunctionBody*>(body);
                    strict_ = body_->strict() || RuntimeContext::TopContext()->strict();
                    AddValueProperty("length", new Number(names.size()), false, false, false);// 14 & 15
                    JSObject* proto = new Object();// 16
                    proto->AddValueProperty("constructor", this, true, false, true);
                    // 15.3.5.2 prototype
                    AddValueProperty("prototype", proto, true, false, false);
                    if(strict_)
                    {
                        // TODO(zhuzilin) thrower
                    }
                }
            }

            virtual LexicalEnvironment* Scope()
            {
                return scope_;
            };
            virtual std::vector<std::string> FormalParameters()
            {
                return formal_params_;
            };
            virtual AST* Code()
            {
                return body_;
            }
            virtual bool strict()
            {
                return strict_;
            }
            bool from_bind()
            {
                return from_bind_;
            }

            // 13.2.1 [[Call]]
            virtual JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments) override
            {
                log::PrintSource("enter FunctionObject::Call ", body_->source());
                EnterFunctionCode(e, this, body_, this_arg, arguments, strict_);
                if(!e->IsOk())
                {
                    return nullptr;
                }

                Completion result;
                if(body_ != nullptr)
                {
                    result = EvalProgram(body_);
                }
                RuntimeContext::Global()->PopContext();// 3

                switch(result.type)
                {
                    case Completion::RETURNING:
                        return result.value;
                    case Completion::THROWING:
                    {
                        std::string message = ::es::ToString(e, result.value);
                        if(result.value->IsObject())
                        {
                            JSObject* obj = static_cast<JSObject*>(result.value);
                            if(obj->obj_type() == JSObject::OBJ_ERROR)
                            {
                                *e = *(static_cast<ErrorObject*>(obj)->e());
                                return nullptr;
                            }
                        }
                        *e = *Error::NativeError(message);
                        return nullptr;
                    }
                    default:
                        assert(result.type == Completion::NORMAL);
                        return Undefined::Instance();
                }
            }

            // 13.2.2 [[Construct]]
            virtual JSObject* Construct(Error* e, const std::vector<JSValue*>& arguments) override
            {
                log::PrintSource("enter FunctionObject::Construct");
                JSObject* obj = new JSObject(OBJ_OTHER, "Object", true, nullptr, false, false);
                JSValue* proto = Get(e, "prototype");
                if(!e->IsOk())
                {
                    return nullptr;
                }
                if(proto->IsObject())
                {// 6
                    obj->SetPrototype(proto);
                }
                else
                {// 7
                    obj->SetPrototype(ObjectProto::Instance());
                }
                JSValue* result = Call(e, obj, arguments);// 8
                if(!e->IsOk())
                {
                    return nullptr;
                }
                if(result->IsObject())
                {// 9
                    return static_cast<JSObject*>(result);
                }
                return obj;// 10
            }

            // 15.3.5.3 [[HasInstance]] (V)
            virtual bool HasInstance(Error* e, JSValue* V) override
            {
                if(!V->IsObject())
                {
                    return false;
                }
                JSValue* O = Get(e, "prototype");
                if(!e->IsOk())
                {
                    return false;
                }
                if(!O->IsObject())
                {
                    *e = *Error::TypeError();
                    return false;
                }
                while(!V->IsNull())
                {
                    if(V == O)
                    {
                        return true;
                    }
                    V = static_cast<JSObject*>(V)->Get(e, "prototype");
                    if(!e->IsOk())
                    {
                        return false;
                    }
                }
                return false;
            }

            // 15.3.5.4 [[Get]] (P)
            JSValue* Get(Error* e, const std::string& P) override
            {
                JSValue* v = JSObject::Get(e, P);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                if(P == "caller")
                {// 2
                    if(v->IsObject())
                    {
                        JSObject* v_obj = static_cast<JSObject*>(v);
                        if(v_obj->IsFunction())
                        {
                            FunctionObject* v_func = static_cast<FunctionObject*>(v);
                            if(v_func->strict())
                            {
                                *e = *Error::TypeError();
                                return nullptr;
                            }
                        }
                    }
                }
                return v;
            }

            std::string ToString() override
            {
                std::string result = "Function(";
                if(!formal_params_.empty())
                {
                    result += log::ToString(formal_params_[0]);
                    for(size_t i = 1; i < formal_params_.size(); i++)
                    {
                        result += "," + log::ToString(formal_params_[i]);
                    }
                }
                result += ")";
                return result;
            }
    };

    class BindFunctionObject : public FunctionObject
    {
        private:
            JSObject* target_function_;
            JSValue* bound_this_;
            std::vector<JSValue*> bound_args_;

        public:
            BindFunctionObject(JSObject* target_function, JSValue* bound_this, const std::vector<JSValue*>& bound_args):
                FunctionObject({}, nullptr, nullptr, true),
                target_function_(target_function),
                bound_this_(bound_this),
                bound_args_(bound_args)
            {
            }

            LexicalEnvironment* Scope() override
            {
                assert(false);
            };
            std::vector<std::string> FormalParameters() override
            {
                assert(false);
            };
            AST* Code() override
            {
                assert(false);
            }
            bool strict() override
            {
                assert(false);
            }

            JSObject* TargetFunction()
            {
                return target_function_;
            }
            JSValue* BoundThis()
            {
                return bound_this_;
            }
            std::vector<JSValue*> BoundArgs()
            {
                return bound_args_;
            }

            virtual JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& extra_args) override
            {
                std::vector<JSValue*> args;
                (void)this_arg;
                args.insert(args.end(), bound_args_.begin(), bound_args_.end());
                args.insert(args.end(), extra_args.begin(), extra_args.end());
                return target_function_->Call(e, bound_this_, args);
            }

            // 13.2.2 [[Construct]]
            virtual JSObject* Construct(Error* e, const std::vector<JSValue*>& extra_args) override
            {
                if(!target_function_->IsConstructor())
                {
                    *e = *Error::TypeError("target function has no [[Construct]] internal method");
                    return nullptr;
                }
                std::vector<JSValue*> args;
                args.insert(args.end(), bound_args_.begin(), bound_args_.end());
                args.insert(args.end(), extra_args.begin(), extra_args.end());
                return target_function_->Construct(e, args);
            }

            // 15.3.4.5.3 [[HasInstance]] (V)
            virtual bool HasInstance(Error* e, JSValue* V) override
            {
                return target_function_->HasInstance(e, V);
            }
    };

    class FunctionConstructor : public JSObject
    {
        public:
            static FunctionConstructor* Instance()
            {
                static FunctionConstructor singleton;
                return &singleton;
            }

            // 15.3.1 The Function Constructor Called as a Function
            JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {}) override
            {
                (void)this_arg;
                return Construct(e, arguments);
            }

            // 15.3.2.1 new Function (p1, p2, … , pn, body)
            JSObject* Construct(Error* e, const std::vector<JSValue*>& arguments) override
            {
                log::PrintSource("enter FunctionConstructor::Construct");
                size_t arg_count = arguments.size();
                std::string P;
                std::string body;
                if(arg_count == 1)
                {
                    body = ::es::ToString(e, arguments[0]);
                    if(!e->IsOk())
                    {
                        return nullptr;
                    }
                }
                else if(arg_count > 1)
                {
                    P += ::es::ToString(e, arguments[0]);
                    if(!e->IsOk())
                    {
                        return nullptr;
                    }
                    for(size_t i = 1; i < arg_count - 1; i++)
                    {
                        P += "," + ::es::ToString(e, arguments[i]);
                        if(!e->IsOk())
                        {
                            return nullptr;
                        }
                    }
                    body = ::es::ToString(e, arguments[arg_count - 1]);
                    if(!e->IsOk())
                    {
                        return nullptr;
                    }
                }
                std::vector<std::string> names;
                AST* body_ast;
                if(!P.empty())
                {
                    Parser parser(P);
                    names = parser.ParseFormalParameterList();
                    if(names.empty())
                    {
                        *e = *Error::SyntaxError("invalid parameter name");
                        return nullptr;
                    }
                }
                {
                    Parser parser(body);
                    body_ast = parser.ParseFunctionBody(Token::TK_EOS);
                    if(body_ast->IsIllegal())
                    {
                        *e = *Error::SyntaxError("failed to parse function body: " + body_ast->source());
                        return nullptr;
                    }
                }
                LexicalEnvironment* scope = LexicalEnvironment::Global();
                bool strict = static_cast<ProgramOrFunctionBody*>(body_ast)->strict();
                if(strict)
                {
                    // 13.1
                    if(HaveDuplicate(names))
                    {
                        *e = *Error::SyntaxError();
                        return nullptr;
                    }
                    for(const auto& name : names)
                    {
                        if(name == "eval" || name == "arguments")
                        {
                            *e = *Error::SyntaxError();
                            return nullptr;
                        }
                    }
                }
                return new FunctionObject(names, body_ast, scope);
            }

            static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                return new String("function Function() { [native code] }");
            }

        private:
            FunctionConstructor() : JSObject(OBJ_OTHER, "Function", true, nullptr, true, true)
            {
            }
    };

    inline JSValue* FunctionProto::toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
    {
        JSValue* val;
        JSObject* obj;
        FunctionObject* func;
        (void)this_arg;
        (void)vals;
        val = RuntimeContext::TopValue();
        if(!val->IsObject())
        {
            *e = *Error::TypeError("Function.prototype.toString called on non-object");
            return nullptr;
        }
        obj = static_cast<JSObject*>(val);
        if(obj->obj_type() != JSObject::OBJ_FUNC)
        {
            *e = *Error::TypeError("Function.prototype.toString called on non-function");
            return nullptr;
        }
        func = static_cast<FunctionObject*>(obj);
        std::string str = "function (";
        auto params = func->FormalParameters();
        if(!params.empty())
        {
            str += params[0];
            for(size_t i = 1; i < params.size(); i++)
            {
                str += "," + params[i];
            }
        }
        str += ")";
        return new String(str);
    }

    // 15.3.4.5 Function.prototype.bind (thisArg [, arg1 [, arg2, …]])
    inline JSValue* FunctionProto::bind(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
    {
        size_t L;
        size_t len;
        std::vector<JSValue*> A;
        JSValue* val;
        JSValue* this_arg_for_F;
        JSObject* target;
        BindFunctionObject* F;
        (void)this_arg;
        val = RuntimeContext::TopValue();
        if(!val->IsCallable())
        {
            *e = *Error::TypeError("Function.prototype.call called on non-callable");
            return nullptr;
        }
        target = static_cast<JSObject*>(val);
        this_arg_for_F = Undefined::Instance();
        if(!vals.empty())
        {
            this_arg_for_F = vals[0];
        }
        if(vals.size() > 1)
        {
            A = std::vector<JSValue*>(vals.begin() + 1, vals.end());
        }
        F = new BindFunctionObject(target, this_arg_for_F, A);
        len = 0;
        if(target->Class() == "Function")
        {
            L = ToNumber(e, target->Get(e, "length"));
            if(L - A.size() > 0)
            {
                len = L - A.size();
            }
        }
        F->AddValueProperty("length", new Number(len), false, false, false);
        // 19
        // TODO(zhuzilin) thrower
        return F;
    }

    inline FunctionObject* InstantiateFunctionDeclaration(Error* e, Function* func_ast)
    {
        assert(func_ast->is_named());
        std::string identifier = func_ast->name();
        auto func_env = LexicalEnvironment::NewDeclarativeEnvironment(// 1
        RuntimeContext::TopLexicalEnv());
        auto env_rec = static_cast<DeclarativeEnvironmentRecord*>(func_env->env_rec());// 2
        env_rec->CreateImmutableBinding(identifier);// 3
        auto body = static_cast<ProgramOrFunctionBody*>(func_ast->body());
        bool strict = body->strict() || RuntimeContext::TopContext()->strict();
        if(strict)
        {
            // 13.1
            if(HaveDuplicate(func_ast->params()))
            {
                *e = *Error::SyntaxError();
                return nullptr;
            }
            for(const auto& name : func_ast->params())
            {
                if(name == "eval" || name == "arguments")
                {
                    *e = *Error::SyntaxError();
                    return nullptr;
                }
            }
            if(func_ast->name() == "eval" || func_ast->name() == "arguments")
            {
                *e = *Error::SyntaxError();
                return nullptr;
            }
        }
        FunctionObject* closure = new FunctionObject(func_ast->params(), func_ast->body(), func_env);// 4
        env_rec->InitializeImmutableBinding(identifier, closure);// 5
        return closure;// 6
    }

    inline JSValue* EvalFunction(Error* e, AST* ast)
    {
        assert(ast->type() == AST::AST_FUNC);
        Function* func_ast = static_cast<Function*>(ast);

        if(func_ast->is_named())
        {
            return InstantiateFunctionDeclaration(e, func_ast);
        }
        else
        {
            auto body = static_cast<ProgramOrFunctionBody*>(func_ast->body());
            bool strict = body->strict() || RuntimeContext::TopContext()->strict();
            if(strict)
            {
                // 13.1
                if(HaveDuplicate(func_ast->params()))
                {
                    *e = *Error::SyntaxError();
                    return nullptr;
                }
                for(const auto& name : func_ast->params())
                {
                    if(name == "eval" || name == "arguments")
                    {
                        *e = *Error::SyntaxError();
                        return nullptr;
                    }
                }
            }
            return new FunctionObject(func_ast->params(), func_ast->body(), RuntimeContext::TopLexicalEnv());
        }
    }

    // TODO(zhuzilin) move this function to a better place
    inline void JSObject::AddFuncProperty(const std::string& name, inner_func callable, bool writable, bool enumerable, bool configurable)
    {
        JSObject* value = new JSObject(OBJ_INNER_FUNC, "InternalFunc", false, nullptr, false, true, std::move(callable));
        value->SetPrototype(FunctionProto::Instance());
        AddValueProperty(name, value, writable, enumerable, configurable);
    }

    class NumberProto : public JSObject
    {
        public:
            static NumberProto* Instance()
            {
                static NumberProto singleton;
                return &singleton;
            }

            static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                assert(false);
            }

            static JSValue* toLocaleString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                assert(false);
            }

            static JSValue* valueOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                JSValue* val;
                JSObject* obj;
                (void)this_arg;
                (void)vals;
                val = RuntimeContext::TopValue();
                if(val->IsObject())
                {
                    obj = static_cast<JSObject*>(val);
                    if(obj->obj_type() == JSObject::OBJ_NUMBER)
                    {
                        return obj->PrimitiveValue();
                    }
                }
                else if(val->IsNumber())
                {
                    return val;
                }
                *e = *Error::TypeError("Number.prototype.valueOf called with non-number");
                return nullptr;
            }

            static JSValue* toFixed(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                assert(false);
            }

            static JSValue* toExponential(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                assert(false);
            }

            static JSValue* toPrecision(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
            {
                (void)e;
                (void)this_arg;
                (void)vals;
                assert(false);
            }

        private:
            NumberProto() : JSObject(OBJ_NUMBER, "Number", true, Number::Zero(), false, false)
            {
            }
    };

    class NumberObject : public JSObject
    {
    public:
        NumberObject(JSValue* primitive_value)
        : JSObject(OBJ_NUMBER,
                   "Number",
                   true,// extensible
                   primitive_value,
                   false,
                   false)
        {
            SetPrototype(NumberProto::Instance());
        }
    };

    class NumberConstructor : public JSObject
    {
    public:
        static NumberConstructor* Instance()
        {
            static NumberConstructor singleton;
            return &singleton;
        }

        // 15.7.1.1 Number ( [ value ] )
        JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {}) override
        {
            Number* js_num;
            (void)this_arg;
            if(arguments.empty())
            {
                js_num = Number::Zero();
            }
            else
            {
                double num = ToNumber(e, arguments[0]);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                js_num = new Number(num);
            }
            return js_num;
        }

        JSObject* Construct(Error* e, const std::vector<JSValue*>& arguments) override
        {
            Number* js_num;
            if(arguments.empty())
            {
                js_num = Number::Zero();
            }
            else
            {
                double num = ToNumber(e, arguments[0]);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                js_num = new Number(num);
            }
            return new NumberObject(js_num);
        }

        static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            return new String("function Number() { [native code] }");
        }

    private:
        NumberConstructor() : JSObject(OBJ_OTHER, "Number", true, nullptr, true, true)
        {
        }
    };

    class BoolProto : public JSObject
    {
    public:
        static BoolProto* Instance()
        {
            static BoolProto singleton;
            return &singleton;
        }

        JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {}) override
        {
            (void)e;
            (void)this_arg;
            (void)arguments;
            return Undefined::Instance();
        }

        static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)vals;
            return ToBoolean(this_arg) ? String::True() : String::False();
        }

        static JSValue* valueOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

    private:
        BoolProto() : JSObject(OBJ_BOOL, "Boolean", true, Bool::False(), false, true)
        {
        }
    };

    class BoolObject : public JSObject
    {
    public:
        BoolObject(JSValue* primitive_value) : JSObject(OBJ_BOOL, "Boolean", true, primitive_value, false, false)
        {
            SetPrototype(BoolProto::Instance());
        }
    };

    class BoolConstructor : public JSObject
    {
    public:
        static BoolConstructor* Instance()
        {
            static BoolConstructor singleton;
            return &singleton;
        }

        JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {}) override
        {
            bool b;
            (void)e;
            (void)this_arg;
            if(arguments.empty())
            {
                b = ToBoolean(Undefined::Instance());
            }
            else
            {
                b = ToBoolean(arguments[0]);
            }
            return Bool::Wrap(b);
        }

        JSObject* Construct(Error* e, const std::vector<JSValue*>& arguments) override
        {
            bool b;
            (void)e;
            if(arguments.empty())
            {
                b = ToBoolean(Undefined::Instance());
            }
            else
            {
                b = ToBoolean(arguments[0]);
            }
            return new BoolObject(Bool::Wrap(b));
        }

        static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            return new String("function Bool() { [native code] }");
        }

    private:
        BoolConstructor() : JSObject(OBJ_OTHER, "Boolean", true, nullptr, true, true)
        {
        }
    };

    double ToInteger(Error* e, JSValue* input);
    double ToUint16(Error* e, JSValue* input);

    class StringProto : public JSObject
    {
    public:
        static StringProto* Instance()
        {
            static StringProto singleton;
            return &singleton;
        }

        static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)this_arg;
            (void)vals;
            JSValue* val = RuntimeContext::TopValue();
            if(!val->IsObject())
            {
                *e = *Error::TypeError("String.prototype.toString called with non-object");
                return nullptr;
            }
            JSObject* obj = static_cast<JSObject*>(val);
            if(obj->obj_type() != JSObject::OBJ_STRING)
            {
                *e = *Error::TypeError("String.prototype.toString called with non-string");
                return nullptr;
            }
            return obj->PrimitiveValue();
        }

        static JSValue* valueOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)this_arg;
            (void)vals;
            JSValue* val = RuntimeContext::TopValue();
            if(!val->IsObject())
            {
                *e = *Error::TypeError("String.prototype.valueOf called with non-object");
                return nullptr;
            }
            JSObject* obj = static_cast<JSObject*>(val);
            if(obj->obj_type() != JSObject::OBJ_STRING)
            {
                *e = *Error::TypeError("String.prototype.valueOf called with non-string");
                return nullptr;
            }
            return obj->PrimitiveValue();
        }

        static JSValue* charAt(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)this_arg;
            if(vals.empty())
            {
                return String::Empty();
            }
            JSValue* val = RuntimeContext::TopValue();
            val->CheckObjectCoercible(e);
            if(!e->IsOk())
            {
                return nullptr;
            }
            std::string S = ::es::ToString(e, val);
            if(!e->IsOk())
            {
                return nullptr;
            }
            int position = ToInteger(e, vals[0]);
            if(!e->IsOk())
            {
                return nullptr;
            }
            if(position < 0 || position >= int(S.size()))
            {
                return String::Empty();
            }
            return new String(S.substr(position, 1));
        }

        static JSValue* charCodeAt(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)this_arg;
            if(vals.empty())
            {
                return Number::NaN();
            }
            JSValue* val = RuntimeContext::TopValue();
            val->CheckObjectCoercible(e);
            if(!e->IsOk())
            {
                return nullptr;
            }
            std::string S = ::es::ToString(e, val);
            if(!e->IsOk())
            {
                return nullptr;
            }
            int position = ToInteger(e, vals[0]);
            if(!e->IsOk())
            {
                return nullptr;
            }
            if(position < 0 || position >= int(S.size()))
            {
                return Number::NaN();
            }
            return new Number((double)S[position]);
        }

        static JSValue* concat(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            JSValue* val = RuntimeContext::TopValue();
            val->CheckObjectCoercible(e);
            if(!e->IsOk())
            {
                return nullptr;
            }
            std::string S = ::es::ToString(e, val);
            if(!e->IsOk())
            {
                return nullptr;
            }
            std::string R = S;
            std::vector<JSValue*> args = vals;
            for(auto arg : args)
            {
                std::string next = ::es::ToString(e, arg);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                R += next;
            }
            return new String(R);
        }

        static JSValue* indexOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)this_arg;
            JSValue* val = RuntimeContext::TopValue();
            val->CheckObjectCoercible(e);
            if(!e->IsOk())
            {
                return nullptr;
            }
            std::string S = ::es::ToString(e, val);
            if(!e->IsOk())
            {
                return nullptr;
            }
            JSValue* search_string;
            if(vals.empty())
            {
                search_string = Undefined::Instance();
            }
            else
            {
                search_string = vals[0];
            }
            std::string search_str = ::es::ToString(e, search_string);
            if(!e->IsOk())
            {
                return nullptr;
            }
            double pos;
            if(vals.size() < 2 || vals[1]->IsUndefined())
            {
                pos = 0;
            }
            else
            {
                pos = ToInteger(e, vals[1]);
                if(!e->IsOk())
                {
                    return nullptr;
                }
            }
            int start = fmin(fmax(pos, 0), S.size());
            size_t find_pos = S.find(search_str, start);
            if(find_pos != std::string::npos)
            {
                return new Number(find_pos);
            }
            return new Number(-1);
        }

        static JSValue* lastIndexOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)this_arg;
            JSValue* val = RuntimeContext::TopValue();
            val->CheckObjectCoercible(e);
            if(!e->IsOk())
            {
                return nullptr;
            }
            std::string S = ::es::ToString(e, val);
            if(!e->IsOk())
            {
                return nullptr;
            }
            JSValue* search_string;
            if(vals.empty())
            {
                search_string = Undefined::Instance();
            }
            else
            {
                search_string = vals[0];
            }
            std::string search_str = ::es::ToString(e, search_string);
            if(!e->IsOk())
            {
                return nullptr;
            }
            double pos;
            if(vals.size() < 2 || vals[1]->IsUndefined())
            {
                pos = nan("");
            }
            else
            {
                pos = ToNumber(e, vals[1]);
                if(!e->IsOk())
                {
                    return nullptr;
                }
            }
            int start;
            if(isnan(pos))
            {
                start = S.size();
            }
            else
            {
                start = fmin(fmax(pos, 0), S.size());
            }
            size_t find_pos = S.rfind(search_str, start);
            if(find_pos != std::string::npos)
            {
                return new Number(find_pos);
            }
            return new Number(-1);
        }

        static JSValue* localeCompare(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* match(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* replace(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* search(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* slice(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* split(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* substring(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)this_arg;
            if(vals.empty())
            {
                return Number::NaN();
            }
            JSValue* val = RuntimeContext::TopValue();
            val->CheckObjectCoercible(e);
            if(!e->IsOk())
            {
                return nullptr;
            }
            std::string S = ::es::ToString(e, val);
            int len = S.size();
            if(!e->IsOk())
            {
                return nullptr;
            }
            int int_start = ToInteger(e, vals[0]);
            if(!e->IsOk())
            {
                return nullptr;
            }
            int int_end;
            if(vals.size() < 2 || vals[0]->IsUndefined())
            {
                int_end = S.size();
            }
            else
            {
                int_end = ToInteger(e, vals[1]);
                if(!e->IsOk())
                {
                    return nullptr;
                }
            }
            int final_start = fmin(fmax(int_start, 0), len);
            int final_end = fmin(fmax(int_end, 0), len);
            int from = fmin(final_start, final_end);
            int to = fmax(final_start, final_end);
            return new String(S.substr(from, to - from));
        }

        static JSValue* toLowerCase(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* toLocaleLowerCase(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* toUpperCase(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* toLocaleUpperCase(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* trim(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

    private:
        StringProto() : JSObject(OBJ_OTHER, "String", true, String::Empty(), false, false)
        {
        }
    };

    class StringObject : public JSObject
    {
    public:
        StringObject(JSValue* primitive_value)
        : JSObject(OBJ_STRING,
                   "String",
                   true,// extensible
                   primitive_value,
                   false,
                   false)
        {
            SetPrototype(StringProto::Instance());
            assert(primitive_value->IsString());
            double length = static_cast<String*>(primitive_value)->data().size();
            AddValueProperty("length", new Number(length), false, false, false);
        }

        JSValue* GetOwnProperty(const std::string& P) override
        {
            JSValue* val = JSObject::GetOwnProperty(P);
            if(!val->IsUndefined())
            {
                return val;
            }
            Error* e = Error::Ok();
            int index = ToInteger(e, new String(P));// this will never has error.
            if(NumberToString(fabs(index)) != P)
            {
                return Undefined::Instance();
            }
            std::string str = static_cast<String*>(PrimitiveValue())->data();
            int len = str.size();
            if(len <= index)
            {
                return Undefined::Instance();
            }
            PropertyDescriptor* desc = new PropertyDescriptor();
            desc->SetDataDescriptor(new String(str.substr(index, 1)), true, false, false);
            return desc;
        }
    };

    class StringConstructor : public JSObject
    {
    public:
        static StringConstructor* Instance()
        {
            static StringConstructor singleton;
            return &singleton;
        }

        // 15.5.1.1 String ( [ value ] )
        JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {}) override
        {
            (void)this_arg;
            if(arguments.empty())
            {
                return String::Empty();
            }
            return new String(::es::ToString(e, arguments[0]));
        }

        // 15.5.2.1 new String ( [ value ] )
        JSObject* Construct(Error* e, const std::vector<JSValue*>& arguments) override
        {
            if(arguments.empty())
            {
                return new StringObject(String::Empty());
            }
            std::string str = ::es::ToString(e, arguments[0]);
            if(!e->IsOk())
            {
                return nullptr;
            }
            return new StringObject(new String(str));
        }

        static JSValue* fromCharCode(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)this_arg;
            std::string result;
            for(JSValue* val : vals)
            {
                char c = ToUint16(e, val);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                result.push_back(c);
            }
            return new String(result);
        }

        static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            return new String("function String() { [native code] }");
        }

    private:
        StringConstructor() : JSObject(OBJ_OTHER, "String", true, nullptr, true, true)
        {
        }
    };

    double ToUint32(Error* e, JSValue* input);

    double StringToNumber(const std::string& source);

    inline bool IsArrayIndex(const std::string& P)
    {
        return P == NumberToString(uint32_t(StringToNumber(P)));
    }

    class ArrayProto : public JSObject
    {
    public:
        static ArrayProto* Instance()
        {
            static ArrayProto singleton;
            return &singleton;
        }

        // 15.4.4.2 Array.prototype.toString ( )
        static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)this_arg;
            JSObject* array = ToObject(e, RuntimeContext::TopValue());
            JSValue* func = array->Get(e, "join");
            if(!e->IsOk())
            {
                return nullptr;
            }
            if(!func->IsCallable())
            {
                func = ObjectProto::Instance()->Get(e, "toString");
                if(!e->IsOk())
                {
                    return nullptr;
                }
            }
            return static_cast<JSObject*>(func)->Call(e, this_arg, vals);
        }

        static JSValue* toLocaleString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* concat(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* join(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)this_arg;
            JSObject* O = ToObject(e, RuntimeContext::TopValue());
            Number* len_val = static_cast<Number*>(O->Get(e, "length"));
            size_t len = len_val->data();

            std::string sep = ",";
            if(!vals.empty() && !vals[0]->IsUndefined())
            {
                sep = ::es::ToString(e, vals[0]);
                if(!e->IsOk())
                {
                    return nullptr;
                }
            }
            if(len == 0)
            {
                return String::Empty();
            }
            JSValue* element0 = O->Get(e, "0");
            if(!e->IsOk())
            {
                return nullptr;
            }
            std::string R;
            if(!element0->IsUndefined() && !element0->IsNull())
            {
                R = ::es::ToString(e, element0);
            }
            for(double k = 1; k < len; k++)
            {
                JSValue* element = O->Get(e, NumberToString(k));
                if(!e->IsOk())
                {
                    return nullptr;
                }
                std::string next;
                if(!element->IsUndefined() && !element->IsNull())
                {
                    next = ::es::ToString(e, element);
                }
                R += sep + next;
            }
            return new String(R);
        }

        // 15.4.4.6 Array.prototype.pop ( )
        static JSValue* pop(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)this_arg;
            (void)vals;
            JSObject* O = ToObject(e, RuntimeContext::TopValue());
            if(!e->IsOk())
            {
                return nullptr;
            }
            size_t len = ToNumber(e, O->Get(e, "length"));
            if(!e->IsOk())
            {
                return nullptr;
            }
            if(len == 0)
            {
                O->Put(e, "length", Number::Zero(), true);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                return Undefined::Instance();
            }
            else
            {
                assert(len > 0);
                std::string indx = NumberToString(len - 1);
                JSValue* element = O->Get(e, indx);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                O->Delete(e, indx, true);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                O->Put(e, "length", new Number(len - 1), true);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                return element;
            }
        }

        // 15.4.4.7 Array.prototype.push ( [ item1 [ , item2 [ , … ] ] ] )
        static JSValue* push(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)this_arg;
            JSObject* O = ToObject(e, RuntimeContext::TopValue());
            if(!e->IsOk())
            {
                return nullptr;
            }
            double n = ToNumber(e, O->Get(e, "length"));
            if(!e->IsOk())
            {
                return nullptr;
            }
            for(JSValue* E : vals)
            {
                O->Put(e, NumberToString(n), E, true);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                n++;
            }
            Number* num = new Number(n);
            O->Put(e, "length", num, true);
            if(!e->IsOk())
            {
                return nullptr;
            }
            return num;
        }

        static JSValue* reverse(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* shift(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* slice(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* sort(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* splice(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* unshift(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* indexOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* lastIndexOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* every(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* some(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* forEach(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals);

        static JSValue* toLocaleUpperCase(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* map(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals);

        static JSValue* filter(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals);

        static JSValue* reduce(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

        static JSValue* reduceRight(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            assert(false);
        }

    private:
        ArrayProto() : JSObject(OBJ_ARRAY, "Array", true, nullptr, false, false)
        {
        }
    };

    class ArrayObject : public JSObject
    {
    public:
        ArrayObject(double len) : JSObject(OBJ_ARRAY, "Array", true, nullptr, false, false)
        {
            SetPrototype(ArrayProto::Instance());
            // Not using AddValueProperty here to by pass the override DefineOwnProperty
            PropertyDescriptor* desc = new PropertyDescriptor();
            desc->SetDataDescriptor(new Number(len), true, false, false);
            JSObject::DefineOwnProperty(nullptr, "length", desc, false);
        }

        bool DefineOwnProperty(Error* e, const std::string& P, PropertyDescriptor* desc, bool throw_flag) override
        {
            auto old_len_desc = static_cast<PropertyDescriptor*>(GetOwnProperty("length"));
            assert(!old_len_desc->IsUndefined());
            double old_len = ToNumber(e, old_len_desc->Value());
            if(P == "length")
            {// 3
                if(!desc->HasValue())
                {// 3.a
                    return JSObject::DefineOwnProperty(e, "length", desc, throw_flag);
                }
                PropertyDescriptor* new_len_desc = new PropertyDescriptor();
                new_len_desc->Set(desc);
                double new_len = ToUint32(e, desc->Value());
                if(!e->IsOk())
                {
                    goto reject;
                }
                double new_num = ToNumber(e, desc->Value());
                if(!e->IsOk())
                {
                    goto reject;
                }
                if(new_len != new_num)
                {
                    *e = *Error::RangeError("length of array need to be uint32.");
                    return false;
                }
                new_len_desc->SetValue(new Number(new_len));
                if(new_len >= old_len)
                {// 3.f
                    return JSObject::DefineOwnProperty(e, "length", new_len_desc, throw_flag);
                }
                if(!old_len_desc->Writable())
                {// 3.g
                    goto reject;
                }
                bool new_writable;
                if(new_len_desc->HasWritable() && new_len_desc->Writable())
                {// 3.h
                    new_writable = true;
                }
                else
                {// 3.l
                    new_writable = false;
                    new_len_desc->SetWritable(true);
                }
                bool succeeded = JSObject::DefineOwnProperty(e, "length", new_len_desc, throw_flag);
                if(!succeeded)
                {
                    return false;// 3.k
                }
                while(new_len < old_len)
                {// 3.l
                    old_len--;
                    bool delete_succeeded = Delete(e, ::es::ToString(e, new Number(old_len)), false);
                    if(!delete_succeeded)
                    {// 3.l.iii
                        new_len_desc->SetValue(new Number(old_len + 1));
                        if(!new_writable)
                        {// 3.l.iii.2
                            new_len_desc->SetWritable(false);
                        }
                        JSObject::DefineOwnProperty(e, "length", new_len_desc, false);
                        goto reject;// 3.l.iii.4
                    }
                }
                if(!new_writable)
                {// 3.m
                    auto tmp = new PropertyDescriptor();
                    tmp->SetWritable(false);
                    assert(JSObject::DefineOwnProperty(e, "length", new_len_desc, false));
                    return true;
                }
                return true;// 3.n
            }
            else
            {
                if(IsArrayIndex(P))
                {// 4
                    double index = StringToNumber(P);
                    if(index >= old_len && !old_len_desc->Writable())
                    {// 4.b
                        goto reject;
                    }
                    bool succeeded = JSObject::DefineOwnProperty(e, P, desc, false);
                    if(!succeeded)
                    {
                        goto reject;
                    }
                    if(index >= old_len)
                    {// 4.e
                        old_len_desc->SetValue(new Number(index + 1));
                        return JSObject::DefineOwnProperty(e, "length", old_len_desc, false);
                    }
                    return true;
                }
            }
            return JSObject::DefineOwnProperty(e, P, desc, throw_flag);
        reject:
            log::PrintSource("Array::DefineOwnProperty reject ", P, " " + desc->ToString());
            if(throw_flag)
            {
                *e = *Error::TypeError();
            }
            return false;
        }

        std::string ToString() override
        {
            size_t num = ToNumber(nullptr, Get(nullptr, "length"));
            return "Array(" + std::to_string(num) + ")";
        }
    };

    class ArrayConstructor : public JSObject
    {
    public:
        static ArrayConstructor* Instance()
        {
            static ArrayConstructor singleton;
            return &singleton;
        }

        // 15.5.1.1 Array ( [ value ] )
        JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {}) override
        {
            (void)this_arg;
            return Construct(e, arguments);
        }

        // 15.5.2.1 new Array ( [ value ] )
        JSObject* Construct(Error* e, const std::vector<JSValue*>& arguments) override
        {
            if(arguments.size() == 1 && arguments[0]->IsNumber())
            {
                Number* len = static_cast<Number*>(arguments[0]);
                if(len->data() == ToUint32(e, len))
                {
                    return new ArrayObject(len->data());
                }
                else
                {
                    *e = *Error::RangeError("Invalid array length");
                    return nullptr;
                }
            }
            ArrayObject* arr = new ArrayObject(arguments.size());
            for(size_t i = 0; i < arguments.size(); i++)
            {
                JSValue* arg = arguments[i];
                arr->AddValueProperty(::es::NumberToString(i), arg, true, true, true);
            }
            return arr;
        }

        static JSValue* isArray(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            if(vals.empty() || !vals[0]->IsObject())
            {
                return Bool::False();
            }
            JSObject* obj = static_cast<JSObject*>(vals[0]);
            return Bool::Wrap(obj->Class() == "Array");
        }

        static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            (void)vals;
            return new String("function Array() { [native code] }");
        }

    private:
        ArrayConstructor() : JSObject(OBJ_OTHER, "Array", true, nullptr, true, true)
        {
        }
    };

    // 15.4.4.18 Array.prototype.forEach ( callbackfn [ , thisArg ] )
    inline JSValue* ArrayProto::forEach(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
    {
        (void)this_arg;
        JSObject* O = ToObject(e, RuntimeContext::TopValue());
        if(!e->IsOk())
        {
            return nullptr;
        }
        size_t len = ToNumber(e, O->Get(e, "length"));
        if(vals.empty() || !vals[0]->IsCallable())
        {// 4
            *e = *Error::TypeError("Array.prototype.forEach called on non-callable");
            return nullptr;
        }
        JSObject* callbackfn = static_cast<JSObject*>(vals[0]);
        JSValue* T;
        if(vals.size() < 2)
        {
            T = Undefined::Instance();
        }
        else
        {
            T = vals[1];
        }
        ArrayObject* A = new ArrayObject(len);
        (void)A;
        for(size_t k = 0; k < len; k++)
        {
            std::string p_k = NumberToString(k);
            bool k_present = O->HasProperty(p_k);
            if(!e->IsOk())
            {
                return nullptr;
            }
            if(k_present)
            {
                JSValue* k_value = O->Get(e, p_k);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                JSValue* mapped_value = callbackfn->Call(e, T, { k_value, new Number(k), O });
                (void)mapped_value;
                if(!e->IsOk())
                {
                    return nullptr;
                }
            }
        }
        return Undefined::Instance();
    }

    // 15.4.4.19 Array.prototype.map ( callbackfn [ , thisArg ] )
    inline JSValue* ArrayProto::map(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
    {
        (void)this_arg;
        JSObject* O = ToObject(e, RuntimeContext::TopValue());
        if(!e->IsOk())
        {
            return nullptr;
        }
        size_t len = ToNumber(e, O->Get(e, "length"));
        if(vals.empty() || !vals[0]->IsCallable())
        {// 4
            *e = *Error::TypeError("Array.prototype.map called on non-callable");
            return nullptr;
        }
        JSObject* callbackfn = static_cast<JSObject*>(vals[0]);
        JSValue* T;
        if(vals.size() < 2)
        {
            T = Undefined::Instance();
        }
        else
        {
            T = vals[1];
        }
        ArrayObject* A = new ArrayObject(len);
        (void)A;
        for(size_t k = 0; k < len; k++)
        {
            std::string p_k = NumberToString(k);
            bool k_present = O->HasProperty(p_k);
            if(!e->IsOk())
            {
                return nullptr;
            }
            if(k_present)
            {
                JSValue* k_value = O->Get(e, p_k);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                JSValue* mapped_value = callbackfn->Call(e, T, { k_value, new Number(k), O });
                if(!e->IsOk())
                {
                    return nullptr;
                }
                A->AddValueProperty(p_k, mapped_value, true, true, true);
            }
        }
        return A;
    }

    // 15.4.4.20 Array.prototype.filter ( callbackfn [ , thisArg ] )
    inline JSValue* ArrayProto::filter(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
    {
        (void)this_arg;
        JSObject* O = ToObject(e, RuntimeContext::TopValue());
        if(!e->IsOk())
        {
            return nullptr;
        }
        size_t len = ToNumber(e, O->Get(e, "length"));
        if(vals.empty() || !vals[0]->IsCallable())
        {// 4
            *e = *Error::TypeError("Array.prototype.filter called on non-callable");
            return nullptr;
        }
        JSObject* callbackfn = static_cast<JSObject*>(vals[0]);
        JSValue* T;
        if(vals.size() < 2)
        {
            T = Undefined::Instance();
        }
        else
        {
            T = vals[1];
        }
        size_t to = 0;
        ArrayObject* A = new ArrayObject(len);
        for(size_t k = 0; k < len; k++)
        {
            std::string p_k = NumberToString(k);
            bool k_present = O->HasProperty(p_k);
            if(!e->IsOk())
            {
                return nullptr;
            }
            if(k_present)
            {
                JSValue* k_value = O->Get(e, p_k);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                JSValue* selected = callbackfn->Call(e, T, { k_value, new Number(k), O });
                if(!e->IsOk())
                {
                    return nullptr;
                }
                if(ToBoolean(selected))
                {
                    A->AddValueProperty(NumberToString(to), k_value, true, true, true);
                    to++;
                }
            }
        }
        return A;
    }

    inline JSValue* ObjectConstructor::keys(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
    {
        (void)this_arg;
        if(vals.empty() || !vals[0]->IsObject())
        {
            *e = *Error::TypeError("Object.keys called on non-object");
            return nullptr;
        }
        JSObject* O = static_cast<JSObject*>(vals[0]);
        auto properties = O->AllEnumerableProperties();
        size_t n = properties.size();
        ArrayObject* arr_obj = new ArrayObject(n);
        for(size_t index = 0; index < n; index++)
        {
            arr_obj->AddValueProperty(NumberToString(index), new String(properties[index].first), true, true, true);
        }
        return arr_obj;
    }

    // 10.6 Arguments Object
    class ArgumentsObject : public JSObject
    {
    public:
        ArgumentsObject(JSObject* parameter_map, size_t len)
        : JSObject(OBJ_OBJECT, "Arguments", true, nullptr, false, false), parameter_map_(parameter_map)
        {
            SetPrototype(ObjectProto::Instance());
            AddValueProperty("length", new Number(len), true, false, true);
        }

        JSObject* ParameterMap()
        {
            return parameter_map_;
        }

        JSValue* Get(Error* e, const std::string& P) override
        {
            JSObject* map = ParameterMap();
            JSValue* is_mapped = map->GetOwnProperty(P);
            if(is_mapped->IsUndefined())
            {// 3
                JSValue* v = JSObject::Get(e, P);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                if(P == "caller")
                {
                    if(v->IsObject())
                    {
                        JSObject* obj = static_cast<JSObject*>(v);
                        if(obj->IsFunction())
                        {
                            FunctionObject* func = static_cast<FunctionObject*>(obj);
                            if(func->strict())
                            {
                                *e = *Error::TypeError("caller could not be function object");
                            }
                        }
                    }
                }
                return v;
            }
            // 4
            return map->Get(e, P);
        }

        JSValue* GetOwnProperty(const std::string& P) override
        {
            JSValue* val = JSObject::GetOwnProperty(P);
            if(val->IsUndefined())
            {
                return val;
            }
            PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(val);
            JSObject* map = ParameterMap();
            JSValue* is_mapped = map->GetOwnProperty(P);
            if(!is_mapped->IsUndefined())
            {// 5
                desc->SetValue(map->Get(nullptr, P));
            }
            return desc;
        }

        bool DefineOwnProperty(Error* e, const std::string& P, PropertyDescriptor* desc, bool throw_flag) override
        {
            JSObject* map = ParameterMap();
            JSValue* is_mapped = map->GetOwnProperty(P);
            bool allowed = JSObject::DefineOwnProperty(e, P, desc, false);
            if(!allowed)
            {
                if(throw_flag)
                {
                    *e = *Error::TypeError("DefineOwnProperty " + P + " failed");
                }
                return false;
            }
            if(!is_mapped->IsUndefined())
            {// 5
                if(desc->IsAccessorDescriptor())
                {
                    map->Delete(e, P, false);
                }
                else
                {
                    if(desc->HasValue())
                    {
                        map->Put(e, P, desc->Value(), false);
                    }
                    if(desc->HasWritable() && !desc->Writable())
                    {
                        map->Delete(e, P, false);
                    }
                }
            }
            return true;
        }

        bool Delete(Error* e, const std::string& P, bool throw_flag) override
        {
            JSObject* map = ParameterMap();
            JSValue* is_mapped = map->GetOwnProperty(P);
            bool result = JSObject::Delete(e, P, throw_flag);
            if(!e->IsOk())
            {
                return false;
            }
            if(result && !is_mapped->IsUndefined())
            {
                map->Delete(e, P, false);
            }
            return result;
        }

        inline std::string ToString() override
        {
            return "ArgumentsObject";
        }

    private:
        JSObject* parameter_map_;
    };

    class Console : public JSObject
    {
    public:
        static Console* Instance()
        {
            static Console singleton;
            return &singleton;
        }

        static JSValue* log(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            (void)e;
            (void)this_arg;
            for(auto val : vals)
            {
                std::cout << val->ToString() << " ";
            }
            std::cout << std::endl;
            return Number::Zero();
        }

    private:
        Console() : JSObject(OBJ_BOOL, "Console", true, Bool::False(), false, false)
        {
            AddFuncProperty("log", log, false, false, false);
        }
    };

    enum CodeType
    {
        CODE_GLOBAL = 0,
        CODE_FUNC,
        CODE_EVAL,
    };

    inline JSValue* MakeArgGetter(const std::string& name, LexicalEnvironment* env)
    {
        Parser parser("return " + name + ";");
        ProgramOrFunctionBody* body = static_cast<ProgramOrFunctionBody*>(parser.ParseFunctionBody(Token::TK_EOS));
        return new FunctionObject({}, body, env);
    }

    inline JSValue* MakeArgSetter(const std::string& name, LexicalEnvironment* env)
    {
        std::string param = name + "_arg";
        Parser parser(name + " = " + param);
        ProgramOrFunctionBody* body = static_cast<ProgramOrFunctionBody*>(parser.ParseFunctionBody(Token::TK_EOS));
        return new FunctionObject({ param }, body, env);
    }

    // 10.6 Arguments Object
    inline JSObject* CreateArgumentsObject(FunctionObject* func, std::vector<JSValue*>& args, LexicalEnvironment* env, bool strict)
    {
        std::vector<std::string> names = func->FormalParameters();
        int len = args.size();
        Object* map = new Object();// 8
        JSObject* obj = new ArgumentsObject(map, len);
        int indx = len - 1;// 10
        std::set<std::string> mapped_names;
        while(indx >= 0)
        {// 11
            JSValue* val = args[indx];// 11.a
            obj->AddValueProperty(NumberToString(indx), val, true, true, true);// 11.b
            if(indx < int(names.size()))
            {// 11.c
                std::string name = names[indx];// 11.c.i
                if(!strict && mapped_names.find(name) == mapped_names.end())
                {// 11.c.ii
                    mapped_names.insert(name);
                    JSValue* g = MakeArgGetter(name, env);
                    JSValue* p = MakeArgSetter(name, env);
                    PropertyDescriptor* desc = new PropertyDescriptor();
                    desc->SetSet(p);
                    desc->SetGet(g);
                    desc->SetConfigurable(true);
                    map->DefineOwnProperty(nullptr, NumberToString(indx), desc, false);
                }
            }
            indx--;// 11.d
        }
        if(!strict)
        {// 13
            obj->AddValueProperty("callee", func, true, false, true);
        }
        else
        {// 14
            // TODO(zhuzilin) thrower
        }
        return obj;// 15
    }

    inline void FindAllVarDecl(const std::vector<AST*>& stmts, const std::vector<VarDecl*>& idecls)
    {
        auto decls = idecls;
        for(auto stmt : stmts)
        {
            switch(stmt->type())
            {
                case AST::AST_STMT_VAR:
                {
                    VarStmt* var_stmt = static_cast<VarStmt*>(stmt);
                    for(auto d : var_stmt->decls())
                    {
                        decls.emplace_back(d);
                    }
                    break;
                }
                case AST::AST_STMT_FOR:
                {
                    For* for_stmt = static_cast<For*>(stmt);
                    if(!for_stmt->expr0s().empty() && for_stmt->expr0s()[0]->type() == AST::AST_STMT_VAR_DECL)
                    {
                        for(AST* ast : for_stmt->expr0s())
                        {
                            VarDecl* d = static_cast<VarDecl*>(ast);
                            decls.emplace_back(d);
                        }
                    }
                    FindAllVarDecl({ for_stmt->statement() }, decls);
                    break;
                }
                case AST::AST_STMT_FOR_IN:
                {
                    ForIn* for_in_stmt = static_cast<ForIn*>(stmt);
                    if(for_in_stmt->expr0()->type() == AST::AST_STMT_VAR_DECL)
                    {
                        VarDecl* d = static_cast<VarDecl*>(for_in_stmt->expr0());
                        decls.emplace_back(d);
                    }
                    FindAllVarDecl({ for_in_stmt->statement() }, decls);
                    break;
                }
                case AST::AST_STMT_BLOCK:
                {
                    Block* block = static_cast<Block*>(stmt);
                    FindAllVarDecl(block->statements(), decls);
                    break;
                }
                case AST::AST_STMT_TRY:
                {
                    Try* try_stmt = static_cast<Try*>(stmt);
                    FindAllVarDecl({ try_stmt->try_block() }, decls);
                    if(try_stmt->catch_block() != nullptr)
                    {
                        FindAllVarDecl({ try_stmt->catch_block() }, decls);
                    }
                    if(try_stmt->finally_block() != nullptr)
                    {
                        FindAllVarDecl({ try_stmt->finally_block() }, decls);
                    }
                    break;
                }
                // TODO(zhuzilin) fill the other statements.
                default:
                    break;
            }
        }
    }

    // 10.5 Declaration Binding Instantiation
    inline void DeclarationBindingInstantiation(
    Error* e, ExecutionContext* context, AST* code, CodeType code_type, FunctionObject* f = nullptr, std::vector<JSValue*> args = {})
    {
        auto env = context->variable_env()->env_rec();// 1
        bool configurable_bindings = false;
        ProgramOrFunctionBody* body = static_cast<ProgramOrFunctionBody*>(code);
        if(code_type == CODE_EVAL)
        {
            configurable_bindings = true;// 2
        }
        bool strict = body->strict();// 3
        if(code_type == CODE_FUNC)
        {// 4
            assert(f != nullptr);
            auto names = f->FormalParameters();// 4.a
            Function* func_ast = static_cast<Function*>(code);
            (void)func_ast;
            size_t arg_count = args.size();// 4.b
            size_t n = 0;// 4.c
            for(const auto& arg_name : names)
            {// 4.d
                JSValue* v = Undefined::Instance();
                if(n < arg_count)
                {// 4.d.i & 4.d.ii
                    v = args[n++];
                }
                bool arg_already_declared = env->HasBinding(arg_name);// 4.d.iii
                if(!arg_already_declared)
                {// 4.d.iv
                    // NOTE(zhuzlin) I'm not sure if this should be false.
                    env->CreateMutableBinding(e, arg_name, false);
                    if(!e->IsOk())
                    {
                        return;
                    }
                }
                env->SetMutableBinding(e, arg_name, v, strict);// 4.d.v
                if(!e->IsOk())
                {
                    return;
                }
            }
        }
        // 5
        for(Function* func_decl : body->func_decls())
        {
            assert(func_decl->is_named());
            std::string fn = func_decl->name();
            FunctionObject* fo = InstantiateFunctionDeclaration(e, func_decl);
            if(!e->IsOk())
            {
                return;
            }
            bool func_already_declared = env->HasBinding(fn);
            if(!func_already_declared)
            {// 5.d
                env->CreateMutableBinding(e, fn, configurable_bindings);
                if(!e->IsOk())
                {
                    return;
                }
            }
            else
            {// 5.e
                auto go = GlobalObject::Instance();
                auto existing_prop = go->GetProperty(fn);
                assert(!existing_prop->IsUndefined());
                auto existing_prop_desc = static_cast<PropertyDescriptor*>(existing_prop);
                if(existing_prop_desc->Configurable())
                {// 5.e.iii
                    auto new_desc = new PropertyDescriptor();
                    new_desc->SetDataDescriptor(Undefined::Instance(), true, true, configurable_bindings);
                    go->DefineOwnProperty(e, fn, new_desc, true);
                    if(!e->IsOk())
                    {
                        return;
                    }
                }
                else
                {// 5.e.iv
                    if(existing_prop_desc->IsAccessorDescriptor()
                       || !(existing_prop_desc->HasConfigurable() && existing_prop_desc->Configurable()
                            && existing_prop_desc->HasEnumerable() && existing_prop_desc->Enumerable()))
                    {
                        *e = *Error::TypeError();
                        return;
                    }
                }
            }
            env->SetMutableBinding(e, fn, fo, strict);// 5.f
        }
        // 6
        bool arguments_already_declared = env->HasBinding("arguments");
        // 7
        if(code_type == CODE_FUNC && !arguments_already_declared)
        {
            auto args_obj = CreateArgumentsObject(f, args, context->variable_env(), strict);
            if(strict)
            {// 7.b
                DeclarativeEnvironmentRecord* decl_env = static_cast<DeclarativeEnvironmentRecord*>(env);
                decl_env->CreateImmutableBinding("arguments");
                decl_env->InitializeImmutableBinding("arguments", args_obj);
            }
            else
            {// 7.c
                // NOTE(zhuzlin) I'm not sure if this should be false.
                env->CreateMutableBinding(e, "arguments", false);
                env->SetMutableBinding(e, "arguments", args_obj, false);
            }
        }
        // 8
        std::vector<VarDecl*> decls;
        FindAllVarDecl(body->statements(), decls);
        for(VarDecl* d : decls)
        {
            std::string dn = d->ident();
            bool var_already_declared = env->HasBinding(dn);
            if(!var_already_declared)
            {
                env->CreateMutableBinding(e, dn, configurable_bindings);
                if(!e->IsOk())
                {
                    return;
                }
                env->SetMutableBinding(e, dn, Undefined::Instance(), strict);
                if(!e->IsOk())
                {
                    return;
                }
            }
        }
    }

    // 10.4.1
    inline void EnterGlobalCode(Error* e, AST* ast)
    {
        ProgramOrFunctionBody* program;
        if(ast->type() == AST::AST_PROGRAM)
        {
            program = static_cast<ProgramOrFunctionBody*>(ast);
        }
        else
        {
            // TODO(zhuzilin) This is for test. Add test label like #ifdefine TEST
            program = new ProgramOrFunctionBody(AST::AST_PROGRAM, false);
            program->AddStatement(ast);
        }
        // 1 10.4.1.1
        LexicalEnvironment* global_env = LexicalEnvironment::Global();
        ExecutionContext* context = new ExecutionContext(global_env, global_env, GlobalObject::Instance(), program->strict());
        RuntimeContext::Global()->AddContext(context);
        // 2
        DeclarationBindingInstantiation(e, context, program, CODE_GLOBAL);
    }

    // 10.4.2
    inline void EnterEvalCode(Error* e, AST* ast)
    {
        assert(ast->type() == AST::AST_PROGRAM);
        ProgramOrFunctionBody* program = static_cast<ProgramOrFunctionBody*>(ast);
        ExecutionContext* context;
        LexicalEnvironment* variable_env;
        LexicalEnvironment* lexical_env;
        JSValue* this_binding;
        if(!GlobalObject::Instance()->direct_eval())
        {// 1
            LexicalEnvironment* global_env = LexicalEnvironment::Global();
            variable_env = global_env;
            lexical_env = global_env;
            this_binding = GlobalObject::Instance();
        }
        else
        {// 2
            ExecutionContext* calling_context = RuntimeContext::TopContext();
            variable_env = calling_context->variable_env();
            lexical_env = calling_context->lexical_env();
            this_binding = calling_context->this_binding();
        }
        bool strict = RuntimeContext::TopContext()->strict() || (program->strict() && GlobalObject::Instance()->direct_eval());
        if(strict)
        {// 3
            LexicalEnvironment* strict_var_env = LexicalEnvironment::NewDeclarativeEnvironment(lexical_env);
            lexical_env = strict_var_env;
            variable_env = strict_var_env;
        }
        context = new ExecutionContext(variable_env, lexical_env, this_binding, strict);
        RuntimeContext::Global()->AddContext(context);
        // 4
        DeclarationBindingInstantiation(e, context, program, CODE_EVAL);
    }

    // 15.1.2.1 eval(X)
    inline JSValue* GlobalObject::eval(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
    {
        (void)this_arg;
        log::PrintSource("enter GlobalObject::eval");
        if(vals.empty())
        {
            return Undefined::Instance();
        }
        if(!vals[0]->IsString())
        {
            return vals[0];
        }
        std::string x = static_cast<String*>(vals[0])->data();
        Parser parser(x);
        AST* program = parser.ParseProgram();
        if(program->IsIllegal())
        {
            *e = *Error::SyntaxError("failed to parse eval");
            return nullptr;
        }
        EnterEvalCode(e, program);
        if(!e->IsOk())
        {
            return nullptr;
        }
        Completion result = EvalProgram(program);
        RuntimeContext::Global()->PopContext();

        switch(result.type)
        {
            case Completion::NORMAL:
                if(result.value != nullptr)
                {
                    return result.value;
                }
                else
                {
                    return Undefined::Instance();
                }
            default:
            {
                assert(result.type == Completion::THROWING);
                std::string message = ::es::ToString(e, result.value);
                if(result.value->IsObject())
                {
                    JSObject* obj = static_cast<JSObject*>(result.value);
                    if(obj->obj_type() == JSObject::OBJ_ERROR)
                    {
                        message = static_cast<ErrorObject*>(obj)->ErrorMessage();
                    }
                }
                *e = *Error::NativeError(message);
                return result.value;
            }
        }
    }

    // 10.4.3
    inline void
    EnterFunctionCode(Error* e, JSObject* f, ProgramOrFunctionBody* body, JSValue* this_arg, const std::vector<JSValue*>& args, bool strict)
    {
        assert(f->obj_type() == JSObject::OBJ_FUNC);
        FunctionObject* func = static_cast<FunctionObject*>(f);
        JSValue* this_binding;
        if(strict)
        {// 1
            this_binding = this_arg;
        }
        else
        {// 2 & 3
            this_binding = (this_arg->IsUndefined() || this_arg->IsNull()) ? GlobalObject::Instance() : this_arg;
        }
        LexicalEnvironment* local_env = LexicalEnvironment::NewDeclarativeEnvironment(func->Scope());
        ExecutionContext* context = new ExecutionContext(local_env, local_env, this_binding, strict);// 8
        RuntimeContext::Global()->AddContext(context);
        // 9
        DeclarationBindingInstantiation(e, context, body, CODE_FUNC, func, args);
    }

    inline void InitGlobalObject()
    {
        auto global_obj = GlobalObject::Instance();
        // 15.1.1 Value Properties of the Global Object
        global_obj->AddValueProperty("NaN", Number::NaN(), false, false, false);
        global_obj->AddValueProperty("Infinity", Number::PositiveInfinity(), false, false, false);
        global_obj->AddValueProperty("undefined", Undefined::Instance(), false, false, false);
        // 15.1.2 Function Properties of the Global Object
        global_obj->AddFuncProperty("eval", GlobalObject::eval, true, false, true);
        global_obj->AddFuncProperty("parseInt", GlobalObject::parseInt, true, false, true);
        global_obj->AddFuncProperty("parseFloat", GlobalObject::parseFloat, true, false, true);
        global_obj->AddFuncProperty("isNaN", GlobalObject::isNaN, true, false, true);
        global_obj->AddFuncProperty("isFinite", GlobalObject::isFinite, true, false, true);
        // 15.1.3 URI Handling Function Properties
        // TODO(zhuzilin)
        // 15.1.4 Constructor Properties of the Global Object
        global_obj->AddValueProperty("Object", ObjectConstructor::Instance(), true, false, true);
        global_obj->AddValueProperty("Function", FunctionConstructor::Instance(), true, false, true);
        global_obj->AddValueProperty("Number", NumberConstructor::Instance(), true, false, true);
        global_obj->AddValueProperty("Boolean", BoolConstructor::Instance(), true, false, true);
        global_obj->AddValueProperty("String", StringConstructor::Instance(), true, false, true);
        global_obj->AddValueProperty("Array", ArrayConstructor::Instance(), true, false, true);

        global_obj->AddValueProperty("Error", ErrorConstructor::Instance(), true, false, true);
        // TODO(zhuzilin) differentiate errors.
        global_obj->AddValueProperty("EvalError", ErrorConstructor::Instance(), true, false, true);
        global_obj->AddValueProperty("RangeError", ErrorConstructor::Instance(), true, false, true);
        global_obj->AddValueProperty("ReferenceError", ErrorConstructor::Instance(), true, false, true);
        global_obj->AddValueProperty("SyntaxError", ErrorConstructor::Instance(), true, false, true);
        global_obj->AddValueProperty("TypeError", ErrorConstructor::Instance(), true, false, true);
        global_obj->AddValueProperty("URIError", ErrorConstructor::Instance(), true, false, true);

        global_obj->AddValueProperty("console", Console::Instance(), true, false, true);
    }

    inline void InitObject()
    {
        ObjectConstructor* constructor = ObjectConstructor::Instance();
        constructor->SetPrototype(FunctionProto::Instance());
        // 15.2.3 Properties of the Object Constructor
        constructor->AddValueProperty("prototype", ObjectProto::Instance(), false, false, false);
        constructor->AddFuncProperty("toString", ObjectConstructor::toString, false, false, false);
        // TODO(zhuzilin) check if the config is correct.
        constructor->AddFuncProperty("getPrototypeOf", ObjectConstructor::getPrototypeOf, false, false, false);
        constructor->AddFuncProperty("getOwnPropertyDescriptor", ObjectConstructor::getOwnPropertyDescriptor, false, false, false);
        constructor->AddFuncProperty("getOwnPropertyNames", ObjectConstructor::getOwnPropertyNames, false, false, false);
        constructor->AddFuncProperty("create", ObjectConstructor::create, false, false, false);
        constructor->AddFuncProperty("defineProperty", ObjectConstructor::defineProperty, false, false, false);
        constructor->AddFuncProperty("defineProperties", ObjectConstructor::defineProperties, false, false, false);
        constructor->AddFuncProperty("seal", ObjectConstructor::seal, false, false, false);
        constructor->AddFuncProperty("freeze", ObjectConstructor::freeze, false, false, false);
        constructor->AddFuncProperty("preventExtensions", ObjectConstructor::preventExtensions, false, false, false);
        constructor->AddFuncProperty("isSealed", ObjectConstructor::isSealed, false, false, false);
        constructor->AddFuncProperty("isFrozen", ObjectConstructor::isFrozen, false, false, false);
        constructor->AddFuncProperty("isExtensible", ObjectConstructor::isExtensible, false, false, false);
        constructor->AddFuncProperty("keys", ObjectConstructor::keys, false, false, false);
        // ES6
        constructor->AddFuncProperty("setPrototypeOf", ObjectConstructor::setPrototypeOf, false, false, false);

        ObjectProto* proto = ObjectProto::Instance();
        // 15.2.4 Properties of the Object Prototype Object
        proto->AddValueProperty("constructor", ObjectConstructor::Instance(), false, false, false);
        proto->AddFuncProperty("toString", ObjectProto::toString, false, false, false);
        proto->AddFuncProperty("toLocaleString", ObjectProto::toLocaleString, false, false, false);
        proto->AddFuncProperty("valueOf", ObjectProto::valueOf, false, false, false);
        proto->AddFuncProperty("hasOwnProperty", ObjectProto::hasOwnProperty, false, false, false);
        proto->AddFuncProperty("isPrototypeOf", ObjectProto::isPrototypeOf, false, false, false);
        proto->AddFuncProperty("propertyIsEnumerable", ObjectProto::propertyIsEnumerable, false, false, false);
    }

    inline void InitFunction()
    {
        FunctionConstructor* constructor = FunctionConstructor::Instance();
        constructor->SetPrototype(FunctionProto::Instance());
        // 15.3.3 Properties of the Function Constructor
        constructor->AddValueProperty("prototype", FunctionProto::Instance(), false, false, false);
        constructor->AddValueProperty("length", Number::One(), false, false, false);
        constructor->AddFuncProperty("toString", FunctionConstructor::toString, false, false, false);

        FunctionProto* proto = FunctionProto::Instance();
        proto->SetPrototype(ObjectProto::Instance());
        // 15.2.4 Properties of the Function Prototype Function
        proto->AddValueProperty("constructor", FunctionConstructor::Instance(), false, false, false);
        proto->AddFuncProperty("toString", FunctionProto::toString, false, false, false);
        proto->AddFuncProperty("apply", FunctionProto::apply, false, false, false);
        proto->AddFuncProperty("call", FunctionProto::call, false, false, false);
        proto->AddFuncProperty("bind", FunctionProto::bind, false, false, false);
    }

    inline void InitNumber()
    {
        NumberConstructor* constructor = NumberConstructor::Instance();
        constructor->SetPrototype(FunctionProto::Instance());
        // 15.3.3 Properties of the Number Constructor
        constructor->AddValueProperty("prototype", NumberProto::Instance(), false, false, false);
        constructor->AddValueProperty("length", Number::One(), false, false, false);
        constructor->AddValueProperty("MAX_VALUE", new Number(1.7976931348623157e308), false, false, false);
        constructor->AddValueProperty("MIN_VALUE", new Number(5e-324), false, false, false);
        constructor->AddValueProperty("NaN", Number::NaN(), false, false, false);
        constructor->AddValueProperty("NEGATIVE_INFINITY", Number::PositiveInfinity(), false, false, false);
        constructor->AddValueProperty("POSITIVE_INFINITY", Number::NegativeInfinity(), false, false, false);

        NumberProto* proto = NumberProto::Instance();
        proto->SetPrototype(ObjectProto::Instance());
        // 15.2.4 Properties of the Number Prototype Number
        proto->AddValueProperty("constructor", NumberConstructor::Instance(), false, false, false);
        proto->AddFuncProperty("toString", NumberProto::toString, false, false, false);
        proto->AddFuncProperty("toLocaleString", NumberProto::toLocaleString, false, false, false);
        proto->AddFuncProperty("valueOf", NumberProto::valueOf, false, false, false);
        proto->AddFuncProperty("toFixed", NumberProto::toFixed, false, false, false);
        proto->AddFuncProperty("toExponential", NumberProto::toExponential, false, false, false);
        proto->AddFuncProperty("toPrecision", NumberProto::toPrecision, false, false, false);
    }

    inline void InitError()
    {
        ErrorConstructor* constructor = ErrorConstructor::Instance();
        constructor->SetPrototype(FunctionProto::Instance());
        // 15.11.3 Properties of the Error Constructor
        constructor->AddValueProperty("prototype", ErrorProto::Instance(), false, false, false);
        constructor->AddValueProperty("length", Number::One(), false, false, false);
        constructor->AddFuncProperty("toString", NumberConstructor::toString, false, false, false);

        ErrorProto* proto = ErrorProto::Instance();
        proto->SetPrototype(ObjectProto::Instance());
        // 15.11.4 Properties of the Error Prototype Object
        proto->AddValueProperty("constructor", ErrorConstructor::Instance(), false, false, false);
        proto->AddValueProperty("name", new String("Error"), false, false, false);
        proto->AddValueProperty("message", String::Empty(), true, false, false);
        proto->AddFuncProperty("call", ErrorProto::toString, false, false, false);
    }

    inline void InitBool()
    {
        BoolConstructor* constructor = BoolConstructor::Instance();
        constructor->SetPrototype(FunctionProto::Instance());
        // 15.6.3 Properties of the Boolean Constructor
        constructor->AddValueProperty("prototype", BoolProto::Instance(), false, false, false);
        constructor->AddFuncProperty("toString", BoolConstructor::toString, false, false, false);

        BoolProto* proto = BoolProto::Instance();
        proto->SetPrototype(ObjectProto::Instance());
        // 15.6.4 Properties of the Boolean Prototype Object
        proto->AddValueProperty("constructor", BoolConstructor::Instance(), false, false, false);
        proto->AddFuncProperty("toString", BoolProto::toString, false, false, false);
        proto->AddFuncProperty("valueOf", BoolProto::valueOf, false, false, false);
    }

    inline void InitString()
    {
        StringConstructor* constructor = StringConstructor::Instance();
        constructor->SetPrototype(FunctionProto::Instance());
        // 15.3.3 Properties of the String Constructor
        constructor->AddValueProperty("prototype", StringProto::Instance(), false, false, false);
        constructor->AddValueProperty("length", Number::One(), true, false, false);
        constructor->AddFuncProperty("fromCharCode", StringConstructor::fromCharCode, false, false, false);
        constructor->AddFuncProperty("toString", StringConstructor::toString, false, false, false);

        StringProto* proto = StringProto::Instance();
        proto->SetPrototype(ObjectProto::Instance());
        // 15.2.4 Properties of the String Prototype String
        proto->AddValueProperty("constructor", StringConstructor::Instance(), false, false, false);
        proto->AddFuncProperty("toString", StringProto::toString, false, false, false);
        proto->AddFuncProperty("valueOf", StringProto::valueOf, false, false, false);
        proto->AddFuncProperty("charAt", StringProto::charAt, false, false, false);
        proto->AddFuncProperty("charCodeAt", StringProto::charCodeAt, false, false, false);
        proto->AddFuncProperty("concat", StringProto::concat, false, false, false);
        proto->AddFuncProperty("indexOf", StringProto::indexOf, false, false, false);
        proto->AddFuncProperty("lastIndexOf", StringProto::lastIndexOf, false, false, false);
        proto->AddFuncProperty("localeCompare", StringProto::localeCompare, false, false, false);
        proto->AddFuncProperty("match", StringProto::match, false, false, false);
        proto->AddFuncProperty("replace", StringProto::replace, false, false, false);
        proto->AddFuncProperty("search", StringProto::search, false, false, false);
        proto->AddFuncProperty("slice", StringProto::slice, false, false, false);
        proto->AddFuncProperty("split", StringProto::split, false, false, false);
        proto->AddFuncProperty("substring", StringProto::substring, false, false, false);
        proto->AddFuncProperty("toLowerCase", StringProto::toLowerCase, false, false, false);
        proto->AddFuncProperty("toLocaleLowerCase", StringProto::toLocaleLowerCase, false, false, false);
        proto->AddFuncProperty("toUpperCase", StringProto::toUpperCase, false, false, false);
        proto->AddFuncProperty("toLocaleUpperCase", StringProto::toLocaleUpperCase, false, false, false);
        proto->AddFuncProperty("trim", StringProto::trim, false, false, false);
    }

    inline void InitArray()
    {
        ArrayConstructor* constructor = ArrayConstructor::Instance();
        constructor->SetPrototype(FunctionProto::Instance());
        // 15.6.3 Properties of the Arrayean Constructor
        constructor->AddValueProperty("length", Number::One(), false, false, false);
        constructor->AddValueProperty("prototype", ArrayProto::Instance(), false, false, false);
        constructor->AddFuncProperty("isArray", ArrayConstructor::isArray, false, false, false);
        constructor->AddFuncProperty("toString", ArrayConstructor::toString, false, false, false);

        ArrayProto* proto = ArrayProto::Instance();
        proto->SetPrototype(ObjectProto::Instance());
        // 15.6.4 Properties of the Arrayean Prototype Object
        proto->AddValueProperty("length", Number::Zero(), false, false, false);
        proto->AddValueProperty("constructor", ArrayConstructor::Instance(), false, false, false);
        proto->AddFuncProperty("toString", ArrayProto::toString, false, false, false);
        proto->AddFuncProperty("toLocaleString", ArrayProto::toLocaleString, false, false, false);
        proto->AddFuncProperty("concat", ArrayProto::concat, false, false, false);
        proto->AddFuncProperty("join", ArrayProto::join, false, false, false);
        proto->AddFuncProperty("pop", ArrayProto::pop, false, false, false);
        proto->AddFuncProperty("push", ArrayProto::push, false, false, false);
        proto->AddFuncProperty("reverse", ArrayProto::reverse, false, false, false);
        proto->AddFuncProperty("shift", ArrayProto::shift, false, false, false);
        proto->AddFuncProperty("slice", ArrayProto::slice, false, false, false);
        proto->AddFuncProperty("sort", ArrayProto::sort, false, false, false);
        proto->AddFuncProperty("splice", ArrayProto::splice, false, false, false);
        proto->AddFuncProperty("unshift", ArrayProto::unshift, false, false, false);
        proto->AddFuncProperty("indexOf", ArrayProto::indexOf, false, false, false);
        proto->AddFuncProperty("lastIndexOf", ArrayProto::lastIndexOf, false, false, false);
        proto->AddFuncProperty("every", ArrayProto::every, false, false, false);
        proto->AddFuncProperty("some", ArrayProto::some, false, false, false);
        proto->AddFuncProperty("forEach", ArrayProto::forEach, false, false, false);
        proto->AddFuncProperty("map", ArrayProto::map, false, false, false);
        proto->AddFuncProperty("filter", ArrayProto::filter, false, false, false);
        proto->AddFuncProperty("reduce", ArrayProto::reduce, false, false, false);
        proto->AddFuncProperty("reduceRight", ArrayProto::reduceRight, false, false, false);
    }

    inline void Init()
    {
        InitGlobalObject();
        InitObject();
        InitFunction();
        InitNumber();
        InitError();
        InitBool();
        InitString();
        InitArray();
    }

    JSValue* ToPrimitive(Error* e, JSValue* input, const std::string& preferred_type);
    bool ToBoolean(JSValue* input);
    double StringToNumber(const std::string& source);
    double StringToNumber(String* str);
    double ToNumber(Error* e, JSValue* input);
    double ToInteger(Error* e, JSValue* input);
    double ToInt32(Error* e, JSValue* input);
    double ToUint(Error* e, JSValue* input, char bits);
    double ToUint32(Error* e, JSValue* input);
    double ToUint16(Error* e, JSValue* input);
    std::string NumberToString(double m);
    std::string NumberToString(Number* num);
    std::string ToString(Error* e, JSValue* input);

    JSObject* ToObject(Error* e, JSValue* input);

    // 11.8.5 The Abstract Relational Comparison Algorithm
    // x < y
    JSValue* LessThan(Error* e, JSValue* x, JSValue* y, bool left_first = true);
    // 11.9.3 The Abstract Equality Comparison Algorithm
    // x == y
    bool Equal(Error* e, JSValue* x, JSValue* y);
    // 11.9.6 The Strict Equality Comparison Algorithm
    // x === y
    bool StrictEqual(Error* e, JSValue* x, JSValue* y);

    Completion EvalStatement(AST* ast);
    Completion EvalStatementList(const std::vector<AST*>& statements);
    Completion EvalBlockStatement(AST* ast);
    std::string EvalVarDecl(Error* e, AST* ast);
    Completion EvalVarStatement(AST* ast);
    Completion EvalIfStatement(AST* ast);
    Completion EvalForStatement(AST* ast);
    Completion EvalForInStatement(AST* ast);
    Completion EvalDoWhileStatement(AST* ast);
    Completion EvalWhileStatement(AST* ast);
    Completion EvalContinueStatement(AST* ast);
    Completion EvalBreakStatement(AST* ast);
    Completion EvalReturnStatement(AST* ast);
    Completion EvalLabelledStatement(AST* ast);
    Completion EvalWithStatement(AST* ast);
    Completion EvalSwitchStatement(AST* ast);
    Completion EvalThrowStatement(AST* ast);
    Completion EvalTryStatement(AST* ast);
    Completion EvalExpressionStatement(AST* ast);

    JSValue* EvalExpression(Error* e, AST* ast);
    JSValue* EvalPrimaryExpression(Error* e, AST* ast);
    Reference* EvalIdentifier(AST* ast);
    Number* EvalNumber(AST* ast);
    String* EvalString(AST* ast);
    Object* EvalObject(Error* e, AST* ast);
    ArrayObject* EvalArray(Error* e, AST* ast);
    JSValue* EvalUnaryOperator(Error* e, AST* ast);
    JSValue* EvalBinaryExpression(Error* e, AST* ast);
    JSValue* EvalBinaryExpression(Error* e, const std::string& op, AST* lhs, AST* rhs);
    JSValue* EvalBinaryExpression(Error* e, const std::string& op, JSValue* lval, JSValue* rval);
    JSValue* EvalArithmeticOperator(Error* e, const std::string& op, JSValue* lval, JSValue* rval);
    JSValue* EvalAddOperator(Error* e, JSValue* lval, JSValue* rval);
    JSValue* EvalBitwiseShiftOperator(Error* e, const std::string& op, JSValue* lval, JSValue* rval);
    JSValue* EvalRelationalOperator(Error* e, const std::string& op, JSValue* lval, JSValue* rval);
    JSValue* EvalEqualityOperator(Error* e, const std::string& op, JSValue* lval, JSValue* rval);
    JSValue* EvalBitwiseOperator(Error* e, const std::string& op, JSValue* lval, JSValue* rval);
    JSValue* EvalLogicalOperator(Error* e, const std::string& op, AST* lhs, AST* rhs);
    JSValue* EvalSimpleAssignment(Error* e, JSValue* lref, JSValue* rval);
    JSValue* EvalCompoundAssignment(Error* e, const std::string& op, JSValue* lref, JSValue* rval);
    JSValue* EvalTripleConditionExpression(Error* e, AST* ast);
    JSValue* EvalAssignmentExpression(Error* e, AST* ast);
    JSValue* EvalLeftHandSideExpression(Error* e, AST* ast);
    std::vector<JSValue*> EvalArgumentsList(Error* e, Arguments* ast);
    JSValue* EvalCallExpression(Error* e, JSValue* ref, const std::vector<JSValue*>& arg_list);
    JSValue* EvalIndexExpression(Error* e, JSValue* base_ref, const std::string& identifier_name, ValueGuard& guard);
    JSValue* EvalIndexExpression(Error* e, JSValue* base_ref, AST* expr, ValueGuard& guard);
    JSValue* EvalExpressionList(Error* e, AST* ast);

    Reference* IdentifierResolution(const std::string& name);

    inline Completion EvalProgram(AST* ast)
    {
        Completion head_result;
        assert(ast->type() == AST::AST_PROGRAM || ast->type() == AST::AST_FUNC_BODY);
        auto prog = static_cast<ProgramOrFunctionBody*>(ast);
        auto statements = prog->statements();
        // 12.9 considered syntactically incorrect if it contains
        //      a return statement that is not within a FunctionBody.
        if(ast->type() != AST::AST_FUNC_BODY)
        {
            for(auto stmt : statements)
            {
                if(stmt->type() == AST::AST_STMT_RETURN)
                {
                    return Completion(Completion::THROWING, new ErrorObject(Error::SyntaxError()), "");
                }
            }
        }
        if(statements.empty())
        {
            return Completion(Completion::NORMAL, nullptr, "");
        }
        for(auto stmt : prog->statements())
        {
            if(head_result.IsAbruptCompletion())
            {
                break;
            }
            Completion tail_result = EvalStatement(stmt);
            if(tail_result.IsThrow())
            {
                return tail_result;
            }
            head_result = Completion(tail_result.type, tail_result.value == nullptr ? head_result.value : tail_result.value,
                                     tail_result.target);
        }
        return head_result;
    }

    inline Completion EvalStatement(AST* ast)
    {
        switch(ast->type())
        {
            case AST::AST_STMT_BLOCK:
                return EvalBlockStatement(ast);
            case AST::AST_STMT_VAR:
                return EvalVarStatement(ast);
            case AST::AST_STMT_EMPTY:
                return Completion(Completion::NORMAL, nullptr, "");
            case AST::AST_STMT_IF:
                return EvalIfStatement(ast);
            case AST::AST_STMT_DO_WHILE:
                return EvalDoWhileStatement(ast);
            case AST::AST_STMT_WHILE:
                return EvalWhileStatement(ast);
            case AST::AST_STMT_FOR:
                return EvalForStatement(ast);
            case AST::AST_STMT_FOR_IN:
                return EvalForInStatement(ast);
            case AST::AST_STMT_CONTINUE:
                return EvalContinueStatement(ast);
            case AST::AST_STMT_BREAK:
                return EvalBreakStatement(ast);
            case AST::AST_STMT_RETURN:
                return EvalReturnStatement(ast);
            case AST::AST_STMT_WITH:
                return EvalWithStatement(ast);
            case AST::AST_STMT_LABEL:
                return EvalLabelledStatement(ast);
            case AST::AST_STMT_SWITCH:
                return EvalSwitchStatement(ast);
            case AST::AST_STMT_THROW:
                return EvalThrowStatement(ast);
            case AST::AST_STMT_TRY:
                return EvalTryStatement(ast);
            case AST::AST_STMT_DEBUG:
                return Completion(Completion::NORMAL, nullptr, "");
            default:
                return EvalExpressionStatement(ast);
        }
    }

    inline Completion EvalStatementList(const std::vector<AST*>& statements)
    {
        Completion sl;
        for(auto stmt : statements)
        {
            Completion s = EvalStatement(stmt);
            if(s.IsThrow())
            {
                return s;
            }
            sl = Completion(s.type, s.value == nullptr ? sl.value : s.value, s.target);
            if(sl.IsAbruptCompletion())
            {
                return sl;
            }
        }
        return sl;
    }

    inline Completion EvalBlockStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_BLOCK);
        Block* block = static_cast<Block*>(ast);
        return EvalStatementList(block->statements());
    }

    inline std::string EvalVarDecl(Error* e, AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_VAR_DECL);
        VarDecl* decl = static_cast<VarDecl*>(ast);
        if(decl->init() == nullptr)
        {
            return decl->ident();
        }
        JSValue* lhs = IdentifierResolution(decl->ident());
        JSValue* rhs = EvalAssignmentExpression(e, decl->init());
        if(!e->IsOk())
        {
            return decl->ident();
        }
        JSValue* value = GetValue(e, rhs);
        if(!e->IsOk())
        {
            return decl->ident();
        }
        PutValue(e, lhs, value);
        if(!e->IsOk())
        {
            return decl->ident();
        }
        return decl->ident();
    }

    inline Completion EvalVarStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_VAR);
        Error* e = Error::Ok();
        VarStmt* var_stmt = static_cast<VarStmt*>(ast);
        for(VarDecl* decl : var_stmt->decls())
        {
            if(decl->init() == nullptr)
            {
                continue;
            }
            EvalVarDecl(e, decl);
            if(!e->IsOk())
            {
                goto error;
            }
        }
        return Completion(Completion::NORMAL, nullptr, "");
    error:
        return Completion(Completion::THROWING, new ErrorObject(e), "");
    }

    inline Completion EvalIfStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_IF);
        Error* e = Error::Ok();
        If* if_stmt = static_cast<If*>(ast);
        JSValue* expr_ref = EvalExpression(e, if_stmt->cond());
        if(!e->IsOk())
        {
            return Completion(Completion::THROWING, new ErrorObject(e), "");
        }
        JSValue* expr = GetValue(e, expr_ref);
        if(!e->IsOk())
        {
            return Completion(Completion::THROWING, new ErrorObject(e), "");
        }
        if(ToBoolean(expr))
        {
            return EvalStatement(if_stmt->if_block());
        }
        else if(if_stmt->else_block() != nullptr)
        {
            return EvalStatement(if_stmt->else_block());
        }
        return Completion(Completion::NORMAL, nullptr, "");
    }

    // 12.6.1 The do-while Statement
    inline Completion EvalDoWhileStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_DO_WHILE);
        Error* e = Error::Ok();
        RuntimeContext::TopContext()->EnterIteration();
        DoWhile* loop_stmt = static_cast<DoWhile*>(ast);
        JSValue* V = nullptr;
        JSValue* expr_ref;
        JSValue* val;
        Completion stmt;
        bool has_label;
        while(true)
        {
            stmt = EvalStatement(loop_stmt->stmt());
            if(stmt.value != nullptr)
            {// 3.b
                V = stmt.value;
            }
            has_label = stmt.target == ast->label() || stmt.target.empty();
            if(stmt.type != Completion::CONTINUING || !has_label)
            {
                if(stmt.type == Completion::BREAKING && has_label)
                {
                    RuntimeContext::TopContext()->ExitIteration();
                    return Completion(Completion::NORMAL, V, "");
                }
                if(stmt.IsAbruptCompletion())
                {
                    RuntimeContext::TopContext()->ExitIteration();
                    return stmt;
                }
            }

            expr_ref = EvalExpression(e, loop_stmt->expr());
            if(!e->IsOk())
            {
                goto error;
            }
            val = GetValue(e, expr_ref);
            if(!e->IsOk())
            {
                goto error;
            }
            if(!ToBoolean(val))
            {
                break;
            }
        }
        RuntimeContext::TopContext()->ExitIteration();
        return Completion(Completion::NORMAL, V, "");
    error:
        RuntimeContext::TopContext()->ExitIteration();
        return Completion(Completion::THROWING, new ErrorObject(e), "");
    }

    // 12.6.2 The while Statement
    inline Completion EvalWhileStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_WHILE);
        Error* e = Error::Ok();
        RuntimeContext::TopContext()->EnterIteration();
        WhileOrWith* loop_stmt = static_cast<WhileOrWith*>(ast);
        JSValue* V = nullptr;
        JSValue* expr_ref;
        JSValue* val;
        Completion stmt;
        bool has_label;
        while(true)
        {
            expr_ref = EvalExpression(e, loop_stmt->expr());
            if(!e->IsOk())
            {
                goto error;
            }
            val = GetValue(e, expr_ref);
            if(!e->IsOk())
            {
                goto error;
            }
            if(!ToBoolean(val))
            {
                break;
            }

            stmt = EvalStatement(loop_stmt->stmt());
            if(stmt.value != nullptr)
            {// 3.b
                V = stmt.value;
            }
            has_label = stmt.target == ast->label() || stmt.target.empty();
            if(stmt.type != Completion::CONTINUING || !has_label)
            {
                if(stmt.type == Completion::BREAKING && has_label)
                {
                    RuntimeContext::TopContext()->ExitIteration();
                    return Completion(Completion::NORMAL, V, "");
                }
                if(stmt.IsAbruptCompletion())
                {
                    RuntimeContext::TopContext()->ExitIteration();
                    return stmt;
                }
            }
        }
        RuntimeContext::TopContext()->ExitIteration();
        return Completion(Completion::NORMAL, V, "");
    error:
        RuntimeContext::TopContext()->ExitIteration();
        return Completion(Completion::THROWING, new ErrorObject(e), "");
    }

    // 12.6.3 The for Statement
    inline Completion EvalForStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_FOR);
        Error* e = Error::Ok();
        RuntimeContext::TopContext()->EnterIteration();
        For* for_stmt = static_cast<For*>(ast);
        JSValue* V = nullptr;
        Completion stmt;
        bool has_label;
        for(auto expr : for_stmt->expr0s())
        {
            if(expr->type() == AST::AST_STMT_VAR_DECL)
            {
                EvalVarDecl(e, expr);
                if(!e->IsOk())
                {
                    goto error;
                }
            }
            else
            {
                JSValue* expr_ref = EvalExpression(e, expr);
                if(!e->IsOk())
                {
                    goto error;
                }
                GetValue(e, expr_ref);
                if(!e->IsOk())
                {
                    goto error;
                }
            }
        }
        while(true)
        {
            if(for_stmt->expr1() != nullptr)
            {
                JSValue* test_expr_ref = EvalExpression(e, for_stmt->expr1());
                if(!e->IsOk())
                {
                    goto error;
                }
                JSValue* test_value = GetValue(e, test_expr_ref);
                if(!e->IsOk())
                {
                    goto error;
                }
                if(!ToBoolean(test_value))
                {
                    break;
                }
            }

            stmt = EvalStatement(for_stmt->statement());
            if(stmt.value != nullptr)
            {// 3.b
                V = stmt.value;
            }
            has_label = stmt.target == ast->label() || stmt.target.empty();
            if(stmt.type != Completion::CONTINUING || !has_label)
            {
                if(stmt.type == Completion::BREAKING && has_label)
                {
                    RuntimeContext::TopContext()->ExitIteration();
                    return Completion(Completion::NORMAL, V, "");
                }
                if(stmt.IsAbruptCompletion())
                {
                    RuntimeContext::TopContext()->ExitIteration();
                    return stmt;
                }
            }

            if(for_stmt->expr2() != nullptr)
            {
                JSValue* inc_expr_ref = EvalExpression(e, for_stmt->expr2());
                if(!e->IsOk())
                {
                    goto error;
                }
                GetValue(e, inc_expr_ref);
                if(!e->IsOk())
                {
                    goto error;
                }
            }
        }
        RuntimeContext::TopContext()->ExitIteration();
        return Completion(Completion::NORMAL, V, "");
    error:
        RuntimeContext::TopContext()->ExitIteration();
        return Completion(Completion::THROWING, new ErrorObject(e), "");
    }

    // 12.6.4 The for-in Statement
    inline Completion EvalForInStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_FOR_IN);
        Error* e = Error::Ok();
        RuntimeContext::TopContext()->EnterIteration();
        ForIn* for_in_stmt = static_cast<ForIn*>(ast);
        JSObject* obj;
        JSValue* expr_ref;
        JSValue* expr_val;
        Completion stmt;
        bool has_label;
        JSValue* V = nullptr;
        if(for_in_stmt->expr0()->type() == AST::AST_STMT_VAR_DECL)
        {
            VarDecl* decl = static_cast<VarDecl*>(for_in_stmt->expr0());
            std::string var_name = EvalVarDecl(e, decl);
            if(!e->IsOk())
            {
                goto error;
            }
            expr_ref = EvalExpression(e, for_in_stmt->expr1());
            if(!e->IsOk())
            {
                goto error;
            }
            expr_val = GetValue(e, expr_ref);
            if(!e->IsOk())
            {
                goto error;
            }
            if(expr_val->IsUndefined() || expr_val->IsNull())
            {
                RuntimeContext::TopContext()->ExitIteration();
                return Completion(Completion::NORMAL, nullptr, "");
            }
            obj = ToObject(e, expr_val);
            if(!e->IsOk())
            {
                goto error;
            }

            for(const auto& pair : obj->AllEnumerableProperties())
            {
                String* P = new String(pair.first);
                Reference* var_ref = IdentifierResolution(var_name);
                PutValue(e, var_ref, P);
                if(!e->IsOk())
                {
                    goto error;
                }

                stmt = EvalStatement(for_in_stmt->statement());
                if(stmt.value != nullptr)
                {
                    V = stmt.value;
                }
                has_label = stmt.target == ast->label() || stmt.target.empty();
                if(stmt.type != Completion::CONTINUING || !has_label)
                {
                    if(stmt.type == Completion::BREAKING && has_label)
                    {
                        RuntimeContext::TopContext()->ExitIteration();
                        return Completion(Completion::NORMAL, V, "");
                    }
                    if(stmt.IsAbruptCompletion())
                    {
                        RuntimeContext::TopContext()->ExitIteration();
                        return stmt;
                    }
                }
            }
        }
        else
        {
            expr_ref = EvalExpression(e, for_in_stmt->expr1());
            if(!e->IsOk())
            {
                goto error;
            }
            expr_ref = EvalExpression(e, for_in_stmt->expr1());
            if(!e->IsOk())
            {
                goto error;
            }
            expr_val = GetValue(e, expr_ref);
            if(!e->IsOk())
            {
                goto error;
            }
            if(expr_val->IsUndefined() || expr_val->IsNull())
            {
                RuntimeContext::TopContext()->ExitIteration();
                return Completion(Completion::NORMAL, nullptr, "");
            }
            obj = ToObject(e, expr_val);
            for(const auto& pair : obj->AllEnumerableProperties())
            {
                String* P = new String(pair.first);
                JSValue* lhs_ref = EvalExpression(e, for_in_stmt->expr0());
                if(!e->IsOk())
                {
                    goto error;
                }
                PutValue(e, lhs_ref, P);
                if(!e->IsOk())
                {
                    goto error;
                }

                stmt = EvalStatement(for_in_stmt->statement());
                if(stmt.value != nullptr)
                {
                    V = stmt.value;
                }
                has_label = stmt.target == ast->label() || stmt.target.empty();
                if(stmt.type != Completion::CONTINUING || !has_label)
                {
                    if(stmt.type == Completion::BREAKING && has_label)
                    {
                        RuntimeContext::TopContext()->ExitIteration();
                        return Completion(Completion::NORMAL, V, "");
                    }
                    if(stmt.IsAbruptCompletion())
                    {
                        RuntimeContext::TopContext()->ExitIteration();
                        return stmt;
                    }
                }
            }
        }
        RuntimeContext::TopContext()->ExitIteration();
        return Completion(Completion::NORMAL, V, "");
    error:
        RuntimeContext::TopContext()->ExitIteration();
        return Completion(Completion::THROWING, new ErrorObject(e), "");
    }

    inline Completion EvalContinueStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_CONTINUE);
        Error* e = Error::Ok();
        if(!RuntimeContext::TopContext()->InIteration())
        {
            *e = *Error::SyntaxError();
            return Completion(Completion::THROWING, new ErrorObject(e), "");
        }
        ContinueOrBreak* stmt = static_cast<ContinueOrBreak*>(ast);
        return Completion(Completion::CONTINUING, nullptr, stmt->ident());
    }

    inline Completion EvalBreakStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_BREAK);
        Error* e = Error::Ok();
        if(!RuntimeContext::TopContext()->InIteration())
        {
            *e = *Error::SyntaxError();
            return Completion(Completion::THROWING, new ErrorObject(e), "");
        }
        ContinueOrBreak* stmt = static_cast<ContinueOrBreak*>(ast);
        return Completion(Completion::BREAKING, nullptr, stmt->ident());
    }

    inline Completion EvalReturnStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_RETURN);
        Error* e = Error::Ok();
        Return* return_stmt = static_cast<Return*>(ast);
        if(return_stmt->expr() == nullptr)
        {
            return Completion(Completion::RETURNING, Undefined::Instance(), "");
        }
        auto exp_ref = EvalExpression(e, return_stmt->expr());
        return Completion(Completion::RETURNING, GetValue(e, exp_ref), "");
    }

    inline Completion EvalLabelledStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_LABEL);
        LabelledStmt* label_stmt = static_cast<LabelledStmt*>(ast);
        label_stmt->statement()->SetLabel(label_stmt->label());
        Completion R = EvalStatement(label_stmt->statement());
        if(R.type == Completion::BREAKING && R.target == label_stmt->label())
        {
            return Completion(Completion::NORMAL, R.value, "");
        }
        return R;
    }

    // 12.10 The with Statement
    inline Completion EvalWithStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_WITH);
        if(RuntimeContext::TopContext()->strict())
        {
            return Completion(Completion::THROWING,
                              new ErrorObject(Error::SyntaxError("cannot have with statement in strict mode")), "");
        }
        Error* e = Error::Ok();
        WhileOrWith* with_stmt = static_cast<WhileOrWith*>(ast);
        JSValue* ref = EvalExpression(e, with_stmt->expr());
        if(!e->IsOk())
        {
            return Completion(Completion::THROWING, new ErrorObject(e), "");
        }
        JSValue* val = GetValue(e, ref);
        if(!e->IsOk())
        {
            return Completion(Completion::THROWING, new ErrorObject(e), "");
        }
        JSObject* obj = ToObject(e, val);
        if(!e->IsOk())
        {
            return Completion(Completion::THROWING, new ErrorObject(e), "");
        }
        LexicalEnvironment* old_env = RuntimeContext::TopLexicalEnv();
        LexicalEnvironment* new_env = LexicalEnvironment::NewObjectEnvironment(obj, old_env, true);
        RuntimeContext::TopContext()->SetLexicalEnv(new_env);
        Completion C = EvalStatement(with_stmt->stmt());
        RuntimeContext::TopContext()->SetLexicalEnv(old_env);
        return C;
    }

    inline JSValue* EvalCaseClause(Error* e, const Switch::CaseClause& C)
    {
        JSValue* exp_ref = EvalExpression(e, C.expr);
        if(!e->IsOk())
        {
            return nullptr;
        }
        return GetValue(e, exp_ref);
    }

    inline Completion EvalCaseBlock(Switch* switch_stmt, JSValue* input)
    {
        Error* e = Error::Ok();
        JSValue* V = nullptr;
        bool found = false;
        for(const auto& C : switch_stmt->before_default_case_clauses())
        {
            if(!found)
            {// 5.a
                JSValue* clause_selector = EvalCaseClause(e, C);
                bool b = StrictEqual(e, input, clause_selector);
                if(!e->IsOk())
                {
                    return Completion(Completion::THROWING, new ErrorObject(e), "");
                }
                if(b)
                {
                    found = true;
                }
            }
            if(found)
            {// 5.b
                Completion R = EvalStatementList(C.stmts);
                if(R.value != nullptr)
                {
                    V = R.value;
                }
                if(R.IsAbruptCompletion())
                {
                    return Completion(R.type, V, R.target);
                }
            }
        }
        bool found_in_b = false;
        size_t i;
        for(i = 0; !found_in_b && i < switch_stmt->after_default_case_clauses().size(); i++)
        {
            auto C = switch_stmt->after_default_case_clauses()[i];
            JSValue* clause_selector = EvalCaseClause(e, C);
            bool b = StrictEqual(e, input, clause_selector);
            if(!e->IsOk())
            {
                return Completion(Completion::THROWING, new ErrorObject(e), "");
            }
            if(b)
            {
                found_in_b = true;
                Completion R = EvalStatementList(C.stmts);
                if(R.value != nullptr)
                {
                    V = R.value;
                }
                if(R.IsAbruptCompletion())
                {
                    return Completion(R.type, V, R.target);
                }
            }
        }
        if(!found_in_b && switch_stmt->has_default_clause())
        {// 8
            Completion R = EvalStatementList(switch_stmt->default_clause().stmts);
            if(R.value != nullptr)
            {
                V = R.value;
            }
            if(R.IsAbruptCompletion())
            {
                return Completion(R.type, V, R.target);
            }
        }
        for(i = 0; i < switch_stmt->after_default_case_clauses().size(); i++)
        {
            auto C = switch_stmt->after_default_case_clauses()[i];
            JSValue* clause_selector = EvalCaseClause(e, C);
            (void)clause_selector;
            Completion R = EvalStatementList(C.stmts);
            if(R.value != nullptr)
            {
                V = R.value;
            }
            if(R.IsAbruptCompletion())
            {
                return Completion(R.type, V, R.target);
            }
        }
        return Completion(Completion::NORMAL, V, "");
    }

    // 12.11 The switch Statement
    inline Completion EvalSwitchStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_SWITCH);
        Error* e = Error::Ok();
        Switch* switch_stmt = static_cast<Switch*>(ast);
        JSValue* expr_ref = EvalExpression(e, switch_stmt->expr());
        if(!e->IsOk())
        {
            return Completion(Completion::THROWING, new ErrorObject(e), "");
        }
        Completion R = EvalCaseBlock(switch_stmt, expr_ref);
        if(R.IsThrow())
        {
            return R;
        }
        bool has_label = ast->label() == R.target;
        if(R.type == Completion::BREAKING && has_label)
        {
            return Completion(Completion::NORMAL, R.value, "");
        }
        return R;
    }

    // 12.13 The throw Statement
    inline Completion EvalThrowStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_THROW);
        Error* e = Error::Ok();
        Throw* throw_stmt = static_cast<Throw*>(ast);
        JSValue* exp_ref = EvalExpression(e, throw_stmt->expr());
        if(es::Error::Ok() == nullptr)
        {
            return Completion(Completion::THROWING, new ErrorObject(e), "");
        }
        JSValue* val = GetValue(e, exp_ref);
        if(es::Error::Ok() == nullptr)
        {
            return Completion(Completion::THROWING, new ErrorObject(e), "");
        }
        return Completion(Completion::THROWING, val, "");
    }

    inline Completion EvalCatch(Try* try_stmt, const Completion& C)
    {
        // NOTE(zhuzilin) Don't gc these two env, during this function.
        Error* e = Error::Ok();
        LexicalEnvironment* old_env = RuntimeContext::TopLexicalEnv();
        LexicalEnvironment* catch_env = LexicalEnvironment::NewDeclarativeEnvironment(old_env);
        catch_env->env_rec()->CreateMutableBinding(e, try_stmt->catch_ident(), false);// 4
        if(!e->IsOk())
        {
            return Completion(Completion::THROWING, new ErrorObject(e), "");
        }
        // NOTE(zhuzilin) The spec say to send C instead of C.value.
        // However, I think it should be send C.value...
        catch_env->env_rec()->SetMutableBinding(e, try_stmt->catch_ident(), C.value, false);// 5
        if(!e->IsOk())
        {
            return Completion(Completion::THROWING, new ErrorObject(e), "");
        }
        RuntimeContext::TopContext()->SetLexicalEnv(catch_env);
        Completion B = EvalBlockStatement(try_stmt->catch_block());
        RuntimeContext::TopContext()->SetLexicalEnv(old_env);
        return B;
    }

    inline Completion EvalTryStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_TRY);
        Error* e = Error::Ok();
        (void)e;
        Try* try_stmt = static_cast<Try*>(ast);
        Completion B = EvalBlockStatement(try_stmt->try_block());
        if(try_stmt->finally_block() == nullptr)
        {// try Block Catch
            if(B.type != Completion::THROWING)
            {
                return B;
            }
            return EvalCatch(try_stmt, B);
        }
        else if(try_stmt->catch_block() == nullptr)
        {// try Block Finally
            Completion F = EvalBlockStatement(try_stmt->finally_block());
            if(F.type == Completion::NORMAL)
            {
                return B;
            }
            return F;
        }
        else
        {// try Block Catch Finally
            Completion C = B;
            if(B.type == Completion::THROWING)
            {
                C = EvalCatch(try_stmt, B);
            }
            Completion F = EvalBlockStatement(try_stmt->finally_block());
            if(F.type == Completion::NORMAL)
            {
                return C;
            }
            return F;
        }
    }

    inline Completion EvalExpressionStatement(AST* ast)
    {
        Error* e = Error::Ok();
        JSValue* val = EvalExpression(e, ast);
        if(!e->IsOk())
        {
            return Completion(Completion::THROWING, new ErrorObject(e), "");
        }
        return Completion(Completion::NORMAL, val, "");
    }

    inline JSValue* EvalExpression(Error* e, AST* ast)
    {
        assert(ast->type() <= AST::AST_EXPR || ast->type() == AST::AST_FUNC);
        JSValue* val;
        switch(ast->type())
        {
            case AST::AST_EXPR_THIS:
            case AST::AST_EXPR_IDENT:
            case AST::AST_EXPR_NULL:
            case AST::AST_EXPR_BOOL:
            case AST::AST_EXPR_NUMBER:
            case AST::AST_EXPR_STRING:
            case AST::AST_EXPR_OBJ:
            case AST::AST_EXPR_ARRAY:
            case AST::AST_EXPR_PAREN:
                val = EvalPrimaryExpression(e, ast);
                break;
            case AST::AST_EXPR_UNARY:
                val = EvalUnaryOperator(e, ast);
                break;
            case AST::AST_EXPR_BINARY:
                val = EvalBinaryExpression(e, ast);
                break;
            case AST::AST_EXPR_TRIPLE:
                val = EvalTripleConditionExpression(e, ast);
                break;
            case AST::AST_EXPR_LHS:
                val = EvalLeftHandSideExpression(e, ast);
                break;
            case AST::AST_EXPR:
                val = EvalExpressionList(e, ast);
            case AST::AST_FUNC:
                val = EvalFunction(e, ast);
                break;
            default:
                assert(false);
        }
        if(!e->IsOk())
        {
            return nullptr;
        }
        return val;
    }

    inline JSValue* EvalPrimaryExpression(Error* e, AST* ast)
    {
        JSValue* val;
        switch(ast->type())
        {
            case AST::AST_EXPR_THIS:
                val = RuntimeContext::TopContext()->this_binding();
                break;
            case AST::AST_EXPR_IDENT:
                val = EvalIdentifier(ast);
                break;
            case AST::AST_EXPR_NULL:
                val = Null::Instance();
                break;
            case AST::AST_EXPR_BOOL:
                val = ast->source() == "true" ? Bool::True() : Bool::False();
                break;
            case AST::AST_EXPR_NUMBER:
                val = EvalNumber(ast);
                break;
            case AST::AST_EXPR_STRING:
                val = EvalString(ast);
                break;
            case AST::AST_EXPR_OBJ:
                val = EvalObject(e, ast);
                break;
            case AST::AST_EXPR_ARRAY:
                val = EvalArray(e, ast);
                break;
            case AST::AST_EXPR_PAREN:
                val = EvalExpression(e, static_cast<Paren*>(ast)->expr());
                break;
            default:
                std::cout << "Not primary expression, type " << ast->type() << std::endl;
                assert(false);
        }
        return val;
    }

    inline Reference* IdentifierResolution(const std::string& name)
    {
        // 10.3.1 Identifier Resolution
        LexicalEnvironment* env = RuntimeContext::TopLexicalEnv();
        bool strict = RuntimeContext::TopContext()->strict();
        return env->GetIdentifierReference(name, strict);
    }

    inline Reference* EvalIdentifier(AST* ast)
    {
        assert(ast->type() == AST::AST_EXPR_IDENT);
        return IdentifierResolution(ast->source());
    }

    inline Number* EvalNumber(const std::string& source)
    {
        double val = 0;
        double frac = 1;
        size_t pos = 0;
        bool dot = false;
        while(pos < source.size())
        {
            char c = source[pos];
            switch(c)
            {
                case u'.':
                    dot = true;
                    break;
                case u'e':
                case u'E':
                {
                    double exp = 0;
                    bool sign = true;
                    pos++;// skip e/E
                    if(c == u'-')
                    {
                        sign = false;
                        pos++;// skip -
                    }
                    else if(c == u'+')
                    {
                        sign = true;
                        pos++;// skip +;
                    }
                    while(pos < source.size())
                    {
                        exp *= 10;
                        exp += character::Digit(c);
                    }
                    if(!sign)
                    {
                        exp = -exp;
                    }
                    return new Number(val * pow(10.0, exp));
                }
                case u'x':
                case u'X':
                {
                    assert(val == 0);
                    pos++;
                    while(pos < source.size())
                    {
                        c = source[pos];
                        val *= 16;
                        val += character::Digit(c);
                        pos++;
                    }
                    return new Number(val);
                }
                default:
                    if(dot)
                    {
                        frac /= 10;
                        val += character::Digit(c) * frac;
                    }
                    else
                    {
                        val *= 10;
                        val += character::Digit(c);
                    }
            }
            pos++;
        }
        return new Number(val);
    }

    inline Number* EvalNumber(AST* ast)
    {
        assert(ast->type() == AST::AST_EXPR_NUMBER);
        auto source = ast->source();
        return EvalNumber(source);
    }

    inline String* EvalString(const std::string& isource)
    {
        auto source = isource.substr(1, isource.size() - 2);
        size_t pos = 0;
        std::vector<std::string> vals;
        while(pos < source.size())
        {
            char c = source[pos];
            switch(c)
            {
                case u'\\':
                {
                    pos++;
                    c = source[pos];
                    switch(c)
                    {
                        case u'b':
                            pos++;
                            vals.emplace_back("\b");
                            break;
                        case u't':
                            pos++;
                            vals.emplace_back("\t");
                            break;
                        case u'n':
                            pos++;
                            vals.emplace_back("\n");
                            break;
                        case u'v':
                            pos++;
                            vals.emplace_back("\v");
                            break;
                        case u'f':
                            pos++;
                            vals.emplace_back("\f");
                            break;
                        case u'r':
                            pos++;
                            vals.emplace_back("\r");
                            break;
                        case u'x':
                        {
                            pos++;// skip 'x'
                            char hex = 0;
                            for(size_t i = 0; i < 2; i++)
                            {
                                hex *= 16;
                                hex += character::Digit(source[pos]);
                                pos++;
                            }
                            vals.emplace_back(std::string(1, hex));
                            break;
                        }
                        case u'u':
                        {
                            pos++;// skip 'u'
                            int hex = 0;
                            for(size_t i = 0; i < 4; i++)
                            {
                                hex *= 16;
                                hex += character::Digit(source[pos]);
                                pos++;
                            }
                            vals.emplace_back(std::string(1, hex));
                            break;
                        }
                        default:
                            c = source[pos];
                            if(character::IsLineTerminator(c))
                            {
                                pos++;
                                continue;
                            }
                            pos++;
                            vals.emplace_back(std::string(1, c));
                    }
                    break;
                }
                default:
                {
                    size_t start = pos;
                    while(true)
                    {
                        if(pos == source.size() || source[pos] == u'\\')
                        {
                            break;
                        }
                        pos++;
                    }
                    size_t end = pos;
                    auto substr = source.substr(start, end - start);
                    vals.emplace_back(std::string(substr.data(), substr.size()));
                }
            }
        }
        if(vals.empty())
        {
            return String::Empty();
        }
        else if(vals.size() == 1)
        {
            return new String(vals[0]);
        }
        return new String(StrCat(vals));
    }

    inline String* EvalString(AST* ast)
    {
        assert(ast->type() == AST::AST_EXPR_STRING);
        auto source = ast->source();
        return EvalString(source);
    }

    inline std::string EvalPropertyName(Error* e, Token token)
    {
        switch(token.type())
        {
            case Token::TK_IDENT:
            case Token::TK_KEYWORD:
            case Token::TK_FUTURE:
                return token.source();
            case Token::TK_NUMBER:
                return ToString(e, EvalNumber(token.source()));
            case Token::TK_STRING:
                return ToString(e, EvalString(token.source()));
            default:
                assert(false);
        }
    }

    inline Object* EvalObject(Error* e, AST* ast)
    {
        assert(ast->type() == AST::AST_EXPR_OBJ);
        ObjectLiteral* obj_ast = static_cast<ObjectLiteral*>(ast);
        bool strict = RuntimeContext::TopContext()->strict();
        Object* obj = new Object();
        // PropertyName : AssignmentExpression
        for(auto property : obj_ast->properties())
        {
            std::string prop_name = EvalPropertyName(e, property.key);
            PropertyDescriptor* desc = new PropertyDescriptor();
            switch(property.type)
            {
                case ObjectLiteral::Property::NORMAL:
                {
                    JSValue* expr_value = EvalAssignmentExpression(e, property.value);
                    JSValue* prop_value = GetValue(e, expr_value);
                    desc->SetDataDescriptor(prop_value, true, true, true);
                    break;
                }
                default:
                {
                    assert(property.value->type() == AST::AST_FUNC);
                    Function* func_ast = static_cast<Function*>(property.value);
                    bool strict_func = static_cast<ProgramOrFunctionBody*>(func_ast->body())->strict();
                    if(strict || strict_func)
                    {
                        for(const auto& name : func_ast->params())
                        {
                            if(name == "eval" || name == "arguments")
                            {
                                *e = *Error::SyntaxError();
                                return nullptr;
                            }
                        }
                    }
                    FunctionObject* closure
                    = new FunctionObject(func_ast->params(), func_ast->body(), RuntimeContext::TopLexicalEnv());
                    if(property.type == ObjectLiteral::Property::GET)
                    {
                        desc->SetGet(closure);
                    }
                    else
                    {
                        desc->SetSet(closure);
                    }
                    desc->SetEnumerable(true);
                    desc->SetConfigurable(true);
                    break;
                }
            }
            auto previous = obj->GetOwnProperty(prop_name);// 3
            if(!previous->IsUndefined())
            {// 4
                PropertyDescriptor* previous_desc = static_cast<PropertyDescriptor*>(previous);
                if(strict && previous_desc->IsDataDescriptor() && desc->IsDataDescriptor())
                {// 4.a
                    *e = *Error::SyntaxError();
                    return nullptr;
                }
                if((previous_desc->IsDataDescriptor() && desc->IsAccessorDescriptor()) || (previous_desc->IsAccessorDescriptor() && desc->IsDataDescriptor()))
                {// 4.c
                    *e = *Error::SyntaxError();
                    return nullptr;
                }
                if(((previous_desc->IsAccessorDescriptor() && desc->IsAccessorDescriptor() &&// 4.d
                   (previous_desc->HasGet() && desc->HasGet())) || (previous_desc->HasSet() && desc->HasSet())))
                {
                    *e = *Error::SyntaxError();
                    return nullptr;
                }
            }
            obj->DefineOwnProperty(e, prop_name, desc, false);
        }
        return obj;
    }

    inline ArrayObject* EvalArray(Error* e, AST* ast)
    {
        assert(ast->type() == AST::AST_EXPR_ARRAY);
        ArrayLiteral* array_ast = static_cast<ArrayLiteral*>(ast);

        ArrayObject* arr = new ArrayObject(array_ast->length());
        for(auto pair : array_ast->elements())
        {
            JSValue* init_result = EvalAssignmentExpression(e, pair.second);
            if(!e->IsOk())
            {
                return nullptr;
            }
            arr->AddValueProperty(NumberToString(pair.first), init_result, true, true, true);
        }
        return arr;
    }

    inline JSValue* EvalAssignmentExpression(Error* e, AST* ast)
    {
        return EvalExpression(e, ast);
    }

    inline JSValue* EvalUnaryOperator(Error* e, AST* ast)
    {
        assert(ast->type() == AST::AST_EXPR_UNARY);
        Unary* u = static_cast<Unary*>(ast);

        JSValue* expr = EvalExpression(e, u->node());
        if(!e->IsOk())
        {
            return nullptr;
        }
        std::string op = u->op().source();

        if(op == "++" || op == "--")
        {// a++, ++a, a--, --a
            if(expr->IsReference())
            {
                Reference* ref = static_cast<Reference*>(expr);
                if(ref->IsStrictReference() && ref->GetBase()->IsEnvironmentRecord()
                   && (ref->GetReferencedName() == "eval" || ref->GetReferencedName() == "arguments"))
                {
                    *e = *Error::SyntaxError();
                    return nullptr;
                }
            }
            JSValue* old_val = GetValue(e, expr);
            if(!e->IsOk())
            {
                return nullptr;
            }
            double num = ToNumber(e, old_val);
            if(!e->IsOk())
            {
                return nullptr;
            }
            JSValue* new_value;
            if(op == "++")
            {
                new_value = new Number(num + 1);
            }
            else
            {
                new_value = new Number(num - 1);
            }
            PutValue(e, expr, new_value);
            if(!e->IsOk())
            {
                return nullptr;
            }
            if(u->prefix())
            {
                return new_value;
            }
            else
            {
                return old_val;
            }
        }
        else if(op == "delete")
        {// 11.4.1 The delete Operator
            if(!expr->IsReference())
            {// 2
                return Bool::True();
            }
            Reference* ref = static_cast<Reference*>(expr);
            if(ref->IsUnresolvableReference())
            {// 3
                if(ref->IsStrictReference())
                {
                    *e = *Error::SyntaxError();
                    return Bool::False();
                }
                return Bool::True();
            }
            if(ref->IsPropertyReference())
            {// 4
                JSObject* obj = ToObject(e, ref->GetBase());
                if(!e->IsOk())
                {
                    return nullptr;
                }
                return Bool::Wrap(obj->Delete(e, ref->GetReferencedName(), ref->IsStrictReference()));
            }
            else
            {
                if(ref->IsStrictReference())
                {
                    *e = *Error::SyntaxError();
                    return Bool::False();
                }
                EnvironmentRecord* bindings = static_cast<EnvironmentRecord*>(ref->GetBase());
                return Bool::Wrap(bindings->DeleteBinding(e, ref->GetReferencedName()));
            }
        }
        else if(op == "typeof")
        {
            if(expr->IsReference())
            {
                Reference* ref = static_cast<Reference*>(expr);
                if(ref->IsUnresolvableReference())
                {
                    return String::Undefined();
                }
            }
            JSValue* val = GetValue(e, expr);
            if(!e->IsOk())
            {
                return nullptr;
            }
            switch(val->type())
            {
                case JSValue::JS_UNDEFINED:
                    return String::Undefined();
                case JSValue::JS_NULL:
                    return new String("object");
                case JSValue::JS_NUMBER:
                    return new String("number");
                case JSValue::JS_STRING:
                    return new String("string");
                default:
                    if(val->IsCallable())
                    {
                        return new String("function");
                    }
                    return new String("object");
            }
        }
        else
        {// +, -, ~, !, void
            JSValue* val = GetValue(e, expr);
            if(!e->IsOk())
            {
                return nullptr;
            }

            if(op == "+")
            {
                double num = ToNumber(e, val);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                return new Number(num);
            }
            else if(op == "-")
            {
                double num = ToNumber(e, val);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                if(isnan(num))
                {
                    return Number::NaN();
                }
                return new Number(-num);
            }
            else if(op == "~")
            {
                int32_t num = ToInt32(e, val);
                if(!e->IsOk())
                {
                    return nullptr;
                }
                return new Number(~num);
            }
            else if(op == "!")
            {
                bool b = ToBoolean(val);
                return Bool::Wrap(!b);
            }
            else if(op == "void")
            {
                return Undefined::Instance();
            }
        }
        assert(false);
    }

    inline JSValue* EvalBinaryExpression(Error* e, AST* ast)
    {
        assert(ast->type() == AST::AST_EXPR_BINARY);
        Binary* b = static_cast<Binary*>(ast);
        return EvalBinaryExpression(e, b->op(), b->lhs(), b->rhs());
    }

    inline JSValue* EvalBinaryExpression(Error* e, const std::string& op, AST* lhs, AST* rhs)
    {
        // && and || are different, as there are not &&= and ||=
        if((op == "&&") || (op == "||"))
        {
            return EvalLogicalOperator(e, op, lhs, rhs);
        }
        if((op == "=") || (op == "*=") || (op == "/=") || (op == "%=") || (op == "+=") || (op == "-=") || (op == "<<=")
           || (op == ">>=") || (op == ">>>=") || (op == "&=") || (op == "^=") || (op == "|="))
        {
            JSValue* lref = EvalLeftHandSideExpression(e, lhs);
            if(!e->IsOk())
            {
                return nullptr;
            }
            // TODO(zhuzilin) The compound assignment should do lval = GetValue(lref)
            // here. Check if changing the order will have any influence.
            JSValue* rref = EvalExpression(e, rhs);
            if(!e->IsOk())
            {
                return nullptr;
            }
            JSValue* rval = GetValue(e, rref);
            if(!e->IsOk())
            {
                return nullptr;
            }
            if(op == "=")
            {
                return EvalSimpleAssignment(e, lref, rval);
            }
            else
            {
                return EvalCompoundAssignment(e, op, lref, rval);
            }
        }

        JSValue* lref = EvalExpression(e, lhs);
        if(!e->IsOk())
        {
            return nullptr;
        }
        JSValue* lval = GetValue(e, lref);
        if(!e->IsOk())
        {
            return nullptr;
        }
        JSValue* rref = EvalExpression(e, rhs);
        if(!e->IsOk())
        {
            return nullptr;
        }
        JSValue* rval = GetValue(e, rref);
        if(!e->IsOk())
        {
            return nullptr;
        }
        return EvalBinaryExpression(e, op, lval, rval);
    }

    inline JSValue* EvalBinaryExpression(Error* e, const std::string& op, JSValue* lval, JSValue* rval)
    {
        if((op == "*") || (op == "/") || (op == "%") || (op == "-"))
        {
            return EvalArithmeticOperator(e, op, lval, rval);
        }
        else if((op == "+"))
        {
            return EvalAddOperator(e, lval, rval);
        }
        else if((op == "<<") || (op == ">>") || (op == ">>>"))
        {
            return EvalBitwiseShiftOperator(e, op, lval, rval);
        }
        else if((op == "<") || (op == ">") || (op == "<=") || (op == ">=") || (op == "instanceof") || (op == "in"))
        {
            return EvalRelationalOperator(e, op, lval, rval);
        }
        else if((op == "==") || (op == "!=") || (op == "===") || (op == "!=="))
        {
            return EvalEqualityOperator(e, op, lval, rval);
        }
        else if((op == "&") || (op == "^") || (op == "|"))
        {
            return EvalBitwiseOperator(e, op, lval, rval);
        }
        assert(false);
    }

    // 11.5 Multiplicative Operators
    inline JSValue* EvalArithmeticOperator(Error* e, const std::string& op, JSValue* lval, JSValue* rval)
    {
        double lnum = ToNumber(e, lval);
        if(!e->IsOk())
        {
            return nullptr;
        }
        double rnum = ToNumber(e, rval);
        if(!e->IsOk())
        {
            return nullptr;
        }
        switch(op[0])
        {
            case u'*':
                return new Number(lnum * rnum);
            case u'/':
                return new Number(lnum / rnum);
            case u'%':
                return new Number(fmod(lnum, rnum));
            case u'-':
                return new Number(lnum - rnum);
            default:
                assert(false);
        }
    }

    // 11.6 Additive Operators
    inline JSValue* EvalAddOperator(Error* e, JSValue* lval, JSValue* rval)
    {
        JSValue* lprim = ToPrimitive(e, lval, "");
        if(!e->IsOk())
        {
            return nullptr;
        }
        JSValue* rprim = ToPrimitive(e, rval, "");
        if(!e->IsOk())
        {
            return nullptr;
        }

        if(lprim->IsString() || rprim->IsString())
        {
            std::string lstr = ToString(e, lprim);
            if(!e->IsOk())
            {
                return nullptr;
            }
            std::string rstr = ToString(e, rprim);
            if(!e->IsOk())
            {
                return nullptr;
            }
            return new String(lstr + rstr);
        }

        double lnum = ToNumber(e, lprim);
        if(!e->IsOk())
        {
            return nullptr;
        }
        double rnum = ToNumber(e, rprim);
        if(!e->IsOk())
        {
            return nullptr;
        }
        return new Number(lnum + rnum);
    }

    // 11.7 Bitwise Shift Operators
    inline JSValue* EvalBitwiseShiftOperator(Error* e, const std::string& op, JSValue* lval, JSValue* rval)
    {
        int32_t lnum = ToInt32(e, lval);
        if(!e->IsOk())
        {
            return nullptr;
        }
        uint32_t rnum = ToUint32(e, rval);
        if(!e->IsOk())
        {
            return nullptr;
        }
        uint32_t shift_count = rnum & 0x1F;
        if(op == "<<")
        {
            return new Number(lnum << shift_count);
        }
        else if(op == ">>")
        {
            return new Number(lnum >> shift_count);
        }
        else if(op == ">>>")
        {
            uint32_t lnum = ToUint32(e, lval);
            return new Number(lnum >> rnum);
        }
        assert(false);
    }

    // 11.8 Relational Operators
    inline JSValue* EvalRelationalOperator(Error* e, const std::string& op, JSValue* lval, JSValue* rval)
    {
        if(op == "<")
        {
            JSValue* r = LessThan(e, lval, rval);
            if(!e->IsOk())
            {
                return nullptr;
            }
            return r->IsUndefined() ? Bool::False() : r;
        }
        else if(op == ">")
        {
            JSValue* r = LessThan(e, rval, lval);
            if(!e->IsOk())
            {
                return nullptr;
            }
            return r->IsUndefined() ? Bool::False() : r;
        }
        else if(op == "<=")
        {
            JSValue* r = LessThan(e, rval, lval);
            if(!e->IsOk())
            {
                return nullptr;
            }
            if(r->IsUndefined())
            {
                return Bool::True();
            }
            return Bool::Wrap(!static_cast<Bool*>(r)->data());
        }
        else if(op == ">=")
        {
            JSValue* r = LessThan(e, lval, rval);
            if(!e->IsOk())
            {
                return nullptr;
            }
            if(r->IsUndefined())
            {
                return Bool::True();
            }
            return Bool::Wrap(!static_cast<Bool*>(r)->data());
        }
        else if(op == "instanceof")
        {
            if(!rval->IsObject())
            {
                *e = *Error::TypeError("Right-hand side of 'instanceof' is not an object");
                return nullptr;
            }
            if(!rval->IsCallable())
            {
                *e = *Error::TypeError("Right-hand side of 'instanceof' is not callable");
                return nullptr;
            }
            JSObject* obj = static_cast<JSObject*>(rval);
            return Bool::Wrap(obj->HasInstance(e, lval));
        }
        else if(op == "in")
        {
            if(!rval->IsObject())
            {
                *e = *Error::TypeError("in called on non-object");
                return nullptr;
            }
            JSObject* obj = static_cast<JSObject*>(rval);
            return Bool::Wrap(obj->HasProperty(ToString(e, lval)));
        }
        assert(false);
    }

    // 11.9 Equality Operators
    inline JSValue* EvalEqualityOperator(Error* e, const std::string& op, JSValue* lval, JSValue* rval)
    {
        if(op == "==")
        {
            return Bool::Wrap(Equal(e, lval, rval));
        }
        else if(op == "!=")
        {
            return Bool::Wrap(!Equal(e, lval, rval));
        }
        else if(op == "===")
        {
            return Bool::Wrap(StrictEqual(e, lval, rval));
        }
        else if(op == "!==")
        {
            return Bool::Wrap(!StrictEqual(e, lval, rval));
        }
        assert(false);
    }

    // 11.10 Binary Bitwise Operators
    inline JSValue* EvalBitwiseOperator(Error* e, const std::string& op, JSValue* lval, JSValue* rval)
    {
        int32_t lnum = ToInt32(e, lval);
        if(!e->IsOk())
        {
            return nullptr;
        }
        int32_t rnum = ToInt32(e, rval);
        if(!e->IsOk())
        {
            return nullptr;
        }
        switch(op[0])
        {
            case u'&':
                return new Number(lnum & rnum);
            case u'^':
                return new Number(lnum ^ rnum);
            case u'|':
                return new Number(lnum | rnum);
            default:
                assert(false);
        }
    }

    // 11.11 Binary Logical Operators
    inline JSValue* EvalLogicalOperator(Error* e, const std::string& op, AST* lhs, AST* rhs)
    {
        JSValue* lref = EvalExpression(e, lhs);
        if(!e->IsOk())
        {
            return nullptr;
        }
        JSValue* lval = GetValue(e, lref);
        if(!e->IsOk())
        {
            return nullptr;
        }
        if(((op == "&&") && !ToBoolean(lval)) || ((op == "||") && ToBoolean(lval)))
        {
            return lval;
        }
        JSValue* rref = EvalExpression(e, rhs);
        if(!e->IsOk())
        {
            return nullptr;
        }
        JSValue* rval = GetValue(e, rref);
        if(!e->IsOk())
        {
            return nullptr;
        }
        return rval;
    }

    // 11.13.1 Simple Assignment ( = )
    inline JSValue* EvalSimpleAssignment(Error* e, JSValue* lref, JSValue* rval)
    {
        if(lref->type() == JSValue::JS_REF)
        {
            Reference* ref = static_cast<Reference*>(lref);
            // NOTE in 11.13.1.
            // TODO(zhuzilin) not sure how to implement the type error part of the note.
            if(ref->IsStrictReference() && ref->IsUnresolvableReference())
            {
                *e = *Error::ReferenceError(ref->GetReferencedName() + " is not defined");
                return nullptr;
            }
            if(ref->IsStrictReference() && ref->GetBase()->type() == JSValue::JS_ENV_REC
               && (ref->GetReferencedName() == "eval" || ref->GetReferencedName() == "arguments"))
            {
                *e = *Error::SyntaxError();
                return nullptr;
            }
        }
        PutValue(e, lref, rval);
        if(!e->IsOk())
        {
            return nullptr;
        }
        return rval;
    }

    // 11.13.2 Compound Assignment ( op= )
    inline JSValue* EvalCompoundAssignment(Error* e, const std::string& op, JSValue* lref, JSValue* rval)
    {
        std::string calc_op = op.substr(0, op.size() - 1);
        JSValue* lval = GetValue(e, lref);
        if(!e->IsOk())
        {
            return nullptr;
        }
        JSValue* r = EvalBinaryExpression(e, calc_op, lval, rval);
        if(!e->IsOk())
        {
            return nullptr;
        }
        return EvalSimpleAssignment(e, lref, r);
    }

    // 11.12 Conditional Operator ( ? : )
    inline JSValue* EvalTripleConditionExpression(Error* e, AST* ast)
    {
        assert(ast->type() == AST::AST_EXPR_TRIPLE);
        TripleCondition* t = static_cast<TripleCondition*>(ast);
        JSValue* lref = EvalExpression(e, t->cond());
        if(!e->IsOk())
        {
            return nullptr;
        }
        JSValue* lval = GetValue(e, lref);
        if(!e->IsOk())
        {
            return nullptr;
        }
        if(ToBoolean(lval))
        {
            JSValue* true_ref = EvalAssignmentExpression(e, t->true_expr());
            if(!e->IsOk())
            {
                return nullptr;
            }
            return GetValue(e, true_ref);
        }
        else
        {
            JSValue* false_ref = EvalAssignmentExpression(e, t->false_expr());
            if(!e->IsOk())
            {
                return nullptr;
            }
            return GetValue(e, false_ref);
        }
    }

    inline JSValue* EvalLeftHandSideExpression(Error* e, AST* ast)
    {
        assert(ast->type() == AST::AST_EXPR_LHS);
        LHS* lhs = static_cast<LHS*>(ast);

        ValueGuard guard;
        JSValue* base = EvalExpression(e, lhs->base());
        if(!e->IsOk())
        {
            return nullptr;
        }

        size_t new_count = lhs->new_count();
        for(auto pair : lhs->order())
        {
            switch(pair.second)
            {
                case LHS::PostfixType::CALL:
                {
                    auto args = lhs->args_list()[pair.first];
                    auto arg_list = EvalArgumentsList(e, args);
                    if(!e->IsOk())
                    {
                        return nullptr;
                    }
                    if(new_count > 0)
                    {
                        base = GetValue(e, base);
                        if(!e->IsOk())
                        {
                            return nullptr;
                        }
                        if(!base->IsConstructor())
                        {
                            *e = *Error::TypeError("base value is not a constructor");
                            return nullptr;
                        }
                        JSObject* constructor = static_cast<JSObject*>(base);
                        base = constructor->Construct(e, arg_list);
                        if(!e->IsOk())
                        {
                            return nullptr;
                        }
                        new_count--;
                    }
                    else
                    {
                        base = EvalCallExpression(e, base, arg_list);
                        if(!e->IsOk())
                        {
                            return nullptr;
                        }
                    }
                    break;
                }
                case LHS::PostfixType::INDEX:
                {
                    auto index = lhs->index_list()[pair.first];
                    base = EvalIndexExpression(e, base, index, guard);
                    if(!e->IsOk())
                    {
                        return nullptr;
                    }
                    break;
                }
                case LHS::PostfixType::PROP:
                {
                    auto prop = lhs->prop_name_list()[pair.first];
                    base = EvalIndexExpression(e, base, prop, guard);
                    if(!e->IsOk())
                    {
                        return nullptr;
                    }
                    break;
                }
                default:
                    assert(false);
            }
        }
        while(new_count > 0)
        {
            base = GetValue(e, base);
            if(!e->IsOk())
            {
                return nullptr;
            }
            if(!base->IsConstructor())
            {
                *e = *Error::TypeError("base value is not a constructor");
                return nullptr;
            }
            JSObject* constructor = static_cast<JSObject*>(base);
            base = constructor->Construct(e, {});
            if(!e->IsOk())
            {
                return nullptr;
            }
            new_count--;
        }
        return base;
    }

    inline std::vector<JSValue*> EvalArgumentsList(Error* e, Arguments* ast)
    {
        std::vector<JSValue*> arg_list;
        for(AST* ast : ast->args())
        {
            JSValue* ref = EvalExpression(e, ast);
            if(!e->IsOk())
            {
                return {};
            }
            JSValue* arg = GetValue(e, ref);
            if(!e->IsOk())
            {
                return {};
            }
            arg_list.emplace_back(arg);
        }
        return arg_list;
    }

    // 11.2.3
    inline JSValue* EvalCallExpression(Error* e, JSValue* ref, const std::vector<JSValue*>& arg_list)
    {
        JSValue* val = GetValue(e, ref);
        if(!e->IsOk())
        {
            return nullptr;
        }
        if(!val->IsObject())
        {// 4
            *e = *Error::TypeError("is not a function");
            return nullptr;
        }
        auto obj = static_cast<JSObject*>(val);
        if(!obj->IsCallable())
        {// 5
            *e = *Error::TypeError("is not a function");
            return nullptr;
        }
        JSValue* this_value;
        if(ref->IsReference())
        {
            Reference* r = static_cast<Reference*>(ref);
            JSValue* base = r->GetBase();
            if(r->IsPropertyReference())
            {
                this_value = base;
            }
            else
            {
                assert(base->IsEnvironmentRecord());
                auto env_rec = static_cast<EnvironmentRecord*>(base);
                this_value = env_rec->ImplicitThisValue();
            }
        }
        else
        {
            this_value = Undefined::Instance();
        }
        // indirect
        if(ref->IsReference() && static_cast<Reference*>(ref)->GetReferencedName() == "eval")
        {
            DirectEvalGuard guard;
            return obj->Call(e, this_value, arg_list);
        }
        else
        {
            return obj->Call(e, this_value, arg_list);
        }
    }

    // 11.2.1 Property Accessors
    inline JSValue* EvalIndexExpression(Error* e, JSValue* base_ref, const std::string& identifier_name, ValueGuard& guard)
    {
        JSValue* base_value = GetValue(e, base_ref);
        if(!e->IsOk())
        {
            return nullptr;
        }
        guard.AddValue(base_value);
        base_value->CheckObjectCoercible(e);
        if(!e->IsOk())
        {
            return nullptr;
        }
        bool strict = RuntimeContext::TopContext()->strict();
        return new Reference(base_value, identifier_name, strict);
    }

    inline JSValue* EvalIndexExpression(Error* e, JSValue* base_ref, AST* expr, ValueGuard& guard)
    {
        JSValue* property_name_ref = EvalExpression(e, expr);
        if(!e->IsOk())
        {
            return nullptr;
        }
        JSValue* property_name_value = GetValue(e, property_name_ref);
        if(!e->IsOk())
        {
            return nullptr;
        }
        std::string property_name_str = ToString(e, property_name_value);
        if(!e->IsOk())
        {
            return nullptr;
        }
        return EvalIndexExpression(e, base_ref, property_name_str, guard);
    }

    inline JSValue* EvalExpressionList(Error* e, AST* ast)
    {
        assert(ast->type() == AST::AST_EXPR);
        Expression* exprs = static_cast<Expression*>(ast);
        assert(!exprs->elements().empty());
        JSValue* val;
        for(AST* expr : exprs->elements())
        {
            JSValue* ref = EvalAssignmentExpression(e, expr);
            if(!e->IsOk())
            {
                return nullptr;
            }
            val = GetValue(e, ref);
            if(!e->IsOk())
            {
                return nullptr;
            }
        }
        return val;
    }

}// namespace es
