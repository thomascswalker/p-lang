#pragma once

#include <string>
#include <fstream>
#include <regex>
#include <iostream>
#include <memory>
#include <sstream>
#include <map>

#include "Core.h"

using namespace Core;

// Tokens
enum ETokenType
{
    Invalid,
    Eof,
    Type,
    Func,
    Name,
    Number,
    String,
    Bool,
    Plus,
    Minus,
    Divide,
    Multiply,
    Comma,
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
    Return,
    Count,
    Period,
    PlusEquals,
    MinusEquals,
    MultEquals,
    DivEquals,
    PlusPlus,
    MinusMinus,
};

static int TOKEN_TYPE_COUNT = ETokenType::Count;

static std::map<ETokenType, std::string> TokenToStringMap{
    {Eof, "/0"}, {Type, "Type"}, {Func, "func"}, {Plus, "+"}, {Minus, "-"},
    {Multiply, "*"}, {Divide, "/"}, {Comma, ","}, {Not, "!"}, {Assign, "="},
    {Equals, "=="}, {NotEquals, "!="}, {Semicolon, ";"}, {LessThan, "<"}, {GreaterThan, ">"},
    {LParen, "("}, {RParen, ")"}, {LBracket, "["}, {RBracket, "]"}, {LCurly, "{"},
    {RCurly, "}"}, {If, "if"}, {Else, "else"}, {For, "for"}, {While, "while"},
    {Return, "return"}, {Period, "."}, {PlusEquals, "+="}, {MinusEquals, "-="}, {MultEquals, "*="},
    {DivEquals, "/="}, {PlusPlus, "++"}, {MinusMinus, "--"}
};

static ETokenType GetTokenTypeFromString(const std::string& InString)
{
    for (auto& [K, V] : TokenToStringMap)
    {
        if (InString == V)
        {
            return K;
        }
    }
    return Invalid;
}

const std::vector<char> TOKENS{'+', '-', '/', '*', '=', '!', ';', '<', '>', '(', ')', '[', ']', '{', '}', ',', '.'};
const std::vector<char> OPERATORS{'+', '-', '/', '*', '=', '.', '!'};
const std::vector<std::string> TYPES{
    "int",
    "float",
    "string",
    "bool",
};
const std::vector<std::string> KEYWORDS{"if", "else", "for", "while", "return"};
const std::map<ETokenType, ETokenType> BLOCK_PAIRS{{LParen, RParen}, {LBracket, RBracket}, {LCurly, RCurly}};
const std::vector<std::string> FUNCTION{"function", "func", "fn", "def"};

// Forward decl
struct Token;
class Lexer;

using TokenArray = std::vector<std::shared_ptr<Token>>;

struct Token
{
    // Properties
    ETokenType Type;
    std::string Content;
    std::string Source;
    int Line = 1;
    int Column = 0;

    // Constructors
    Token()
        : Type(Invalid)
    {
    }
    Token(const ETokenType InType, const std::string& InContent, int InLine, int InColumn)
        : Type(InType)
          , Content(InContent)
          , Line(InLine)
          , Column(InColumn)
    {
    }

    // Methods
    std::string ToString() const
    {
        std::ostringstream Stream;
        Stream << Type << ", " << Content << ", line " << (Line + 1) << ", col " << Column;
        return Stream.str();
    }
    void Print() const { std::cout << ToString() << '\n'; }
};

class Lexer
{
    std::string Source;
    int Position = 0;
    int Line = 1;
    int Column = 0;
    std::vector<std::string> Lines;

    char GetCurrentChar() const { return Source[Position]; }
    char GetNextChar() const { return Source[Position + 1]; }
    std::string GetPair() const { return Source.substr(Position, 2); }
    std::string GetRemaining() const { return Source.substr(Position); }
    char Advance(int Offset = 1)
    {
        Position += Offset;
        Column += Offset;
        return GetCurrentChar();
    }
    bool IsWhitespace()
    {
        const std::string Slice = GetRemaining();
        const std::string Pair = GetPair();
        if (Pair == "//")
        {
            Advance(2); // Consume '//'
            while (GetCurrentChar() != '\n')
            {
                Advance();
            }
            Line += 1;
            Column = 0;
            return true;
        }

        if (Pair == "/*")
        {
            Advance(2); // Consume '/*'
            auto C = GetCurrentChar();
            while (GetPair() != "*/")
            {
                C = GetCurrentChar();
                if (C == '\n')
                {
                    Line += 1;
                    Column = 0;
                }
                Advance();
            }
            Advance(3); // '*/\n'
            Line += 1;
            Column = 0;

            return true;
        }

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
    static bool IsAscii(const char C) { return C >= 'a' && C <= 'z' || C >= 'A' && C <= 'Z'; }
    static bool IsDigit(const char C) { return C >= '0' && C <= '9'; }
    static bool IsSymbol(const char C) { return Contains(TOKENS, C); }

    bool AtEnd() const { return Position >= Source.size(); }

public:
    explicit Lexer(const std::string& InSource)
        : Source(InSource)
    {
    }

    Token Next()
    {
        char C = GetCurrentChar();

        // Advance whitespace, new lines, and tabs
        while (Position < Source.size() && IsWhitespace())
        {
            C = Advance();
        }

        // Operators, blocks
        if (IsSymbol(C))
        {
            ETokenType Type;
            std::string Op;
            // Equals operator
            if (Contains(OPERATORS, C) && Contains(OPERATORS, GetNextChar()))
            {
                Op = std::string(Source.begin() + Position, Source.begin() + Position + 2);
                Type = GetTokenTypeFromString(Op);
                Advance(2);
            }
            else
            {
                Op = std::string(1, C);
                Type = GetTokenTypeFromString(Op);
                Advance(); // Consume single char operator
            }

            return Token{Type, Op, Line, Column};
        }

        // Numbers
        if (IsDigit(C))
        {
            std::string Number;
            while (IsDigit(C) || C == '.')
            {
                Number += C;
                C = Advance();
            }

            return Token{ETokenType::Number, Number, Line, Column};
        }

        // Types, Names
        if (IsAscii(C))
        {
            std::string String;
            while (IsAscii(C) || C == '_')
            {
                String += C;
                C = Advance();
            }

            if (Contains(FUNCTION, String))
            {
                return Token{Func, String, Line, Column};
            }

            if (Contains(TYPES, String))
            {
                return Token{ETokenType::Type, String, Line, Column};
            }

            if (Contains(KEYWORDS, String))
            {
                return Token{GetTokenTypeFromString(String), String, Line, Column};
            }

            if (Contains({"true", "false"}, String))
            {
                return Token{ETokenType::Bool, String, Line, Column};
            }

            return Token{ETokenType::Name, String, Line, Column};
        }

        // Strings
        if (C == '"' || C == '\'')
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
            return Token{ETokenType::String, String, Line, Column};
        }

        // End of file
        if (C == '\0')
        {
            return Token{ETokenType::Eof, "\0", Line, Column};
        }

        throw(std::runtime_error(std::format("Invalid character found: {}", C)));
    }

    std::vector<Token> Tokenize()
    {
        // First split the source into separate lines
        Position = 0;
        auto Stream = std::stringstream{Source};

        // In input is a single line
        if (std::ranges::count(Source, '\n') <= 1)
        {
            Lines.push_back(Source);
        }
        // If input is multiple lines
        else
        {
            for (std::string LineContent; std::getline(Stream, LineContent, '\n');)
            {
                Lines.push_back(LineContent);
            }
        }

        // Then tokenize the source
        std::vector<Token> Tokens;
        while (Position < Source.size())
        {
            if (Position >= Source.size())
            {
                break;
            }
            Token T = Next();
            if (T.Line > Lines.size())
            {
                break;
            }
            T.Source = Lines[T.Line - 1];
            Tokens.push_back(T);
        }
        return Tokens;
    }
};
