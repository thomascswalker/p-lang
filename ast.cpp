#include "ast.h"

using namespace Core;

//////////////
// Literals //
//////////////

void LiteralAdd(const Literal& Left, const Literal& Right, Literal& Value)
{
	switch (Left.index())
	{
		case 0 :
			Value = std::get<int>(Left) + std::get<int>(Right);
			break;
		case 1 :
			Value = std::get<float>(Left) + std::get<float>(Right);
			break;
		case 2 :
			Value = std::get<std::string>(Left) + std::get<std::string>(Right);
			break;
	}
}

void LiteralSub(const Literal& Left, const Literal& Right, Literal& Value)
{
	switch (Left.index())
	{
		case 0 :
			Value = std::get<int>(Left) - std::get<int>(Right);
			break;
		case 1 :
			Value = std::get<float>(Left) - std::get<float>(Right);
			break;
	}
}

void LiteralMul(const Literal& Left, const Literal& Right, Literal& Value)
{
	switch (Left.index())
	{
		case 0 :
			Value = std::get<int>(Left) * std::get<int>(Right);
			break;
		case 1 :
			Value = std::get<float>(Left) * std::get<float>(Right);
			break;
	}
}

void LiteralDiv(const Literal& Left, const Literal& Right, Literal& Value)
{
	switch (Left.index())
	{
		case 0 :
			Value = std::get<int>(Left) / std::get<int>(Right);
			break;
		case 1 :
			Value = std::get<float>(Left) / std::get<float>(Right);
			break;
	}
}

void LiteralEq(const Literal& Left, const Literal& Right, Literal& Value)
{
	switch (Left.index())
	{
		case 0 :
			Value = std::get<int>(Left) == std::get<int>(Right);
			break;
		case 1 :
			Value = std::get<float>(Left) == std::get<float>(Right);
			break;
		case 2 :
			Value = std::get<std::string>(Left) == std::get<std::string>(Right);
			break;
		case 3 :
			Value = std::get<bool>(Left) == std::get<bool>(Right);
			break;
	}
}

void LiteralNotEq(const Literal& Left, const Literal& Right, Literal& Value)
{
	switch (Left.index())
	{
		case 0 :
			Value = std::get<int>(Left) != std::get<int>(Right);
			break;
		case 1 :
			Value = std::get<float>(Left) != std::get<float>(Right);
			break;
		case 2 :
			Value = std::get<std::string>(Left) != std::get<std::string>(Right);
			break;
		case 3 :
			Value = std::get<bool>(Left) != std::get<bool>(Right);
			break;
	}
}

//////////////
// Visitors //
//////////////

void Visitor::Push(const Literal& V)
{
	Stack.push_back(V);
}

Literal Visitor::Pop()
{
	Literal Value = Stack.back();
	Stack.pop_back();
	return Value;
}

bool Visitor::IsVariable(const std::string& Name)
{
	return Variables.find(Name) != Variables.end();
}

void Visitor::Visit(ASTLiteral* Node)
{
	Debug("Visiting literal.");

	switch (Node->Value.index())
	{
		case 0 :
			Push(Node->GetInt());
			Debug(std::format("Got literal int: {}", std::get<int>(Stack.back())));
			break;
		case 1 :
			Push(Node->GetFloat());
			Debug(std::format("Got literal int: {}", std::get<float>(Stack.back())));
			break;
		case 2 :
			Push(Node->GetString());
			Debug(std::format("Got literal string: {}", std::get<std::string>(Stack.back())));
			break;
		case 3 :
			Push(Node->GetBool());
			Debug(std::format("Got literal bool: {}", std::get<bool>(Stack.back())));
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
	if (Cast<ASTLiteral>(Node->Left))
	{
		Visit(Cast<ASTLiteral>(Node->Left));
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
	if (Cast<ASTLiteral>(Node->Right))
	{
		Visit(Cast<ASTLiteral>(Node->Right));
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

	// If either values are floats, cast the other to a float
	if (Left.index() == 0 && Right.index() == 1)
	{
		auto LValue = std::get<int>(Left);
		Left = Literal((float)LValue);
	}
	else if (Left.index() == 1 && Right.index() == 0)
	{
		auto RValue = std::get<int>(Right);
		Right = Literal((float)RValue);
	}

	// Execute the operator on the left and right value
	Literal Result;
	switch (Node->Op)
	{
		case Plus :
			LiteralAdd(Left, Right, Result);
			break;
		case Minus :
			LiteralSub(Left, Right, Result);
			break;
		case Multiply :
			LiteralMul(Left, Right, Result);
			break;
		case Divide :
			LiteralDiv(Left, Right, Result);
			break;
		case Equals :
			LiteralEq(Left, Right, Result);
			break;
		case NotEquals :
			LiteralNotEq(Left, Right, Result);
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

	if (Cast<ASTLiteral>(Node->Right))
	{
		Visit(Cast<ASTLiteral>(Node->Right));
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

void Visitor::Visit(ASTConditional* Node)
{
	Debug("Visiting conditional.");
	bool bResult = false;

	if (Cast<ASTLiteral>(Node->Cond))
	{
		Visit(Cast<ASTLiteral>(Node->Cond));
	}
	else if (Cast<ASTVariable>(Node->Cond))
	{
		Visit(Cast<ASTVariable>(Node->Cond));
	}
	else if (Cast<ASTBinOp>(Node->Cond))
	{
		Visit(Cast<ASTBinOp>(Node->Cond));
	}

	bResult = std::get<bool>(Pop());
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
		else if (Cast<ASTLiteral>(E))
		{
			Visit(Cast<ASTLiteral>(E));
		}
		else if (Cast<ASTConditional>(E))
		{
			Visit(Cast<ASTConditional>(E));
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

bool AST::Expect(const std::string& Type, int Offset)
{
	// Make sure the offset is valid
	if (Position + Offset > Tokens.size())
	{
		Error("WARNING: Outside token bounds.");
		return false;
	}

	Debug(std::format("Expect(): {} at {}: {}", Type, Position + Offset, Tokens[Position + Offset].ToString()));

	return Tokens[Position + Offset].Type == Type;
}

bool AST::Expect(const std::initializer_list<std::string>& Types)
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

ASTNode* AST::ParseLiteralExpr()
{
	Debug("Parsing literal.");

	// Parse numbers (floats and ints)
	if (Expect("Number"))
	{
		Literal Value;

		// Parse float
		if (CurrentToken->Content.find(".") != std::string::npos)
		{
			Value = std::stof(CurrentToken->Content);
			Debug(std::format("Parsing number: {}", std::get<float>(Value)));
		}
		// Parse int
		else
		{
			Value = std::stoi(CurrentToken->Content);
			Debug(std::format("Parsing number: {}", std::get<int>(Value)));
		}

		Accept(); // Consume number
		Nodes.push_back(new ASTLiteral(Value));
		Debug("Exiting parse literal.");
		return Nodes.back();
	}
	// Parse strings
	else if (Expect("String"))
	{
		std::string String = CurrentToken->Content;
		Accept(); // Consume string
		Debug(std::format("Parsing string: {}", String));
		Nodes.push_back(new ASTLiteral(String));
		Debug("Exiting parse literal.");
		return Nodes.back();
	}
	// Parse names (Variables, functions, etc.)
	else if (Expect("Name"))
	{
		std::string Name = CurrentToken->Content;
		Accept(); // Consume name
		Debug(std::format("Parsing name: {}", Name));
		Nodes.push_back(new ASTVariable(Name));
		Debug("Exiting parse literal.");
		return Nodes.back();
	}
	else if (Expect("Bool"))
	{
		bool Value = CurrentToken->Content == "true" ? true : false;
		Accept(); // Consume bool
		Debug(std::format("Parsing bool: {}", Value));
		Nodes.push_back(new ASTLiteral(Value));
		Debug("Exiting parse literal.");
		return Nodes.back();
	}
	else if (Expect("("))
	{
		Debug("Parsing parenthesis block.");
		Debug("Exiting parse literal.");
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

	if (!Expect("("))
	{
		Error(std::format("Expected '{}' starting conditional. Got '{}'.", "(", CurrentToken->Content));
		return nullptr;
	}
	Accept(); // Consume '('
	ASTNode* Expr = ParseExpression();
	if (!Expect(")"))
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

	if (!Expect("{"))
	{
		Error(std::format("Expected '{}' starting block. Got '{}'.", "{", CurrentToken->Content));
		return nullptr;
	}
	Accept(); // Consume '{'

	std::vector<ASTNode*> Body;
	while (true)
	{
		ASTNode* Expr = ParseExpression();
		if (Expect(";"))
		{
			Accept(); // Consume ';'
		}
		else
		{
			Error("Expected semicolon.");
			return nullptr;
		}
		Body.push_back(Expr);
		if (Expect("}"))
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

	ASTNode* Expr = ParseLiteralExpr();
	while (Expect("*") || Expect("/"))
	{
		std::string Op = CurrentToken->Content;
		Accept(); // Consume '*' or '/'
		Nodes.push_back(new ASTBinOp(Expr, ParseLiteralExpr(), Op));
		Expr = Nodes.back();
	}

	Debug("Exiting parse mult.");
	return Expr;
}

ASTNode* AST::ParseAdditiveExpr()
{
	Debug("Parsing add.");

	ASTNode* Expr = ParseMultiplicativeExpr();
	while (Expect("+") || Expect("-"))
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
	while (Expect("==") || Expect("!="))
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
	if (Expect("Type"))
	{
		Type = CurrentToken->Content; // Get the type
		Accept();					  // Consume type
	}

	if (Expect("Name"))
	{
		// Handle existing variable assignment, or continue with a new assignment
		Name = CurrentToken->Content; // Get the name
		Accept();					  // Consume name
		if (Expect("="))
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

ASTNode* AST::ParseConditional()
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
	if (Expect("else"))
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

	if (!Expect(";"))
	{
		Error("Missing semicolon at end of conditional statement.");
		return nullptr;
	}

	Nodes.push_back(new ASTConditional(Cond, TrueBody, FalseBody));

	Debug("Exiting parse cond.");

	return Nodes.back();
}

ASTNode* AST::ParseExpression()
{
	Debug("Parsing expression.");

	ASTNode* Expr;

	// int MyVar = ...;
	if (Expect({ "Type", "Name", "=" }))
	{
		Expr = ParseAssignment();
	}
	// MyVar = ...;
	else if (Expect({ "Name", "=" }))
	{
		Expr = ParseAssignment();
	}
	// 5 + ...;
	// "Test" + ...;
	// MyVar + ...;
	else if (Expect("Bool") || Expect("Number") || Expect("String") || Expect("Name") || Expect("("))
	{
		Expr = ParseEqualityExpr();
	}
	else if (Expect("if"))
	{
		Expr = ParseConditional();
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
		if (Expect(";"))
		{
			Accept(); // Consume ';'
		}
		else
		{
			Error("Expected semicolon.");
			break;
		}
	}
}
