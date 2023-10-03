#include <utility>
#include "eval.h"

int Compile(std::string FileName)
{
	std::string Source = ReadFile(FileName);
	if (Source == "")
	{
		std::cout << "ERROR: File " << FileName << " not found." << std::endl;
		return -1;
	}
	std::cout << "Source:\n" << Source << std::endl << std::endl;

	// Tokenize the source code
	Lexer			   Lex(Source);
	std::vector<Token> Tokens = Lex.Tokenize();

	// Construct a syntax tree from the tokens
	AST			Ast(Tokens);
	ASTProgram* Program = Ast.GetTree();
	if (Program->Errors.size() == 0)
	{
		Evaluator* Evaluator;
		std::cout << "Tree:\n"
				  << "\x1B[32m" << Program->ToString() << "\033[0m" << std::endl;
	}
	else
	{
		for (const auto& E : Program->Errors)
		{
			std::cout << "\x1B[31m"
					  << "ERROR: " << E.ToString() << "\033[0m" << std::endl;
		}
	}

	return 0;
}

int main()
{
	return Compile("hello.p");
}