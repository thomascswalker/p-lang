#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <variant>
#include <typeinfo>
#include <format>
#include <any>

#include "BuiltIns.h"
#include "Logging.h"
#include "Token.h"
#include "Value.h"

using namespace Core;
using namespace Values;

#define CHECK_ERRORS                                                             \
    if (Logging::Logger::GetInstance()->GetCount(Logging::LogLevel::_Error) > 0) \
    {                                                                            \
        DEBUG_EXIT                                                               \
        return false;                                                            \
    }

#define CHECK_ACCEPT(X)    \
    if (!X->Accept(*this)) \
    {                      \
        DEBUG_EXIT         \
        return false;      \
    }

static int		   WHILE_MAX_LOOP = 100000;
static int		   LINE;
static int		   COLUMN;
static std::string SOURCE;

class Visitor;

class ASTNode;
class ASTValue;
class ASTIdentifier;
class ASTUnaryExpr;
class ASTBinOp;
class ASTAssignment;
class ASTCall;
class ASTIf;
class ASTWhile;
class ASTFunction;
class ASTReturn;
class ASTBody;

enum ECallType
{
    Function,
    IndexOf
};

static TFunctionMap FunctionMap = BuiltIns::InitFunctionMap();

static bool IsBuiltIn(const std::string& Name)
{
    for (const auto& [K, V] : FunctionMap)
    {
        if (K == Name)
        {
            return true;
        }
    }
    return false;
}

struct Frame;

struct Frame
{
    std::vector<TObject>		   Stack;
    std::map<std::string, TObject> Identifiers;

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
        TObject* Result = nullptr;

        for (const auto& [K, V] : Identifiers)
        {
            if (K == Name)
            {
                return Result = &Identifiers.at(K);
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
        auto Ident = GetIdentifier(Name);
        return Ident != nullptr;
    }

    void SetIdentifier(const std::string& Name, const TObject& Value)
    {
        DEBUG_ENTER
        Identifiers[Name] = Value;
        DEBUG_EXIT
    }

    int Push(const TObject& Value)
    {
        DEBUG_ENTER
        if (Value.GetType() == NullType)
        {
            DEBUG_EXIT
            return -1;
        }
        Stack.push_back(Value);
        DEBUG_EXIT
    }

    TObject Pop()
    {
        DEBUG_ENTER
        TObject Result;
        if (IsEmpty())
        {
            Logging::Error("Stack is empty.");
            DEBUG_EXIT
            return Result;
        }
        Result = Stack.back();
        Stack.pop_back();
        DEBUG_EXIT
        return Result;
    }

    bool IsEmpty()
    {
        int Total = Stack.size();
        if (Outer != nullptr)
        {
            Total += Stack.size();
        }
        return Total == 0;
    }
    void PrintStack()
    {
        for (const auto& V : Stack)
        {
            std::cout << V.ToString() << std::endl;
        }
    }
};

class Visitor
{
    bool		 IsFunctionDeclared(const std::string& Name);
    ASTFunction* GetFunction(const std::string& Name);

public:
    std::map<std::string, ASTFunction*> Functions;

    int				   FrameDepth = 0;
    Frame			   RootFrame;
    Frame*			   CurrentFrame;
    std::vector<Frame> Frames;

    Visitor()
    {
        RootFrame = Frame();
        Frames.push_back(RootFrame);
        CurrentFrame = &Frames.back();
    };
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
    bool Visit(ASTValue* Node);
    bool Visit(ASTIdentifier* Node);
    bool Visit(ASTUnaryExpr* Node);
    bool Visit(ASTBinOp* Node);
    bool Visit(ASTAssignment* Node);
    bool Visit(ASTCall* Node);
    bool Visit(ASTIf* Node);
    bool Visit(ASTWhile* Node);
    bool Visit(ASTFunction* Node);
    bool Visit(ASTReturn* Node);
    bool Visit(ASTBody* Node);

    bool Succeeded() { return true; } // TODO: Update this!
    void Dump();

    void GoIn()
    {
        CurrentFrame->CreateInnerFrame();
        CurrentFrame = CurrentFrame->Inner;
        FrameDepth += 1;
        // std::cout << std::format("In: {}", FrameDepth) << std::endl;
    }

    void GoOut()
    {
        if (CurrentFrame->Outer != nullptr)
        {
            CurrentFrame = CurrentFrame->Outer;
            delete CurrentFrame->Inner;
            FrameDepth -= 1;
            // std::cout << std::format("Out: {}\n-----\n", FrameDepth) << std::endl;
        }
    }
};

// Base AST Node class
class ASTNode
{
public:
    virtual ~ASTNode(){};
    virtual bool Accept(Visitor& V) = 0;

    /// <summary>
    /// Converts this node to a formatted string.
    /// </summary>
    /// <returns>This node formatted as a string.</returns>
    virtual std::string ToString() const = 0;
    virtual Token		GetContext() const = 0;
};

class ASTValue : public ASTNode
{
public:
    TObject Value;
    Token	Context;

    ASTValue(TObject& InValue, Token InContext) : Value(InValue), Context(InContext){};
    ASTValue(const TObject& InValue, Token InContext) : Value(InValue), Context(InContext){};

    bool  Accept(Visitor& V) override { return V.Visit(this); }
    Token GetContext() const override { return Context; }

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

    TBoolValue*	  AsBool() const { return Value.AsBool(); }
    TIntValue*	  AsInt() const { return Value.AsInt(); }
    TFloatValue*  AsFloat() const { return Value.AsFloat(); }
    TStringValue* AsString() const { return Value.AsString(); }
    TArrayValue*  AsArray() const { return Value.AsArray(); }

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

class ASTIdentifier : public ASTNode
{
public:
    std::string Name;
    TObject		Value;
    Token		Context;

    ASTIdentifier(const std::string& InName, Token InContext) : Name(InName), Context(InContext){};
    virtual std::string ToString() const { return "Variable: " + Name + ", " + Value.ToString(); }
    bool				Accept(Visitor& V) override { return V.Visit(this); }
    Token				GetContext() const override { return Context; }
};

class ASTUnaryExpr : public ASTNode
{
public:
    ETokenType Op;
    ASTNode*   Right = nullptr;
    Token	   Context;

    ASTUnaryExpr(ETokenType InOp, ASTNode* InRight, Token InContext) : Op(InOp), Right(InRight), Context(InContext){};
    virtual std::string ToString() const
    {
        return std::format("UnaryExpr: {}{}", TokenToStringMap[Op], Right->ToString());
    }
    bool  Accept(Visitor& V) override { return V.Visit(this); }
    Token GetContext() const override { return Context; }
};

class ASTBinOp : public ASTNode
{
    std::string OpString;
    Token		Context;

public:
    ASTNode*   Left = nullptr;
    ASTNode*   Right = nullptr;
    ETokenType Op = Invalid;

    ASTBinOp(ASTNode* InLeft, ASTNode* InRight, ETokenType& InOp, Token InContext)
        : Left(InLeft), Right(InRight), Op(InOp), Context(InContext){};
    virtual std::string ToString() const
    {
        return "BinOp{\"Left: " + Left->ToString() + ", \"Op: \"" + OpString
               + "\", Right: " + (Right ? Right->ToString() : "none") + "}";
    }

    bool  Accept(Visitor& V) override { return V.Visit(this); }
    Token GetContext() const override { return Context; }
};

class ASTAssignment : public ASTNode
{
public:
    std::string Name;
    ASTNode*	Right;
    Token		Context;

    ASTAssignment(const std::string& InName, ASTNode* InRight, Token InContext)
        : Name(InName), Right(InRight), Context(InContext){};
    virtual std::string ToString() const { return "Assign: " + Name + " => {" + Right->ToString() + "}"; }

    bool  Accept(Visitor& V) override { return V.Visit(this); }
    Token GetContext() const override { return Context; }
};

class ASTCall : public ASTNode
{
public:
    std::string			  Identifier;
    ECallType			  Type;
    std::vector<ASTNode*> Args;
    Token				  Context;

    ASTCall(const std::string& InIdentifier, ECallType InType, std::vector<ASTNode*> InArgs, Token InContext)
        : Identifier(InIdentifier), Type(InType), Args(InArgs), Context(InContext){};
    virtual std::string ToString() const { return "Call"; }

    bool  Accept(Visitor& V) override { return V.Visit(this); }
    Token GetContext() const override { return Context; }
};

class ASTIf : public ASTNode
{
public:
    ASTNode* Cond = nullptr;
    ASTNode* TrueBody = nullptr;
    ASTNode* FalseBody = nullptr;
    Token	 Context;

    ASTIf(ASTNode* InCond, ASTNode* InTrueBody, ASTNode* InFalseBody, Token InContext)
        : Cond(InCond), TrueBody(InTrueBody), FalseBody(InFalseBody), Context(InContext){};
    virtual std::string ToString() const { return "Conditional"; }
    bool				Accept(Visitor& V) override { return V.Visit(this); }
    Token				GetContext() const override { return Context; }
};

class ASTWhile : public ASTNode
{
public:
    ASTNode* Cond = nullptr;
    ASTNode* Body = nullptr;
    Token	 Context;

    ASTWhile(ASTNode* InCond, ASTNode* InBody, Token InContext) : Cond(InCond), Body(InBody), Context(InContext){};
    virtual std::string ToString() const { return "While"; }
    bool				Accept(Visitor& V) override { return V.Visit(this); }
    Token				GetContext() const override { return Context; }
};

class ASTFunction : public ASTNode
{
public:
    std::string				 Name;
    std::vector<std::string> Args;
    ASTNode*				 Body = nullptr;
    Token					 Context;

    ASTFunction(const std::string& InName, std::vector<std::string> InArgs, ASTNode* InBody, Token InContext)
        : Name(InName), Args(InArgs), Body(InBody), Context(InContext){};
    virtual std::string ToString() const { return "FunctionDecl"; }
    bool				Accept(Visitor& V) override { return V.Visit(this); }
    Token				GetContext() const override { return Context; }
};

class ASTReturn : public ASTNode
{
public:
    ASTNode* Expr;
    Token	 Context;
    ASTReturn(ASTNode* InExpr, Token InContext) : Expr(InExpr), Context(InContext){};
    virtual std::string ToString() const { return "FunctionDecl"; }
    bool				Accept(Visitor& V) override { return V.Visit(this); }
    Token				GetContext() const override { return Context; }
};

class ASTBody : public ASTNode
{
public:
    std::vector<std::string> Errors;
    std::vector<ASTNode*>	 Expressions;
    Token					 Context;

    ASTBody(std::vector<ASTNode*> InBody, Token InContext) : Expressions(InBody), Context(InContext){};
    virtual std::string ToString() const
    {
        std::string Out;
        for (const ASTNode* E : Expressions)
        {
            Out += E->ToString() + "\n";
        };
        return Out;
    }
    bool  Accept(Visitor& V) override { return V.Visit(this); }
    bool  Succeeded() { return Errors.size() == 0; }
    Token GetContext() const override { return Context; }
};

/// <summary>
/// Parses a list of tokens into an Abstract Syntax Tree (AST).
/// </summary>
class AST
{
private:
    ASTBody*			  Program;
    std::vector<Token>	  Tokens{};
    std::vector<ASTNode*> Expressions{};
    Token*				  CurrentToken;
    int					  Position;

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
                auto C = *CurrentToken;
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
    bool Expect(ETokenType Type, int Offset = 0)
    {
        // Make sure the offset is valid
        if (Position + Offset > Tokens.size())
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

    bool ExpectValue(int Offset = 0) { return ExpectAny({ Name, Bool, Number, String }, Offset); }
    bool ExpectAssignOperator(int Offset = 0)
    {
        return ExpectAny({ Assign, PlusEquals, MinusEquals, MultEquals, DivEquals }, Offset);
    }
    bool ExpectUnaryOperator(int Offset = 0)
    {
        return ExpectAny({ Minus, PlusPlus, MinusMinus, Period, LBracket }, Offset);
    }

    ASTNode* ParseValueExpr();
    ASTNode* ParseIdentifier();
    ASTNode* ParseUnaryExpr();
    ASTNode* ParseMultiplicativeExpr();
    ASTNode* ParseAdditiveExpr();
    ASTNode* ParseEqualityExpr();
    ASTNode* ParseAssignment();
    ASTNode* ParseParenExpr();
    ASTNode* ParseBracketExpr();
    ASTNode* ParseCurlyExpr();
    ASTNode* ParseIf();
    ASTNode* ParseWhile();
    ASTNode* ParseFunctionDecl();
    ASTNode* ParseExpression();
    ASTNode* ParseBody();

public:
    AST(std::vector<Token>& InTokens) : Tokens(InTokens)
    {
        CurrentToken = &Tokens[0];
        Position = 0;
        Program = Cast<ASTBody>(ParseBody());
    }

    /// <summary>
    /// Get the parsed node tree.
    /// </summary>
    /// <returns>The root AST node.</returns>
    ASTBody* GetTree() { return Program; }
};