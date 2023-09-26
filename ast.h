#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <variant>

#include "token.h"

// Base AST Node class
class ASTNode
{
public:
	virtual ~ASTNode(){};
	virtual std::string ToString() const = 0;
};

class ASTLiteral : public ASTNode
{
public:
	int Value;
	ASTLiteral(int InValue) : Value(InValue){};
	virtual std::string ToString() const { return std::to_string(Value); }
};

class ASTBinOp : public ASTNode
{
public:
	ASTNode*	Left;
	ASTNode*	Right;
	std::string Op;
	ASTBinOp(ASTNode* InLeft, ASTNode* InRight, const std::string& InOp) : Left(InLeft), Right(InRight), Op(InOp){};
	virtual std::string ToString() const { return "(" + Left->ToString() + " " + Op + " " + Right->ToString() + ")"; }
};

class AST
{
private:
	std::vector<Token>	  Tokens{};
	std::vector<ASTNode*> Nodes{};
	Token*				  CurrentToken;

	void Next()
	{
		Token* End = &Tokens.back();
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
		ASTNode* Expr = ParseLiteralExpr();
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
		ASTNode* Expr = ParseMultiplicativeExpr();
		while (Match("+") || Match("-"))
		{
			std::string Op = CurrentToken->Content;
			Next();
			Nodes.push_back(new ASTBinOp(Expr, ParseMultiplicativeExpr(), Op));
			Expr = Nodes.back();
		}
		return Expr;
	}

	int EvalBinOp(ASTNode* Node)
	{
		ASTBinOp* BinOp = dynamic_cast<ASTBinOp*>(Node);
		if (!BinOp)
		{
			throw std::runtime_error("Invalid node.");
		}

		// Evaluate the left value
		int LeftValue;
		if (dynamic_cast<ASTBinOp*>(BinOp->Left))
		{
			LeftValue = EvalBinOp(BinOp->Left);
		}
		else if (dynamic_cast<ASTLiteral*>(BinOp->Left))
		{
			auto Literal = dynamic_cast<ASTLiteral*>(BinOp->Left);
			LeftValue = Literal->Value;
		}
		else
		{
			throw std::runtime_error("Bad left value.");
		}

		// Evaluate the right value
		int RightValue;
		if (dynamic_cast<ASTBinOp*>(BinOp->Right))
		{
			RightValue = EvalBinOp(BinOp->Right);
		}
		else if (dynamic_cast<ASTLiteral*>(BinOp->Right))
		{
			auto Literal = dynamic_cast<ASTLiteral*>(BinOp->Right);
			RightValue = Literal->Value;
		}
		else
		{
			throw std::runtime_error("Bad right value.");
		}

		switch (*BinOp->Op.c_str())
		{
			case '+' :
				return LeftValue + RightValue;
			case '-' :
				return LeftValue - RightValue;
			case '*' :
				return LeftValue * RightValue;
			case '/' :
				return LeftValue / RightValue;
			default :
				throw std::runtime_error("Unsupported operator.");
		}
	}

public:
	AST(std::vector<Token>& InTokens) : Tokens(InTokens) { CurrentToken = &Tokens[0]; }

	ASTNode* Parse()
	{
		ASTNode* ProgramResult = ParseAdditiveExpr();
		return ProgramResult;
	}

	int Eval(ASTNode* Node) { return EvalBinOp(Node); }
};