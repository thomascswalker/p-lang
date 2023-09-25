#include <utility>
#include "ast.h"

int Compile(std::string FileName)
{
	std::cout << "Reading source file: " << FileName << std::endl;
	std::string Source = ReadFile(FileName);
	if (Source == "")
	{
		std::cout << "ERROR: File " << FileName << " not found." << std::endl;
		return -1;
	}

	std::cout << "Evaluating: " << Source << std::endl;

	Lexer Lex(Source);

	std::vector<Token> Tokens = Lex.Tokenize();
	std::cout << "Constructed " << Tokens.size() << " tokens" << std::endl;

	AST	 Ast(Tokens);
	auto Tree = Ast.Parse();
	if (Tree)
	{
		std::cout << "Tree: " << Tree->ToString() << std::endl;
		int Result = Ast.Eval(Tree);
		std::cout << "Result: " << Result << std::endl;
	}

	return 0;
}

int main()
{
	return Compile("hello.p");
}