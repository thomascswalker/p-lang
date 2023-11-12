#pragma once

#include "logging.h"
#include "value.h"

using namespace Values;

typedef std::map<std::string, TFunction> TFunctionMap;
namespace BuiltIns
{
	// Forward declaration of all built-in functions
	static void PrintInternal(TArguments* Arguments, TObject* ReturnValue, bool& bResult);
	static void AppendInternal(TArguments* Arguments, TObject* ReturnValue, bool& bResult);
	static void ReadFileInternal(TArguments* Arguments, TObject* ReturnValue, bool& bResult);

	static void IndexOf(TArguments* Arguments, TObject* ReturnValue, bool& bResult);
	static void SizeOf(TArguments* Arguments, TObject* ReturnValue, bool& bResult);

	// Initialize the function map of keywords to actual C++ functions
	TFunctionMap InitFunctionMap();
} // namespace BuiltIns
