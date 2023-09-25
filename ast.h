#include <vector>
#include <memory>
#include <optional>
#include <functional>
#include <variant>
#include <string>

#include "token.h"

// Base AST Node class
class ASTNode
{
public:
	ASTNode(){};
	virtual std::string ToString() const = 0;
	virtual void		Print() const = 0;
};

// Node for constant values (numbers, strings, etc.)
class ASTFactor : public ASTNode
{
public:
	std::string Value;
	std::string Type;
	ASTFactor(std::string& InValue, std::string& InType) : Value(InValue), Type(InType){};
	std::string ToString() const { return Value; }
	void		Print() const { std::cout << ToString() << std::endl; }
};

class ASTTerm : public ASTNode
{
public:
	ASTTerm(){};
	ASTTerm(std::string& InOp) : Op(InOp){};
	std::shared_ptr<ASTFactor> Left;
	std::shared_ptr<ASTTerm>   Right;
	std::string				   Op;

	std::string ToString() const
	{
		if (Op == "" || !Right)
		{
			return "{" + Left->ToString() + "}";
		}
		else
		{

			return "{" + Left->ToString() + ", " + Op + ", " + Right->ToString() + "}";
		}
	}
	void Print() const { std::cout << ToString() << std::endl; }
};

class AST
{
	std::vector<Token> Tokens{};

	Token Pop()
	{
		if (Tokens.size() > 0)
		{
			Token T = Tokens[0];
			Tokens.erase(Tokens.begin());
			return T;
		}
		else
		{
			std::cout << "End of tokens." << std::endl;
		}
	}

	std::shared_ptr<ASTFactor> ParseFactor()
	{
		std::shared_ptr<ASTFactor> Factor;
		Token					   T = Pop();

		// If the current token is a number
		if (T.Type == "Number")
		{
			// Construct a new factor and return it
			Factor = std::make_shared<ASTFactor>(T.Content, T.Type);
			return Factor;
		}
		else
		{
			return nullptr;
		}
	}

	std::shared_ptr<ASTTerm> ParseTerm()
	{
		// Get the left-hand factor
		auto					 Left = ParseFactor();
		std::shared_ptr<ASTTerm> Term = std::make_shared<ASTTerm>();
		Term->Left = Left;

		// While the next token is either + or -, get the right-hand factor
		while (Tokens.size() > 0 && Contains({ "+", "-" }, Tokens[0].Content))
		{
			// Get the actual operator (+ or -)
			std::string Op = Pop().Content;
			Term->Op = Op;

			// Recursively parse the next term
			Term->Right = ParseTerm();
		}

		return Term;
	}

public:
	AST(std::vector<Token>& InTokens) : Tokens(InTokens){};
	std::shared_ptr<ASTTerm> Parse()
	{
		auto Result = ParseTerm();
		if (Result)
		{
			return Result;
		}
		else
		{
			std::cout << "ERROR: Unable to parse syntax tree." << std::endl;
			return nullptr;
		}
	}
	int Eval(std::shared_ptr<ASTTerm> Term)
	{
		int Result;
		int Left = std::stoi(Term->Left->Value);
		if (Term->Op == "" || !Term->Right)
		{
			return Left;
		}
		std::string Op = Term->Op;
		int			Right = Eval(Term->Right);

		switch (*Op.c_str())
		{
			case '+' :
				return Left + Right;
			case '-' :
				return Left - Right;
			case '*' :
				return Left * Right;
			case '/' : 
				return Left / Right;
			default :
				std::string Msg = "Operator not supported: " + Op;
				std::cout << Msg << std::endl;
				throw(std::runtime_error(Msg));
		}
	}
};