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
		V->Dump();
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

	//auto A = TFloatValue(5.34f);
	//auto B = TIntValue(3);

	//auto C = A == B;
	//std::cout << (std::string)C << std::endl;

	//auto StringA = TStringValue("Test");
	//auto StringB = TStringValue("34234");
	//std::cout << (std::string)(StringA + StringB) << std::endl;


	return Result;
}