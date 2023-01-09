#pragma once
#include <string>

#ifdef _WIN32
extern "C" __declspec(dllexport) bool parse(std::string & filecontents);
#else
extern "C" bool parse(std::string & filecontents);
#endif