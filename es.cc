#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <codecvt>
#include <locale>

#include <es/parser/parser.h>
#include <es/types/property_descriptor_object_conversion.h>
#include <es/enter_code.h>
#include <es/eval.h>
#include <es/utils/helper.h>
#include <es/gc/heap.h>
#include <es/impl.h>

using namespace es;

// NOTE(zhuzilin) There are some copy and paste from stackoverflow...
std::u16string ReadUTF8FileToUTF16String(std::string filename) {
  // https://stackoverflow.com/a/2602258/5163915
  std::ifstream file(filename);
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string utf8_file_content = buffer.str();

  // https://stackoverflow.com/a/7235204/5163915
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
  std::u16string utf16_file_content = convert.from_bytes(utf8_file_content);

  return utf16_file_content;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "no filename presented" << "\n";
    return 0;
  }
  std::string filename(argv[1]);
  std::u16string source = ReadUTF8FileToUTF16String(filename);

  Parser parser(source);
  AST* ast = parser.ParseProgram();
  if (ast->IsIllegal()) {
    size_t start = ast->start();
    for (size_t i = 0; i < 10; i++) {
      if (start == 0 || character::IsLineTerminator(source[start - 1]))
        break;
      start--;
    }
    size_t end = ast->end();
    for (size_t i = 0; i < 10; i++) {
      if (end == source.size() || character::IsLineTerminator(source[end]))
        break;
      end++;
    }
    std::cout << "\033[1;31m" << "ParserError: " << "\033[0m"
              << log::ToString(source.substr(start, ast->start() - start))
              << "\033[1;31m" << log::ToString(ast->source()) << "\033[0m"
              << log::ToString(source.substr(ast->end(), end - ast->end())) << "\n";
    return 0;
  }

  Init();
  JSValue e = error::Ok();
  EnterGlobalCode(e, ast);
  if (unlikely(!error::IsOk(e))) {
    std::cout << "\033[1;31m" << "Enter global failed: " << JSValue::ToString(e) << "\033[0m" << "\n";
    return 0;
  }
  Completion res = EvalProgram(ast);
  switch (res.type()) {
    case Completion::THROW: {
      std::cout << "\033[1;31m" << "Uncaught ";
      JSValue val;
      if (res.value().IsError()) {
        JSValue error = res.value();
        val = error::value(error);
        switch (error::error_type(error)) {
          case error::E_EVAL:
            std::cout << "EvalError: ";
            break;
          case error::E_RANGE:
            std::cout << "RangeError: ";
            break;
          case error::E_REFERENCE:
            std::cout << "ReferenceError: ";
            break;
          case error::E_SYNTAX:
            std::cout << "SyntaxError: ";
            break;
          case error::E_TYPE:
            std::cout << "TypeError: ";
            break;
          case error::E_URI:
            std::cout << "URIError: ";
            break;
          default:
            break;
        }
      } else {
        val = res.value();
      }
      log::Debugger::TurnOff();
      JSValue e = error::Ok();
      JSValue msg = ToString(e, val);
      if (unlikely(!error::IsOk(e)))
        std::cout << JSValue::ToString(val) << "\033[0m" << "\n";
      else
        std::cout << log::ToString(string::data(msg)) << "\033[0m" << "\n";
      break;
    }
    default:
      break;
  } 
}
