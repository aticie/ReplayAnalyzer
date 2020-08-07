#include "string_utils.h"

bool endsWith(const std::string& str, const char* suffix, unsigned suffixLen)
{
	return str.size() >= suffixLen && 0 == str.compare(str.size() - suffixLen, suffixLen, suffix, suffixLen);
}

bool endsWith(const std::string& str, const char* suffix)
{
	return endsWith(str, suffix, std::string::traits_type::length(suffix));
}

bool startsWith(const std::string& str, const char* prefix, unsigned prefixLen)
{
	return str.size() >= prefixLen && 0 == str.compare(0, prefixLen, prefix, prefixLen);
}

bool startsWith(const std::string& str, const char* prefix)
{
	return startsWith(str, prefix, std::string::traits_type::length(prefix));
}

std::vector<std::string> splitString(const std::string& str, const std::string& delim)
{
	std::vector<std::string> tokens;
	size_t prev = 0, pos;
	do
	{
		pos = str.find(delim, prev);
		if (pos == std::string::npos)
			pos = str.length();
		std::string token = str.substr(prev, pos - prev);
		if (!token.empty())
			tokens.push_back(token);
		prev = pos + delim.length();
	}
	while (pos < str.length() && prev < str.length());
	return tokens;
}

std::vector<std::wstring> splitString(const std::wstring& str, const std::wstring& delim)
{
	std::vector<std::wstring> tokens;
	size_t prev = 0, pos;
	do
	{
		pos = str.find(delim, prev);
		if (pos == std::string::npos)
			pos = str.length();
		std::wstring token = str.substr(prev, pos - prev);
		if (!token.empty())
			tokens.push_back(token);
		prev = pos + delim.length();
	}
	while (pos < str.length() && prev < str.length());
	return tokens;
}
