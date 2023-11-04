#pragma once

#include "value.h"

using namespace Values;
using TArgument = std::pair<std::string, TObject>;
using TArguments = std::vector<TArgument>;

namespace BuiltIns
{
	static bool PrintInternal(const TArguments& Arguments)
	{
		if (Arguments.size() == 0)
		{
			Error("Invalid number of arguments for print()");
			return false;
		}
		TArrayValue Values;
		for (const auto& Arg : Arguments)
		{
			Values.Append(Arg.second);
		}
		std::string ArgString = TStringValue::Join(Values, ",");
		printf("%s\n", ArgString.c_str());
		return true;
	}

	static bool AppendInternal(TArguments& Arguments, TArrayValue& Result)
	{
		if (Arguments.size() != 2)
		{
			Error("Invalid number of arguments for append()");
			return false;
		}
		TArrayValue* Array = Arguments[0].second.AsArray();
		TObject		 Value = Arguments[1].second;
		Array->Append(Value);
		Result = *Array;

		return true;
	}

} // namespace BuiltIns