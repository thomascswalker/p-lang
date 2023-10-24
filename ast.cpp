#include "ast.h"
#include <functional>

using namespace Core;

//////////////
// Visitors //
//////////////

void Visitor::Push(const TObject& V)
{
	Stack.push_back(V);
}

TObject Visitor::Pop()
{
	TObject Value = Stack.back();
	Stack.pop_back();
	return Value;
}

bool Visitor::IsVariable(const std::string& Name)
{
	return Variables.find(Name) != Variables.end();
}

void Visitor::Visit(ASTValue* Node)
{
	Debug("Visiting value.");
	switch (Node->Value.GetType())
	{
		case IntType :
			Push(TObject(Node->GetInt().GetValue()));
			break;
		case FloatType :
			Push(TObject(Node->GetFloat().GetValue()));
			break;
		case StringType :
			Push(TObject(Node->GetString().GetValue()));
			break;
		case BoolType :
			Push(TObject(Node->GetBool().GetValue()));
			break;
		default :
			Error("Invalid type.");
			break;
	}
}

void Visitor::Visit(ASTVariable* Node)
{

	// If the variable is found, set the current Value to the variable's value
	if (Variables.count(Node->Name))
	{
		Push(Variables[Node->Name]);
	}
	else
	{
		std::string Msg = std::format("Variable {} not found.", Node->Name);
		throw std::runtime_error(Msg);
	}
}

void Visitor::Visit(ASTBinOp* Node)
{

	// Visit the left value
	if (Cast<ASTValue>(Node->Left))
	{
		Visit(Cast<ASTValue>(Node->Left));
	}
	else if (Cast<ASTVariable>(Node->Left))
	{
		Visit(Cast<ASTVariable>(Node->Left));
	}
	else if (Cast<ASTBinOp>(Node->Left))
	{
		Visit(Cast<ASTBinOp>(Node->Left));
	}
	// After visiting the left value, pop it off the stack and store it here
	auto Left = Pop();

	// Visit the right value
	if (Cast<ASTValue>(Node->Right))
	{
		Visit(Cast<ASTValue>(Node->Right));
	}
	else if (Cast<ASTVariable>(Node->Right))
	{
		Visit(Cast<ASTVariable>(Node->Right));
	}
	else if (Cast<ASTBinOp>(Node->Right))
	{
		Visit(Cast<ASTBinOp>(Node->Right));
	}
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
			Result = Left != Right;
			break;
	}

	// Push the resulting value to the stack
	Push(Result);
}

void Visitor::Visit(ASTAssignment* Node)
{

	// Check if the variable exists, and if it doesn't, make a new one
	if (!IsVariable(Node->Name))
	{
		Variables[Node->Name] = 0; // Fill with a dummy value for now
	}
	auto Var = &Variables[Node->Name];

	if (Cast<ASTValue>(Node->Right))
	{
		Visit(Cast<ASTValue>(Node->Right));
	}
	else if (Cast<ASTVariable>(Node->Right))
	{
		Visit(Cast<ASTVariable>(Node->Right));
	}
	else if (Cast<ASTBinOp>(Node->Right))
	{
		Visit(Cast<ASTBinOp>(Node->Right));
	}
	else
	{
		std::cout << "WARNING: Bad assignment!" << std::endl;
	}

	Variables[Node->Name] = Pop();
}

void Visitor::Visit(ASTIf* Node)
{
	Debug("Visiting conditional.");
	TBoolValue bResult = false;

	if (Cast<ASTValue>(Node->Cond))
	{
		Visit(Cast<ASTValue>(Node->Cond));
	}
	else if (Cast<ASTVariable>(Node->Cond))
	{
		Visit(Cast<ASTVariable>(Node->Cond));
	}
	else if (Cast<ASTBinOp>(Node->Cond))
	{
		Visit(Cast<ASTBinOp>(Node->Cond));
	}

	bResult = Pop().GetBool();
	Debug(std::format("Conditional result is {}", bResult ? "true" : "false"));
	if (bResult)
	{
		auto Body = Cast<ASTBody>(Node->TrueBody);
		Visit(Body);
	}
	else
	{
		auto Body = Cast<ASTBody>(Node->FalseBody);
		Visit(Body);
	}
}

void Visitor::Visit(ASTWhile* Node)
{

	Debug("Visiting while.");
	TBoolValue bResult = true;
	int		   Count = 1;

	while (bResult)
	{
		if (Cast<ASTValue>(Node->Cond))
		{
			Visit(Cast<ASTValue>(Node->Cond));
		}
		else if (Cast<ASTVariable>(Node->Cond))
		{
			Visit(Cast<ASTVariable>(Node->Cond));
		}
		else if (Cast<ASTBinOp>(Node->Cond))
		{
			Visit(Cast<ASTBinOp>(Node->Cond));
		}
		else
		{
			throw std::runtime_error("Invalid AST node.");
		}

		bResult = Pop().GetBool();
		if (!bResult)
		{
			break;
		}
		Debug(std::format("While result is {}", bResult ? "true" : "false"));
		auto Body = Cast<ASTBody>(Node->Body);
		Visit(Body);
		Count++;

		if (Count == WHILE_MAX_LOOP)
		{
			Error("Hit max loop count (10000).");
			break;
		}
	}
}

void Visitor::Visit(ASTBody* Node)
{
	for (const auto& E : Node->Expressions)
	{
		if (Cast<ASTBinOp>(E))
		{
			Visit(Cast<ASTBinOp>(E));
		}
		else if (Cast<ASTAssignment>(E))
		{
			Visit(Cast<ASTAssignment>(E));
		}
		else if (Cast<ASTValue>(E))
		{
			Visit(Cast<ASTValue>(E));
		}
		else if (Cast<ASTIf>(E))
		{
			Visit(Cast<ASTIf>(E));
		}
		else if (Cast<ASTWhile>(E))
		{
			Visit(Cast<ASTWhile>(E));
		}
		else
		{
			std::cout << "WARNING: Bad expression during visiting!" << std::endl;
		}
	}
}

/////////
// AST //
/////////

void AST::Accept()
{
	// Get the last token in the token list
	Token* End = &Tokens.back();

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

bool AST::Expect(TokenType Type, int Offset)
{
	// Make sure the offset is valid
	if (Position + Offset > Tokens.size())
	{
		Error("WARNING: Outside token bounds.");
		return false;
	}

	Debug(std::format("Expect(): {} at {}: {}", (int)Type, Position + Offset, Tokens[Position + Offset].ToString()));

	return Tokens[Position + Offset].Type == Type;
}

bool AST::Expect(const std::initializer_list<TokenType>& Types)
{
	for (auto [I, T] : Enumerate(Types))
	{
		if (!Expect(T, (int)I))
		{
			return false;
		}
	}
	return true;
}

ASTNode* AST::ParseValueExpr()
{
	Debug("Parsing value.");

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
		Debug("Exiting parse value.");
		return Nodes.back();
	}
	// Parse strings
	else if (Expect(String))
	{
		std::string String = CurrentToken->Content;
		Accept(); // Consume string
		Debug(std::format("Parsing string: {}", String));
		Nodes.push_back(new ASTValue(String));
		Debug("Exiting parse value.");
		return Nodes.back();
	}
	// Parse names (Variables, functions, etc.)
	else if (Expect(Name))
	{
		std::string Name = CurrentToken->Content;
		Accept(); // Consume name
		Debug(std::format("Parsing name: {}", Name));
		Nodes.push_back(new ASTVariable(Name));
		Debug("Exiting parse value.");
		return Nodes.back();
	}
	else if (Expect(Bool))
	{
		bool Value = CurrentToken->Content == "true" ? true : false;
		Accept(); // Consume bool
		Debug(std::format("Parsing bool: {}", Value));
		Nodes.push_back(new ASTValue(Value));
		Debug("Exiting parse value.");
		return Nodes.back();
	}
	else if (Expect(LParen))
	{
		Debug("Parsing parenthesis block.");
		Debug("Exiting parse value.");
		return ParseParenExpr();
	}
	else
	{
		Error("Unable to parse factor.");
		return nullptr;
	}
}

ASTNode* AST::ParseParenExpr()
{
	Debug("Parsing paren.");

	if (!Expect(LParen))
	{
		Error(std::format("Expected '{}' starting conditional. Got '{}'.", "(", CurrentToken->Content));
		return nullptr;
	}
	Accept(); // Consume '('
	ASTNode* Expr = ParseExpression();
	if (!Expect(RParen))
	{
		Error(std::format("Expected '{}' ending conditional. Got '{}'.", ")", CurrentToken->Content));
		return nullptr;
	}
	Accept(); // Consume ')'
	Debug("Exiting parse paren.");
	return Expr;
}

ASTNode* AST::ParseCurlyExpr()
{
	Debug("Parsing curly.");

	if (!Expect(LCurly))
	{
		Error(std::format("Expected '{}' starting block. Got '{}'.", "{", CurrentToken->Content));
		return nullptr;
	}
	Accept(); // Consume '{'

	std::vector<ASTNode*> Body;
	while (true)
	{
		ASTNode* Expr = ParseExpression();
		Body.push_back(Expr);
		if (Expect(RCurly))
		{
			break;
		}
	}

	Accept(); // Consume '}'

	Debug("Exiting parse curly.");

	Nodes.push_back(new ASTBody(Body));
	return Nodes.back();
}

ASTNode* AST::ParseMultiplicativeExpr()
{
	Debug("Parsing mult.");

	ASTNode* Expr = ParseValueExpr();
	while (Expect(Multiply) || Expect(Divide))
	{
		std::string Op = CurrentToken->Content;
		Accept(); // Consume '*' or '/'
		Nodes.push_back(new ASTBinOp(Expr, ParseValueExpr(), Op));
		Expr = Nodes.back();
	}

	Debug("Exiting parse mult.");
	return Expr;
}

ASTNode* AST::ParseAdditiveExpr()
{
	Debug("Parsing add.");

	ASTNode* Expr = ParseMultiplicativeExpr();
	while (Expect(Plus) || Expect(Minus))
	{
		std::string Op = CurrentToken->Content;
		Accept(); // Consume '+' or '-'
		Nodes.push_back(new ASTBinOp(Expr, ParseMultiplicativeExpr(), Op));
		Expr = Nodes.back();
	}

	Debug("Exiting parse add.");

	return Expr;
}

ASTNode* AST::ParseEqualityExpr()
{
	Debug("Parsing equality.");
	ASTNode* Expr = ParseAdditiveExpr();
	while (Expect(Equals) || Expect(NotEquals) || Expect(LessThan) || Expect(GreaterThan))
	{
		std::string Op = CurrentToken->Content;
		Accept(); // Consume '==' or '!='
		Nodes.push_back(new ASTBinOp(Expr, ParseAdditiveExpr(), Op));
		Expr = Nodes.back();
	}

	Debug("Exiting parse equality.");

	return Expr;
}

ASTNode* AST::ParseAssignment()
{
	Debug("Parsing assignment.");

	std::string Type;
	std::string Name;

	// Handle new variable assignments
	if (Expect(TokenType::Type))
	{
		Type = CurrentToken->Content; // Get the type
		Accept();					  // Consume type
	}

	if (Expect(TokenType::Name))
	{
		// Handle existing variable assignment, or continue with a new assignment
		Name = CurrentToken->Content; // Get the name
		Accept();					  // Consume name
		if (Expect(Assign))
		{
			Accept(); // Consume '='
			ASTNode* Expr = ParseExpression();
			Nodes.push_back(new ASTAssignment(Type, Name, Expr));
			return Nodes.back();
		}
	}

	Debug("Exiting parse assignment.");

	return nullptr;
}

ASTNode* AST::ParseIf()
{
	Debug("Parsing cond.");

	Accept(); // Consume 'if'
	auto Cond = ParseParenExpr();
	if (!Cond)
	{
		Error("Unable to parse 'if'.");
		return nullptr;
	}

	Debug("Parsing 'if' block.");
	auto TrueBody = ParseCurlyExpr();
	if (!TrueBody)
	{
		Error("Unable to parse true body of 'if'.");
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
			return nullptr;
		}
	}

	Nodes.push_back(new ASTIf(Cond, TrueBody, FalseBody));

	Debug("Exiting parse cond.");

	return Nodes.back();
}

ASTNode* AST::ParseWhile()
{
	Debug("Parsing while.");
	Accept(); // Consume 'while'
	auto Cond = ParseParenExpr();

	Debug("Parsing while block.");
	auto Body = ParseCurlyExpr();

	if (!Body)
	{
		Error("Unable to parse while body.");
		return nullptr;
	}

	Nodes.push_back(new ASTWhile(Cond, Body));
	Debug("Exiting parse while.");
	return Nodes.back();
}

ASTNode* AST::ParseExpression()
{
	Debug("Parsing expression.");

	ASTNode* Expr;

	// int MyVar = ...;
	if (Expect({ Type, Name, Assign }))
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
	// 5 + ...;
	// "Test" + ...;
	// MyVar + ...;
	else if (Expect(Bool) || Expect(Number) || Expect(String) || Expect(Name) || Expect(LParen))
	{
		Expr = ParseEqualityExpr();
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
		Error("Unable to parse expression.");
		return nullptr;
	}

	Debug("Exiting parsing expression.");
	return Expr;
}

void AST::ParseBody()
{
	Debug("Parsing body.");

	Program = new ASTBody();
	while (CurrentToken != NULL && CurrentToken != &Tokens.back())
	{
		auto Expr = ParseExpression();
		if (!Expr)
		{
			break;
		}
		Program->Expressions.push_back(Expr);
	}
}
