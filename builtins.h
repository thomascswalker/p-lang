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
		for (auto Arg : *InArguments)
		{
			TObject Value = Arg->GetValue();
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

		TObject Value = Arg2->GetValue();

		Arg1->GetValuePtr()->AsArray()->Append(Value);
		bResult = true;
	}

} // namespace BuiltIns