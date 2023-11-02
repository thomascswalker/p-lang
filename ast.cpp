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
		CHECK_EXIT
		CurrentValue = Pop();
	}
	else if (Cast<ASTVariable>(Node->Right))
	{
		auto Expr = Cast<ASTVariable>(Node->Right);
		Visit(Expr);
		CHECK_EXIT
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
	CHECK_EXIT

	// After visiting the left value, pop it off the stack and store it here
	TObject Left = Pop();

	// Visit the right value
	Node->Right->Accept(*this);
	CHECK_EXIT

	// After visiting the right value, pop it off the stack and store it here
	TObject Right = Pop();

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
	LOG(std::format("BINOP: {} {} {} = {}", Left.ToString(), TokenStringMap[Node->Op], Right.ToString(),
					Result.ToString()));
	DEBUG_EXIT
}

void Visitor::Visit(ASTAssignment* Node)
{
	DEBUG_ENTER

	Node->Right->Accept(*this);
	CHECK_EXIT

	TObject Value = Pop();
	SetVariable(Node->Name, Value);

	LOG(std::format("ASSIGN: {} <= {}", Node->Name, Value.ToString()));
	DEBUG_EXIT
}

void Visitor::Visit(ASTIf* Node)
{
	DEBUG_ENTER
	TObject bResult;

	Node->Cond->Accept(*this);
	CHECK_EXIT

	bResult = Pop();
	LOG(std::format("IF: {}", bResult ? "true" : "false"));
	if (bResult)
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
		CHECK_EXIT

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

void Visitor::Visit(ASTFunctionDef* Node) {}

void Visitor::Visit(ASTReturn* Node) {}

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
			LOG(std::format("VALUE: Parsing number: {}", Value.GetFloat().GetValue()));
		}
		// Parse int
		else
		{
			Value = std::stoi(CurrentToken->Content);
			LOG(std::format("VALUE: Parsing number: {}", Value.GetInt().GetValue()));
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
		LOG(std::format("VALUE: Parsing string: {}", String));
		Nodes.push_back(new ASTValue(String));
		DEBUG_EXIT
		return Nodes.back();
	}
	// Parse names (Variables, functions, etc.)
	else if (Expect(Name))
	{
		std::string Name = CurrentToken->Content;
		Accept(); // Consume name
		LOG(std::format("VALUE: Parsing name: {}", Name));
		DEBUG_EXIT
		Nodes.push_back(new ASTVariable(Name));
		return Nodes.back();
	}
	else if (Expect(Bool))
	{
		bool Value = CurrentToken->Content == "true" ? true : false;
		Accept(); // Consume bool
		LOG(std::format("VALUE: Parsing bool: {}", Value));
		Nodes.push_back(new ASTValue(Value));
		DEBUG_EXIT
		return Nodes.back();
	}

	DEBUG_EXIT
	return nullptr;
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
		std::string Op = CurrentToken->Content;
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
		std::string Op = CurrentToken->Content;
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
		std::string Op = CurrentToken->Content;
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

	Nodes.push_back(new ASTAssignment(Name, ParseExpression()));
	DEBUG_EXIT
	return Nodes.back();
}

ASTNode* AST::ParseReturnExpr()
{
	DEBUG_ENTER
	Accept(); // Consume return
	ASTNode* Expr = ParseExpression();
	Nodes.push_back(new ASTReturn(Expr));
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
			LOG(std::format("BRACKET: Parsing loop in {}.", __FUNCTION__));
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
		LOG(std::format("CURLY: Parsing loop in {}.", __FUNCTION__));
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

ASTNode* AST::ParseFunctionDef()
{
	DEBUG_ENTER

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
	DEBUG_EXIT
	return nullptr;
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
	if (Expect(Name) && ExpectAny({ Assign }, 1))
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
