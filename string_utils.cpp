#include "pch.h"
#include "string_utils.h"

// Deprecation error
#pragma warning(disable : 4996) // Deprecation error
std::string wstr_to_str(const wchar_t* wstr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.to_bytes(wstr);
}
