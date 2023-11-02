#include <string>
#include <fstream>
#include <format>

#include "core.h"

void Core::Log(const std::string& InMsg)
{
	auto Fmt = std::format("[{}] ", __TIMESTAMP__) + InMsg + "\n";
	printf(Fmt.c_str());
}

void Core::Debug(const std::string& InMsg)
{
#if _DEBUG
	Log(InMsg);
#endif
}

void Core::Warning(const std::string& InMsg)
{
	Log(std::format("\x1B[33m{}\033[37m", InMsg));
}

void Core::Error(const std::string& InMsg)
{
	Log(std::format("\x1B[31m{}\033[37m", InMsg));
}

void Core::Success(const std::string& InMsg)
{
	Log(std::format("\x1B[32m{}\033[37m", InMsg));
}

std::string Core::ReadFile(const std::string& FileName)
{
	std::ifstream Stream(FileName.c_str());
	return std::string(std::istreambuf_iterator<char>(Stream), std::istreambuf_iterator<char>());
};