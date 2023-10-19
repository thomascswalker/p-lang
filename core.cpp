#include <string>
#include <fstream>

#include "core.h"

void Core::Log(const std::string& InMsg)
{
	printf(InMsg.c_str());
}

void Core::Debug(const std::string& InMsg)
{
#if _DEBUG
	Log(InMsg + "\n");
#endif
}

std::string Core::ReadFile(const std::string& FileName)
{
	std::ifstream Stream(FileName.c_str());
	return std::string(std::istreambuf_iterator<char>(Stream), std::istreambuf_iterator<char>());
};