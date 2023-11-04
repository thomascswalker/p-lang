#pragma once

#include "value.h"

using namespace Values;

namespace BuiltIns
{
	// Forward declaration of all built-in functions
	static void PrintInternal(TArguments* InArguments, bool& bResult);
	static void AppendInternal(TArguments* Arguments, bool& bResult);

	// Initialize the function map of keywords to actual C++ functions
	typedef std::map<std::string, TFunction> TFunctionMap;
	static TFunctionMap						 InitFunctionMap()
	{
		TFunctionMap Map;
		Map["print"] = &PrintInternal;
		Map["append"] = &AppendInternal;
		return Map;
	}
	static TFunctionMap FunctionMap = InitFunctionMap();

	static TObject GetArgumentValue(TIdentifier* Arg)
	{
		auto ArgLiteral = Cast<TLiteral>(Arg);
		auto ArgVariable = Cast<TVariable>(Arg);

		TObject Value;
		if (ArgLiteral)
		{
			Value = ArgLiteral->Value;
		}
		else if (ArgVariable)
		{
			Value = *ArgVariable->GetValue();
		}
		return Value;
	}

	static void PrintInternal(TArguments* InArguments, bool& bResult)
	{
		std::vector<TObject> Objects;
		for (auto A : *InArguments)
		{
			auto Value = GetArgumentValue(A);
			Objects.push_back(Value);
		}
		std::string Out = TStringValue::Join(Objects, ",");
		Log(Out);

		bResult = true;
	};

	static void AppendInternal(TArguments* Arguments, bool& bResult)
	{
		auto Arg1 = Cast<TVariable>(Arguments->at(0));
		auto Arg2 = Arguments->at(1);

		TObject Value = GetArgumentValue(Arg2);

		Arg1->GetValue()->AsArray()->Append(Value);
		bResult = true;
	}

} // namespace BuiltIns