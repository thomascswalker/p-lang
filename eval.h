#pragma once
#include "ast.h"

class Evaluator
{
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
			LeftValue = Literal->GetInt();
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
			RightValue = Literal->GetInt();
		}
		else
		{
			throw std::runtime_error("Bad right value.");
		}

		// Execute Left-Op-Right
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
	int Eval(ASTNode* Node) { return EvalBinOp(Node); }

	void EvalNode(ASTNode* Node) {}
};