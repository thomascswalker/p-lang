#include "ast.h"

//////////////
// Variants //
//////////////

void VariantAdd(const Literal& Left, const Literal& Right, Literal& Value)
{
	Value = std::visit(
		[](auto L, auto R) -> Literal {
			if constexpr (!std::is_same_v<decltype(L), decltype(R)>)
			{
				throw;
			}
			else
			{
				return L + R;
			}
		},
		Left, Right);
}

void VariantSub(const Literal& Left, const Literal& Right, Literal& Value)
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

void VariantMul(const Literal& Left, const Literal& Right, Literal& Value)
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

void VariantDiv(const Literal& Left, const Literal& Right, Literal& Value)
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
	switch (Node->Value.index())
	{
		case 0 :
			Push(Node->GetInt());
			break;
		case 1 :
			Push(Node->GetFloat());
			break;
		case 2 :
			Push(Node->GetString());
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
	switch (*Node->Op.c_str())
	{
		case '+' :
			VariantAdd(Left, Right, Result);
			break;
		case '-' :
			VariantSub(Left, Right, Result);
			break;
		case '*' :
			VariantMul(Left, Right, Result);
			break;
		case '/' :
			VariantDiv(Left, Right, Result);
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

void Visitor::Visit(ASTProgram* Node)
{
	for (const auto& E : Node->Statements)
	{
		if (Cast<ASTBinOp>(E))
		{
			Visit(Cast<ASTBinOp>(E));
		}
		else if (Cast<ASTAssignment>(E))
		{
			Visit(Cast<ASTAssignment>(E));
		}
		else
		{
			std::cout << "WARNING: Bad expression!" << std::endl;
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
		CurrentToken++;
		Position++;
	}
}

bool AST::Expect(const std::string& Type, int Offset)
{
	// Make sure the offset is valid
	if (Position + Offset > Tokens.max_size())
	{
		Error("WARNING: Outside token bounds.");
		return false;
	}

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

ASTNode* AST::ParseFactorExpr()
{
	// Parse numbers (floats and ints)
	if (Expect("Number"))
	{
		Literal Value;

		// Parse float
		if (CurrentToken->Content.find(".") != std::string::npos)
		{
			Value = std::stof(CurrentToken->Content);
		}
		// Parse int
		else
		{
			Value = std::stoi(CurrentToken->Content);
		}
		Accept(); // Consume number
		Nodes.push_back(new ASTLiteral(Value));
		return Nodes.back();
	}
	// Parse strings
	else if (Expect("String"))
	{
		std::string String = CurrentToken->Content;
		Accept(); // Consume string
		Nodes.push_back(new ASTLiteral(String));
		return Nodes.back();
	}
	// Parse names (Variables, functions, etc.)
	else if (Expect("Name"))
	{
		std::string Name = CurrentToken->Content;
		Accept(); // Consume name
		Nodes.push_back(new ASTVariable(Name));
		return Nodes.back();
	}
	else if (Expect("("))
	{
		return ParseEncapsulatedExpr();
	}
	else
	{
		return nullptr;
	}
}

ASTNode* AST::ParseEncapsulatedExpr()
{
	Accept(); // Consume '('
	ASTNode* Expr = ParseExpression();
	if (!Expect(")"))
	{
		Error("Missing ')'.");
		return nullptr;
	}
	Accept(); // Consume ')'
	return Expr;
}

ASTNode* AST::ParseMultiplicativeExpr()
{
	ASTNode* Expr = ParseFactorExpr();
	while (Expect("*") || Expect("/"))
	{
		std::string Op = CurrentToken->Content;
		Accept(); // Consume '*' or '/'
		Nodes.push_back(new ASTBinOp(Expr, ParseFactorExpr(), Op));
		Expr = Nodes.back();
	}
	return Expr;
}

ASTNode* AST::ParseAdditiveExpr()
{
	ASTNode* Expr = ParseMultiplicativeExpr();
	while (Expect("+") || Expect("-"))
	{
		std::string Op = CurrentToken->Content;
		Accept(); // Consume '+' or '-'
		Nodes.push_back(new ASTBinOp(Expr, ParseMultiplicativeExpr(), Op));
		Expr = Nodes.back();
	}
	return Expr;
}

ASTNode* AST::ParseAssignment()
{
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

	return nullptr;
}

ASTNode* AST::ParseExpression()
{
	// int MyVar = ...;
	if (Expect({ "Type", "Name", "=" }))
	{
		return ParseAssignment();
	}
	// MyVar = ...;
	else if (Expect({ "Name", "=" }))
	{
		return ParseAssignment();
	}
	// 5 + ...;
	// "Test" + ...;
	// MyVar + ...;
	else if (Expect("Number") || Expect("String") || Expect("Name") || Expect("("))
	{
		return ParseAdditiveExpr();
	}
	else
	{
		Error("Syntax Error");
		return nullptr;
	}
}

void AST::ParseProgram()
{
	Program = new ASTProgram();
	while (CurrentToken != NULL && CurrentToken != &Tokens.back())
	{
		auto Expr = ParseExpression();
		if (!Expr)
		{
			break;
		}
		Program->Statements.push_back(Expr);
		if (Expect(";"))
		{
			Accept(); // Consume ';'
		}
		else
		{
			Error(std::format("Missing semicolon, got '{}'", CurrentToken->Type));
			break;
		}
	}
}
