#include <fstream>

#include <array>

#include "../Public/BuiltIns.h"
#include "../Public/Core.h"

using namespace BuiltIns;

#define CHECK_MIN_ARG_COUNT(Args, Count)                                                            \
    if (Args->size() < Count)                                                                       \
    {                                                                                               \
        Logging::Error("Invalid argument count. Wanted at least {}, got {}.", Count, Args->size()); \
        bResult = false;                                                                            \
        return;                                                                                     \
    }
#define CHECK_EXACT_ARG_COUNT(Args, Count)                                                 \
    if (Args->size() != Count)                                                             \
    {                                                                                      \
        Logging::Error("Invalid argument count. Wanted {}, got {}.", Count, Args->size()); \
        bResult = false;                                                                   \
        return;                                                                            \
    }

void BuiltIns::Print_Internal(TArguments* Arguments, TObject* ReturnValue, bool& bResult)
{
    CHECK_EXACT_ARG_COUNT(Arguments, 1);
    std::vector<TObject> Objects;
    for (auto Arg : *Arguments)
    {
        if (!Arg->IsValid())
        {
            Logging::Error("{}", Arg->ToString());
            bResult = false;
            return;
        }
        TObject Value = Arg->GetValue();
        Objects.push_back(Value);
    }
    std::string Out = TStringValue::Join(Objects, ",");
    std::cout << Out << std::endl;

    bResult = true;
}
void BuiltIns::Printf_Internal(TArguments* Arguments, TObject* ReturnValue, bool& bResult)
{
    CHECK_MIN_ARG_COUNT(Arguments, 2);

    auto Arg1 = Arguments->at(0)->GetValue();
    if (Arg1.GetType() != StringType)
    {
        Logging::Error("Wanted 'string' for first argument, got ", Arg1.ToString());
        bResult = false;
        return;
    }

    // Get the format string (first argument)
    std::string            Fmt = Arg1.GetString();
    size_t                 ArgCount = 0;
    std::string::size_type Pos = 0;

    while ((Pos = Fmt.find("{}", Pos)) != std::string::npos)
    {
        ArgCount++;
        Pos += 2;
    }

    if (ArgCount != Arguments->size() - 1)
    {
        Logging::Error("Printf argument count mismatch. Wanted {}, got {}.", ArgCount, Arguments->size() - 1);
        bResult = false;
        return;
    }

    // Pop the first argument off so we can loop through the rest
    // Arguments->erase(Arguments->begin(), Arguments->begin());

    std::vector<TObject> Objects;
    for (auto [Index, Arg] : Enumerate(*Arguments))
    {
        // Skip the first argument
        if (Index == 0)
        {
            continue;
        }

        if (!Arg->IsValid())
        {
            Logging::Error("{}", Arg->ToString());
            bResult = false;
            return;
        }
        TObject Value = Arg->GetValue();
        Objects.push_back(Value.ToString());
    }

    int         Index = 0; // Start at the second argument as the first is the fmt string itself
    std::string Out = Fmt;
    while (Out.find("{}") != std::string::npos)
    {
        auto First = Out.find_first_of("{}");
        auto Offset = Out.begin() + First;
        Out.replace(Offset, Offset + 2, Objects.at(Index).ToString().c_str());
        Index++;
    }
    std::cout << Out << std::endl;
    bResult = true;
};

void BuiltIns::Append_Internal(TArguments* Arguments, TObject* ReturnValue, bool& bResult)
{
    CHECK_EXACT_ARG_COUNT(Arguments, 2)

    auto Arg1 = Cast<TVariable>(Arguments->at(0));
    auto Arg2 = Arguments->at(1);

    TObject Value = Arg2->GetValue();

    Arg1->GetValuePtr()->AsArray()->Append(Value);
    bResult = true;
}

void BuiltIns::ReadFile_Internal(TArguments* Arguments, TObject* ReturnValue, bool& bResult)
{
    CHECK_EXACT_ARG_COUNT(Arguments, 1)

    auto Arg1 = Arguments->at(0)->GetValue();
    if (Arg1.GetType() != StringType)
    {
        bResult = false;
        Logging::Error("Wanted a string as the first argument.");
        return;
    }

    auto          FileName = Arg1.GetString();
    std::ifstream Stream(FileName.GetValue().c_str());
    if (!Stream.good())
    {
        bResult = false;
        Logging::Error("File '{}' not found.", FileName.GetValue());
        return;
    }

    auto Content = std::string(std::istreambuf_iterator<char>(Stream), std::istreambuf_iterator<char>());
    *ReturnValue = Content;

    bResult = true;
}

void BuiltIns::IndexOf_Internal(TArguments* Arguments, TObject* ReturnValue, bool& bResult)
{
    // size_of ( container, index, out )
    CHECK_EXACT_ARG_COUNT(Arguments, 2);

    auto Container = Arguments->at(0)->GetValue();
    auto Index = Arguments->at(1)->GetValue().GetInt().GetValue();

    TObject Value;
    switch (Container.GetType())
    {
        case StringType :
            Value = Container.GetString().GetValue().at(Index);
            break;
        case ArrayType :
            Value = Container.GetArray().GetValue().at(Index);
            break;
        default :
            bResult = false;
            Logging::Error("Type does not have an 'index'.");
            return;
    }

    *ReturnValue = Value;
    bResult = true;
}

void BuiltIns::SizeOf_Internal(TArguments* Arguments, TObject* ReturnValue, bool& bResult)
{
    CHECK_EXACT_ARG_COUNT(Arguments, 1);

    auto Container = Arguments->at(0)->GetValue();

    int Size = 0;
    switch (Container.GetType())
    {
        case StringType :
            Size = Container.AsString()->GetValue().size();
            break;
        case ArrayType :
            Size = Container.AsArray()->GetValue().size();
            break;
        case MapType :
            Size = Container.AsMap()->GetValue().size();
            break;
        default :
            bResult = false;
            Logging::Error("Type does not have a 'size'.");
            return;
    }

    *ReturnValue = Size;
    bResult = true;
}

void BuiltIns::Contains_Internal(TArguments* Arguments, TObject* ReturnValue, bool& bResult)
{
    CHECK_EXACT_ARG_COUNT(Arguments, 2);
    auto Container = Arguments->at(0)->GetValue();
    auto Value = Arguments->at(1)->GetValue();

    bool bContainsValue = false;
    switch (Container.GetType())
    {
        case StringType :
            bContainsValue = Container.AsString()->GetValue().find(Value.GetString().GetValue()) != std::string::npos;
            break;
        case ArrayType :
            bContainsValue = Container.AsArray()->Contains(Value);
            break;
        case MapType :
            bContainsValue = Container.AsMap()->HasKey(Value.AsString()->GetValue());
            break;
        default :
            bResult = false;
            Logging::Error("Type is not a 'container'.");
            return;
    }
    *ReturnValue = bContainsValue;
    bResult = true;
}

// Initialize the function map of keywords to actual C++ functions
TFunctionMap BuiltIns::InitFunctionMap()
{
    TFunctionMap Map;

    // Containers
    Map["size_of"] = &SizeOf_Internal;
    Map["index_of"] = &IndexOf_Internal;
    Map["append"] = &Append_Internal;
    Map["contains"] = &Contains_Internal;

    // IO
    Map["print"] = &Print_Internal;
    Map["printf"] = &Printf_Internal;
    Map["read_file"] = &ReadFile_Internal;

    return Map;
}
