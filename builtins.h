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

	static void PrintInternal(TArguments* InArguments, bool& bResult)
	{
		std::vector<TObject> Objects;
		for (auto A : *InArguments)
		{
			if (Cast<TVariable>(A))
			{
				auto B = Cast<TVariable>(A);
				Objects.push_back(*B->GetValue());
			}
			else if (Cast<TLiteral>(A))
			{
				auto L = Cast<TLiteral>(A);
				Objects.push_back(L->Value);
			}
		}
		std::string Out = TStringValue::Join(Objects, ",");
		Log(Out);

		bResult = true;
	};

	static void AppendInternal(TArguments* Arguments, bool& bResult)
	{
		auto Arg1 = Cast<TVariable>(Arguments->at(0));

		TObject Value;

		auto Arg2Literal = Cast<TLiteral>(Arguments->at(1));
		auto Arg2Variable = Cast<TVariable>(Arguments->at(1));
		if (Arg2Literal)
		{
			Value = Arg2Literal->Value;
		}
		else if (Arg2Variable)
		{
			Value = *Arg2Variable->GetValue();
		}

		Arg1->GetValue()->AsArray()->Append(Value);
		bResult = true;
	}

} // namespace BuiltIns