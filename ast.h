#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <variant>
#include <typeinfo>
#include <format>

#include "token.h"
#include "value.h"

using namespace Core;
using namespace Values;

class VisitorBase;
class Visitor;

class ASTNode;
class ASTValue;
class ASTVariable;
class ASTUnaryExpr;
class ASTBinOp;
class ASTAssignment;
class ASTIf;
class ASTWhile;
class ASTFunctionDef;
class ASTReturn;
class ASTBody;

class VisitorBase
{
public:
	virtual void Visit(ASTValue* Node) = 0;
	virtual void Visit(ASTVariable* Node) = 0;
	virtual void Visit(ASTUnaryExpr* Node) = 0;
	virtual void Visit(ASTBinOp* Node) = 0;
	virtual void Visit(ASTAssignment* Node) = 0;
	virtual void Visit(ASTIf* Node) = 0;
	virtual void Visit(ASTWhile* Node) = 0;
	virtual void Visit(ASTFunctionDef* Node) = 0;
	virtual void Visit(ASTReturn* Node) = 0;
	virtual void Visit(ASTBody* Node) = 0;
};

#define CHECK_EXIT if (bExited) { return; }

class Visitor : public VisitorBase
{
	bool bExited = false;
	void Exit(const std::string& Msg)
	{
		Error(Msg);
		bExited = true;
	}
	
	void	Push(const TObject& V);
	TObject Pop();
	bool	IsVariable(const std::string& Name);
	TObject GetVariable(const std::string& Name);
	void	SetVariable(const std::string& Name, const TObject& InValue);

public:
	std::map<std::string, TObject> Variables;
	std::vector<TObject>		   Stack;

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
	void Visit(ASTValue* Node) override;
	void Visit(ASTVariable* Node) override;
	void Visit(ASTUnaryExpr* Node) override;
	void Visit(ASTBinOp* Node) override;
	void Visit(ASTAssignment* Node) override;
	void Visit(ASTIf* Node) override;
	void Visit(ASTWhile* Node) override;
	void Visit(ASTFunctionDef* Node) override;
	void Visit(ASTReturn* Node) override;
	void Visit(ASTBody* Node) override;

	bool Succeeded() { return !bExited; }
	void Dump();
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

class ASTValue : public ASTNode
{
public:
	TObject Value;

	ASTValue(TObject& InValue) : Value(InValue){};
	ASTValue(const TObject& InValue) : Value(InValue){};

	void Accept(Visitor& V) override { V.Visit(this); }

	bool IsInt() { return Value.GetType() == IntType; }
	bool IsFloat() { return Value.GetType() == FloatType; }
	bool IsString() { return Value.GetType() == StringType; }
	bool IsBool() { return Value.GetType() == BoolType; }
	bool IsArray() { return Value.GetType() == ArrayType; }

	TBoolValue	 GetBool() const { return Value.GetBool(); }
	TIntValue	 GetInt() const { return Value.GetInt(); }
	TFloatValue	 GetFloat() const { return Value.GetFloat(); }
	TStringValue GetString() const { return Value.GetString(); }
	TArrayValue	 GetArray() const { return Value.GetArray(); }

	TObject GetValue()
	{
		if (IsBool())
		{
			return GetBool().GetValue();
		}
		else if (IsInt())
		{
			return GetInt().GetValue();
		}
		else if (IsFloat())
		{
			return GetFloat().GetValue();
		}
		else if (IsString())
		{
			return GetString().GetValue();
		}
		else if (IsArray())
		{
			return GetArray();
		}
		throw std::runtime_error("Unable to get value for this object.");
	}

	virtual std::string ToString() const
	{
		std::string Result;
		return Result;
	}
};

class ASTVariable : public ASTNode
{
public:
	std::string Name;
	TObject		Value;
	ASTVariable(const std::string& InName, const TObject& InValue = TObject()) : Name(InName), Value(InValue){};
	virtual std::string ToString() const { return "Variable: " + Name + ", " + Value.ToString(); }
	void				Accept(Visitor& V) override { V.Visit(this); }
};

class ASTUnaryExpr : public ASTNode
{
public:
	std::string Name;
	ETokenType	Op;
	ASTNode*	OpArg = nullptr;
	ASTUnaryExpr(const std::string& InName, ETokenType InOp, ASTNode* InOpArg = nullptr)
		: Name(InName), Op(InOp), OpArg(InOpArg){};
	virtual std::string ToString() const { return "UnaryExpr: " + Name + "(" + OpArg->ToString() + ")"; }
	void				Accept(Visitor& V) override { V.Visit(this); }
};

class ASTBinOp : public ASTNode
{
	std::string OpString;

public:
	ASTNode*   Left;
	ASTNode*   Right;
	ETokenType Op = Invalid;

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

using ArgValue = std::pair<EValueType, std::string>;

class ASTFunctionDef : public ASTNode
{
public:
	EValueType			  ReturnType;
	std::string			  Name;
	std::vector<ArgValue> Arguments;
	ASTNode*			  Body;

	ASTFunctionDef(EValueType InReturnType, const std::string& InName, std::vector<ArgValue> InArguments,
				   ASTNode* InBody)
		: ReturnType(InReturnType), Name(InName), Arguments(InArguments), Body(InBody){};
	virtual std::string ToString() const { return "FunctionDef"; }
	void				Accept(Visitor& V) override { V.Visit(this); }
};

class ASTReturn : public ASTNode
{
public:
	ASTNode* Expression;
	ASTReturn(ASTNode* InExpression) : Expression(InExpression){};
	virtual std::string ToString() const { return "Return"; }
	void				Accept(Visitor& V) override { V.Visit(this); }
};

class ASTBody : public ASTNode
{
public:
	ASTBody*			  Parent = nullptr;
	std::vector<ASTError> Errors;
	std::vector<ASTNode*> Expressions;
	ASTBody(std::vector<ASTNode*> InBody = {}) : Expressions(InBody){};
	virtual std::string ToString() const
	{
		std::string Out;
		for (const ASTNode* E : Expressions)
		{
			Out += E->ToString() + "\n";
		};
		return Out;
	}
	void Accept(Visitor& V) override { V.Visit(this); }
	bool Succeeded() { return Errors.size() == 0; }
};

/// <summary>
/// Parses a list of tokens into an Abstract Syntax Tree (AST).
/// </summary>
class AST
{
private:
	ASTBody*								Program;
	std::vector<Token>						Tokens{};
	std::vector<ASTNode*>					Nodes{};
	std::vector<ASTNode*>					Expressions{};
	Token*									CurrentToken;
	int										Position;
	const std::map<std::string, EValueType> StringTypeMap{
		{ "void", NullType },
		{ "bool", BoolType },
		{ "int", IntType },
		{ "float", FloatType },
	};

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
	void Accept()
	{
		// Get the last token in the token list
		Token* End = &Tokens.back();

		if (!CurrentToken)
		{
			return;
		}

		// If we're at the end, set the CurrentToken to be null
		if (CurrentToken == End)
		{
			CurrentToken = nullptr;
		}
		// Otherwise increment the CurrentToken pointer and the position
		else
		{
			Debug(std::format("Accept(): {}", CurrentToken->Content));
			CurrentToken++;
			Position++;
		}
	}

	/// <summary>
	/// Expect the specified <paramref name="Type"/> at the specified <paramref name="Offset"/>, relative to the current
	/// position.
	/// </summary>
	/// <param name="Type">The type to check for.</param>
	/// <param name="Offset">The offset position.</param>
	/// <returns>Whether the type is found.</returns>
	bool Expect(ETokenType Type, int Offset = 0)
	{
		// Make sure the offset is valid
		if (Position + Offset > Tokens.size())
		{
			Error("WARNING: Outside token bounds.");
			return false;
		}

		// Debug(std::format("Expect(): {} at {}: {}", (int)Type, Position + Offset, Tokens[Position +
		// Offset].ToString()));

		return Tokens[Position + Offset].Type == Type;
	}

	/// <summary>
	/// Expect the given <paramref name="Types"/> in sequential order at the current position.
	/// </summary>
	/// <param name="Types">The types to check for.</param>
	/// <returns>Whether the types are all found.</returns>
	bool ExpectSequence(const std::initializer_list<ETokenType>& Types, int Offset = 0)
	{
		for (auto [I, T] : Enumerate(Types))
		{
			if (!Expect(T, Offset + (int)I))
			{
				return false;
			}
		}
		return true;
	}

	bool ExpectAny(const std::initializer_list<ETokenType>& Types, int Offset = 0)
	{
		for (const auto& T : Types)
		{
			if (Expect(T, Offset))
			{
				return true;
			}
		}
		return false;
	}

	bool ExpectValue() { return ExpectAny({ Name, Bool, Number, String }); }
	bool ExpectUnaryOperator() { return ExpectAny({ Minus, PlusPlus, MinusMinus, Period, LBracket }); }

	ASTNode* ParseValueExpr();
	ASTNode* ParseUnaryExpr();
	ASTNode* ParseParenExpr();
	ASTNode* ParseBracketExpr();
	ASTNode* ParseCurlyExpr();
	ASTNode* ParseMultiplicativeExpr();
	ASTNode* ParseAdditiveExpr();
	ASTNode* ParseEqualityExpr();
	ASTNode* ParseAssignment();
	ASTNode* ParseReturnExpr();
	ASTNode* ParseIf();
	ASTNode* ParseWhile();
	ASTNode* ParseFunctionDef();
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