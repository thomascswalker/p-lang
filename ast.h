#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <variant>

#include "token.h"

// Base AST Node class
struct ASTNode
{
	virtual ~ASTNode(){};
	virtual std::string ToString() const = 0;
};

struct ASTLiteral : ASTNode
{
	int Value;

	ASTLiteral(int InValue) : Value(InValue){};
	virtual std::string ToString() const { return std::to_string(Value); }
};

struct ASTBinOp : ASTNode
{
	ASTNode*	Left;
	ASTNode*	Right;
	std::string Op;

	ASTBinOp(ASTNode* InLeft, ASTNode* InRight, const std::string& InOp) : Left(InLeft), Right(InRight), Op(InOp){};
	virtual std::string ToString() const { return "[" + Left->ToString() + " " + Op + " " + Right->ToString() + "]"; }
};

class AST
{
private:
	std::vector<Token>	  Tokens{};
	std::vector<ASTNode*> Nodes{};
	Token*				  CurrentToken;

	void Next()
	{
		auto End = &Tokens.back();
		if (CurrentToken == End)
		{
			CurrentToken = nullptr;
		}
		else
		{
			CurrentToken++;
		}
	}

	bool Match(const std::string& Value)
	{
		if (!CurrentToken)
		{
			return false;
		}
		return CurrentToken->Type == Value;
	}

	ASTNode* ParseLiteralExpr()
	{
		int Value = std::stoi(CurrentToken->Content);
		Next();
		Nodes.push_back(new ASTLiteral(Value));
		return Nodes.back();
	}

	ASTNode* ParseMultiplicativeExpr()
	{
		auto Expr = ParseLiteralExpr();
		while (Match("*") || Match("/"))
		{
			std::string Op = CurrentToken->Content;
			Next();
			Nodes.push_back(new ASTBinOp(Expr, ParseLiteralExpr(), Op));
			Expr = Nodes.back();
		}
		return Expr;
	}

	ASTNode* ParseAdditiveExpr()
	{
		auto Expr = ParseMultiplicativeExpr();
		while (Match("+") || Match("-"))
		{
			std::string Op = CurrentToken->Content;
			Next();
			Nodes.push_back(new ASTBinOp(Expr, ParseMultiplicativeExpr(), Op));
			Expr = Nodes.back();
		}
		return Expr;
	}

public:
	AST(std::vector<Token>& InTokens) : Tokens(InTokens) { CurrentToken = &Tokens[0]; }

	ASTNode* Parse()
	{
		ASTNode* ProgramResult = ParseAdditiveExpr();
		return ProgramResult;
	}
};