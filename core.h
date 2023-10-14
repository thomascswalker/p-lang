#pragma once

#include <string>
#include <iostream>

template <class Out, class In>
inline Out* Cast(In InValue)
{
	return dynamic_cast<Out*>(InValue);
}

inline std::string ReadFile(std::string FileName)
{
	std::ifstream Stream(FileName.c_str());
	return std::string(std::istreambuf_iterator<char>(Stream), std::istreambuf_iterator<char>());
};