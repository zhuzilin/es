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
    std::cout << "no filename presented" << std::endl;
    return 0;
  }
  std::string filename(argv[1]);
  std::u16string source = ReadUTF8FileToUTF16String(filename);

  es::Parser parser(source);
  es::AST* ast = parser.ParseProgram();
  if (ast->IsIllegal()) {
    size_t start = ast->start();
    for (size_t i = 0; i < 10; i++) {
      if (start == 0 || es::character::IsLineTerminator(source[start - 1]))
        break;
      start--;
    }
    size_t end = ast->end();
    for (size_t i = 0; i < 10; i++) {
      if (end == source.size() || es::character::IsLineTerminator(source[end]))
        break;
      end++;
    }
    std::cout << "\033[1;31m" << "ParserError: " << "\033[0m"
              << es::log::ToString(source.substr(start, ast->start() - start))
              << "\033[1;31m" << es::log::ToString(ast->source()) << "\033[0m"
              << es::log::ToString(source.substr(ast->end(), end - ast->end())) << std::endl;
    return 0;
  }

  es::Init();
  es::Error* e = es::Error::Ok();
  es::EnterGlobalCode(e, ast);
  if (!e->IsOk()) {
    std::cout << "enter global failed" << std::endl;
    return 0;
  }
  es::Completion res = es::EvalProgram(ast);
  switch (res.type) {
    case es::Completion::THROW: {
      std::cout << "\033[1;31m" << "Uncaught ";
      if (res.value->IsObject()) {
        es::JSObject* obj = static_cast<es::JSObject*>(res.value);
        if (obj->obj_type() == es::JSObject::OBJ_ERROR) {
          es::ErrorObject* error = static_cast<es::ErrorObject*>(obj);
          switch (error->ErrorType()) {
            case es::Error::E_EVAL:
              std::cout << "Eval";
              break;
            case es::Error::E_RANGE:
              std::cout << "Range";
              break;
            case es::Error::E_REFERENCE:
              std::cout << "Reference";
              break;
            case es::Error::E_SYNTAX:
              std::cout << "Syntax";
              break;
            case es::Error::E_TYPE:
              std::cout << "Type";
              break;
            case es::Error::E_URI:
              std::cout << "URI";
              break;
            default:
              break;
          }
        }
      }
      std::cout << "Error: " << res.value->ToString() << "\033[0m" << std::endl;
      break;
    }
    default:
      break;
  } 
}
