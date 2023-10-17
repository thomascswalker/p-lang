#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <variant>
#include <typeinfo>
#include <format>

#include "token.h"
#include "core.h"

enum DataType
{
	Int,
	Float,
	String,
	Array,
	Map
};

using LiteralVariant = std::variant<int, float, std::string>;
struct Variable
{
	std::string Name;
	void*		Value;
	DataType	Type;

	auto GetInt() { return *Cast<int>(Value); }
	auto GetFloat() { return *Cast<float>(Value); }
	auto GetString() { return *Cast<std::string>(Value); }
};

void VariantAdd(const LiteralVariant& Left, const LiteralVariant& Right, LiteralVariant& Value);
void VariantSub(const LiteralVariant& Left, const LiteralVariant& Right, LiteralVariant& Value);
void VariantMul(const LiteralVariant& Left, const LiteralVariant& Right, LiteralVariant& Value);
void VariantDiv(const LiteralVariant& Left, const LiteralVariant& Right, LiteralVariant& Value);

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
	void		   Push(const LiteralVariant& V);
	LiteralVariant Pop();
	bool		   IsVariable(const std::string& Name);

public:
	std::map<std::string, LiteralVariant> Variables;
	std::vector<LiteralVariant>			  Stack;

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
	std::string	   Type = "Unknown";
	LiteralVariant Value;

	ASTLiteral(LiteralVariant& InValue) : Value(InValue)
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
		return "BinOp{\"Left: " + Left->ToString() + ", \"Op: \"" + Op + "\", Right: " + Right->ToString() + "}";
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
	std::vector<ASTNode*> Statements;
	ASTProgram(){};
	virtual std::string ToString() const
	{

		std::string Out;
		for (const auto& E : Statements)
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
	ASTProgram*			  Program;
	std::vector<Token>	  Tokens{};
	std::vector<ASTNode*> Nodes{};
	std::vector<ASTNode*> Expressions{};
	Token*				  CurrentToken;
	int					  Position;

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
	bool Expect(const std::string& Type, int Offset = 0);

	/// <summary>
	/// Expect the given <paramref name="Types"/> in sequential order at the current position.
	/// </summary>
	/// <param name="Types">The types to check for.</param>
	/// <returns>Whether the types are all found.</returns>
	bool Expect(const std::initializer_list<std::string>& Types);

	ASTNode* ParseFactorExpr();
	ASTNode* ParseEncapsulatedExpr();
	ASTNode* ParseMultiplicativeExpr();
	ASTNode* ParseAdditiveExpr();
	ASTNode* ParseAssignment();
	ASTNode* ParseExpression();
	void	 ParseProgram();

public:
	AST(std::vector<Token>& InTokens) : Tokens(InTokens)
	{
		CurrentToken = &Tokens[0];
		Position = 0;
		ParseProgram();
	}

	/// <summary>
	/// Logs an error with the specified <paramref name="InMsg"/> at the current line and column.
	/// </summary>
	/// <param name="InMsg">The error message to display.</param>
	void Error(const std::string& InMsg)
	{
		Program->Errors.push_back({ InMsg, CurrentToken->Line, CurrentToken->Column });
	}

	/// <summary>
	/// Get the parsed node tree.
	/// </summary>
	/// <returns>The root AST node.</returns>
	ASTProgram* GetTree() { return Program; }
};