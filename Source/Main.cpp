#include "Public/Ast.h"
#include <string>
#include <iostream>

using namespace Core;
using namespace Values;
using namespace Logging;

int Compile(std::string FileName)
{
    std::string Source = ReadFile(FileName);
    if (Source.empty())
    {
        Error("File not found or empty: {}", FileName);
        return -1;
    }

    // Tokenize the source code
    Lexer Lex(Source);
    std::vector<Token> Tokens = Lex.Tokenize();

    // Construct a syntax tree from the tokens
    Ast Ast(Tokens);
    AstBody* Program = Ast.GetTree();

    auto V = Visitor();
    V.Visit(Program);
    
    int ErrorCount = GetLogger()->GetCount(LogLevel::Error);
    std::cout << std::format("Program compiled with {} errors.", ErrorCount) << '\n';
    if (ErrorCount > 0)
    {
        for (auto Msg : GetLogger()->GetMessages(LogLevel::Error))
        {
            std::cout << std::format("{}ERROR: {}{}", "\033[31m", Msg, "\033[0m") << '\n';
        }
    }

    return 0;
}

int Interpret()
{
    int Result = 0;
    Visitor V = Visitor();
    printf("Penguin Interpreter\nType below and press enter to run commands.\n");
    while (true)
    {
        printf(">>> "); // Decorator for each input line
        std::string Line;
        std::getline(std::cin, Line);

        // Tokenize the source code
        Lexer Lex(Line);
        std::vector<Token> Tokens = Lex.Tokenize();
        if (Tokens.empty())
        {
            Error("Zero tokens");
            break;
        }

        // Construct a syntax tree from the tokens
        Ast Ast(Tokens);
        const AstBody* Program = Ast.GetTree();

        V.Visit(Program);

        for (const std::string& Msg : GetLogger()->GetMessages(LogLevel::Error))
        {
            std::cout << std::format("{}ERROR: {}{}", "\033[31m", Msg, "\033[0m") << '\n';
        }
        GetLogger()->Clear();
    }

    return Result;
}

// Main entrypoint
int main(int argc, char* argv[])
{
    int Result;
    if (argc == 1)// [1]cmd
    {
        Result = Interpret();
    }
    else if (argc == 2) // [1]cmd [2]<filename>.p
    {
        const std::string FileName = argv[1];
        Result = Compile(FileName);
    }
    else
    {
        printf("Invalid argument count.");
        return -1;
    }
    
    std::cout << "Press ENTER to exit.\n";
    return Result;
}
