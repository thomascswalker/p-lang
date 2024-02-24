#include "../Public/Ast.h"

#include <cassert>
#include <ranges>

using namespace Core;


bool IsBuiltIn(const std::string& Name)
{
    for (const auto& Key : FUNCTION_MAP | std::views::keys)
    {
        if (Key == Name)
        {
            return true;
        }
    }
    return false;
}

std::string FormatSource()
{
    return std::format("line {}, column {}\n\t{}\n\t{}^", LINE, COLUMN, SOURCE, std::string(COLUMN, ' '));
}

//////////////
// Visitors //
//////////////

bool Visitor::IsFunctionDeclared(const std::string& Name)
{
    const auto Func = GetFunction(Name);
    return (Func != nullptr);
}

AstFunction* Visitor::GetFunction(const std::string& Name)
{
    for (const auto& [K, V] : Functions)
    {
        if (K == Name)
        {
            return Functions.at(K);
        }
    }
    return nullptr;
}

bool Visitor::Visit(AstValue* Node) const
{
    DEBUG_ENTER
    CurrentFrame->Push(&Node->Value);

    DEBUG_EXIT
    return true;
}

bool Visitor::Visit(AstIdentifier* Node) const
{
    DEBUG_ENTER

    auto T = CurrentFrame->GetIdentifier(Node->Name);
    if (T == nullptr)
    {
        Logging::Error("'{}' is undefined.", Node->Name);
        auto Context = Node->GetContext();
        Logging::Error("line {}, column {}", Context.Line, Context.Column);
        return false;
    }
    Node->Value = *T;
    if (Node->Value.GetType() == NullType)
    {
        Logging::Error("'{}' is undefined.", Node->Name);
        auto Context = Node->GetContext();
        Logging::Error("line {}, column {}", Context.Line, Context.Column);
        return false;
    }

    // If the variable is found, push the variable's value to the stack
    Logging::Debug("'{}' is {}.", Node->Name, Node->Value.ToString());
    CurrentFrame->Push(&Node->Value);
    DEBUG_EXIT
    return true;
}

bool Visitor::Visit(const AstUnaryExpr* Node)
{
    DEBUG_ENTER
    CHECK_ACCEPT(Node->Right)
    TObject* CurrentValue = CurrentFrame->Pop();

    switch (Node->Op)
    {
    case Not :
        *CurrentValue = *CurrentValue - TObject(1);
        break;
    case Minus :
        *CurrentValue = *CurrentValue * TObject(-1);
        break;
    default :
        Logging::Error("Operator is not a valid unary operator.");
        CHECK_ERRORS
    }

    CurrentFrame->Push(CurrentValue);
    DEBUG_EXIT
    return true;
}

bool Visitor::Visit(AstBinOp* Node)
{
    DEBUG_ENTER
    // Visit the left value
    CHECK_ACCEPT(Node->Left)

    // After visiting the left value, pop it off the stack and store it here
    TObject* Left = CurrentFrame->Pop();
    TObject OriginalLeftValue = *Left;

    // Visit the right value
    CHECK_ACCEPT(Node->Right)

    // After visiting the right value, pop it off the stack and store it here
    TObject* Right = CurrentFrame->Pop();

    // Execute the operator on the left and right value
    switch (Node->Op)
    {
    case Plus :
    case PlusEquals :
        *Left = *Left + *Right;
        break;
    case Minus :
    case MinusEquals :
        *Left = *Left - *Right;
        break;
    case Multiply :
    case MultEquals :
        *Left = *Left * *Right;
        break;
    case Divide :
    case DivEquals :
        *Left = *Left / *Right;
        break;
    case LessThan :
        *Left = *Left < *Right;
        break;
    case GreaterThan :
        *Left = *Left > *Right;
        break;
    case Equals :
        *Left = *Left == *Right;
        break;
    case NotEquals :
        *Left = *Left != *Right;
        break;
    default :
        break;
    }

    // Push the resulting value to the stack
    CurrentFrame->Push(Left);
    Logging::Debug("BINOP: {} {} {} = {}", Left->ToString(), TokenToStringMap[Node->Op], Right->ToString(),
                   OriginalLeftValue.ToString());
    DEBUG_EXIT
    return true;
}

bool Visitor::Visit(AstAssignment* Node)
{
    DEBUG_ENTER

    CHECK_ACCEPT(Node->Right)
    // CHECK_ERRORS

    TObject* Value = CurrentFrame->Pop();
    CHECK_ERRORS

    if (Value->GetType() == NullType)
    {
        Logging::Error("Cannot assign nulltype.\n{}", FormatSource());
        DEBUG_EXIT
        return false;
    }

    CurrentFrame->SetIdentifier(Node->Name, Value);

    Logging::Debug("ASSIGN: {} <= {}", Node->Name, Value->ToString());
    DEBUG_EXIT
    return true;
}

bool Visitor::Visit(AstCall* Node)
{
    DEBUG_ENTER

    if (Node->Type == IndexOf)
    {
        if (Node->Args.size() != 1)
        {
            Logging::Error("Invalid argument count for subscript operator.");
            CHECK_ERRORS
        }
        CHECK_ACCEPT(Node->Args[0])

        const TObject* Index = CurrentFrame->Pop();
        CHECK_ERRORS
        int IndexValue = Index->GetInt().GetValue();

        TObject* IdentifierPtr = CurrentFrame->GetIdentifier(Node->Identifier);
        if (!IdentifierPtr)
        {
            Logging::Error("Unable to find identifier {}.", Node->Identifier);
            CHECK_ERRORS
        }
        TObject* Value;
        TObject Char;
        switch (IdentifierPtr->GetType())
        {
        case StringType :
            Char = IdentifierPtr->At(IndexValue);
            CurrentFrame->Push(&Char);
            break;
        case ArrayType :
            Value = IdentifierPtr->AsArray()->GetValue().data() + IndexValue;
            CurrentFrame->Push(Value);
            break;
        default :
            Logging::Error("Invalid identifier type.");
            CHECK_ERRORS
            break ;
        }
    }
    else if (Node->Type == Function)
    {
        // Parse argument values
        TArguments InArgs;
        for (AstNode* Arg : Node->Args)
        {
            // Evaluate identifiers
            if (Cast<AstIdentifier>(Arg))
            {
                // Cast to an identifier
                AstIdentifier* Identifier = Cast<AstIdentifier>(Arg);

                // Get the corresponding identifier name and pointer
                std::string ArgName = Identifier->Name;
                TObject* ArgValue = CurrentFrame->GetIdentifier(ArgName);

                // Add this as a new variable argument. The key here is the
                // ArgValue is a pointer to the `Identifiers` array so we can
                // modify that value in place rather than pass around a bunch
                // of copies.
                InArgs.push_back(std::make_shared<TVariable>(ArgName, ArgValue));
            }
            // Evaluate literal values
            else if (AstValue* ValueArg = Cast<AstValue>(Arg))
            {
                // Add the copy of the literal value to the argument list.
                InArgs.push_back(std::make_shared<TLiteral>(ValueArg->Value));
            }
            else
            {
                Logging::Error("Invalid argument type.");
                CHECK_ERRORS
            }
        }

        // Handle built-in functions
        if (IsBuiltIn(Node->Identifier))
        {
            // Temporary return value for the function
            TObject* ReturnValue = nullptr;

            // Get the corresponding function pointer to the identifier name
            auto Func = FUNCTION_MAP[Node->Identifier];

            // Invoke the function with the arguments parsed above
            bool bResult = Func.Invoke(&InArgs, ReturnValue);
            if (!bResult && !ReturnValue)
            {
                auto Context = Node->GetContext();
                auto Space = std::string(Context.Column, ' ');
                Logging::Error("line {}, column {}\n\t{}\n\t{}^", Context.Line, Context.Column, Context.Source, Space);
                CHECK_ERRORS
            }

            if (ReturnValue)
            {
                CurrentFrame->Push(ReturnValue);
            }
        }
        // Handle user-defined functions
        else if (IsFunctionDeclared(Node->Identifier))
        {
            // Get the function AST node for this identifier
            auto Func = Functions[Node->Identifier];

            // Make sure in arguments are the same count as expected arguments
            if (InArgs.size() != Func->Args.size())
            {
                Logging::Error("Argument count mismatch for '{}'. Got {}, wanted {}.", Node->Identifier, InArgs.size(),
                               Func->Args.size());
                CHECK_ERRORS
            }

            // Push arguments to the variable table
            for (const auto& [Index, InArg] : Enumerate(InArgs))
            {
                TObject* Value = InArg->GetValuePtr();
                CurrentFrame->SetIdentifier(Func->Args[Index], Value);
            }

            // Execute the function body
            CHECK_ACCEPT(Func->Body)
        }
        else
        {
            Logging::Error("Function '{}' is undeclared.", Node->Identifier);
            CHECK_ERRORS
        }
    }

    DEBUG_EXIT
    return true;
}

bool Visitor::Visit(AstIf* Node)
{
    DEBUG_ENTER

    CHECK_ACCEPT(Node->Cond)

    bool bResult = CurrentFrame->Pop();

    Logging::Debug("IF: {}", bResult? "true" : "false");
    if (bResult)
    {
        CHECK_ACCEPT(Node->TrueBody)
    }
    else
    {
        CHECK_ACCEPT(Node->FalseBody)
    }
    DEBUG_EXIT
    return true;
}

bool Visitor::Visit(const AstWhile* Node)
{
    DEBUG_ENTER
    TBoolValue bResult = true;
    int Count = 1;

    while (bResult)
    {
        // std::cout << std::format("While count: {}", Count) << std::endl;
        CHECK_ACCEPT(Node->Cond)

        bResult = CurrentFrame->Pop()->GetBool();
        Logging::Debug("WHILE ({}): {}", Count, bResult ? "true" : "false");
        if (!bResult)
        {
            break;
        }

        CHECK_ACCEPT(Node->Body)

        Count++;
        if (Count == WHILE_MAX_LOOP)
        {
            Logging::Error("ERROR: Hit max loop count ({}).", WHILE_MAX_LOOP);

            CHECK_ERRORS
        }
    }

    DEBUG_EXIT
    return true;
}

bool Visitor::Visit(AstFunction* Node)
{
    if (!Functions.contains(Node->Name))
    {
        Functions[Node->Name] = Node;
    }
    else
    {
        Logging::Error("Function {} not defined.", Node->Name);
        CHECK_ERRORS
    }
    DEBUG_EXIT
    return true;
}

bool Visitor::Visit(const AstReturn* Node)
{
    Node->Expr->Accept(this);
    if (CurrentFrame->Stack.size() > 0)
    {
        TObject* Value = CurrentFrame->Pop();
        CHECK_ERRORS
        CurrentFrame->Push(Value);
    }
    return true;
}

bool Visitor::Visit(const AstBody* Node)
{
    DEBUG_ENTER
    for (const auto& E : Node->Expressions)
    {
        E->Accept(this);
    }
    DEBUG_EXIT
    return true;
}

void Visitor::Dump() const
{
    std::cout << "Variables:\n";
    for (const auto& [K, V] : CurrentFrame->Identifiers)
    {
        std::cout << K << " : " << V->ToString() << '\n';
    }
}

/////////
// AST //
/////////

void Ast::PrintCurrentToken() const
{
    if (!CurrentToken)
    {
        return;
    }
    std::cout << (std::format("{}", CurrentToken->Content)) << '\n';
}

AstNode* Ast::ParseValueExpr()
{
    DEBUG_ENTER

    // Parse numbers (floats and ints)
    if (Expect(Number))
    {
        TObject Value;

        // Parse float
        if (CurrentToken->Content.find('.') != std::string::npos)
        {
            Value = std::stof(CurrentToken->Content);
            Logging::Debug("VALUE: Parsing number: {}", Value.GetFloat().GetValue());
        }
        // Parse int
        else
        {
            Value = std::stoi(CurrentToken->Content);
            Logging::Debug("VALUE: Parsing number: {}", Value.GetInt().GetValue());
        }

        const auto Expr = new AstValue(Value, *CurrentToken);
        Accept(); // Consume number
        DEBUG_EXIT
        return Expr;
    }
    // Parse strings
    else if (Expect(String))
    {
        std::string String = CurrentToken->Content;
        Logging::Debug("VALUE: Parsing string: {}", String);
        const auto Expr = new AstValue(TObject(String), *CurrentToken);
        Accept(); // Consume string
        DEBUG_EXIT
        return Expr;
    }
    // Parse names (Variables, functions, etc.)
    else if (Expect(Name))
    {
        const auto Expr = ParseIdentifier();
        DEBUG_EXIT
        return Expr;
    }
    else if (Expect(Bool))
    {
        bool Value = CurrentToken->Content == "true" ? true : false;
        Logging::Debug("VALUE: Parsing bool: {}", Value);
        const auto Expr = new AstValue(Value, *CurrentToken);
        Accept(); // Consume bool
        DEBUG_EXIT
        return Expr;
    }

    DEBUG_EXIT
    return nullptr;
}

AstNode* Ast::ParseIdentifier()
{
    DEBUG_ENTER

    const auto Identifier = new AstIdentifier(CurrentToken->Content, *CurrentToken);
    const auto IdentifierToken = *CurrentToken;
    Accept(); // Consume the variable

    if (!ExpectAny({LParen, LBracket, Period}))
    {
        DEBUG_EXIT
        return Identifier;
    }

    const auto StartTok = CurrentToken->Type;
    const auto EndTok = BLOCK_PAIRS.at(CurrentToken->Type);
    Accept(); // Consume '['

    ECallType CallType;
    switch (StartTok)
    {
    case LBracket :
        CallType = IndexOf;
        break;
    case LParen :
        CallType = Function;
        break;
    default :
        Logging::Error("Token {} not supported.", TokenToStringMap[StartTok]);
        DEBUG_EXIT
        return nullptr;
    }
    std::vector<AstNode*> Args;
    while (!Expect(EndTok))
    {
        if (auto Arg = ParseExpression())
        {
            Args.push_back(Arg);
        }
        else
        {
            Logging::Error("Unable to parse argument.");
            DEBUG_EXIT
            return nullptr;
        }
        if (Expect(EndTok))
        {
            break;
        }
        if (!Expect(Comma))
        {
            Logging::Error("Expected ',', got '{}'.", CurrentToken->Content);
            DEBUG_EXIT
            return nullptr;
        }
        Accept(); // Consume ','
    }
    Accept(); // Consume end token

    DEBUG_EXIT
    return new AstCall(Identifier->Name, CallType, Args, IdentifierToken);
}

AstNode* Ast::ParseUnaryExpr()
{
    DEBUG_ENTER

    AstNode* Expr = ParseValueExpr();
    if (ExpectAny({Not, Minus}))
    {
        const auto Op = CurrentToken->Type;
        Accept(); // Consume '!' or '-'
        Expr = new AstUnaryExpr(Op, ParseValueExpr(), *CurrentToken);
    }

    DEBUG_EXIT
    return Expr;
}

AstNode* Ast::ParseMultiplicativeExpr()
{
    DEBUG_ENTER

    AstNode* Expr = ParseUnaryExpr();
    while (Expect(Multiply) || Expect(Divide))
    {
        auto Op = CurrentToken->Type;
        Accept(); // Consume '*' or '/'
        Expr = new AstBinOp(Expr, ParseUnaryExpr(), Op, *CurrentToken);
    }

    DEBUG_EXIT
    return Expr;
}

AstNode* Ast::ParseAdditiveExpr()
{
    DEBUG_ENTER

    AstNode* Expr = ParseMultiplicativeExpr();
    while (Expect(Plus) || Expect(Minus))
    {
        auto Op = CurrentToken->Type;
        Accept(); // Consume '+' or '-'
        Expr = new AstBinOp(Expr, ParseMultiplicativeExpr(), Op, *CurrentToken);
    }

    DEBUG_EXIT
    return Expr;
}

AstNode* Ast::ParseEqualityExpr()
{
    DEBUG_ENTER
    AstNode* Expr = ParseAdditiveExpr();
    while (ExpectAny({GreaterThan, LessThan, NotEquals, Equals}))
    {
        auto Op = CurrentToken->Type;
        Accept(); // Consume '==' or '!=' or '<' or '>'
        Expr = new AstBinOp(Expr, ParseAdditiveExpr(), Op, *CurrentToken);
    }

    DEBUG_EXIT
    return Expr;
}

AstNode* Ast::ParseAssignment()
{
    DEBUG_ENTER

    const std::string Name = CurrentToken->Content; // Get the name
    const auto NameToken = *CurrentToken;
    Accept(); // Consume name
    auto Op = CurrentToken->Type; // Get the assignment operator
    Accept(); // Consume assignment operator

    auto Expr = ParseExpression();
    if (Op == PlusEquals || Op == MinusEquals || Op == MultEquals || Op == DivEquals)
    {
        Expr = new AstBinOp(new AstIdentifier(Name, NameToken), Expr, Op, *CurrentToken);
    }
    DEBUG_EXIT
    return new AstAssignment(Name, Expr, *CurrentToken);
}

AstNode* Ast::ParseParenExpr()
{
    DEBUG_ENTER

    if (!Expect(LParen))
    {
        Logging::Error("Expected '{}' starting conditional. Got '{}'.", "(", CurrentToken->Content);
        DEBUG_EXIT
        return nullptr;
    }
    Accept(); // Consume '('
    AstNode* Expr = ParseExpression();
    if (!Expect(RParen))
    {
        Logging::Error("Expected '{}' ending conditional. Got '{}'.", ")", CurrentToken->Content);
        DEBUG_EXIT
        return nullptr;
    }
    Accept(); // Consume ')'
    DEBUG_EXIT
    return Expr;
}

AstNode* Ast::ParseBracketExpr()
{
    DEBUG_ENTER

    if (Expect(LBracket))
    {
        Accept(); // Consume '['

        TArrayValue Values{};
        while (!Expect(RBracket))
        {
            Logging::Debug("BRACKET: Parsing loop in {}.", __FUNCTION__);
            const auto Value = ParseExpression();
            const auto CastValue = Cast<AstValue>(Value);
            if (!CastValue)
            {
                Logging::Error("Unable to cast value.");
                DEBUG_EXIT
                return nullptr;
            }
            Values.Append(CastValue->GetValue());
            if (Expect(RBracket))
            {
                break;
            }

            if (!Expect(Comma))
            {
                Logging::Error("Expected comma.");
                DEBUG_EXIT
                return nullptr;
            }
            Accept(); // Consume ','
        }

        Accept(); // Consume ']'

        AstValue* Value;
        if (Values.Size().GetValue() == 1)
        {
            Value = new AstValue(Values[0], *CurrentToken);
        }
        else
        {
            Value = new AstValue(Values, *CurrentToken);
        }
        DEBUG_EXIT
        return Value;
    }
    DEBUG_EXIT
    return nullptr;
}

AstNode* Ast::ParseCurlyExpr()
{
    DEBUG_ENTER

    if (!Expect(LCurly))
    {
        Logging::Error("Expected '{}' starting block. Got '{}'.", "{", CurrentToken->Content);
        DEBUG_EXIT
        return nullptr;
    }

    const auto CurlyToken = *CurrentToken;
    Accept(); // Consume '{'

    std::vector<AstNode*> Body;
    while (!Expect(RCurly))
    {
        Logging::Debug("CURLY: Parsing loop in {}.", __FUNCTION__);

        AstNode* Expr = ParseExpression();
        Body.push_back(Expr);

        // Handle any dangling semicolons
        if (Expect(Semicolon))
        {
            Accept();
        }
    }

    Accept(); // Consume '}'

    DEBUG_EXIT
    return new AstBody(Body, CurlyToken);
}

AstNode* Ast::ParseIf()
{
    DEBUG_ENTER

    const auto IfToken = *CurrentToken;
    Accept(); // Consume 'if'
    const auto Cond = ParseParenExpr();
    if (!Cond)
    {
        Logging::Error("Unable to parse 'if'.");
        DEBUG_EXIT
        return nullptr;
    }

    Logging::Debug("IF: Parsing 'if' block.");
    const auto TrueBody = ParseCurlyExpr();
    if (!TrueBody)
    {
        Logging::Error("Unable to parse true body of 'if'.");
        DEBUG_EXIT
        return nullptr;
    }

    AstNode* FalseBody = nullptr;
    if (Expect(Else))
    {
        Accept(); // Consume 'else'
        Logging::Debug("IF: Parsing 'else' block.");
        FalseBody = ParseCurlyExpr();
        if (!FalseBody)
        {
            Logging::Error("Unable to parse false body of 'else'.");
            DEBUG_EXIT
            return nullptr;
        }
    }

    DEBUG_EXIT
    return new AstIf(Cond, TrueBody, FalseBody, IfToken);
}

AstNode* Ast::ParseWhile()
{
    DEBUG_ENTER
    const Token WhileToken = *CurrentToken;
    Accept(); // Consume 'while'

    if (!Expect(LParen))
    {
        Logging::Error("Expected '(', got {}", CurrentToken->Content);
        DEBUG_EXIT
        return nullptr;
    }
    Accept(); // Consume '('
    const auto Cond = ParseEqualityExpr();
    if (!Expect(RParen))
    {
        Logging::Error("Expected ')', got {}", CurrentToken->Content);
        DEBUG_EXIT
        return nullptr;
    }
    Accept(); // Consume ')'
    const auto Body = ParseCurlyExpr();

    if (!Body)
    {
        Logging::Error("Unable to parse while body.");
        DEBUG_EXIT
        return nullptr;
    }

    DEBUG_EXIT
    return new AstWhile(Cond, Body, WhileToken);
}

AstNode* Ast::ParseFunctionDecl()
{
    DEBUG_ENTER
    if (!Expect(Func))
    {
        Logging::Error("Expected function declaration.");
        DEBUG_EXIT
        return nullptr;
    }

    const auto FuncToken = *CurrentToken;
    Accept(); // Consume 'func'

    if (!Expect(Name))
    {
        Logging::Error("Expected function name.");
        DEBUG_EXIT
        return nullptr;
    }

    const std::string FuncName = CurrentToken->Content;
    Accept(); // Consume function name

    if (!Expect(LParen))
    {
        Logging::Error("Expected '('.");
        DEBUG_EXIT
        return nullptr;
    }
    Accept(); // Consume '('

    std::vector<std::string> Args;
    while (!Expect(RParen))
    {
        if (Expect(Name))
        {
            Args.push_back(CurrentToken->Content);
        }
        else
        {
            Logging::Error("Unable to parse argument.");
            DEBUG_EXIT
            return nullptr;
        }
        Accept(); // Consume argument name
        if (Expect(RParen))
        {
            break;
        }
        if (!Expect(Comma))
        {
            Logging::Error("Expected ',', got '{}'.", CurrentToken->Content);
            DEBUG_EXIT
            return nullptr;
        }
        Accept(); // Consume ','
    }
    Accept(); // Consume ')'

    const auto Body = ParseCurlyExpr();
    if (!Body)
    {
        Logging::Error("Unable to parse function def body.");
        DEBUG_EXIT
        return nullptr;
    }

    DEBUG_EXIT
    return new AstFunction(FuncName, Args, Body, FuncToken);
}

AstNode* Ast::ParseExpression()
{
    DEBUG_ENTER

    AstNode* Expr;
    if (CurrentToken == nullptr)
    {
        DEBUG_EXIT
        return nullptr;
    }

    // MyVar = ...;
    if (Expect(Name) && ExpectAssignOperator(1))
    {
        Expr = ParseAssignment();
        if (Expect(Semicolon))
        {
            Accept(); // Consume ';'
        }
    }
    else if (ExpectSequence({Name, LParen}))
    {
        Expr = ParseIdentifier();
        if (Expect(Semicolon))
        {
            Accept(); // Consume ';'
        }
    }
    else if (Expect(Return))
    {
        Accept(); // Consume 'return'
        Expr = ParseExpression();
        if (Expect(Semicolon))
        {
            Accept(); // Consume ';'
        }
    }
    else if (ExpectAny({Not, Minus}))
    {
        Expr = ParseUnaryExpr();
    }
    // 5 + ...;
    // "Test" + ...;
    // MyVar + ...;
    else if (ExpectAny({Name, Number, String, Bool}))
    {
        Expr = ParseEqualityExpr();
    }
    // [1,2,3 ...]
    else if (Expect(LBracket))
    {
        Expr = ParseBracketExpr();
    }
    else if (Expect(If))
    {
        Expr = ParseIf();
    }
    else if (Expect(While))
    {
        Expr = ParseWhile();
    }
    else if (Expect(Func))
    {
        Expr = ParseFunctionDecl();
    }
    else
    {
        Logging::Error("Unable to parse expression: {}", CurrentToken->ToString());
        DEBUG_EXIT
        return nullptr;
    }

    DEBUG_EXIT
    return Expr;
}

AstNode* Ast::ParseBody()
{
    DEBUG_ENTER
    const auto Body = new AstBody({}, *CurrentToken);
    while (CurrentToken != nullptr && CurrentToken != &Tokens.back())
    {
        auto Expr = ParseExpression();
        if (!Expr)
        {
            break;
        }
        Body->Expressions.push_back(Expr);
    }
    DEBUG_EXIT
    return Body;
}
