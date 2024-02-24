#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <variant>
#include <typeinfo>
#include <format>

#include "BuiltIns.h"
#include "Logging.h"
#include "Token.h"
#include "Value.h"

using namespace Core;
using namespace Values;

#define CHECK_ERRORS                                                             \
    if (Logging::Logger::GetInstance()->GetCount(Logging::LogLevel::Error) > 0) \
    {                                                                            \
        DEBUG_EXIT                                                               \
        return false;                                                            \
    }

#define CHECK_ACCEPT(X)    \
    if (!(X)->Accept(this)) \
    {                      \
        DEBUG_EXIT         \
        return false;      \
    }

static int WHILE_MAX_LOOP = 100000;
static int LINE;
static int COLUMN;
static std::string SOURCE;

class Visitor;

class AstNode;
class AstValue;
class AstIdentifier;
class AstUnaryExpr;
class AstBinOp;
class AstAssignment;
class AstCall;
class AstIf;
class AstWhile;
class AstFunction;
class AstReturn;
class AstBody;

enum ECallType
{
    Function,
    IndexOf
};

static TFunctionMap FUNCTION_MAP = BuiltIns::InitFunctionMap();

static bool IsBuiltIn(const std::string& Name);

static std::string FormatSource();

struct Frame
{
    std::vector<TObject*> Stack;
    std::map<std::string, TObject*> Identifiers;

    Frame* Outer = nullptr;
    Frame* Inner;

    Frame* CreateInnerFrame()
    {
        Inner = new Frame();
        Inner->Outer = this;
        return Inner;
    }

    TObject* GetIdentifier(const std::string& Name)
    {
        for (const auto& Key : Identifiers | std::views::keys)
        {
            if (Key == Name)
            {
                return Identifiers.at(Key);
            }
        }

        if (Outer != nullptr)
        {
            // std::cout << std::format("Looking up the stack for {}", Name) << std::endl;
            return Outer->GetIdentifier(Name);
        }

        return nullptr;
    }

    bool IsIdentifier(const std::string& Name)
    {
        const TObject* Ident = GetIdentifier(Name);
        return Ident != nullptr;
    }

    void SetIdentifier(const std::string& Name, TObject* Value)
    {
        DEBUG_ENTER
        Identifiers[Name] = Value;
        DEBUG_EXIT
    }

    int Push(TObject* Value)
    {
        DEBUG_ENTER
        if (Value->GetType() == NullType)
        {
            DEBUG_EXIT
            return -1;
        }
        Stack.push_back(Value);
        DEBUG_EXIT

        return static_cast<int>(Stack.size());
    }

    TObject* Pop()
    {
        DEBUG_ENTER
        if (IsEmpty())
        {
            Logging::Error("Stack is empty.");
            DEBUG_EXIT
            return nullptr;
        }
        TObject* Result = Stack.back();
        Stack.pop_back();
        DEBUG_EXIT
        return Result;
    }

    bool IsEmpty() const
    {
        size_t Total = Stack.size();
        if (Outer != nullptr)
        {
            Total += Stack.size();
        }
        return Total == 0;
    }
    
    void PrintStack() const
    {
        for (const auto& V : Stack)
        {
            std::cout << V->ToString() << '\n';
        }
    }
};

class Visitor
{
    bool IsFunctionDeclared(const std::string& Name);
    AstFunction* GetFunction(const std::string& Name);

public:
    std::map<std::string, AstFunction*> Functions;

    int FrameDepth = 0;
    Frame RootFrame;
    Frame* CurrentFrame;
    std::vector<Frame> Frames;

    Visitor()
    {
        RootFrame = Frame();
        Frames.push_back(RootFrame);
        CurrentFrame = &Frames.back();
    }
    Visitor(Visitor& Other)
    {
        RootFrame = Other.RootFrame;
        CurrentFrame = Other.CurrentFrame;
        Frames = Other.Frames;
    }
    Visitor(const Visitor& Other)
    {
        RootFrame = Other.RootFrame;
        CurrentFrame = Other.CurrentFrame;
        Frames = Other.Frames;
    }
    bool Visit(AstValue* Node) const;
    bool Visit(AstIdentifier* Node) const;
    bool Visit(const AstUnaryExpr* Node);
    bool Visit(AstBinOp* Node);
    bool Visit(AstAssignment* Node);
    bool Visit(AstCall* Node);
    bool Visit(AstIf* Node);
    bool Visit(const AstWhile* Node);
    bool Visit(AstFunction* Node);
    bool Visit(const AstReturn* Node);
    bool Visit(const AstBody* Node);
    void Dump() const;
};

// Base AST Node class
class AstNode
{
public:
    virtual ~AstNode() = default;
    virtual bool Accept(Visitor* V) = 0;

    /// <summary>
    /// Converts this node to a formatted string.
    /// </summary>
    /// <returns>This node formatted as a string.</returns>
    virtual std::string ToString() const = 0;
    virtual Token GetContext() const = 0;
};

class AstValue : public AstNode
{
public:
    TObject Value;
    Token Context;

    AstValue(TObject& InValue, const Token& InContext)
        : Value(InValue)
          , Context(InContext)
    {
    }
    AstValue(const TObject& InValue, const Token& InContext)
        : Value(InValue)
          , Context(InContext)
    {
    }

    bool Accept(Visitor* V) override { return V->Visit(this); }
    Token GetContext() const override { return Context; }

    bool IsInt() { return Value.GetType() == IntType; }
    bool IsFloat() { return Value.GetType() == FloatType; }
    bool IsString() { return Value.GetType() == StringType; }
    bool IsBool() { return Value.GetType() == BoolType; }
    bool IsArray() { return Value.GetType() == ArrayType; }

    TBoolValue GetBool() const { return Value.GetBool(); }
    TIntValue GetInt() const { return Value.GetInt(); }
    TFloatValue GetFloat() const { return Value.GetFloat(); }
    TStringValue GetString() const { return Value.GetString(); }
    TArrayValue GetArray() const { return Value.GetArray(); }

    TBoolValue* AsBool() const { return Value.AsBool(); }
    TIntValue* AsInt() const { return Value.AsInt(); }
    TFloatValue* AsFloat() const { return Value.AsFloat(); }
    TStringValue* AsString() const { return Value.AsString(); }
    TArrayValue* AsArray() const { return Value.AsArray(); }

    TObject GetValue()
    {
        if (IsBool())
        {
            return GetBool();
        }
        if (IsInt())
        {
            return GetInt();
        }
        if (IsFloat())
        {
            return GetFloat();
        }
        if (IsString())
        {
            return GetString();
        }
        if (IsArray())
        {
            return GetArray();
        }
        throw std::runtime_error("Unable to get value for this object.");
    }

    std::string ToString() const override
    {
        std::string Result;
        return Result;
    }
};

class AstIdentifier : public AstNode
{
public:
    std::string Name;
    TObject Value;
    Token Context;

    AstIdentifier(const std::string& InName, const Token& InContext)
        : Name(InName)
          , Context(InContext)
    {
    }
    std::string ToString() const override { return "Variable: " + Name + ", " + Value.ToString(); }
    bool Accept(Visitor* V) override { return V->Visit(this); }
    Token GetContext() const override { return Context; }
};

class AstUnaryExpr : public AstNode
{
public:
    ETokenType Op;
    AstNode* Right = nullptr;
    Token Context;

    AstUnaryExpr(ETokenType InOp, AstNode* InRight, const Token& InContext)
        : Op(InOp)
          , Right(InRight)
          , Context(InContext)
    {
    }
    std::string ToString() const override
    {
        return std::format("UnaryExpr: {}{}", TokenToStringMap[Op], Right->ToString());
    }
    bool Accept(Visitor* V) override { return V->Visit(this); }
    Token GetContext() const override { return Context; }
};

class AstBinOp : public AstNode
{
    std::string OpString;
    Token Context;

public:
    AstNode* Left = nullptr;
    AstNode* Right = nullptr;
    ETokenType Op = Invalid;

    AstBinOp(AstNode* InLeft, AstNode* InRight, const ETokenType& InOp, const Token& InContext)
        : Context(InContext)
          , Left(InLeft)
          , Right(InRight)
          , Op(InOp)
    {
    }
    std::string ToString() const override
    {
        return "BinOp{\"Left: " + Left->ToString() + ", \"Op: \"" + OpString
            + "\", Right: " + (Right ? Right->ToString() : "none") + "}";
    }

    bool Accept(Visitor* V) override { return V->Visit(this); }
    Token GetContext() const override { return Context; }
};

class AstAssignment : public AstNode
{
public:
    std::string Name;
    AstNode* Right;
    Token Context;

    AstAssignment(const std::string& InName, AstNode* InRight, const Token& InContext)
        : Name(InName)
          , Right(InRight)
          , Context(InContext)
    {
    }
    std::string ToString() const override { return "Assign: " + Name + " => {" + Right->ToString() + "}"; }

    bool Accept(Visitor* V) override { return V->Visit(this); }
    Token GetContext() const override { return Context; }
};

class AstCall : public AstNode
{
public:
    std::string Identifier;
    ECallType Type;
    std::vector<AstNode*> Args;
    Token Context;

    AstCall(const std::string& InIdentifier, const ECallType InType, const std::vector<AstNode*>& InArgs, const Token& InContext)
        : Identifier(InIdentifier)
          , Type(InType)
          , Args(InArgs)
          , Context(InContext)
    {
    }
    std::string ToString() const override { return "Call"; }

    bool Accept(Visitor* V) override { return V->Visit(this); }
    Token GetContext() const override { return Context; }
};

class AstIf : public AstNode
{
public:
    AstNode* Cond = nullptr;
    AstNode* TrueBody = nullptr;
    AstNode* FalseBody = nullptr;
    Token Context;

    AstIf(AstNode* InCond, AstNode* InTrueBody, AstNode* InFalseBody, const Token& InContext)
        : Cond(InCond)
          , TrueBody(InTrueBody)
          , FalseBody(InFalseBody)
          , Context(InContext)
    {
    }
    std::string ToString() const override { return "Conditional"; }
    bool Accept(Visitor* V) override { return V->Visit(this); }
    Token GetContext() const override { return Context; }
};

class AstWhile : public AstNode
{
public:
    AstNode* Cond = nullptr;
    AstNode* Body = nullptr;
    Token Context;

    AstWhile(AstNode* InCond, AstNode* InBody, const Token& InContext)
        : Cond(InCond)
          , Body(InBody)
          , Context(InContext)
    {
    }
    std::string ToString() const override { return "While"; }
    bool Accept(Visitor* V) override { return V->Visit(this); }
    Token GetContext() const override { return Context; }
};

class AstFunction : public AstNode
{
public:
    std::string Name;
    std::vector<std::string> Args;
    AstNode* Body = nullptr;
    Token Context;

    AstFunction(const std::string& InName, const std::vector<std::string>& InArgs, AstNode* InBody, const Token& InContext)
        : Name(InName)
          , Args(InArgs)
          , Body(InBody)
          , Context(InContext)
    {
    }
    std::string ToString() const override { return "FunctionDecl"; }
    bool Accept(Visitor* V) override { return V->Visit(this); }
    Token GetContext() const override { return Context; }
};

class AstReturn : public AstNode
{
public:
    AstNode* Expr;
    Token Context;
    AstReturn(AstNode* InExpr, const Token& InContext)
        : Expr(InExpr)
          , Context(InContext)
    {
    }
    std::string ToString() const override { return "FunctionDecl"; }
    bool Accept(Visitor* V) override { return V->Visit(this); }
    Token GetContext() const override { return Context; }
};

class AstBody : public AstNode
{
public:
    std::vector<std::string> Errors;
    std::vector<AstNode*> Expressions;
    Token Context;

    AstBody(const std::vector<AstNode*>& InBody, const Token& InContext)
        : Expressions(InBody)
          , Context(InContext)
    {
    }
    std::string ToString() const override
    {
        std::string Out;
        for (const AstNode* E : Expressions)
        {
            Out += E->ToString() + "\n";
        }
        return Out;
    }
    bool Accept(Visitor* V) override { return V->Visit(this); }
    bool Succeeded() const { return Errors.empty(); }
    Token GetContext() const override { return Context; }
};

/// <summary>
/// Parses a list of tokens into an Abstract Syntax Tree (AST).
/// </summary>
class Ast
{
    AstBody* Program;
    std::vector<Token> Tokens{};
    std::vector<AstNode*> Expressions{};
    Token* CurrentToken;
    int Position;

    const std::map<std::string, EValueType> StringTypeMap{
        {"void", NullType},
        {"bool", BoolType},
        {"int", IntType},
        {"float", FloatType},
    };

    void PrintCurrentToken() const;

    /// <summary>
    /// Accept the current token. This will increment the <paramref name="CurrentToken"/> pointer as well as increment
    /// the <paramref name="Position"/> value.
    /// </summary>
    void Accept()
    {
        // Get the last token in the token list
        const Token* End = &Tokens.back();

        if (!CurrentToken)
        {
            return;
        }

        // std::cout << std::format("Accepting '{}': {}", CurrentToken->Content, CurrentToken->Source) << std::endl;

        // If we're at the end, set the CurrentToken to be null
        if (CurrentToken == End)
        {
            CurrentToken = nullptr;
        }
        // Otherwise increment the CurrentToken pointer and the position
        else
        {
            CurrentToken++;
            Position++;

            if (CurrentToken != nullptr)
            {
                const Token C = *CurrentToken;
                LINE = C.Line;
                COLUMN = C.Column;
                SOURCE = C.Source;
            }
            else
            {
                LINE = 0;
                COLUMN = 0;
                SOURCE = "eof";
            }
        }
    }

    /// <summary>
    /// Expect the specified <paramref name="Type"/> at the specified <paramref name="Offset"/>, relative to the current
    /// position.
    /// </summary>
    /// <param name="Type">The type to check for.</param>
    /// <param name="Offset">The offset position.</param>
    /// <returns>Whether the type is found.</returns>
    bool Expect(const ETokenType Type, const int Offset = 0) const
    {
        // Make sure the offset is valid
        if (Position + Offset > static_cast<int>(Tokens.size()))
        {
            Logging::Error("Outside token bounds.");
            return false;
        }

        return Tokens[Position + Offset].Type == Type;
    }

    /// <summary>
    /// Expect the given <paramref name="Types"/> in sequential order at the current position.
    /// </summary>
    /// <param name="Types">The types to check for.</param>
    /// <param name="Offset">The token offset to start at.</param>
    /// <returns>Whether the types are all found.</returns>
    bool ExpectSequence(const std::initializer_list<ETokenType>& Types, int Offset = 0) const
    {
        for (auto [Index, Type] : Enumerate(Types))
        {
            if (!Expect(Type, Offset + static_cast<int>(Index)))
            {
                return false;
            }
        }
        return true;
    }

    bool ExpectAny(const std::initializer_list<ETokenType>& Types, int Offset = 0) const
    {
        for (const auto& Type : Types)
        {
            if (Expect(Type, Offset))
            {
                return true;
            }
        }
        return false;
    }

    bool ExpectValue(int Offset = 0) const { return ExpectAny({Name, Bool, Number, String}, Offset); }
    bool ExpectAssignOperator(int Offset = 0) const
    {
        return ExpectAny({Assign, PlusEquals, MinusEquals, MultEquals, DivEquals}, Offset);
    }
    bool ExpectUnaryOperator(int Offset = 0) const
    {
        return ExpectAny({Minus, PlusPlus, MinusMinus, Period, LBracket}, Offset);
    }

    AstNode* ParseValueExpr();
    AstNode* ParseIdentifier();
    AstNode* ParseUnaryExpr();
    AstNode* ParseMultiplicativeExpr();
    AstNode* ParseAdditiveExpr();
    AstNode* ParseEqualityExpr();
    AstNode* ParseAssignment();
    AstNode* ParseParenExpr();
    AstNode* ParseBracketExpr();
    AstNode* ParseCurlyExpr();
    AstNode* ParseIf();
    AstNode* ParseWhile();
    AstNode* ParseFunctionDecl();
    AstNode* ParseExpression();
    AstNode* ParseBody();

public:
    explicit Ast(const std::vector<Token>& InTokens)
        : Tokens(InTokens)
    {
        CurrentToken = Tokens.data();
        Position = 0;
        Program = Cast<AstBody>(ParseBody());
    }

    /// <summary>
    /// Get the parsed node tree.
    /// </summary>
    /// <returns>The root AST node.</returns>
    AstBody* GetTree() const { return Program; }
};
