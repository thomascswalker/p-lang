#include <string>
#include <fstream>
#include <regex>
#include <iostream>
#include <memory>
#include <sstream>
#include <map>

#include "core.h"

using namespace Core;

// Tokens
enum TokenType
{
	Invalid,
	Eof,
	Type,
	Name,
	Number,
	String,
	Bool,
	Plus,
	Minus,
	Divide,
	Multiply,
	Not,
	Assign,
	Equals,
	NotEquals,
	Semicolon,
	LessThan,
	GreaterThan,
	LParen,
	RParen,
	LBracket,
	RBracket,
	LCurly,
	RCurly,
	If,
	Else,
	For,
	While,
	Count
};

static int TokenTypeCount = (int)TokenType::Count;

static std::map<TokenType, std::string> TokenStringMap{
	{ Eof, "/0" },		{ Type, "Type" },  { Plus, "+" },		 { Minus, "-" },   { Multiply, "*" },
	{ Divide, "/" },	{ Not, "!" },	   { Assign, "=" },		 { Equals, "==" }, { NotEquals, "!=" },
	{ Semicolon, ";" }, { LessThan, "<" }, { GreaterThan, ">" }, { LParen, "(" },  { RParen, ")" },
	{ LBracket, "[" },	{ RBracket, "]" }, { LCurly, "{" },		 { RCurly, "}" },  { If, "if" },
	{ Else, "else" },	{ For, "for" },	   { While, "while" }
};

static TokenType GetTokenTypeFromString(const std::string& InString)
{
	for (auto& [K, V] : TokenStringMap)
	{
		if (InString == V)
		{
			return K;
		}
	}
	return Invalid;
}

const std::vector<char>		   TOKENS{ '+', '-', '/', '*', '=', '!', ';', '<', '>', '(', ')', '[', ']', '{', '}' };
const std::vector<std::string> TYPES{
	"int",
	"float",
	"string",
	"bool",
};
const std::vector<std::string> KEYWORDS{ "if", "else", "for", "while" };

// Forward decl
struct Token;
class Lexer;

typedef std::vector<std::shared_ptr<Token>> TokenArray;

struct Token
{
	// Properties
	TokenType Type;
	std::string Content = "";
	int			Line = 0;
	int			Column = 0;

	// Constructors
	Token() : Type(Invalid){};
	Token(TokenType InType, const std::string& InContent, int InLine, int InColumn)
		: Type(InType), Content(InContent), Line(InLine), Column(InColumn){};

	// Methods
	std::string ToString() const
	{
		std::ostringstream Stream;
		Stream << (int)Type << ", " << Content << ", line " << (Line + 1) << ", col " << Column;
		return Stream.str();
	}
	void Print() const { std::cout << ToString() << std::endl; }
};

class Lexer
{
	std::string Source = "";
	int			Position = 0;
	int			Line = 0;
	int			Column = 0;

	char		GetCurrentChar() { return Source[Position]; }
	char		GetNextChar() { return Source[Position + 1]; }
	std::string GetRemaining() { return Source.substr(Position); }
	const char	Advance(int Offset = 1)
	{
		Position += Offset;
		Column += Offset;
		return GetCurrentChar();
	}
	bool IsWhitespace()
	{
		std::string Slice = GetRemaining();
		if (Slice[0] == ' ' || Slice[0] == '\t' || Slice[0] == '\n')
		{
			if (Slice[0] == '\n')
			{
				Line += 1;
				Column = 0;
			}
			else
			{
				Column++;
			}
			return true;
		}
		return false;
	}
	bool IsAscii(char C) { return C >= 'a' && C <= 'z' || C >= 'A' && C <= 'Z'; }
	bool IsDigit(char C) { return C >= '0' && C <= '9'; }
	bool IsSymbol(char C) { return Contains(TOKENS, C); }

public:
	Lexer(std::string InSource) : Source(InSource){};

	Token Next()
	{
		// Advance whitespace, new lines, and tabs
		while (Position < Source.size() && IsWhitespace())
		{
			Advance();
		}

		char C = GetCurrentChar();

		// Operators, blocks
		if (IsSymbol(C))
		{
			std::string Op;
			// Equals operator
			if (C == '=' && GetNextChar() == '=')
			{
				Op = "==";
				Advance(2); // Consume double char operator
			}
			// Not equals operator
			else if (C == '!' && GetNextChar() == '=')
			{
				Op = "!=";
				Advance(2); // Consume double char operator
			}
			// Returns operator
			else if (C == '-' && GetNextChar() == '>')
			{
				Op = "->";
				Advance(2); // Consume double char operator
			}
			else
			{
				Op = std::string(1, C);
				Advance(); // Consume single char operator
			}

			return Token{ GetTokenTypeFromString(Op), Op, Line, Column };
		}
		// Numbers
		else if (IsDigit(C))
		{
			std::string Number;
			while (IsDigit(C) || C == '.')
			{
				Number += C;
				C = Advance();
			}

			return Token{ TokenType::Number, Number, Line, Column };
		}
		// Types, Names
		else if (IsAscii(C))
		{
			std::string String;
			while (IsAscii(C) || C == '_')
			{
				String += C;
				C = Advance();
			}

			if (Contains(TYPES, String))
			{
				return Token{ TokenType::Type, String, Line, Column };
			}

			if (Contains(KEYWORDS, String))
			{
				return Token{ GetTokenTypeFromString(String), String, Line, Column };
			}

			if (Contains({ "true", "false" }, String))
			{
				return Token{ TokenType::Bool, String, Line, Column };
			}

			return Token{ TokenType::Name, String, Line, Column };
		}
		// Strings
		else if (C == '"' || C == '\'')
		{
			C = Advance(); // Skip first quotation
			std::string String;

			// Get all characters until the next quotation
			while (C != '\"')
			{
				String += C;
				C = Advance();
			}
			Advance(); // Skip last quotation
			return Token{ TokenType::String, String, Line, Column };
		}
		// End of file
		else if (C == '\0')
		{
			return Token{ TokenType::Eof, "\0", Line, Column };
		}
		else
		{
			throw(std::runtime_error("Invalid character found: " + C));
		}
	}

	std::vector<Token> Tokenize()
	{
		std::vector<Token> Tokens;
		while (Position < Source.size())
		{
			Token T = Next();
			Debug(std::format("Token: {}", T.ToString()));
			Tokens.push_back(T);
		}
		return Tokens;
	}
};