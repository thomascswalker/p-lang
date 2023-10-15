#include "ast.h"

void VariantAdd(const Variant& Left, const Variant& Right, Variant& Value)
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

void VariantSub(const Variant& Left, const Variant& Right, Variant& Value)
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

void VariantMul(const Variant& Left, const Variant& Right, Variant& Value)
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

void VariantDiv(const Variant& Left, const Variant& Right, Variant& Value)
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

void Visitor::Push(const Variant& V)
{
	Stack.push_back(V);
}

Variant Visitor::Pop()
{
	Variant Value = Stack.back();
	Stack.pop_back();
	return Value;
}

bool Visitor::IsVariable(const std::string& Name)
{
	return Variables.find(Name) != Variables.end();
}

void Visitor::Visit(ASTLiteral* Node)
{
	if (Node->IsInt())
	{
		Push(Node->GetInt());
	}
	else if (Node->IsFloat())
	{
		Push(Node->GetFloat());
	}
	else if (Node->IsString())
	{
		Push(Node->GetString());
	}

	//Node->Accept(*this);
}

void Visitor::Visit(ASTVariable* Node)
{

	// If the variable is found, set the current Value to the variable's value
	if (Variables.find(Node->Name) != Variables.end())
	{
		Push(Variables[Node->Name]);
		//Node->Accept(*this);
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

	// Execute the operator on the left and right value
	Variant Result;
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

	// Finally, accept this node
	//Node->Accept(*this);
}

void Visitor::Visit(ASTAssignment* Node)
{

	// Check if the variable exists, and if it doesn't, make a new one
	if (!IsVariable(Node->Name))
	{
		Variables[Node->Name] = 0; // Fill with a dummy value for now
	}

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
	Variables[Node->Name] = Pop();

	//Node->Accept(*this);
}

void Visitor::Visit(ASTProgram* Node)
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
		else
		{
			std::cout << "WARNING: Bad expression!" << std::endl;
		}
	}
	//Node->Accept(*this);
}
