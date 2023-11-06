#include "builtins.h"

using namespace BuiltIns;

void BuiltIns::PrintInternal(TArguments* Arguments, bool& bResult)
{
	std::vector<TObject> Objects;
	for (auto Arg : *Arguments)
	{
		TObject Value = Arg->GetValue();
		Objects.push_back(Value);
	}
	std::string Out = TStringValue::Join(Objects, ",");
	Log(Out);

	bResult = true;
};

void BuiltIns::AppendInternal(TArguments* Arguments, bool& bResult)
{
	auto Arg1 = Cast<TVariable>(Arguments->at(0));
	auto Arg2 = Arguments->at(1);

	TObject Value = Arg2->GetValue();

	Arg1->GetValuePtr()->AsArray()->Append(Value);
	bResult = true;
}

void BuiltIns::ReadFileInternal(TArguments* Arguments, bool& bResult)
{
	Log("Reading file.");
	auto Arg1 = Arguments->at(0)->GetValue();
	if (Arg1.GetType() != StringType)
	{
		bResult = false;
		return;
	}
	Log(Arg1);
	auto Arg2 = Arguments->at(1)->GetValue();
	if (Arg2.GetType() != StringType)
	{
		bResult = false;
		return;
	}
	Log(Arg2);

	auto FileName = Arg1.GetString();
	auto Content = Core::ReadFile(FileName);
	auto OutString = Arguments->at(1)->GetValuePtr()->AsString();
	*OutString = Content;

	bResult = true;
}

// Initialize the function map of keywords to actual C++ functions
TFunctionMap BuiltIns::InitFunctionMap()
{
	TFunctionMap Map;
	Map["print"] = &PrintInternal;
	Map["append"] = &AppendInternal;
	Map["read_file"] = &ReadFileInternal;
	return Map;
}

