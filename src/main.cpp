#include "nex_lexer.hpp"
#include "nex_parser.hpp"
#include "nex_resolver.hpp"
#include "nex_interpreter.hpp"

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sstream>
#include <type_traits>

using namespace nex::ast;

int main(int argc, const char** argv)
{
    if (argc < 2) {
        std::wcout << L"Nex Lang Version 0.1" << std::endl;
        auto interp = std::make_shared<nex::Interpreter>();
        while (true) {
            std::wcout << "$ ";
            std::wstring line;
            std::getline(std::wcin, line);

            auto stream = std::wistringstream(line);
            nex::Lexer lex(stream);
            auto tokens = lex.scan();

            if (lex.error()) {
                continue;
            }

            nex::Parser parser(tokens);
            auto stmts = parser.parse();

            if (parser.error()) {
                continue;
            }

            auto resolver = std::make_shared<nex::Resolver>(interp);
            resolver->resolve(stmts);

            if (resolver->error()) {
                exit(65);
            }

            interp->interpret(stmts);
        }

        exit(0);
    }

    std::wifstream src;
    src.open(argv[1]);

    if (!src.is_open()) {
        std::cout << "nexc: " << "error: no such file "
            << argv[1] << std::endl;
        exit(10);
    }

    nex::Lexer lex(src);
    auto tokens = lex.scan();

    if (lex.error()) {
        exit(65);
    }

    nex::Parser parser(tokens);
    auto stmts = parser.parse();

    if (parser.error()) {
        exit(65);
    }

    auto interp = std::make_shared<nex::Interpreter>();
    auto resolver = std::make_shared<nex::Resolver>(interp);
    resolver->resolve(stmts);

    if (resolver->error()) {
        exit(65);
    }

    interp->interpret(stmts);

    return 0;
}
