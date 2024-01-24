#pragma once

#include "Value.h"

using namespace Values;

using TFunctionMap = std::map<std::string, TFunction>;

namespace BuiltIns
{
    // Forward declaration of all built-in functions

    // IO
    static void Print_Internal(TArguments* Arguments, TObject* ReturnValue, bool& bResult);
    static void Printf_Internal(TArguments* Arguments, TObject* ReturnValue, bool& bResult);
    static void ReadFile_Internal(TArguments* Arguments, TObject* ReturnValue, bool& bResult);

    // Containers
    static void IndexOf_Internal(TArguments* Arguments, TObject* ReturnValue, bool& bResult);
    static void SizeOf_Internal(TArguments* Arguments, TObject* ReturnValue, bool& bResult);
    static void Append_Internal(TArguments* Arguments, TObject* ReturnValue, bool& bResult);
    static void Contains_Internal(TArguments* Arguments, TObject* ReturnValue, bool& bResult);

    // Initialize the function map of keywords to actual C++ functions
    TFunctionMap InitFunctionMap();
} // namespace BuiltIns
