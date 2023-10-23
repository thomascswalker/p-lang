#include <utility>

#include "ast.h"

using namespace Core;

int Compile(std::string FileName)
{
	std::string Source = ReadFile(FileName);
	if (Source == "")
	{
		Error(std::format("ERROR: File not found or empty: {}", FileName));
		return -1;
	}
	std::cout << "Source:\n\"\"\"\n" << Source << "\n\"\"\"" << std::endl << std::endl;

	// Tokenize the source code
	Lexer			   Lex(Source);
	std::vector<Token> Tokens = Lex.Tokenize();

	// Construct a syntax tree from the tokens
	AST		 Ast(Tokens);
	ASTBody* Program = Ast.GetTree();
	if (Program->Errors.size() == 0)
	{
		auto V = std::make_unique<Visitor>();
		V->Visit(Program);

		for (const auto& Var : V->Variables)
		{
			if (std::holds_alternative<int>(Var.second))
			{
				std::cout << Var.first << " : " << std::to_string(std::get<int>(Var.second)) << std::endl;
			}
			else if (std::holds_alternative<float>(Var.second))
			{
				std::cout << Var.first << " : " << std::to_string(std::get<float>(Var.second)) << std::endl;
			}
			else if (std::holds_alternative<std::string>(Var.second))
			{
				std::cout << Var.first << " : \"" << std::get<std::string>(Var.second) << "\"" << std::endl;
			}
			else if (std::holds_alternative<bool>(Var.second))
			{
				std::cout << Var.first << " : " << (std::get<bool>(Var.second) ? "true" : "false") << std::endl;
			}
		}
	}
	else
	{
		for (const auto& E : Program->Errors)
		{
			Error(E.ToString());
		}
		return 1;
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
#ifdef _DEBUG
	FileName = "example.p";
#else
	if (argc == 2) // [1]cmd [2]<filename>.p
	{
		FileName = argv[1];
	}
	else
	{
		printf("Invalid argument. Please enter a filename.");
		return -1;
	}
#endif
	auto Result = Compile(FileName);
	return Result;
}