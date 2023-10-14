#pragma once
#include "core.h"
#include "ast.h"

struct Variable
{
	std::string							  Name;
	std::variant<int, float, std::string> Value;

	Variable(const std::string& InName) : Name(InName){};
	Variable(const std::string& InName, const std::variant<int, float, std::string> InValue)
		: Name(InName), Value(InValue){};
};

class Evaluator
{
	std::vector<Variable*> Variables;

	Variable* GetVariable(const std::string& Name)
	{
		for (const auto& V : Variables)
		{
			if (V->Name == Name)
			{
				return V;
			}
		}
		return nullptr;
	}

	std::variant<int, float, std::string> EvalOp(std::variant<int, float, std::string>& Left,
												 std::variant<int, float, std::string>& Right, const char* Op)
	{
		std::variant<int, float, std::string> Result;

		switch (*Op)
		{
			case '+' :
				if (std::holds_alternative<int>(Left) && std::holds_alternative<int>(Right))
				{
					Result = std::get<int>(Left) + std::get<int>(Right);
				}
				else if (std::holds_alternative<float>(Left) && std::holds_alternative<float>(Right))
				{
					Result = std::get<float>(Left) + std::get<float>(Right);
				}
				else if (std::holds_alternative<std::string>(Left) && std::holds_alternative<std::string>(Right))
				{
					Result = std::get<std::string>(Left) + std::get<std::string>(Right);
				}
				break;
			case '-' :
				if (std::holds_alternative<int>(Left) && std::holds_alternative<int>(Right))
				{
					Result = std::get<int>(Left) - std::get<int>(Right);
				}
				else if (std::holds_alternative<float>(Left) && std::holds_alternative<float>(Right))
				{
					Result = std::get<float>(Left) - std::get<float>(Right);
				}
				break;
			case '*' :
				if (std::holds_alternative<int>(Left) && std::holds_alternative<int>(Right))
				{
					Result = std::get<int>(Left) * std::get<int>(Right);
				}
				else if (std::holds_alternative<float>(Left) && std::holds_alternative<float>(Right))
				{
					Result = std::get<float>(Left) * std::get<float>(Right);
				}
				break;
			case '/' :
				if (std::holds_alternative<int>(Left) && std::holds_alternative<int>(Right))
				{
					Result = std::get<int>(Left) / std::get<int>(Right);
				}
				else if (std::holds_alternative<float>(Left) && std::holds_alternative<float>(Right))
				{
					Result = std::get<float>(Left) / std::get<float>(Right);
				}
				break;
		}

		return Result;
	}

	std::variant<int, float, std::string> EvalBinOp(ASTBinOp* BinOp)
	{
		// Evaluate the left value
		std::variant<int, float, std::string> LeftValue;
		if (Cast<ASTBinOp>(BinOp->Left))
		{
			LeftValue = EvalBinOp(Cast<ASTBinOp>(BinOp->Left));
		}
		else if (Cast<ASTLiteral>(BinOp->Left))
		{
			auto Literal = Cast<ASTLiteral>(BinOp->Left);
			LeftValue = Literal->Value;
		}
		else if (Cast<ASTVariable>(BinOp->Left))
		{
			auto V = GetVariable(Cast<ASTVariable>(BinOp->Left)->Name);
			if (!V)
			{
				throw std::runtime_error("Variable referenced before assignment.");
			}
			LeftValue = V->Value;
		}
		else
		{
			throw std::runtime_error("Bad left value.");
		}

		// Evaluate the right value
		std::variant<int, float, std::string> RightValue;
		if (Cast<ASTBinOp>(BinOp->Right))
		{
			RightValue = EvalBinOp(Cast<ASTBinOp>(BinOp->Right));
		}
		else if (Cast<ASTLiteral>(BinOp->Right))
		{
			auto Literal = Cast<ASTLiteral>(BinOp->Right);
			RightValue = Literal->Value;
		}
		else if (Cast<ASTVariable>(BinOp->Right))
		{
			auto V = GetVariable(Cast<ASTVariable>(BinOp->Right)->Name);
			if (!V)
			{
				throw std::runtime_error("Variable referenced before assignment.");
			}
			RightValue = V->Value;
		}
		else
		{
			throw std::runtime_error("Bad right value.");
		}

		// Execute Left-Op-Right
		std::variant<int, float, std::string> Result = EvalOp(LeftValue, RightValue, BinOp->Op.c_str());

		return Result;
	}

	void EvalAssignment(ASTAssignment* Assignment)
	{
		// Look for an existing variable of this name
		Variable* V = GetVariable(Assignment->Name);

		// If it doesn't exist, make a new one
		if (!V)
		{
			Variables.push_back(new Variable{ Assignment->Name });
			V = Variables.back();
		}

		std::variant<int, float, std::string> Value;
		if (Cast<ASTLiteral>(Assignment->Right))
		{
			Value = Cast<ASTLiteral>(Assignment->Right)->Value;
		}
		else if (Cast<ASTBinOp>(Assignment->Right))
		{
			Value = EvalBinOp(Cast<ASTBinOp>(Assignment->Right));
		}

		V->Value = Value;
	}

public:
	ASTProgram* Program;
	Evaluator(ASTProgram* InProgram) : Program(InProgram){};

	void EvalNode(ASTNode* Node)
	{
		if (Cast<ASTBinOp>(Node))
		{
			EvalBinOp(Cast<ASTBinOp>(Node));
		}
		else if (Cast<ASTAssignment>(Node))
		{
			EvalAssignment(Cast<ASTAssignment>(Node));
		}
	}

	void EvalProgram()
	{
		for (const auto& Node : Program->Expressions)
		{
			EvalNode(Node);
		}
	}
};