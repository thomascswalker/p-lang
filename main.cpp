#include <utility>

#include "ast.h"

using namespace Core;

int Compile(std::string FileName)
{
	std::string Source = ReadFile(FileName);
	if (Source == "")
	{
		std::cout << "\x1B[31m"
				  << "ERROR: File not found or empty: " << FileName << "\033[0m" << std::endl;
		return -1;
	}
	std::cout << "Source:\n\"\"\"\n" << Source << "\n\"\"\"" << std::endl << std::endl;

	// Tokenize the source code
	Lexer			   Lex(Source);
	std::vector<Token> Tokens = Lex.Tokenize();

	// Construct a syntax tree from the tokens
	AST			Ast(Tokens);
	ASTBody* Program = Ast.GetTree();
	if (Program->Errors.size() == 0)
	{
		auto V = std::make_unique<Visitor>();
		V->Visit(Program);

		std::cout << "Variables:\n"
				  << "\x1B[36m";
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
		std::cout << "\x1B[33m";
	}
	else
	{
		for (const auto& E : Program->Errors)
		{
			std::cout << "\x1B[31m"
					  << "ERROR: " << E.ToString() << "\033[0m" << std::endl;
		}
		return 1;
	}

	return 0;
}

int main()
{
	auto Result = Compile("example.p");
	return Result;
}