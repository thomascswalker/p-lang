#include <utility>
#include "Public/Ast.h"

using namespace Core;
using namespace Values;
using namespace Logging;

int Compile(std::string FileName)
{
	std::string Source = ReadFile(FileName);
	if (Source == "")
	{
		Error("File not found or empty: {}", FileName);
		return -1;
	}
	std::cout << "Running " << FileName << "\n-----------" << std::endl;
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

	auto V = std::make_unique<Visitor>();
	Debug("Evaluating AST...");
	V->Visit(Program);
	Debug("Evaluation complete.");

	int ErrorCount = GetLogger()->GetCount(_Error);
	std::cout << std::format("Program compiled with {} errors.", ErrorCount) << std::endl;
	if (ErrorCount > 0)
	{
		auto ErrorMsgs = GetLogger()->GetMessages(_Error);
		for (auto Msg : ErrorMsgs)
		{
			std::cout << std::format("{}ERROR: {}{}", "\033[31m", Msg, "\033[0m") << std::endl;
		}
	}

	return 0;
}

// Main entrypoint
int main(int argc, char* argv[])
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
	auto Result = Compile(FileName);

	return Result;
}