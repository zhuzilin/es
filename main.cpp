
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#if __has_include(<readline/readline.h>)
#include <readline/readline.h>
#include <readline/history.h>
#else
    #define NO_READLINE
#endif
#include "es.h"
#include "optionparser.h"

// NOTE(zhuzilin) There are some copy and paste from stackoverflow...
std::string ReadFile(const std::string& filename)
{
    // https://stackoverflow.com/a/2602258/5163915
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void print_result(const es::Completion& c)
{
    auto v = c.value;
}

bool execute(const std::string& code, es::Completion& res)
{
    es::Error* e;
    es::Parsing::AST* ast;
    es::Parsing::Parser parser(code);
    ast = parser.ParseProgram();
    if(ast->IsIllegal())
    {
        std::cout << "ParserError: " << es::log::ToString(ast->source()) << std::endl;
        return false;
    }
    e = es::Error::Ok();
    es::EnterGlobalCode(e, ast);
    if(!e->IsOk())
    {
        std::cout << "enter global failed" << std::endl;
        return false;
    }
    res = es::EvalProgram(ast);
    switch(res.type)
    {
        case es::Completion::THROWING:
            {
                std::cout << "Uncaught ";
                if(res.value->IsObject())
                {
                    auto obj = static_cast<es::JSObject*>(res.value);
                    if(obj->obj_type() == es::JSObject::OBJ_ERROR)
                    {
                        es::ErrorObject* error = static_cast<es::ErrorObject*>(obj);
                        switch(error->ErrorType())
                        {
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
                std::cout << "Error: " << res.value->ToString() << std::endl;
                return false;
            }
            break;
        default:
            {
            }
            break;
    }
    return true;
}


#if !defined(NO_READLINE)
/*
* returns true if $line is more than just space.
* used to ignore blank lines.
*/
static bool notjustspace(const char* line)
{
    int c;
    size_t i;
    for(i=0; (c = line[i]) != 0; i++)
    {
        if(!isspace(c))
        {
            return true;
        }
    }
    return false;
}

int do_repl()
{
    size_t varid;
    size_t nowid;
    char* line;
    std::string exeme;
    std::string varname;
    es::Completion res;
    varid = 0;
    while(true)
    {
        line = readline ("> ");
        fflush(stdout);
        if(line == NULL)
        {
            fprintf(stderr, "readline() returned NULL\n");
        }
        if((line != NULL) && (line[0] != '\0') && notjustspace(line))
        {
            exeme = line;
            add_history(line);
            if(execute(exeme, res))
            {
                nowid = varid;
                varid++;
                print_result(res);
            }

        }
    }
}
#endif

int main(int argc, char* argv[])
{
    bool alsoprint;
    bool forcerepl;
    bool havecodechunk;
    std::string filename;
    std::string codechunk;
    es::Completion res;
    alsoprint = false;
    forcerepl = false;
    havecodechunk = false;
    OptionParser prs;

    prs.on({"-i", "--repl"}, "force run REPL", [&]
    {
        forcerepl = true; 
    });
    prs.on({"-e?", "--eval=?"}, "evaluate <arg>, then exit", [&](const auto& v)
    {
        codechunk = v.str();
        havecodechunk = true;
        prs.stopParsing();
    });
    prs.on({"-p", "--print"}, "when using '-e', print the result", [&]
    {
        alsoprint = true;
    });
    try
    {
        prs.parse(argc, argv);
    }
    catch(OptionParser::Error& e)
    {
        std::cerr << "failed to process options: " << e.what() << std::endl;
        return 1;
    }
    auto rest = prs.positional();
    es::Init();
    if(havecodechunk)
    {
        if(!execute(codechunk, res))
        {
            return 1;
        }
        return 0;
    }
    else
    {
        if(rest.size() > 0)
        {
            std::string filename(argv[1]);
            auto content = ReadFile(filename);
            if(!execute(content, res))
            {
                return 1;
            }
            return 0;
        }
        else
        {
            #if !defined(NO_READLINE)
                do_repl();
            #else
                std::cerr << "no readline, so no repl" << std::endl;
            #endif
        }
    }
    return 0;
}
