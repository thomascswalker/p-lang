#include <functional>

#include "ast.h"

using namespace Core;

//////////////
// Visitors //
//////////////

void Visitor::Push(const TObject& V)
{
	Debug(std::format("Visiting {}.", __FUNCSIG__));
	Stack.push_back(V);
}

TObject Visitor::Pop()
{
	Debug(std::format("Visiting {}.", __FUNCSIG__));
	TObject Value = Stack.back();
	Stack.pop_back();
	return Value;
}

bool Visitor::IsVariable(const std::string& Name)
{
	Debug(std::format("Visiting {}.", __FUNCSIG__));
	auto Variable = GetVariable(Name);
	return (Variable.GetType() != NullType);
}

TObject Visitor::GetVariable(const std::string& Name)
{
	for (const auto& [K, V] : Variables)
	{
		if (K == Name)
		{
			return Variables.at(K);
		}
	}
	return TObject();
}

void Visitor::SetVariable(const std::string& Name, const TObject& InValue)
{
	Debug(std::format("Visiting {}.", __FUNCSIG__));
	Variables[Name] = InValue;
}

void Visitor::Visit(ASTValue* Node)
{
	Debug(std::format("Visiting {}.", __FUNCSIG__));
	switch (Node->Value.GetType())
	{
		case BoolType :
			Push(TObject(Node->GetBool().GetValue()));
			break;
		case IntType :
			Push(TObject(Node->GetInt().GetValue()));
			break;
		case FloatType :
			Push(TObject(Node->GetFloat().GetValue()));
			break;
		case StringType :
			Push(TObject(Node->GetString().GetValue()));
			break;
		case ArrayType :
			Push(TObject(Node->GetArray()));
			break;
		default :
			Exit("Invalid type.");
			break;
	}
}

void Visitor::Visit(ASTVariable* Node)
{
	Debug(std::format("Visiting {}.", __FUNCSIG__));

	Node->Value = GetVariable(Node->Name);

	// If the variable is found, push the variable's value to the stack
	Debug(std::format("Variable {} is {}.", Node->Name, Node->Value.ToString()));
	Push(Node->Value);
}

void Visitor::Visit(ASTUnaryExpr* Node)
{
	Debug(std::format("Visiting {}.", __FUNCSIG__));
	auto V = GetVariable(Node->Name);
	if (V)
	{
		ETokenType Op = Node->Op;
		TObject	   NewValue;
		TObject	   Index;

		switch (Op)
		{
			case PlusPlus :
				NewValue = V.GetInt().GetValue() + 1;
				SetVariable(Node->Name, NewValue);
				break;
			case MinusMinus :
				NewValue = V.GetInt().GetValue() - 1;
				SetVariable(Node->Name, NewValue);
				break;
			case LBracket :
				Node->OpArg->Accept(*this);
				Index = Pop();
				if (V.GetType() == ArrayType)
				{
					NewValue = V.AsArray()->GetValue().at(Index.GetInt().GetValue());
				}
				else if (V.GetType() == StringType)
				{
					NewValue = V.AsString()->GetValue().at(Index.GetInt().GetValue());
				}
				break;
			case Period :
				break;
			default :
				return;
		}

		Push(NewValue);
	}
	else
	{
		Error(std::format("Unable to parse unary expression: undefined variable {}", Node->Name));
	}
}

void Visitor::Visit(ASTBinOp* Node)
{
	Debug(std::format("Visiting {}.", __FUNCSIG__));
	// Visit the left value
	Node->Left->Accept(*this);
	// After visiting the left value, pop it off the stack and store it here
	auto Left = Pop();

	// Visit the right value
	Node->Right->Accept(*this);
	// After visiting the right value, pop it off the stack and store it here
	auto Right = Pop();

	// Execute the operator on the left and right value
	TObject Result;
	switch (Node->Op)
	{
		case Plus :
			Result = Left + Right;
			break;
		case Minus :
			Result = Left - Right;
			break;
		case Multiply :
			Result = Left * Right;
			break;
		case Divide :
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
}

void Visitor::Visit(ASTAssignment* Node)
{
	Debug(std::format("Visiting {}.", __FUNCSIG__));

	Node->Right->Accept(*this);
	auto Value = Pop();
	SetVariable(Node->Name, Value);

	Debug(std::format("Variable {} assigned value of {}", Node->Name, Value.ToString()));
}

void Visitor::Visit(ASTIf* Node)
{
	Debug(std::format("Visiting {}.", __FUNCSIG__));
	TObject bResult;

	Node->Cond->Accept(*this);

	bResult = Pop();
	Debug(std::format("Conditional result is {}", bResult ? "true" : "false"));
	if (bResult)
	{
		Node->TrueBody->Accept(*this);
	}
	else
	{
		Node->FalseBody->Accept(*this);
	}
}

void Visitor::Visit(ASTWhile* Node)
{
	Debug(std::format("Visiting {}.", __FUNCSIG__));
	TBoolValue bResult = true;
	int		   Count = 1;

	while (bResult)
	{
		Node->Cond->Accept(*this);

		bResult = Pop().GetBool();
		Debug(std::format("While result is {}", bResult ? "true" : "false"));
		if (!bResult)
		{
			break;
		}
		Node->Body->Accept(*this);
		Count++;

		if (Count == WHILE_MAX_LOOP)
		{
			Error(std::format("Hit max loop count ({}).", WHILE_MAX_LOOP));
			break;
		}
	}
}

void Visitor::Visit(ASTFunctionDef* Node) {}

void Visitor::Visit(ASTReturn* Node) {}

void Visitor::Visit(ASTBody* Node)
{
	Debug(std::format("Visiting {}.", __FUNCSIG__));
	for (const auto& E : Node->Expressions)
	{
		E->Accept(*this);
	}
}

void Visitor::Dump()
{
	std::cout << "Variables:" << std::endl;
	for (const auto& [K, V] : Variables)
	{
		std::cout << K << " : " << V.ToString() << std::endl;
	}
}

/////////
// AST //
/////////

void AST::Accept()
{
	// Get the last token in the token list
	Token* End = &Tokens.back();

	if (!CurrentToken)
	{
		return;
	}

	// If we're at the end, set the CurrentToken to be null
	if (CurrentToken == End)
	{
		CurrentToken = nullptr;
	}
	// Otherwise increment the CurrentToken pointer and the position
	else
	{
		Debug(std::format("Accept(): {}", CurrentToken->Content));
		CurrentToken++;
		Position++;
	}
}

bool AST::Expect(ETokenType Type, int Offset)
{
	// Make sure the offset is valid
	if (Position + Offset > Tokens.size())
	{
		Error("WARNING: Outside token bounds.");
		return false;
	}

	//Debug(std::format("Expect(): {} at {}: {}", (int)Type, Position + Offset, Tokens[Position + Offset].ToString()));

	return Tokens[Position + Offset].Type == Type;
}

bool AST::Expect(const std::initializer_list<ETokenType>& Types, int Offset)
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

bool AST::ExpectAny(const std::initializer_list<ETokenType>& Types, int Offset)
{
	for (const auto& T : Types)
	{
		if (!Expect(T, Offset))
		{
			return false;
		}
	}
	return true;
}

ASTNode* AST::ParseValueExpr()
{
	Debug(std::format("Parsing {}.", __FUNCTION__));

	// Parse numbers (floats and ints)
	if (Expect(Number))
	{
		TObject Value;

		// Parse float
		if (CurrentToken->Content.find(".") != std::string::npos)
		{
			Value = std::stof(CurrentToken->Content);
			Debug(std::format("Parsing number: {}", Value.GetFloat().GetValue()));
		}
		// Parse int
		else
		{
			Value = std::stoi(CurrentToken->Content);
			Debug(std::format("Parsing number: {}", Value.GetInt().GetValue()));
		}

		Accept(); // Consume number
		Nodes.push_back(new ASTValue(Value));
		Debug(std::format("Exiting {}.", __FUNCTION__));
		return Nodes.back();
	}
	// Parse strings
	else if (Expect(String))
	{
		std::string String = CurrentToken->Content;
		Accept(); // Consume string
		Debug(std::format("Parsing string: {}", String));
		Nodes.push_back(new ASTValue(String));
		Debug(std::format("Exiting {}.", __FUNCTION__));
		return Nodes.back();
	}
	// Parse names (Variables, functions, etc.)
	else if (Expect(Name))
	{
		std::string Name = CurrentToken->Content;
		Accept(); // Consume name
		Debug(std::format("Parsing name: {}", Name));
		Debug(std::format("Exiting {}.", __FUNCTION__));
		Nodes.push_back(new ASTVariable(Name));
		return Nodes.back();
	}
	else if (Expect(Bool))
	{
		bool Value = CurrentToken->Content == "true" ? true : false;
		Accept(); // Consume bool
		Debug(std::format("Parsing bool: {}", Value));
		Nodes.push_back(new ASTValue(Value));
		Debug(std::format("Exiting {}.", __FUNCTION__));
		return Nodes.back();
	}
	else if (Expect(LParen))
	{
		auto Expr = ParseParenExpr();
		Debug(std::format("Exiting {}.", __FUNCTION__));
		return Expr;
	}
	else
	{
		Error("Unable to parse factor.");
		Debug(std::format("Exiting {}.", __FUNCTION__));
		return nullptr;
	}
}

ASTNode* AST::ParseUnaryExpr()
{
	Debug(std::format("Parsing {}.", __FUNCTION__));

	if (Expect(ETokenType::Name))
	{
		// Handle existing variable assignment, or continue with a new assignment
		std::string Name = CurrentToken->Content; // Get the name
		Accept();								  // Consume name
		auto Op = CurrentToken->Type;			  // Get the operator used
		//Accept();								  // Consume the operator

		ASTNode* Expr = nullptr;
		switch (Op)
		{
			case PlusPlus :
				Accept(); // Consume '++'
				break;
			case MinusMinus :
				Accept(); // Consume '--'
				break;
			case LBracket :
				Expr = ParseBracketExpr();
				break;
			case Period :
				break;
			default :
				break;
		}

		Nodes.push_back(new ASTUnaryExpr(Name, Op, Expr)); // Construct a new unary expression
		Debug(std::format("Exiting {}.", __FUNCTION__));
		return Nodes.back();
	}
}

ASTNode* AST::ParseParenExpr()
{
	Debug(std::format("Parsing {}.", __FUNCTION__));

	if (!Expect(LParen))
	{
		Error(std::format("Expected '{}' starting conditional. Got '{}'.", "(", CurrentToken->Content));
		Debug(std::format("Exiting {}.", __FUNCTION__));
		return nullptr;
	}
	Accept(); // Consume '('
	ASTNode* Expr = ParseExpression();
	if (!Expect(RParen))
	{
		Error(std::format("Expected '{}' ending conditional. Got '{}'.", ")", CurrentToken->Content));
		Debug(std::format("Exiting {}.", __FUNCTION__));
		return nullptr;
	}
	Accept(); // Consume ')'
	Debug(std::format("Exiting {}.", __FUNCTION__));
	return Expr;
}

ASTNode* AST::ParseBracketExpr()
{
	Debug(std::format("Parsing {}.", __FUNCTION__));

	if (Expect(LBracket))
	{
		Accept(); // Consume '['

		TArrayValue Values{};
		while (!Expect(RBracket))
		{
			Debug(std::format("Parsing loop in {}.", __FUNCTION__));
			auto Value = ParseExpression();
			auto CastValue = Cast<ASTValue>(Value);
			if (!CastValue)
			{
				Error("Unable to cast value.");
				Debug(std::format("Exiting {}.", __FUNCTION__));
				return nullptr;
			}
			Values.Append(CastValue->GetValue());
			if (Expect(RBracket))
			{
				break;
			}

			if (!Expect(Comma))
			{
				Error("Expected comma.");
				Debug(std::format("Exiting {}.", __FUNCTION__));
				return nullptr;
			}
			Accept(); // Consume ','
		}

		Accept(); // Consume ']'

		if (Values.Size().GetValue() == 1)
		{
			Nodes.push_back(new ASTValue(Values[0]));
		}
		else
		{
			Nodes.push_back(new ASTValue(Values));
		}
		Debug(std::format("Exiting {}.", __FUNCTION__));
		return Nodes.back();
	}
	Debug(std::format("Exiting {}.", __FUNCTION__));
	return nullptr;
}

ASTNode* AST::ParseCurlyExpr()
{
	Debug(std::format("Parsing {}.", __FUNCTION__));

	if (!Expect(LCurly))
	{
		Error(std::format("Expected '{}' starting block. Got '{}'.", "{", CurrentToken->Content));
		Debug(std::format("Exiting {}.", __FUNCTION__));
		return nullptr;
	}
	Accept(); // Consume '{'

	std::vector<ASTNode*> Body;
	while (!Expect(RCurly))
	{
		Debug(std::format("Parsing loop in {}.", __FUNCTION__));
		if (Expect(Return))
		{
			ASTNode* Expr = ParseReturnExpr();
			Body.push_back(Expr);
			break; // Early exit after the return
		}
		else
		{
			CurrentToken;
			ASTNode* Expr = ParseExpression();
			Body.push_back(Expr);
		}

		// Handle any dangling semicolons
		if (Expect(Semicolon))
		{
			Accept();
		}
	}

	Accept(); // Consume '}'

	Nodes.push_back(new ASTBody(Body));
	Debug(std::format("Exiting {}.", __FUNCTION__));
	return Nodes.back();
}

ASTNode* AST::ParseMultiplicativeExpr()
{
	Debug(std::format("Parsing {}.", __FUNCTION__));

	ASTNode* Expr = ParseValueExpr();
	while (Expect(Multiply) || Expect(Divide))
	{
		std::string Op = CurrentToken->Content;
		Accept(); // Consume '*' or '/'
		Nodes.push_back(new ASTBinOp(Expr, ParseValueExpr(), Op));
		Expr = Nodes.back();
	}

	Debug(std::format("Exiting {}.", __FUNCTION__));
	return Expr;
}

ASTNode* AST::ParseAdditiveExpr()
{
	Debug(std::format("Parsing {}.", __FUNCTION__));

	ASTNode* Expr = ParseMultiplicativeExpr();
	while (Expect(Plus) || Expect(Minus))
	{
		std::string Op = CurrentToken->Content;
		Accept(); // Consume '+' or '-'
		Nodes.push_back(new ASTBinOp(Expr, ParseMultiplicativeExpr(), Op));
		Expr = Nodes.back();
	}

	Debug(std::format("Exiting {}.", __FUNCTION__));
	return Expr;
}

ASTNode* AST::ParseEqualityExpr()
{
	Debug(std::format("Parsing {}.", __FUNCTION__));
	ASTNode* Expr = ParseAdditiveExpr();
	while (Expect(Equals) || Expect(NotEquals) || Expect(LessThan) || Expect(GreaterThan))
	{
		std::string Op = CurrentToken->Content;
		Accept(); // Consume '==' or '!=' or '<' or '>'
		Nodes.push_back(new ASTBinOp(Expr, ParseAdditiveExpr(), Op));
		Expr = Nodes.back();
	}

	Debug(std::format("Exiting {}.", __FUNCTION__));
	return Expr;
}

ASTNode* AST::ParseAssignment()
{
	Debug(std::format("Parsing {}.", __FUNCTION__));

	std::string Type;
	std::string Name;

	// Handle new variable assignments
	if (Expect(ETokenType::Type))
	{
		Type = CurrentToken->Content; // Get the type
		Accept();					  // Consume type
	}

	if (Expect(ETokenType::Name))
	{
		// Handle existing variable assignment, or continue with a new assignment
		Name = CurrentToken->Content; // Get the name
		Accept();					  // Consume name
		if (Expect(Assign))
		{
			Accept(); // Consume '='
			ASTNode* Expr = ParseExpression();
			Nodes.push_back(new ASTAssignment(Type, Name, Expr));
			Debug(std::format("Exiting {}.", __FUNCTION__));
			return Nodes.back();
		}
	}

	Debug(std::format("Exiting {}.", __FUNCTION__));
	return nullptr;
}

ASTNode* AST::ParseReturnExpr()
{
	Debug(std::format("Parsing {}.", __FUNCTION__));
	Accept(); // Consume return
	ASTNode* Expr = ParseExpression();
	Nodes.push_back(new ASTReturn(Expr));
	Debug(std::format("Exiting {}.", __FUNCTION__));
	return Nodes.back();
}

ASTNode* AST::ParseIf()
{
	Debug(std::format("Parsing {}.", __FUNCTION__));

	Accept(); // Consume 'if'
	auto Cond = ParseParenExpr();
	if (!Cond)
	{
		Error("Unable to parse 'if'.");
		Debug(std::format("Exiting {}.", __FUNCTION__));
		return nullptr;
	}

	Debug("Parsing 'if' block.");
	auto TrueBody = ParseCurlyExpr();
	if (!TrueBody)
	{
		Error("Unable to parse true body of 'if'.");
		Debug(std::format("Exiting {}.", __FUNCTION__));
		return nullptr;
	}

	ASTNode* FalseBody = nullptr;
	if (Expect(Else))
	{
		Accept(); // Consume 'else'
		Debug("Parsing 'else' block.");
		FalseBody = ParseCurlyExpr();
		if (!FalseBody)
		{
			Error("Unable to parse false body of 'else'.");
			Debug(std::format("Exiting {}.", __FUNCTION__));
			return nullptr;
		}
	}

	Nodes.push_back(new ASTIf(Cond, TrueBody, FalseBody));
	Debug(std::format("Exiting {}.", __FUNCTION__));
	return Nodes.back();
}

ASTNode* AST::ParseWhile()
{
	Debug(std::format("Parsing {}.", __FUNCTION__));
	Accept(); // Consume 'while'

	if (!Expect(LParen))
	{
		Error(std::format("Expected '(', got {}", CurrentToken->Content));
		return nullptr;
	}
	Accept(); // Consume '('
	auto Cond = ParseEqualityExpr();
	if (!Expect(RParen))
	{
		Error(std::format("Expected ')', got {}", CurrentToken->Content));
		return nullptr;
	}
	Accept(); // Consume ')'
	auto Body = ParseCurlyExpr();

	if (!Body)
	{
		Error("Unable to parse while body.");
		Debug(std::format("Exiting {}.", __FUNCTION__));
		return nullptr;
	}

	Nodes.push_back(new ASTWhile(Cond, Body));
	Debug(std::format("Exiting {}.", __FUNCTION__));
	return Nodes.back();
}

ASTNode* AST::ParseFunctionDef()
{
	Debug(std::format("Parsing {}.", __FUNCTION__));

	std::string TypeString = CurrentToken->Content;
	EValueType	ReturnType = StringTypeMap.at(TypeString);
	Accept(); // Consume return type
	std::string Name = CurrentToken->Content;
	Accept(); // Consume name
	Accept(); // Parse left parenthesis

	std::vector<ArgValue> Arguments;

	// If there isn't immediately a right parenthesis, then parse
	// the arguments listed
	if (!Expect(RParen))
	{
		EValueType ArgType = StringTypeMap.at(CurrentToken->Content);
		Accept(); // Consume the argument type
		std::string ArgName = CurrentToken->Content;
		Accept(); // Consume the argument name

		while (Expect(Comma))
		{
			Accept(); // Consume comma
			EValueType ArgType = StringTypeMap.at(CurrentToken->Content);
			Accept(); // Consume the argument type
			std::string ArgName = CurrentToken->Content;
			Accept(); // Consume the argument name
		}
	}

	Accept();						  // Consume right parenthesis
	ASTNode* Body = ParseCurlyExpr(); // Parse the body, expecting a return statement

	Nodes.push_back(new ASTFunctionDef(ReturnType, Name, Arguments, Body));
	Debug(std::format("Exiting {}.", __FUNCTION__));
	return nullptr;
}

ASTNode* AST::ParseExpression()
{
	Debug(std::format("Parsing {}.", __FUNCTION__));

	ASTNode* Expr;
	if (CurrentToken == nullptr)
	{
		Debug(std::format("Exiting {}.", __FUNCTION__));
		return nullptr;
	}

	// int MyFunc() { ... }
	if (Expect({ Type, Name, LParen }))
	{
		Expr = ParseFunctionDef();
	}

	// int MyVar = ...;
	else if (Expect({ Type, Name, Assign }))
	{
		Expr = ParseAssignment();
		Accept(); // Consume ';'
	}
	// MyVar = ...;
	else if (Expect({ Name, Assign }))
	{
		Expr = ParseAssignment();
		Accept(); // Consume ';'
	}
	// MyVar++;
	// MyVar--;
	// MyVar.Func();
	// MyArr[Index];
	else if (Expect(Name) && Expect(PlusPlus, 1) || Expect(MinusMinus, 1) || Expect(Period, 1) || Expect(LBracket, 1))
	{
		Expr = ParseUnaryExpr();
	}

	// 5 + ...;
	// "Test" + ...;
	// MyVar + ...;
	else if (Expect(Bool) || Expect(Number) || Expect(String) || Expect(Name) || Expect(LParen))
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
	else
	{
		Error(std::format("Unable to parse expression: {}", CurrentToken->ToString()));
		Debug(std::format("Exiting {}.", __FUNCTION__));
		return nullptr;
	}

	Debug(std::format("Exiting {}.", __FUNCTION__));
	return Expr;
}

void AST::ParseBody()
{
	Debug(std::format("Parsing {}.", __FUNCTION__));

	Program = new ASTBody();
	while (CurrentToken != nullptr && CurrentToken != &Tokens.back())
	{
		auto Expr = ParseExpression();
		if (!Expr)
		{
			break;
		}
		Program->Expressions.push_back(Expr);
	}

	Debug(std::format("Expression count: {}", Program->Expressions.size()));
}
