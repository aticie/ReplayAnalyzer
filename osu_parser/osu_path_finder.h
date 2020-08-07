#pragma once
#include <wtypes.h>
#include <string>
#include "../utils/string_utils.h"
#include "osu_database.h"

#ifndef osu_parser_osu_path_finder_h
#define osu_parser_osu_path_finder_h

std::wstring GetOsuPath(void);
std::string get_beatmap_path(BeatmapInfo bmap, std::wstring osu_path);
LONG GetStringRegKey(HKEY hKey, const std::wstring& strValueName, std::wstring& strValue, const std::wstring& strDefaultValue);
LONG GetDWORDRegKey(HKEY hKey, const std::wstring& strValueName, DWORD& nValue, DWORD nDefaultValue);
LONG GetBoolRegKey(HKEY hKey, const std::wstring& strValueName, bool& bValue, bool bDefaultValue);

std::wstring GetOsuPath(void) {
    HKEY hKey;
    LONG lRes = RegOpenKeyExW(HKEY_CLASSES_ROOT, L"osu\\shell\\open\\command", KEY_WOW64_32KEY, KEY_READ, &hKey);
    bool bExistsAndSuccess(lRes == ERROR_SUCCESS);
    bool bDoesNotExistsSpecifically(lRes == ERROR_FILE_NOT_FOUND);
    std::wstring strKeyDefaultValue;
    GetStringRegKey(hKey, L"", strKeyDefaultValue, L"bad");

    std::wstring osu_exe_path = splitString(strKeyDefaultValue, L" ")[0];
    std::wstring osu_path = osu_exe_path.substr(1, osu_exe_path.size() - 10);

    return osu_path;
}

std::string get_beatmap_path(BeatmapInfo bmap, std::wstring osu_path) {
    std::string _osu_path(osu_path.begin(), osu_path.end());
    std::string full_beatmap_path;
    if (bmap.foldername.find("Songs\\") != std::string::npos) {
        full_beatmap_path += _osu_path + bmap.foldername + "\\" + bmap.filename;
    }
    else {
        full_beatmap_path += _osu_path + "Songs\\" + bmap.foldername + "\\" + bmap.filename;
    }
    return full_beatmap_path;
}

LONG GetDWORDRegKey(HKEY hKey, const std::wstring& strValueName, DWORD& nValue, DWORD nDefaultValue)
{
    nValue = nDefaultValue;
    DWORD dwBufferSize(sizeof(DWORD));
    DWORD nResult(0);
    LONG nError = ::RegQueryValueExW(hKey,
        strValueName.c_str(),
        0,
        NULL,
        reinterpret_cast<LPBYTE>(&nResult),
        &dwBufferSize);
    if (ERROR_SUCCESS == nError)
    {
        nValue = nResult;
    }
    return nError;
}


LONG GetBoolRegKey(HKEY hKey, const std::wstring& strValueName, bool& bValue, bool bDefaultValue)
{
    DWORD nDefValue((bDefaultValue) ? 1 : 0);
    DWORD nResult(nDefValue);
    LONG nError = GetDWORDRegKey(hKey, strValueName.c_str(), nResult, nDefValue);
    if (ERROR_SUCCESS == nError)
    {
        bValue = (nResult != 0) ? true : false;
    }
    return nError;
}


LONG GetStringRegKey(HKEY hKey, const std::wstring& strValueName, std::wstring& strValue, const std::wstring& strDefaultValue)
{
    strValue = strDefaultValue;
    WCHAR szBuffer[512];
    DWORD dwBufferSize = sizeof(szBuffer);
    ULONG nError;
    nError = RegQueryValueExW(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
    if (ERROR_SUCCESS == nError)
    {
        strValue = szBuffer;
    }
    return nError;
}

#endif // !1


