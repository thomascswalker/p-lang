#include <utility>
#include "ast.h"

int Compile(std::string FileName)
{
	std::string Source = ReadFile(FileName);
	if (Source == "")
	{
		std::cout << "ERROR: File " << FileName << " not found." << std::endl;
		return -1;
	}

	Lexer Lex(Source);

	std::vector<Token> Tokens = Lex.Tokenize();

	AST	 Ast(Tokens);
	auto Tree = Ast.Parse();
	for (const auto& E : Tree)
	{
		std::cout << E->ToString() << std::endl;
	}

	return 0;
}

int main()
{
	return Compile("hello.p");
}