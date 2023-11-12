#include <fstream>

#include "builtins.h"
#include "core.h"

using namespace BuiltIns;

#define CHECK_ARG_COUNT(Args, Count)                                                       \
	if (Args->size() != Count)                                                             \
	{                                                                                      \
		Logging::Error("Invalid argument count. Wanted {}, got {}.", Count, Args->size()); \
		bResult = false;                                                                   \
		return;                                                                            \
	}

void BuiltIns::PrintInternal(TArguments* Arguments, bool& bResult)
{
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
};

void BuiltIns::AppendInternal(TArguments* Arguments, bool& bResult)
{
	CHECK_ARG_COUNT(Arguments, 2)

	auto Arg1 = Cast<TVariable>(Arguments->at(0));
	auto Arg2 = Arguments->at(1);

	TObject Value = Arg2->GetValue();

	Arg1->GetValuePtr()->AsArray()->Append(Value);
	bResult = true;
}

void BuiltIns::ReadFileInternal(TArguments* Arguments, bool& bResult)
{
	CHECK_ARG_COUNT(Arguments, 2)

	auto Arg1 = Arguments->at(0)->GetValue();
	if (Arg1.GetType() != StringType)
	{
		bResult = false;
		Logging::Error("Wanted a string as the first argument.");
		return;
	}

	auto Arg2 = Arguments->at(1)->GetValue();
	if (Arg2.GetType() != StringType)
	{
		bResult = false;
		Logging::Error("Wanted a string as the second argument.");
		return;
	}

	auto		  FileName = Arg1.GetString();
	std::ifstream Stream(FileName.GetValue().c_str());
	if (!Stream.good())
	{
		bResult = false;
		Logging::Error("File '{}' not found.", FileName.GetValue());
		return;
	}

	auto Content = std::string(std::istreambuf_iterator<char>(Stream), std::istreambuf_iterator<char>());
	auto OutString = Arguments->at(1)->GetValuePtr()->AsString();
	*OutString = Content;

	bResult = true;
}

void BuiltIns::IndexOf(TArguments* Arguments, bool& bResult)
{
	// size_of ( iterable, index, out )
	CHECK_ARG_COUNT(Arguments, 3);

	auto Iterable = Arguments->at(0)->GetValue();
	auto Index = Arguments->at(1)->GetValue().GetInt().GetValue();
	auto Out = Arguments->at(2);

	TObject Value;
	switch (Iterable.GetType())
	{
		case StringType :
			Value = Iterable.GetString().GetValue().at(Index);
			break;
		case ArrayType :
			Value = Iterable.GetArray().GetValue().at(Index);
			break;
		default :
			bResult = false;
			Logging::Error("Type does not have an 'index'.");
			return;
	}
	auto OutPtr = Out->GetValuePtr();
	*OutPtr = Value;

	bResult = true;
}

void BuiltIns::SizeOf(TArguments* Arguments, bool& bResult)
{
	CHECK_ARG_COUNT(Arguments, 2);

	auto Arg1 = Arguments->at(0)->GetValue();
	auto Arg2 = Arguments->at(1);

	int Size = 0;
	switch (Arg1.GetType())
	{
		case StringType :
			Size = Arg1.AsString()->GetValue().size();
			break;
		case ArrayType :
			Size = Arg1.AsArray()->GetValue().size();
			break;
		case MapType :
			Size = Arg1.AsMap()->GetValue().size();
			break;
		default :
			bResult = false;
			Logging::Error("Type does not have a 'size'.");
			return;

	}
	auto Arg2Ptr = Arg2->GetValuePtr()->AsInt();
	*Arg2Ptr = Size;

	bResult = true;
}

// Initialize the function map of keywords to actual C++ functions
TFunctionMap BuiltIns::InitFunctionMap()
{
	TFunctionMap Map;

	// Misc
	Map["size_of"] = &SizeOf;
	Map["index_of"] = &IndexOf;

	// IO
	Map["print"] = &PrintInternal;
	Map["read_file"] = &ReadFileInternal;

	// Strings
	Map["append"] = &AppendInternal;

	return Map;
}
