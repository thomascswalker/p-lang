#include "Public/Ast.h"

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
    std::cout << "Running " << FileName << "\n-----------" << '\n';
    Debug("\n{}\n", Source);

    // Tokenize the source code
    Lexer Lex(Source);
    Debug("Lexing tokens...");
    std::vector<Token> Tokens = Lex.Tokenize();
    Debug("Lexing complete.");

    // Construct a syntax tree from the tokens
    Debug("Constructing AST...");
    AST Ast(Tokens);
    Debug("AST constructed.");
    ASTBody* Program = Ast.GetTree();

    auto V = Visitor();
    Debug("Evaluating AST...");
    V.Visit(Program);
    Debug("Evaluation complete.");

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

// Main entrypoint
int main(int argc, char* argv[]) // ReSharper disable
{
    if (argc == 1)
    {
        return -1;
    }
    std::string FileName;
    if (argc == 2) // [1]cmd [2]<filename>.p
    {
        FileName = argv[1];
    }
    else
    {
        printf("Invalid argument. Please enter a filename.");
        return -1;
    }
    const auto Result = Compile(FileName);

    return Result;
}
