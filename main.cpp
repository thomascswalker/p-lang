#include <utility>
#include "ast.h"

using namespace Core;
using namespace Values;

int Compile(std::string FileName)
{
	std::string Source = ReadFile(FileName);
	if (Source == "")
	{
		Error(std::format("ERROR: File not found or empty: {}", FileName));
		return -1;
	}
	std::cout << "Running " << FileName << "\n-----------" << std::endl;
	Debug("\n" + Source + "\n");

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
	if (!Program->Succeeded())
	{
		Error("Compilation failed.");
		for (const auto& E : Program->Errors)
		{
			Error(E);
		}
		return 1;
	}

	auto V = std::make_unique<Visitor>();
	Debug("Evaluating AST...");
	V->Visit(Program);
	Debug("Evaluation complete.");
	if (!V->Succeeded())
	{
		Error("Compilation failed.");
		for (const auto& E : V->Errors)
		{
			Error(E);
		}
		return 1;
	}
	else
	{
		Debug("Compilation successful.");
		// V->Dump();
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