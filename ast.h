#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <variant>
#include <typeinfo>
#include <format>

#include "token.h"

using namespace Core;
using Literal = std::variant<int, float, std::string, bool>;

static void LiteralAdd(const Literal& Left, const Literal& Right, Literal& Value);
static void LiteralSub(const Literal& Left, const Literal& Right, Literal& Value);
static void LiteralMul(const Literal& Left, const Literal& Right, Literal& Value);
static void LiteralDiv(const Literal& Left, const Literal& Right, Literal& Value);
static void LiteralEq(const Literal& Left, const Literal& Right, Literal& Value);
static void LiteralNotEq(const Literal& Left, const Literal& Right, Literal& Value);
static void LiteralLessThan(const Literal& Left, const Literal& Right, Literal& Value);
static void LiteralGreaterThan(const Literal& Left, const Literal& Right, Literal& Value);

class VisitorBase;
class Visitor;

class ASTNode;
class ASTLiteral;
class ASTVariable;
class ASTBinOp;
class ASTAssignment;
class ASTIf;
class ASTWhile;
class ASTBody;

class VisitorBase
{
public:
	virtual void Visit(ASTLiteral* Node) = 0;
	virtual void Visit(ASTVariable* Node) = 0;
	virtual void Visit(ASTBinOp* Node) = 0;
	virtual void Visit(ASTAssignment* Node) = 0;
	virtual void Visit(ASTIf* Node) = 0;
	virtual void Visit(ASTWhile* Node) = 0;
	virtual void Visit(ASTBody* Node) = 0;
};

class Visitor : public VisitorBase
{
	void	Push(const Literal& V);
	Literal Pop();
	bool	IsVariable(const std::string& Name);

public:
	std::map<std::string, Literal> Variables;
	std::vector<Literal>		   Stack;

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
	void Visit(ASTIf* Node) override;
	void Visit(ASTWhile* Node) override;
	void Visit(ASTBody* Node) override;
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
	virtual void Accept(Visitor& V) = 0;

	/// <summary>
	/// Converts this node to a formatted string.
	/// </summary>
	/// <returns>This node formatted as a string.</returns>
	virtual std::string ToString() const = 0;
};

class ASTLiteral : public ASTNode
{
public:
	std::string Type = "Unknown";
	Literal		Value;

	ASTLiteral(Literal& InValue) : Value(InValue)
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
		else if (IsBool())
		{
			Type = "Bool";
		}
	}
	ASTLiteral(int InValue) : Value(InValue) { Type = "Int"; };
	ASTLiteral(float InValue) : Value(InValue) { Type = "Float"; };
	ASTLiteral(const std::string& InValue) : Value(InValue) { Type = "String"; };
	ASTLiteral(const bool InValue) : Value(InValue) { Type = "Bool"; };

	void Accept(Visitor& V) override { V.Visit(this); }

	bool IsInt() const { return std::holds_alternative<int>(Value); }
	bool IsFloat() const { return std::holds_alternative<float>(Value); }
	bool IsString() const { return std::holds_alternative<std::string>(Value); }
	bool IsBool() const { return std::holds_alternative<bool>(Value); }

	int			GetInt() const { return std::get<int>(Value); }
	float		GetFloat() const { return std::get<float>(Value); }
	std::string GetString() const { return std::get<std::string>(Value); }
	bool		GetBool() const { return std::get<bool>(Value); }

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
		else if (IsBool())
		{
			Result = GetBool() ? "true" : "false";
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
	std::string OpString;

public:
	ASTNode*  Left;
	ASTNode*  Right;
	TokenType Op = Invalid;

	ASTBinOp(ASTNode* InLeft, ASTNode* InRight, const std::string& InOp)
		: Left(InLeft), Right(InRight), Op(GetTokenTypeFromString(InOp)){};
	virtual std::string ToString() const
	{
		return "BinOp{\"Left: " + Left->ToString() + ", \"Op: \"" + OpString + "\", Right: " + Right->ToString() + "}";
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

class ASTIf : public ASTNode
{
public:
	ASTNode* Cond;
	ASTNode* TrueBody;
	ASTNode* FalseBody;

	ASTIf(ASTNode* InCond, ASTNode* InTrueBody, ASTNode* InFalseBody = nullptr)
		: Cond(InCond), TrueBody(InTrueBody), FalseBody(InFalseBody){};
	virtual std::string ToString() const { return "Conditional"; }
	void				Accept(Visitor& V) override { V.Visit(this); }
};

class ASTWhile : public ASTNode
{
public:
	ASTNode* Cond;
	ASTNode* Body;

	ASTWhile(ASTNode* InCond, ASTNode* InBody) : Cond(InCond), Body(InBody){};
	virtual std::string ToString() const { return "While"; }
	void				Accept(Visitor& V) override { V.Visit(this); }
};

class ASTBody : public ASTNode
{
public:
	std::vector<ASTError> Errors;
	std::vector<ASTNode*> Expressions;
	ASTBody(std::vector<ASTNode*> InBody = {}) : Expressions(InBody){};
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

/// <summary>
/// Parses a list of tokens into an Abstract Syntax Tree (AST).
/// </summary>
class AST
{
private:
	ASTBody*			  Program;
	std::vector<Token>	  Tokens{};
	std::vector<ASTNode*> Nodes{};
	std::vector<ASTNode*> Expressions{};
	Token*				  CurrentToken;
	int					  Position;

	void PrintCurrentToken()
	{
		if (CurrentToken == NULL)
		{
			return;
		}
		std::cout << (std::format("{}", CurrentToken->Content)) << std::endl;
	}

	/// <summary>
	/// Accept the current token. This will increment the <paramref name="CurrentToken"/> pointer as well as increment
	/// the <paramref name="Position"/> value.
	/// </summary>
	void Accept();

	/// <summary>
	/// Expect the specified <paramref name="Type"/> at the specified <paramref name="Offset"/>, relative to the current
	/// position.
	/// </summary>
	/// <param name="Type">The type to check for.</param>
	/// <param name="Offset">The offset position.</param>
	/// <returns>Whether the type is found.</returns>
	bool Expect(TokenType Type, int Offset = 0);

	/// <summary>
	/// Expect the given <paramref name="Types"/> in sequential order at the current position.
	/// </summary>
	/// <param name="Types">The types to check for.</param>
	/// <returns>Whether the types are all found.</returns>
	bool Expect(const std::initializer_list<TokenType>& Types);

	ASTNode* ParseLiteralExpr();
	ASTNode* ParseParenExpr();
	ASTNode* ParseCurlyExpr();
	ASTNode* ParseMultiplicativeExpr();
	ASTNode* ParseAdditiveExpr();
	ASTNode* ParseEqualityExpr();
	ASTNode* ParseAssignment();
	ASTNode* ParseIf();
	ASTNode* ParseWhile();
	ASTNode* ParseExpression();
	void	 ParseBody();

public:
	AST(std::vector<Token>& InTokens) : Tokens(InTokens)
	{
		CurrentToken = &Tokens[0];
		Position = 0;
		ParseBody();
	}

	/// <summary>
	/// Logs an error with the specified <paramref name="InMsg"/> at the current line and column.
	/// </summary>
	/// <param name="InMsg">The error message to display.</param>
	void Error(const std::string& InMsg)
	{
		if (CurrentToken != nullptr)
		{
			Program->Errors.push_back({ InMsg, CurrentToken->Line, CurrentToken->Column });
		}
		else
		{
			Program->Errors.push_back({ InMsg, -1, -1 });
		}
	}

	/// <summary>
	/// Get the parsed node tree.
	/// </summary>
	/// <returns>The root AST node.</returns>
	ASTBody* GetTree() { return Program; }
};