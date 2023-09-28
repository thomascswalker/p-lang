#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <variant>

#include "token.h"

int StringToInt(const std::string& String)
{
	int Out = 0;
	for (const auto& C : String)
	{
		Out = (Out * 10) + (C - 48);
	}
	return Out;
}

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
	std::string							  Type = "Unknown";
	std::variant<int, float, std::string> Value;

	ASTLiteral(std::variant<int, float, std::string>& InValue) : Value(InValue)
	{
		if (IsInt())
		{
			Type = "Int";
		}
		else if (IsFloat())
		{
			Type = "Float";
		}
		else if (IsString())
		{
			Type = "String";
		}
	}
	ASTLiteral(int InValue) : Value(InValue) { Type = "Int"; };
	ASTLiteral(float InValue) : Value(InValue) { Type = "Float"; };
	ASTLiteral(const std::string& InValue) : Value(InValue) { Type = "String"; };

	bool IsInt() const { return std::holds_alternative<int>(Value); }
	bool IsFloat() const { return std::holds_alternative<float>(Value); }
	bool IsString() const { return std::holds_alternative<std::string>(Value); }

	int			GetInt() const { return std::get<int>(Value); }
	float		GetFloat() const { return std::get<float>(Value); }
	std::string GetString() const { return std::get<std::string>(Value); }

	virtual std::string ToString() const
	{
		std::string Result;
		if (IsInt())
		{
			Result = std::to_string(GetInt());
		}
		else if (IsFloat())
		{
			Result = std::to_string(GetFloat());
		}
		else if (IsString())
		{
			Result = "\"" + GetString() + "\"";
		}
		else
		{
			Result = "Unknown";
		}

		return Result;
	}
};

class ASTVariable : public ASTNode
{
public:
	std::string Name;
	ASTVariable(const std::string& InName) : Name(InName){};
	virtual std::string ToString() const { return "Variable: " + Name; }
};

class ASTBinOp : public ASTNode
{
public:
	ASTNode*	Left;
	ASTNode*	Right;
	std::string Op;
	ASTBinOp(ASTNode* InLeft, ASTNode* InRight, const std::string& InOp) : Left(InLeft), Right(InRight), Op(InOp){};
	virtual std::string ToString() const
	{
		return "\"BinOp\": {\"Left: " + Left->ToString() + ", \"Op: \"" + Op + "\", Right: " + Right->ToString() + "}";
	}
};

class ASTAssignment : public ASTNode
{
public:
	std::string Type;
	std::string Name;
	ASTNode*	Right;

	ASTAssignment(const std::string& InType, const std::string& InName, ASTNode* InRight)
		: Type(InType), Name(InName), Right(InRight){};
	virtual std::string ToString() const { return "Assign: (" + Type + ") " + Name + " = " + Right->ToString(); }
};

class AST
{
private:
	std::vector<Token>	  Tokens{};
	std::vector<ASTNode*> Nodes{};
	std::vector<ASTNode*> Expressions{};
	Token*				  CurrentToken;

	void Accept()
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

	bool Expect(const std::string& Type)
	{
		if (!CurrentToken)
		{
			return false;
		}
		return CurrentToken->Type == Type;
	}

	ASTNode* ParseFactorExpr()
	{
		if (Expect("Number"))
		{
			std::variant<int, float, std::string> Value;

			// Parse float

			if (CurrentToken->Content.find(".") != std::string::npos)
			{
				Value = std::stof(CurrentToken->Content);
			}
			// Parse int
			else
			{
				Value = std::stoi(CurrentToken->Content);
			}
			Accept();
			Nodes.push_back(new ASTLiteral(Value));
			return Nodes.back();
		}
		else if (Expect("String"))
		{
			std::string String = CurrentToken->Content;
			Accept();
			Nodes.push_back(new ASTLiteral(String));
			return Nodes.back();
		}
		else if (Expect("Name"))
		{
			std::string Name = CurrentToken->Content;
			Accept();
			Nodes.push_back(new ASTVariable(Name));
			return Nodes.back();
		}
		else
		{
			return nullptr;
		}
	}

	ASTNode* ParseMultiplicativeExpr()
	{
		ASTNode* Expr = ParseFactorExpr();
		while (Expect("*") || Expect("/"))
		{
			std::string Op = CurrentToken->Content;
			Accept();
			Nodes.push_back(new ASTBinOp(Expr, ParseFactorExpr(), Op));
			Expr = Nodes.back();
		}
		return Expr;
	}

	ASTNode* ParseAdditiveExpr()
	{
		ASTNode* Expr = ParseMultiplicativeExpr();
		while (Expect("+") || Expect("-"))
		{
			std::string Op = CurrentToken->Content;
			Accept();
			Nodes.push_back(new ASTBinOp(Expr, ParseMultiplicativeExpr(), Op));
			Expr = Nodes.back();
		}
		return Expr;
	}

	ASTNode* ParseAssignment()
	{
		if (Expect("Type"))
		{
			std::string Type = CurrentToken->Content;
			Accept();
			if (Expect("Name"))
			{
				std::string Name = CurrentToken->Content;
				Accept();
				if (Expect("="))
				{
					Accept();
					ASTNode* Expr = ParseAdditiveExpr();
					Nodes.push_back(new ASTAssignment(Type, Name, Expr));
					return Nodes.back();
				}
			}
		}

		return nullptr;
	}

	ASTNode* ParseExpr()
	{
		ASTNode* Expr;
		if (Expect("Type"))
		{
			Expr = ParseAssignment();
		}
		else if (Expect("Number") || Expect("String"))
		{
			Expr = ParseAdditiveExpr();
		}
		else
		{
			throw std::runtime_error("Syntax error.");
		}

		if (Expect(";"))
		{
			Accept();
			return Expr;
		}
		else
		{
			throw std::runtime_error("Missing semicolon.");
		}
	}

	std::vector<ASTNode*> ParseProgram()
	{
		while (CurrentToken != NULL && CurrentToken != &Tokens.back())
		{
			auto Expr = ParseExpr();
			Expressions.push_back(Expr);
		}
		return Expressions;
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
	AST(std::vector<Token>& InTokens) : Tokens(InTokens) { CurrentToken = &Tokens[0]; }

	std::vector<ASTNode*> Parse()
	{
		std::vector<ASTNode*> ProgramResult = ParseProgram();
		return ProgramResult;
	}

	int Eval(ASTNode* Node) { return EvalBinOp(Node); }
};