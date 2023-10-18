#include <string>
#include <fstream>

#include "core.h"

std::string ReadFile(const std::string& FileName)
{
	std::ifstream Stream(FileName.c_str());
	return std::string(std::istreambuf_iterator<char>(Stream), std::istreambuf_iterator<char>());
};