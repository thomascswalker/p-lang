#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <variant>
#include <typeinfo>
#include <format>

#include "token.h"
#include "core.h"

using Variant = std::variant<int, float, std::string>;

void VariantAdd(const Variant& Left, const Variant& Right, Variant& Value);
void VariantSub(const Variant& Left, const Variant& Right, Variant& Value);
void VariantMul(const Variant& Left, const Variant& Right, Variant& Value);
void VariantDiv(const Variant& Left, const Variant& Right, Variant& Value);

class VisitorBase;
class Visitor;

class ASTNode;
class ASTLiteral;
class ASTVariable;
class ASTBinOp;
class ASTAssignment;
class ASTProgram;

class VisitorBase
{
public:
	virtual void Visit(ASTLiteral* Node) = 0;
	virtual void Visit(ASTVariable* Node) = 0;
	virtual void Visit(ASTBinOp* Node) = 0;
	virtual void Visit(ASTAssignment* Node) = 0;
	virtual void Visit(ASTProgram* Node) = 0;
};

class Visitor : public VisitorBase
{
	void	Push(const Variant& V);
	Variant Pop();
	bool	IsVariable(const std::string& Name);

public:
	std::map<std::string, Variant> Variables;
	std::vector<Variant>		   Stack;

	Visitor(){};
	Visitor(Visitor& Other)
	{
		Variables = Other.Variables;
		Stack = Other.Stack;
	}
	Visitor(const Visitor& Other)
	{
		Variables = Other.Variables;
		Stack = Other.Stack;
	}
	void Visit(ASTLiteral* Node) override;
	void Visit(ASTVariable* Node) override;
	void Visit(ASTBinOp* Node) override;
	void Visit(ASTAssignment* Node) override;
	void Visit(ASTProgram* Node) override;
};

struct ASTError
{
	std::string Msg;
	int			Line;
	int			Pos;

	ASTError(const std::string& InMsg, int InLine, int InPos) : Msg(InMsg), Line(InLine), Pos(InPos){};
	std::string ToString() const
	{
		return (Msg + " (line " + std::to_string(Line) + ", pos " + std::to_string(Pos) + ")");
	}
};

// Base AST Node class
class ASTNode
{
public:
	virtual ~ASTNode(){};
	virtual void		Accept(Visitor& V) = 0;
	virtual std::string ToString() const = 0;
};

class ASTLiteral : public ASTNode
{
public:
	std::string Type = "Unknown";
	Variant		Value;

	ASTLiteral(Variant& InValue) : Value(InValue)
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

	void Accept(Visitor& V) override { V.Visit(this); }

	bool IsInt() const { return std::holds_alternative<int>(Value); }
	bool IsFloat() const { return std::holds_alternative<float>(Value); }
	bool IsString() const { return std::holds_alternative<std::string>(Value); }

	int			GetInt() const { return std::get<int>(Value); }
	float		GetFloat() const { return std::get<float>(Value); }
	std::string GetString() const { return std::get<std::string>(Value); }

	template <typename T>
	T GetValue() const
	{
		return std::get<T>(Value);
	}

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
	void				Accept(Visitor& V) override { V.Visit(this); }
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

	void Accept(Visitor& V) override { V.Visit(this); }
};

class ASTAssignment : public ASTNode
{
public:
	std::string Type;
	std::string Name;
	ASTNode*	Right;

	ASTAssignment(const std::string& InType, const std::string& InName, ASTNode* InRight)
		: Type(InType), Name(InName), Right(InRight){};
	virtual std::string ToString() const
	{
		return "Assign: (" + Type + ") " + Name + " => {" + Right->ToString() + "}";
	}

	void Accept(Visitor& V) override { V.Visit(this); }
};

class ASTProgram : public ASTNode
{
public:
	std::vector<ASTError> Errors;
	std::vector<ASTNode*> Expressions;
	ASTProgram(){};
	virtual std::string ToString() const
	{

		std::string Out;
		for (const auto& E : Expressions)
		{
			Out += E->ToString() + "\n";
		};
		return Out;
	}
	void Accept(Visitor& V) override { V.Visit(this); }
};

class AST
{
private:
	ASTProgram*			  Program;
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
			auto Content = CurrentToken->Content;
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

	bool Peek(const std::string& Type, int Index = 0)
	{
		if (!CurrentToken)
		{
			return false;
		}
		Token* PeekToken = &Tokens.back() + Index;
		if (!PeekToken)
		{
			return false;
		}
		return PeekToken->Type == Type;
	}

	ASTNode* ParseFactorExpr()
	{
		// Parse numbers (floats and ints)
		if (Expect("Number"))
		{
			Variant Value;

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
		// Parse strings
		else if (Expect("String"))
		{
			std::string String = CurrentToken->Content;
			Accept();
			Nodes.push_back(new ASTLiteral(String));
			return Nodes.back();
		}
		// Parse names (Variables, functions, etc.)
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

	ASTNode* ParseEncapsulatedExpr()
	{
		ASTNode* Expr = ParseAdditiveExpr();
		return Expr;
	}

	ASTNode* ParseAssignment()
	{
		std::string Type;
		std::string Name;

		// Handle new variable assignments
		if (Expect("Type"))
		{
			Type = CurrentToken->Content;
			Accept();
			if (!Expect("Name"))
			{
				return nullptr;
			}
		}

		// Handle existing variable assignment, or continue with a new assignment
		Name = CurrentToken->Content;
		Accept();
		if (Expect("="))
		{
			Accept();
			ASTNode* Expr = ParseEncapsulatedExpr();
			Nodes.push_back(new ASTAssignment(Type, Name, Expr));
			return Nodes.back();
		}

		return nullptr;
	}

	ASTNode* ParsePrimaryExpr()
	{
		ASTNode* Expr;

		// int MyVar = 5;
		// MyVar = 5;
		if (Expect("Type") || Expect("Name"))
		{
			Expr = ParseAssignment();
		}
		// 5 + 10;
		// "Test" + "String";
		else if (Expect("Number") || Expect("String"))
		{
			Expr = ParseAdditiveExpr();
		}
		else
		{
			Error("Syntax Error");
			return nullptr;
		}

		// At this point, we should be at the end of the expression, and there should
		// be a semicolon
		if (Expect(";"))
		{
			Accept();
			return Expr ? Expr : nullptr;
		}
		else
		{
			Error(std::format("Missing semicolon, got '{}'", CurrentToken->Type));
			return nullptr;
		}
	}

	void ParseProgram()
	{
		Program = new ASTProgram();
		while (CurrentToken != NULL && CurrentToken != &Tokens.back())
		{
			auto Expr = ParsePrimaryExpr();
			Program->Expressions.push_back(Expr);
		}
	}

public:
	AST(std::vector<Token>& InTokens) : Tokens(InTokens)
	{
		CurrentToken = &Tokens[0];
		ParseProgram();
	}

	void Error(const std::string& InMsg)
	{
		Program->Errors.push_back({ InMsg, CurrentToken->Line, CurrentToken->Column });
	}

	ASTProgram* GetTree() { return Program; }
};