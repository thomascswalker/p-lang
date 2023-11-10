#include <fstream>

#include "builtins.h"
#include "core.h"

using namespace BuiltIns;

void BuiltIns::PrintInternal(TSignature* Signature, bool& bResult)
{
	std::vector<TObject> Objects;
	for (auto Arg : *Signature)
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

void BuiltIns::AppendInternal(TSignature* Signature, bool& bResult)
{
	auto Arg1 = Cast<TVariable>(Signature->at(0));
	auto Arg2 = Signature->at(1);

	TObject Value = Arg2->GetValue();

	Arg1->GetValuePtr()->AsArray()->Append(Value);
	bResult = true;
}

void BuiltIns::ReadFileInternal(TSignature* Signature, bool& bResult)
{
	auto Arg1 = Signature->at(0)->GetValue();
	if (Arg1.GetType() != StringType)
	{
		bResult = false;
		Logging::Error("Wanted a string as the first argument.");
		return;
	}

	auto Arg2 = Signature->at(1)->GetValue();
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

	auto Content = Core::ReadFile(FileName);
	auto OutString = Signature->at(1)->GetValuePtr()->AsString();
	*OutString = Content;

	bResult = true;
}

// Initialize the function map of keywords to actual C++ functions
TFunctionMap BuiltIns::InitFunctionMap()
{
	TFunctionMap Map;

	// IO
	Map["print"] = &PrintInternal;
	Map["read_file"] = &ReadFileInternal;

	// Strings
	Map["append"] = &AppendInternal;

	return Map;
}
