#include <string>
#include <fstream>
#include <regex>
#include <iostream>
#include <memory>
#include <sstream>
#include <map>

#include "core.h"

// Token Literals
const std::vector<char> TOKENS{
	'+', '-', '/', '*', '=', ';',
};
const std::vector<std::string> KEYWORDS{
	"int",
	"float",
	"string",
};

// Forward decl
struct Token;
class Lexer;

typedef std::vector<std::shared_ptr<Token>> TokenArray;

template <typename T>
bool Contains(const std::vector<T> Array, T Value)
{
	for (auto A : Array)
	{
		if (A == Value)
		{
			return true;
		}
	}
	return false;
}

struct Token
{
	// Properties
	std::string Type = "";
	std::string Content = "";
	int			Line = 0;
	int			Column = 0;

	// Constructors
	Token(){};
	Token(const std::string& InType, const std::string& InContent, int InLine, int InColumn)
		: Type(InType), Content(InContent), Line(InLine), Column(InColumn){};

	// Methods
	std::string ToString() const
	{
		std::ostringstream Stream;
		Stream << "(" << Type << ") " << Content << " [" << Line << "]";
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
	std::string GetRemaining() { return Source.substr(Position); }
	const char	Advance()
	{
		Position++;
		Column++;
		return GetCurrentChar();
	}
	bool		IsWhitespace()
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

		// Numbers
		if (IsSymbol(C))
		{
			Advance();
			return Token{ std::string(1, C), std::string(1, C), Line, Column };
		}
		else if (IsDigit(C))
		{
			std::string Number;
			while (IsDigit(C) || C == '.')
			{
				Number += C;
				C = Advance();
			}

			return Token{ "Number", Number, Line, Column };
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

			if (Contains(KEYWORDS, String))
			{
				return Token{ "Type", String, Line, Column };
			}

			return Token{ "Name", String, Line, Column };
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
			return Token{ "String", String, Line, Column };
		}
		// End of file
		else if (C == '\0')
		{
			return Token{ "EOF", "\0", Line, Column };
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
			Tokens.push_back(T);
		}
		return Tokens;
	}
};