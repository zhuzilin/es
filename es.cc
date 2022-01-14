#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include <es/parser/parser.h>
#include <es/enter_code.h>
#include <es/eval.h>
#include <es/helper.h>

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
  std::u16string filecontent = ReadUTF8FileToUTF16String(filename);

  es::Parser parser(filecontent);
  es::AST* ast = parser.ParseProgram();
  if (ast->IsIllegal()) {
    es::log::PrintSource("ParserError: ", ast->source());
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
    case es::Completion::THROW:
      std::cout << "(THROW, " << res.value->ToString() << ", )" << std::endl;
      break;
    default:
      std::cout << "(NORMAL, " << res.value->ToString() << ", )" << std::endl;
      break;
  } 
}
