#pragma once

#include "value.h"

using namespace Values;

typedef std::map<std::string, TFunction> TFunctionMap;
namespace BuiltIns
{
	// Forward declaration of all built-in functions
	static void PrintInternal(TSignature* Signature, bool& bResult);
	static void AppendInternal(TSignature* Signature, bool& bResult);
	static void ReadFileInternal(TSignature* Signature, bool& bResult);

	// Initialize the function map of keywords to actual C++ functions
	TFunctionMap InitFunctionMap();
} // namespace BuiltIns
