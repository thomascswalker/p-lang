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
	std::cout << "Source:\n" << Source << std::endl << std::endl;

	// Tokenize the source code
	Lexer Lex(Source);
	std::vector<Token> Tokens = Lex.Tokenize();

	// Construct a syntax tree from the tokens
	AST	 Ast(Tokens);
	auto Tree = Ast.Parse();
	std::cout << "Tree:" << std::endl;
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