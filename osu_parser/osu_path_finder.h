#pragma once
#include <string>
#include "../utils/string_utils.h"
#include "osu_database.h"

#ifndef OSU_PARSER_OSU_PATH_FINDER_H
#define OSU_PARSER_OSU_PATH_FINDER_H

std::wstring get_osu_path();
std::string get_beatmap_path(const beatmap_info& bmap, std::wstring osu_path);
LONG get_string_reg_key(HKEY h_key, const std::wstring& str_value_name, std::wstring& str_value,
                     const std::wstring& str_default_value);
LONG get_dword_reg_key(HKEY h_key, const std::wstring& str_value_name, DWORD& n_value, DWORD n_default_value);
LONG get_bool_reg_key(HKEY h_key, const std::wstring& str_value_name, bool& b_value, bool b_default_value);

inline std::wstring get_osu_path()
{
	HKEY h_key;
	const auto l_res = RegOpenKeyExW(HKEY_CLASSES_ROOT, L"osu\\shell\\open\\command", KEY_WOW64_32KEY, KEY_READ,
	                                 &h_key);
	const auto b_exists_and_success(l_res == ERROR_SUCCESS);

	if (b_exists_and_success)
	{
		std::wstring str_key_default_value;
		get_string_reg_key(h_key, L"", str_key_default_value, L"bad");

		const auto osu_exe_path = splitString(str_key_default_value, L" ")[0];
		auto osu_path = osu_exe_path.substr(1, osu_exe_path.size() - 10);

		return osu_path;
	}
	else 
		return L"";
}

inline std::string get_beatmap_path(const beatmap_info& bmap, std::wstring osu_path)
{
	const std::string _osu_path(osu_path.begin(), osu_path.end());
	std::string full_beatmap_path;
	if (bmap.foldername.find("Songs\\") != std::string::npos)
	{
		full_beatmap_path += _osu_path + bmap.foldername + "\\" + bmap.filename;
	}
	else
	{
		full_beatmap_path += _osu_path + "Songs\\" + bmap.foldername + "\\" + bmap.filename;
	}
	return full_beatmap_path;
}

inline auto get_dword_reg_key(HKEY h_key, const std::wstring& str_value_name, DWORD& n_value,
                           const DWORD n_default_value) -> LONG
{
	n_value = n_default_value;
	DWORD dw_buffer_size(sizeof(DWORD));
	DWORD n_result(0);
	const auto n_error = RegQueryValueExW(h_key,
	                                     str_value_name.c_str(),
	                                     nullptr,
	                                     nullptr,
	                                     reinterpret_cast<LPBYTE>(&n_result),
	                                     &dw_buffer_size);
	if (ERROR_SUCCESS == n_error)
	{
		n_value = n_result;
	}
	return n_error;
}


inline LONG get_bool_reg_key(HKEY h_key, const std::wstring& str_value_name, bool& b_value, const bool b_default_value)
{
	const DWORD n_def_value((b_default_value) ? 1 : 0);
	auto n_result(n_def_value);
	const auto n_error = get_dword_reg_key(h_key, str_value_name, n_result, n_def_value);
	if (ERROR_SUCCESS == n_error)
	{
		b_value = (n_result != 0) ? true : false;
	}
	return n_error;
}


inline auto get_string_reg_key(HKEY h_key, const std::wstring& str_value_name, std::wstring& str_value,
                            const std::wstring& str_default_value) -> LONG
{
	str_value = str_default_value;
	WCHAR sz_buffer[512];
	DWORD dw_buffer_size = sizeof(sz_buffer);
	const ULONG n_error = RegQueryValueExW(h_key, str_value_name.c_str(), nullptr, nullptr, reinterpret_cast<LPBYTE>(sz_buffer), &dw_buffer_size);
	if (ERROR_SUCCESS == n_error)
	{
		str_value = sz_buffer;
	}
	return n_error;
}

#endif // !1
