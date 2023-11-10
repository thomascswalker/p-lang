#include <functional>

#include "ast.h"

using namespace Core;

//////////////
// Visitors //
//////////////

void Visitor::Push(const TObject& Value)
{
	if (!Value.IsValid())
	{
		Logging::Error("Cannot push null object.");
		return;
	}
	Logging::Debug("PUSH: '{}'", Value.ToString());
	Stack.push_back(Value);
}

TObject Visitor::Pop()
{
	if (Stack.size() == 0)
	{
		Logging::Error("Stack is empty.");
		return TObject();
	}
	TObject Value = Stack.back();
	Logging::Debug("POP: '{}'", Value.ToString());
	Stack.pop_back();
	return Value;
}

TObject Visitor::Back()
{
	if (Stack.size() == 0)
	{
		Logging::Error("Stack is empty.");
		return TObject();
	}
	return Stack.back();
}

bool Visitor::IsIdentifier(const std::string& Name)
{
	auto Variable = GetIdentifier(Name);
	return (Variable.GetType() != NullType);
}

TObject Visitor::GetIdentifier(const std::string& Name)
{
	for (const auto& [K, V] : Identifiers)
	{
		if (K == Name)
		{
			return Identifiers.at(K);
		}
	}
	return TObject();
}

TObject* Visitor::GetIdentifierPtr(const std::string& Name)
{
	for (const auto& [K, V] : Identifiers)
	{
		if (K == Name)
		{
			return &Identifiers.at(K);
		}
	}
	return nullptr;
}

void Visitor::SetIdentifierValue(const std::string& Name, const TObject& InValue)
{
	Identifiers[Name] = InValue;
}

ASTFunction* Visitor::GetFunction(const std::string& Name)
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

bool Visitor::IsFunctionDeclared(const std::string& Name)
{
	auto Func = GetFunction(Name);
	return (Func != nullptr);
}

void Visitor::DefineFunction(const std::string& Name, ASTFunction* Func)
{
	Functions[Name] = Func;
}

bool Visitor::Visit(ASTValue* Node)
{
	DEBUG_ENTER

	switch (Node->Value.GetType())
	{
		case BoolType :
			Push(*Node->AsBool());
			break;
		case IntType :
			Push(*Node->AsInt());
			break;
		case FloatType :
			Push(*Node->AsFloat());
			break;
		case StringType :
			Push(*Node->AsString());
			break;
		case ArrayType :
			Push(*Node->AsArray());
			break;
		default :
			Logging::Error("Invalid type.");
			DEBUG_EXIT
			return false;
	}
	DEBUG_EXIT
	return true;
}

bool Visitor::Visit(ASTIdentifier* Node)
{
	DEBUG_ENTER

	Node->Value = GetIdentifier(Node->Name);
	if (Node->Value.GetType() == NullType)
	{
		Logging::Error("'{}' is undefined.", Node->Name);
		auto Context = Node->GetContext();
		Logging::Error("line {}, column {}", Context.Line, Context.Column);
		CHECK_ERRORS
	}

	// If the variable is found, push the variable's value to the stack
	Logging::Debug("'{}' is {}.", Node->Name, Node->Value.ToString());
	Push(Node->Value);
	DEBUG_EXIT
	return true;
}

bool Visitor::Visit(ASTUnaryExpr* Node)
{
	DEBUG_ENTER
	Node->Right->Accept(*this);
	CHECK_ERRORS
	auto CurrentValue = Pop();

	switch (Node->Op)
	{
		case Not :
			CurrentValue = CurrentValue - TObject(1);
			break;
		case Minus :
			CurrentValue = CurrentValue * TObject(-1);
			break;
		default :
			Logging::Error("Operator is not a valid unary operator.");
			CHECK_ERRORS
	}

	Push(CurrentValue);
	DEBUG_EXIT
	return true;
}

bool Visitor::Visit(ASTBinOp* Node)
{
	DEBUG_ENTER
	// Visit the left value
	Node->Left->Accept(*this);
	CHECK_ERRORS

	// After visiting the left value, pop it off the stack and store it here
	TObject Left = Pop();

	// Visit the right value
	Node->Right->Accept(*this);
	CHECK_ERRORS

	// After visiting the right value, pop it off the stack and store it here
	TObject Right = Pop();

	// Execute the operator on the left and right value
	TObject Result;
	switch (Node->Op)
	{
		case Plus :
		case PlusEquals :
			Result = Left + Right;
			break;
		case Minus :
		case MinusEquals :
			Result = Left - Right;
			break;
		case Multiply :
		case MultEquals :
			Result = Left * Right;
			break;
		case Divide :
		case DivEquals :
			Result = Left / Right;
			break;
		case LessThan :
			Result = Left < Right;
			break;
		case GreaterThan :
			Result = Left > Right;
			break;
		case Equals :
			Result = Left == Right;
			break;
		case NotEquals :
			Result = !(Left == Right);
			break;
	}

	// Push the resulting value to the stack
	Push(Result);
	Logging::Debug("BINOP: {} {} {} = {}", Left.ToString(), TokenStringMap[Node->Op], Right.ToString(),
				   Result.ToString());
	DEBUG_EXIT
	return true;
}

bool Visitor::Visit(ASTAssignment* Node)
{
	DEBUG_ENTER

	Node->Right->Accept(*this);
	CHECK_ERRORS

	TObject Value = Pop();
	CHECK_ERRORS

	SetIdentifierValue(Node->Name, Value);

	Logging::Debug("ASSIGN: {} <= {}", Node->Name, Value.ToString());
	DEBUG_EXIT
	return true;
}

bool Visitor::Visit(ASTCall* Node)
{
	DEBUG_ENTER

	if (Node->Type == IndexOf)
	{
		if (Node->Args.size() != 1)
		{
			Logging::Error("Invalid argument count for subscript operator.");
			CHECK_ERRORS
		}
		TObject Object;
		TObject Result;
		TObject Index;
		int		IndexValue;

		Node->Args[0]->Accept(*this);
		CHECK_ERRORS

		Index = Pop();
		CHECK_ERRORS
		IndexValue = Index.GetInt().GetValue();

		switch (Object.GetType())
		{
			case StringType :
				Result = Object.AsString()->At(IndexValue);
				Push(Result);
				break;
			case ArrayType :
				auto Temp = Object.AsArray()->At(IndexValue);
				Result = *Temp;
				Push(Result);
				break;
		}
	}
	else if (Node->Type == Function)
	{
		// Parse argument values
		TSignature InArgs;
		for (const auto& Arg : Node->Args)
		{
			// Evaluate identifiers
			if (Cast<ASTIdentifier>(Arg))
			{
				// Cast to an identifier
				auto Identifier = Cast<ASTIdentifier>(Arg);

				// Get the corresponding identifier name and pointer
				auto ArgName = Identifier->Name;
				auto ArgValue = GetIdentifierPtr(Identifier->Name);

				// Add this as a new variable argument. The key here is the
				// ArgValue is a pointer to the `Identifiers` array so we can
				// modify that value in place rather than pass around a bunch
				// of copies.
				InArgs.push_back(new TVariable{ ArgName, ArgValue });
			}
			// Evaluate literal values
			else if (Cast<ASTValue>(Arg))
			{
				// Because this is a literal value, we can just do a normal
				// accept and pop to get a copy of the value.
				Arg->Accept(*this);
				CHECK_ERRORS
				auto ArgValue = Pop();
				CHECK_ERRORS

				// Add the copy of the literal value to the argument list.
				InArgs.push_back(new TLiteral{ ArgValue });
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
			// Get the corresponding function pointer to the identifier name
			auto Func = FunctionMap[Node->Identifier];

			// Invoke the function with the arguments parsed above
			bool bResult = Func.Invoke(&InArgs);
			if (!bResult)
			{
				auto Context = Node->GetContext();
				auto Space = std::string(Context.Column, ' ');
				Logging::Error("line {}, column {}\n\t{}\n\t{}^", Context.Line, Context.Column, Context.Source, Space);
				CHECK_ERRORS
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
				auto FuncArg = Func->Args[Index];
				auto FuncArgType = FuncArg.Type;
				auto InArgType = InArg->GetValue().GetType();

				if (InArg->GetValuePtr() == nullptr)
				{
					Logging::Error("Argument {} is null.", Index);
					CHECK_ERRORS
				}

				if (FuncArgType != AnyType && !InArg->GetValuePtr()->CanCast(FuncArgType))
				{
					Logging::Error("Argument {} type mismatch for '{}'. Got {}, wanted {}.", Index, Node->Identifier, (int)InArgType,
								   (int)FuncArgType);
					CHECK_ERRORS
				}
				SetIdentifierValue(Func->Args[Index].Name, InArg->GetValue());
			}

			// Execute the function body
			Func->Body->Accept(*this);
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

bool Visitor::Visit(ASTIf* Node)
{
	DEBUG_ENTER
	TObject bResult;

	Node->Cond->Accept(*this);
	CHECK_ERRORS

	bResult = Pop();
	if (bResult.GetType() != BoolType)
	{
		Logging::Error("Did not get a bool result inside if conditional.");
		CHECK_ERRORS
	}

	Logging::Debug("IF: {}", bResult.GetBool().GetValue() ? "true" : "false");
	if (bResult.GetBool().GetValue())
	{
		Node->TrueBody->Accept(*this);
	}
	else
	{
		Node->FalseBody->Accept(*this);
	}
	DEBUG_EXIT
	return true;
}

bool Visitor::Visit(ASTWhile* Node)
{
	DEBUG_ENTER
	TBoolValue bResult = true;
	int		   Count = 1;

	while (bResult)
	{
		Node->Cond->Accept(*this);
		CHECK_ERRORS

		bResult = Pop().GetBool();
		Logging::Debug("WHILE ({}): {}", Count, bResult ? "true" : "false");
		if (!bResult)
		{
			break;
		}
		Node->Body->Accept(*this);
		CHECK_ERRORS

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

bool Visitor::Visit(ASTFunction* Node)
{
	if (Functions.find(Node->Name) == Functions.end())
	{
		DefineFunction(Node->Name, Node);
	}
	else
	{
		Logging::Error("Function {} not defined.", Node->Name);
		CHECK_ERRORS
	}
	DEBUG_EXIT
	return true;
}

bool Visitor::Visit(ASTBody* Node)
{
	DEBUG_ENTER
	for (const auto& E : Node->Expressions)
	{
		E->Accept(*this);
	}
	DEBUG_EXIT
	return true;
}

void Visitor::Dump()
{
	std::cout << "Variables:" << std::endl;
	for (const auto& [K, V] : Identifiers)
	{
		std::cout << K << " : " << V.ToString() << std::endl;
	}
}

/////////
// AST //
/////////

ASTNode* AST::ParseValueExpr()
{
	DEBUG_ENTER

	// Parse numbers (floats and ints)
	if (Expect(Number))
	{
		TObject Value;

		// Parse float
		if (CurrentToken->Content.find(".") != std::string::npos)
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

		Nodes.push_back(new ASTValue(Value, *CurrentToken));
		Accept(); // Consume number
		DEBUG_EXIT
		return Nodes.back();
	}
	// Parse strings
	else if (Expect(String))
	{
		std::string String = CurrentToken->Content;
		Logging::Debug("VALUE: Parsing string: {}", String);
		Nodes.push_back(new ASTValue(String, *CurrentToken));
		Accept(); // Consume string
		DEBUG_EXIT
		return Nodes.back();
	}
	// Parse names (Variables, functions, etc.)
	else if (Expect(Name))
	{
		auto Expr = ParseIdentifier();
		DEBUG_EXIT
		return Expr;
	}
	else if (Expect(Bool))
	{
		bool Value = CurrentToken->Content == "true" ? true : false;
		Logging::Debug("VALUE: Parsing bool: {}", Value);
		Nodes.push_back(new ASTValue(Value, *CurrentToken));
		Accept(); // Consume bool
		DEBUG_EXIT
		return Nodes.back();
	}

	DEBUG_EXIT
	return nullptr;
}

ASTNode* AST::ParseIdentifier()
{
	DEBUG_ENTER

	auto Identifier = new ASTIdentifier(CurrentToken->Content, *CurrentToken);
	auto IdentifierToken = *CurrentToken;
	Accept(); // Consume the variable

	if (!ExpectAny({ LParen, LBracket, Period }))
	{
		DEBUG_EXIT
		return Identifier;
	}

	auto StartTok = CurrentToken->Type;
	auto EndTok = BLOCK_PAIRS.at(CurrentToken->Type);
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
			Logging::Error("Token {} not supported.", TokenStringMap[StartTok]);
			DEBUG_EXIT
			return nullptr;
	}
	std::vector<ASTNode*> Args;
	while (!Expect(EndTok))
	{
		auto Arg = ParseExpression();
		if (Arg)
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

	Nodes.push_back(new ASTCall(Identifier->Name, CallType, Args, IdentifierToken));
	DEBUG_EXIT
	return Nodes.back();
}

ASTNode* AST::ParseUnaryExpr()
{
	DEBUG_ENTER

	ASTNode* Expr = ParseValueExpr();
	if (ExpectAny({ Not, Minus }))
	{
		auto Op = CurrentToken->Type;
		Accept(); // Consume '!' or '-'
		Nodes.push_back(new ASTUnaryExpr(Op, ParseValueExpr(), *CurrentToken));
		Expr = Nodes.back();
	}

	DEBUG_EXIT
	return Expr;
}

ASTNode* AST::ParseMultiplicativeExpr()
{
	DEBUG_ENTER

	ASTNode* Expr = ParseUnaryExpr();
	while (Expect(Multiply) || Expect(Divide))
	{
		auto Op = CurrentToken->Type;
		Accept(); // Consume '*' or '/'
		Nodes.push_back(new ASTBinOp(Expr, ParseUnaryExpr(), Op, *CurrentToken));
		Expr = Nodes.back();
	}

	DEBUG_EXIT
	return Expr;
}

ASTNode* AST::ParseAdditiveExpr()
{
	DEBUG_ENTER

	ASTNode* Expr = ParseMultiplicativeExpr();
	while (Expect(Plus) || Expect(Minus))
	{
		auto Op = CurrentToken->Type;
		Accept(); // Consume '+' or '-'
		Nodes.push_back(new ASTBinOp(Expr, ParseMultiplicativeExpr(), Op, *CurrentToken));
		Expr = Nodes.back();
	}

	DEBUG_EXIT
	return Expr;
}

ASTNode* AST::ParseEqualityExpr()
{
	DEBUG_ENTER
	ASTNode* Expr = ParseAdditiveExpr();
	while (ExpectAny({ GreaterThan, LessThan, NotEquals, Equals }))
	{
		auto Op = CurrentToken->Type;
		Accept(); // Consume '==' or '!=' or '<' or '>'
		Nodes.push_back(new ASTBinOp(Expr, ParseAdditiveExpr(), Op, *CurrentToken));
		Expr = Nodes.back();
	}

	DEBUG_EXIT
	return Expr;
}

ASTNode* AST::ParseAssignment()
{
	DEBUG_ENTER

	std::string Name = CurrentToken->Content; // Get the name
	auto		NameToken = *CurrentToken;
	Accept();					  // Consume name
	auto Op = CurrentToken->Type; // Get the assignment operator
	Accept();					  // Consume assignment operator

	auto Expr = ParseExpression();
	if (Op == PlusEquals || Op == MinusEquals || Op == MultEquals || Op == DivEquals)
	{
		Expr = new ASTBinOp(new ASTIdentifier(Name, NameToken), Expr, Op, *CurrentToken);
	}
	Nodes.push_back(new ASTAssignment(Name, Expr, *CurrentToken));
	DEBUG_EXIT
	return Nodes.back();
}

ASTNode* AST::ParseParenExpr()
{
	DEBUG_ENTER

	if (!Expect(LParen))
	{
		Logging::Error("Expected '{}' starting conditional. Got '{}'.", "(", CurrentToken->Content);
		DEBUG_EXIT
		return nullptr;
	}
	Accept(); // Consume '('
	ASTNode* Expr = ParseExpression();
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

ASTNode* AST::ParseBracketExpr()
{
	DEBUG_ENTER

	if (Expect(LBracket))
	{
		Accept(); // Consume '['

		TArrayValue Values{};
		while (!Expect(RBracket))
		{
			Logging::Debug("BRACKET: Parsing loop in {}.", __FUNCTION__);
			auto Value = ParseExpression();
			auto CastValue = Cast<ASTValue>(Value);
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

		if (Values.Size().GetValue() == 1)
		{
			Nodes.push_back(new ASTValue(Values[0], *CurrentToken));
		}
		else
		{
			Nodes.push_back(new ASTValue(Values, *CurrentToken));
		}
		DEBUG_EXIT
		return Nodes.back();
	}
	DEBUG_EXIT
	return nullptr;
}

ASTNode* AST::ParseCurlyExpr()
{
	DEBUG_ENTER

	if (!Expect(LCurly))
	{
		Logging::Error("Expected '{}' starting block. Got '{}'.", "{", CurrentToken->Content);
		DEBUG_EXIT
		return nullptr;
	}

	auto CurlyToken = *CurrentToken;
	Accept(); // Consume '{'

	std::vector<ASTNode*> Body;
	while (!Expect(RCurly))
	{
		Logging::Debug("CURLY: Parsing loop in {}.", __FUNCTION__);
		ASTNode* Expr = ParseExpression();
		Body.push_back(Expr);

		// Handle any dangling semicolons
		if (Expect(Semicolon))
		{
			Accept();
		}
	}

	Accept(); // Consume '}'

	Nodes.push_back(new ASTBody(Body, CurlyToken));
	DEBUG_EXIT
	return Nodes.back();
}

ASTNode* AST::ParseIf()
{
	DEBUG_ENTER

	auto IfToken = *CurrentToken;
	Accept(); // Consume 'if'
	auto Cond = ParseParenExpr();
	if (!Cond)
	{
		Logging::Error("Unable to parse 'if'.");
		DEBUG_EXIT
		return nullptr;
	}

	Logging::Debug("IF: Parsing 'if' block.");
	auto TrueBody = ParseCurlyExpr();
	if (!TrueBody)
	{
		Logging::Error("Unable to parse true body of 'if'.");
		DEBUG_EXIT
		return nullptr;
	}

	ASTNode* FalseBody = nullptr;
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

	Nodes.push_back(new ASTIf(Cond, TrueBody, FalseBody, IfToken));
	DEBUG_EXIT
	return Nodes.back();
}

ASTNode* AST::ParseWhile()
{
	DEBUG_ENTER
	auto WhileToken = *CurrentToken;
	Accept(); // Consume 'while'

	if (!Expect(LParen))
	{
		Logging::Error("Expected '(', got {}", CurrentToken->Content);
		DEBUG_EXIT
		return nullptr;
	}
	Accept(); // Consume '('
	auto Cond = ParseEqualityExpr();
	if (!Expect(RParen))
	{
		Logging::Error("Expected ')', got {}", CurrentToken->Content);
		DEBUG_EXIT
		return nullptr;
	}
	Accept(); // Consume ')'
	auto Body = ParseCurlyExpr();

	if (!Body)
	{
		Logging::Error("Unable to parse while body.");
		DEBUG_EXIT
		return nullptr;
	}

	Nodes.push_back(new ASTWhile(Cond, Body, WhileToken));
	DEBUG_EXIT
	return Nodes.back();
}

ASTNode* AST::ParseFunctionDecl()
{
	DEBUG_ENTER
	if (!Expect(Func))
	{
		Logging::Error("Expected function declaration.");
		DEBUG_EXIT
		return nullptr;
	}

	auto FuncToken = *CurrentToken;
	Accept(); // Consume 'func'

	if (!Expect(Name))
	{
		Logging::Error("Expected function name.");
		DEBUG_EXIT
		return nullptr;
	}

	std::string FuncName = CurrentToken->Content;
	Accept(); // Consume function name

	if (!Expect(LParen))
	{
		Logging::Error("Expected '('.");
		DEBUG_EXIT
		return nullptr;
	}
	Accept(); // Consume '('

	std::vector<TArg> Args;
	while (!Expect(RParen))
	{
		if (Expect(Name))
		{
			std::string ArgName = CurrentToken->Content;
			Accept(); // Consume the argument name
			EValueType ArgType = AnyType;
			if (Expect(Colon))
			{
				Accept(); // Consume ':'
				if (!Expect(Type))
				{
					Logging::Error("Expected type after ':'.");
					DEBUG_EXIT
					return nullptr;
				}
				ArgType = StringValueTypeMap[CurrentToken->Content];
				Accept(); // Consume 'type'
			}
			Args.push_back({ ArgName, ArgType });
		}
		else
		{
			Logging::Error("Unable to parse argument.");
			DEBUG_EXIT
			return nullptr;
		}

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

	auto Body = ParseCurlyExpr();
	if (!Body)
	{
		Logging::Error("Unable to parse function def body.");
		DEBUG_EXIT
		return nullptr;
	}

	Nodes.push_back(new ASTFunction(FuncName, Args, Body, FuncToken));
	DEBUG_EXIT
	return Nodes.back();
}

ASTNode* AST::ParseExpression()
{
	DEBUG_ENTER

	ASTNode* Expr;
	if (CurrentToken == nullptr)
	{
		DEBUG_EXIT
		return nullptr;
	}

	// MyVar = ...;
	if (Expect(Name) && ExpectAssignOperator(1))
	{
		Expr = ParseAssignment();
		Accept(); // Consume ';'
	}
	else if (Expect(Name) && Expect(LParen, 1))
	{
		Expr = ParseIdentifier();
		if (Expect(Semicolon))
		{
			Accept();
		}
	}
	else if (ExpectAny({ Not, Minus }))
	{
		Expr = ParseUnaryExpr();
	}
	// 5 + ...;
	// "Test" + ...;
	// MyVar + ...;
	else if (ExpectAny({ Name, Number, String, Bool }))
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

void AST::ParseBody()
{
	DEBUG_ENTER
	Program = new ASTBody({}, *CurrentToken);
	while (CurrentToken != nullptr && CurrentToken != &Tokens.back())
	{
		auto Expr = ParseExpression();
		if (!Expr)
		{
			break;
		}
		Program->Expressions.push_back(Expr);
	}
	DEBUG_EXIT
}
