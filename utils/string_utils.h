#pragma once
#ifndef utils_string_utils_h
#define utils_string_utils_h

#include <string>
#include <vector>

bool endsWith(const std::string& str, const char* suffix, unsigned suffixLen);
bool endsWith(const std::string& str, const char* suffix);
bool startsWith(const std::string& str, const char* prefix, unsigned prefixLen);
bool startsWith(const std::string& str, const char* prefix);
std::vector<std::string> splitString(const std::string& str, const std::string& delim);
std::vector<std::wstring> splitString(const std::wstring& str, const std::wstring& delim);

#endif // utils_string_utils_h