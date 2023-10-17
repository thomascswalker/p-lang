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

class AST
{
private:
	ASTProgram*			  Program;
	std::vector<Token>	  Tokens{};
	std::vector<ASTNode*> Nodes{};
	std::vector<ASTNode*> Expressions{};
	Token*				  CurrentToken;
	int					  Position;

	void Accept()
	{
		Token* End = &Tokens.back();
		if (CurrentToken == End)
		{
			CurrentToken = nullptr;
		}
		else
		{
			if (CurrentToken)
			{
				std::cout << std::format("Accept(): {}", CurrentToken->ToString()) << std::endl;
			}
			CurrentToken++;
			Position++;
		}
	}

	bool Expect(const std::string& Type, int Index = 0)
	{
		// Make sure the index is valid
		if (Position + Index > Tokens.max_size())
		{
			std::cout << "WARNING: Outside token bounds." << std::endl;
			return false;
		}

		std::cout << std::format("\tExpect {} at {}", Type, Position + Index) << std::endl;
		return Tokens[Position + Index].Type == Type;
	}

	bool Expect(const std::initializer_list<std::string>& Types)
	{
		for (auto [I, T] : Enumerate(Types))
		{
			if (!Expect(T, (int)I))
			{
				return false;
			}
		}
		return true;
	}

	ASTNode* ParseFactorExpr()
	{
		std::cout << "Parsing Factor" << std::endl;
		// Parse numbers (floats and ints)
		if (Expect("Number"))
		{
			std::cout << "Parsing Number" << std::endl;
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
			Accept(); // Consume number
			Nodes.push_back(new ASTLiteral(Value));
			return Nodes.back();
		}
		// Parse strings
		else if (Expect("String"))
		{
			std::string String = CurrentToken->Content;
			Accept(); // Consume string
			Nodes.push_back(new ASTLiteral(String));
			return Nodes.back();
		}
		// Parse names (Variables, functions, etc.)
		else if (Expect("Name"))
		{
			std::cout << "Parsing Name" << std::endl;
			std::string Name = CurrentToken->Content;
			Accept(); // Consume name
			Nodes.push_back(new ASTVariable(Name));
			return Nodes.back();
		}
		else if (Expect("("))
		{
			return ParseEncapsulatedExpr();
		}
		else
		{
			return nullptr;
		}
	}

	ASTNode* ParseEncapsulatedExpr()
	{
		std::cout << "Parsing Encapsulated" << std::endl;
		Accept(); // Consume '('
		ASTNode* Expr = ParseExpr();
		if (!Expect(")"))
		{
			Error("Missing ')'.");
			return nullptr;
		}
		Accept(); // Consume ')'
		return Expr;
	}

	ASTNode* ParseMultiplicativeExpr()
	{
		std::cout << "Parsing Mul/Div" << std::endl;
		ASTNode* Expr = ParseFactorExpr();
		while (Expect("*") || Expect("/"))
		{
			std::string Op = CurrentToken->Content;
			Accept(); // Consume '*' or '/'
			Nodes.push_back(new ASTBinOp(Expr, ParseFactorExpr(), Op));
			Expr = Nodes.back();
		}
		return Expr;
	}

	ASTNode* ParseAdditiveExpr()
	{
		std::cout << "Parsing Add/Sub" << std::endl;
		ASTNode* Expr = ParseMultiplicativeExpr();
		while (Expect("+") || Expect("-"))
		{
			std::string Op = CurrentToken->Content;
			Accept(); // Consume '+' or '-'
			Nodes.push_back(new ASTBinOp(Expr, ParseMultiplicativeExpr(), Op));
			Expr = Nodes.back();
		}
		return Expr;
	}

	ASTNode* ParseAssignment()
	{
		std::cout << "Parsing Assignment" << std::endl;
		std::string Type;
		std::string Name;

		// Handle new variable assignments
		if (Expect("Type"))
		{
			Type = CurrentToken->Content; // Get the type
			Accept();					  // Consume type
		}

		if (Expect("Name"))
		{
			// Handle existing variable assignment, or continue with a new assignment
			Name = CurrentToken->Content; // Get the name
			Accept();					  // Consume name
			if (Expect("="))
			{
				Accept(); // Consume '='
				ASTNode* Stmt = ParseStatement();
				Nodes.push_back(new ASTAssignment(Type, Name, Stmt));
				return Nodes.back();
			}
		}

		return nullptr;
	}

	ASTNode* ParseExpr()
	{
		std::cout << "Parsing Expression" << std::endl;
		// int MyVar = ...;
		if (Expect({ "Type", "Name", "=" }))
		{
			return ParseAssignment();
		}
		// MyVar = ...;
		else if (Expect({ "Name", "=" }))
		{
			return ParseAssignment();
		}
		// 5 + ...;
		// "Test" + ...;
		// MyVar + ...;
		else if (Expect("Number") || Expect("String") || Expect("Name"))
		{
			return ParseAdditiveExpr();
		}
		else
		{
			Error("Syntax Error");
			return nullptr;
		}
	}

	ASTNode* ParseStatement()
	{
		std::cout << "Parsing Statement" << std::endl;
		ASTNode* Expr;

		if (Expect("("))
		{
			Expr = ParseEncapsulatedExpr();
		}
		else
		{
			Expr = ParseExpr();
		}

		return Expr;
	}

	void ParseProgram()
	{
		Program = new ASTProgram();
		while (CurrentToken != NULL && CurrentToken != &Tokens.back())
		{
			auto Stmt = ParseStatement();
			if (!Stmt)
			{
				break;
			}
			Program->Statements.push_back(Stmt);
			if (Expect(";"))
			{
				Accept();
			}
			else
			{
				Error(std::format("Missing semicolon, got '{}'", CurrentToken->Type));
				break;
			}
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