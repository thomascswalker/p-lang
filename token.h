#include <string>
#include <fstream>
#include <regex>
#include <iostream>
#include <optional>
#include <memory>
#include <sstream>
#include <map>

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

std::string ReadFile(std::string FileName)
{
	std::ifstream Stream(FileName.c_str());
	return std::string(std::istreambuf_iterator<char>(Stream), std::istreambuf_iterator<char>());
};

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
	std::string Type = "";
	std::string Content = "";
	int			Line = 0;

	Token(){};
	Token(std::string InType, std::string InContent, int InLine) : Type(InType), Content(InContent), Line(InLine){};
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

	char		GetCurrentChar() { return Source[Position]; }
	std::string GetRemaining() { return Source.substr(Position); }
	void		Advance() { Position++; }
	bool		IsWhitespace()
	{
		std::string Slice = GetRemaining();
		if (Slice[0] == ' ' || Slice[0] == '\t' || Slice[0] == '\n')
		{
			if (Slice[0] == '\n')
			{
				Line += 1;
			}
			return true;
		}
		return false;
	}
	bool IsAscii(char C) { return C >= 'a' && C <= 'A' && C >= 'z' && C <= 'Z'; }
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
			return Token{ std::string(1, C), std::string(1, C), Line };
		}
		else if (IsDigit(C))
		{
			std::string Number;
			while (IsDigit(C) || C == '.')
			{
				Number += C;
				Advance();
				C = GetCurrentChar();
			}

			return Token{ "Number", Number, Line };
		}
		// Types, Names
		else if (IsAscii(C))
		{
			std::string String;
			while (IsAscii(C))
			{
				String += C;
				Advance();
				C = GetCurrentChar();
			}

			if (Contains(KEYWORDS, String))
			{
				return Token{ "Type", String, Line };
			}

			return Token{ "Name", String, Line };
		}
		// End of file
		else if (C == '\0')
		{
			return Token{ "EOF", "\0", Line };
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