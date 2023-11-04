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

	//TLiteral  L(5.48320f);
	//TVariable A("A", { 1, 2, 3, {1,2,3} });
	//TVariable B("B", { 4, 5, 6 });
	//TVariable C("C", { 7, 8, 9 });
	//C.GetValue()->AsArray()->Append(TStringValue("test!"));

	//auto F = TFunction(&BuiltIns::PrintInternal);

	//TArguments Args{ &A, &B, &C, &L };
	//F.Invoke(Args);

	return Result;
}