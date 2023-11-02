#include <functional>

#include "ast.h"

using namespace Core;

//////////////
// Visitors //
//////////////

void Visitor::Push(const TObject& Value)
{
	LOG(std::format("PUSH: '{}'", Value.ToString()));
	Stack.push_back(Value);
}

TObject Visitor::Pop()
{
	TObject Value = Stack.back();
	LOG(std::format("POP: '{}'", Value.ToString()));
	Stack.pop_back();
	return Value;
}

bool Visitor::IsVariable(const std::string& Name)
{
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
	Variables[Name] = InValue;
}

void Visitor::Visit(ASTValue* Node)
{
	DEBUG_ENTER
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
			Error("Invalid type.");
			break;
	}
	DEBUG_EXIT
}

void Visitor::Visit(ASTVariable* Node)
{
	DEBUG_ENTER

	if (!IsVariable(Node->Name))
	{
		Error(std::format("VARIABLE: '{}' is undefined.", Node->Name));
		DEBUG_EXIT
		return;
	}
	Node->Value = GetVariable(Node->Name);

	// If the variable is found, push the variable's value to the stack
	LOG(std::format("VARIABLE: '{}' is {}.", Node->Name, Node->Value.ToString()));
	Push(Node->Value);
	DEBUG_EXIT
}

void Visitor::Visit(ASTUnaryExpr* Node)
{
	DEBUG_ENTER
	TObject CurrentValue;
	TObject NewValue;
	if (Cast<ASTValue>(Node->Right))
	{
		auto Expr = Cast<ASTValue>(Node->Right);
		Visit(Expr);
		CHECK_ERRORS
		CurrentValue = Pop();
	}
	else if (Cast<ASTVariable>(Node->Right))
	{
		auto Expr = Cast<ASTVariable>(Node->Right);
		Visit(Expr);
		CHECK_ERRORS
		CurrentValue = Pop();
	}
	else
	{
		ERROR("Invalid rhs for unary expr");
		DEBUG_EXIT
		return;
	}

	switch (Node->Op)
	{
		case Not :
			NewValue = TObject(1) - CurrentValue;
			break;
		case Minus :
			NewValue = CurrentValue * TObject(-1);
			break;
		default :
			ERROR("Operator is not a valid unary operator.");
			DEBUG_EXIT
			return;
	}

	Push(NewValue);
	DEBUG_EXIT
}

void Visitor::Visit(ASTBinOp* Node)
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
	LOG(std::format("BINOP: {} {} {} = {}", Left.ToString(), TokenStringMap[Node->Op], Right.ToString(),
					Result.ToString()));
	DEBUG_EXIT
}

void Visitor::Visit(ASTAssignment* Node)
{
	DEBUG_ENTER

	Node->Right->Accept(*this);
	CHECK_ERRORS

	TObject Value = Pop();
	SetVariable(Node->Name, Value);

	LOG(std::format("ASSIGN: {} <= {}", Node->Name, Value.ToString()));
	DEBUG_EXIT
}

void Visitor::Visit(ASTCall* Node)
{
	DEBUG_ENTER

	auto V = GetVariable(Node->Name);
	auto A = V.AsArray();

	Node->Args[0]->Accept(*this);
	CHECK_ERRORS

	TObject RawValue = Pop();
	auto	Index = RawValue.GetInt().GetValue();
	auto	Size = A->Size().GetValue();
	if (Index < -Size || Index >= Size)
	{
		Error(std::format("Array '{}' index '{}' out of range (max is {}).", Node->Name, Index, Size - 1));
		DEBUG_EXIT
		return;
	}
	if (Index < 0)
	{
		Index = Size - abs(Index);
	}
	Push(A->At(Index));

	DEBUG_EXIT
}

void Visitor::Visit(ASTIf* Node)
{
	DEBUG_ENTER
	TObject bResult;

	Node->Cond->Accept(*this);
	CHECK_ERRORS

	bResult = Pop();
	if (bResult.GetType() != BoolType)
	{
		Error("Did not get a bool result inside if conditional.");
		DEBUG_EXIT
		return;
	}

	LOG(std::format("IF: {}", bResult.GetBool().GetValue() ? "true" : "false"));
	if (bResult.GetBool().GetValue())
	{
		Node->TrueBody->Accept(*this);
	}
	else
	{
		Node->FalseBody->Accept(*this);
	}
	DEBUG_EXIT
}

void Visitor::Visit(ASTWhile* Node)
{
	DEBUG_ENTER
	TBoolValue bResult = true;
	int		   Count = 1;

	while (bResult)
	{
		Node->Cond->Accept(*this);
		CHECK_ERRORS

		bResult = Pop().GetBool();
		LOG(std::format("WHILE ({}): {}", Count, bResult ? "true" : "false"));
		if (!bResult)
		{
			break;
		}
		Node->Body->Accept(*this);
		Count++;

		if (Count == WHILE_MAX_LOOP)
		{
			Error(std::format("ERROR: Hit max loop count ({}).", WHILE_MAX_LOOP));
			break;
		}
	}
	DEBUG_EXIT
}

void Visitor::Visit(ASTBody* Node)
{
	DEBUG_ENTER
	for (const auto& E : Node->Expressions)
	{
		E->Accept(*this);
	}
	DEBUG_EXIT
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
			Debug(std::format("VALUE: Parsing number: {}", Value.GetFloat().GetValue()));
		}
		// Parse int
		else
		{
			Value = std::stoi(CurrentToken->Content);
			Debug(std::format("VALUE: Parsing number: {}", Value.GetInt().GetValue()));
		}

		Accept(); // Consume number
		Nodes.push_back(new ASTValue(Value));
		DEBUG_EXIT
		return Nodes.back();
	}
	// Parse strings
	else if (Expect(String))
	{
		std::string String = CurrentToken->Content;
		Accept(); // Consume string
		Debug(std::format("VALUE: Parsing string: {}", String));
		Nodes.push_back(new ASTValue(String));
		DEBUG_EXIT
		return Nodes.back();
	}
	// Parse names (Variables, functions, etc.)
	else if (Expect(Name))
	{
		return ParseVariable();
	}
	else if (Expect(Bool))
	{
		bool Value = CurrentToken->Content == "true" ? true : false;
		Accept(); // Consume bool
		Debug(std::format("VALUE: Parsing bool: {}", Value));
		Nodes.push_back(new ASTValue(Value));
		DEBUG_EXIT
		return Nodes.back();
	}

	DEBUG_EXIT
	return nullptr;
}

ASTNode* AST::ParseVariable()
{
	std::string Name = CurrentToken->Content;
	Accept(); // Consume the variable

	if (!ExpectAny({ LParen, LBracket, Period }))
	{
		return new ASTVariable(Name);
	}

	auto StartTok = CurrentToken->Type;
	Accept(); // Consume '['

	std::vector<ASTNode*> Args;
	if (!Expect(RBracket))
	{
		while (!Expect(RBracket))
		{
			auto Arg = ParseExpression();
			if (Arg)
			{
				Args.push_back(Arg);
			}
			if (Expect(RBracket))
			{
				break;
			}
			if (!Expect(Comma))
			{
				Error(std::format("Expected ',', got '{}'.", CurrentToken->Content));
				DEBUG_EXIT
				return nullptr;
			}
			Accept(); // Consume ','
		}
	}

	Accept(); // Consume ']'
	Nodes.push_back(new ASTCall(Name, Args));
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
		Nodes.push_back(new ASTUnaryExpr(Op, ParseValueExpr()));
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
		Nodes.push_back(new ASTBinOp(Expr, ParseUnaryExpr(), Op));
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
		Nodes.push_back(new ASTBinOp(Expr, ParseMultiplicativeExpr(), Op));
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
		Nodes.push_back(new ASTBinOp(Expr, ParseAdditiveExpr(), Op));
		Expr = Nodes.back();
	}

	DEBUG_EXIT
	return Expr;
}

ASTNode* AST::ParseAssignment()
{
	DEBUG_ENTER

	std::string Name = CurrentToken->Content; // Get the name
	Accept();								  // Consume name
	auto Op = CurrentToken->Type;			  // Get the assignment operator
	Accept();								  // Consume assignment operator

	auto Expr = ParseExpression();
	if (Op == PlusEquals || Op == MinusEquals || Op == MultEquals || Op == DivEquals)
	{
		Expr = new ASTBinOp(new ASTVariable(Name), Expr, Op);
	}
	Nodes.push_back(new ASTAssignment(Name, Expr));
	DEBUG_EXIT
	return Nodes.back();
}

ASTNode* AST::ParseParenExpr()
{
	DEBUG_ENTER

	if (!Expect(LParen))
	{
		Error(std::format("Expected '{}' starting conditional. Got '{}'.", "(", CurrentToken->Content));
		DEBUG_EXIT
		return nullptr;
	}
	Accept(); // Consume '('
	ASTNode* Expr = ParseExpression();
	if (!Expect(RParen))
	{
		Error(std::format("Expected '{}' ending conditional. Got '{}'.", ")", CurrentToken->Content));
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
			Debug(std::format("BRACKET: Parsing loop in {}.", __FUNCTION__));
			auto Value = ParseExpression();
			auto CastValue = Cast<ASTValue>(Value);
			if (!CastValue)
			{
				Error("Unable to cast value.");
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
				Error("Expected comma.");
				DEBUG_EXIT
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
		Error(std::format("Expected '{}' starting block. Got '{}'.", "{", CurrentToken->Content));
		DEBUG_EXIT
		return nullptr;
	}
	Accept(); // Consume '{'

	std::vector<ASTNode*> Body;
	while (!Expect(RCurly))
	{
		Debug(std::format("CURLY: Parsing loop in {}.", __FUNCTION__));
		CurrentToken;
		ASTNode* Expr = ParseExpression();
		Body.push_back(Expr);

		// Handle any dangling semicolons
		if (Expect(Semicolon))
		{
			Accept();
		}
	}

	Accept(); // Consume '}'

	Nodes.push_back(new ASTBody(Body));
	DEBUG_EXIT
	return Nodes.back();
}

ASTNode* AST::ParseIf()
{
	DEBUG_ENTER

	Accept(); // Consume 'if'
	auto Cond = ParseParenExpr();
	if (!Cond)
	{
		Error("Unable to parse 'if'.");
		DEBUG_EXIT
		return nullptr;
	}

	LOG("IF: Parsing 'if' block.");
	auto TrueBody = ParseCurlyExpr();
	if (!TrueBody)
	{
		Error("Unable to parse true body of 'if'.");
		DEBUG_EXIT
		return nullptr;
	}

	ASTNode* FalseBody = nullptr;
	if (Expect(Else))
	{
		Accept(); // Consume 'else'
		LOG("IF: Parsing 'else' block.");
		FalseBody = ParseCurlyExpr();
		if (!FalseBody)
		{
			Error("Unable to parse false body of 'else'.");
			DEBUG_EXIT
			return nullptr;
		}
	}

	Nodes.push_back(new ASTIf(Cond, TrueBody, FalseBody));
	DEBUG_EXIT
	return Nodes.back();
}

ASTNode* AST::ParseWhile()
{
	DEBUG_ENTER
	Accept(); // Consume 'while'

	if (!Expect(LParen))
	{
		Error(std::format("Expected '(', got {}", CurrentToken->Content));
		DEBUG_EXIT
		return nullptr;
	}
	Accept(); // Consume '('
	auto Cond = ParseEqualityExpr();
	if (!Expect(RParen))
	{
		Error(std::format("Expected ')', got {}", CurrentToken->Content));
		DEBUG_EXIT
		return nullptr;
	}
	Accept(); // Consume ')'
	auto Body = ParseCurlyExpr();

	if (!Body)
	{
		Error("Unable to parse while body.");
		DEBUG_EXIT
		return nullptr;
	}

	Nodes.push_back(new ASTWhile(Cond, Body));
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
	else
	{
		WARNING(std::format("Unable to parse expression: {}", CurrentToken->ToString()));
		DEBUG_EXIT
		return nullptr;
	}

	DEBUG_EXIT
	return Expr;
}

void AST::ParseBody()
{
	DEBUG_ENTER
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
	DEBUG_EXIT
}
