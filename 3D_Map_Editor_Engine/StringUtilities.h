#ifndef STRINGUTILITIES_H
#define STRINGUTILITIES_H

#include <string>
#include <codecvt>
#include <locale>
#include <DirectXMath.h>

using convert_t = std::codecvt_utf8<wchar_t>;
static std::wstring_convert<convert_t, wchar_t> strconverter;

static std::string to_string(std::wstring wstr)
{
    return strconverter.to_bytes(wstr);
}

static std::wstring to_wstring(std::string str)
{
    return strconverter.from_bytes(str);
}

static std::string f3ToString(DirectX::XMFLOAT3 f3, std::string delimiter = ", ")
{
    return std::to_string(f3.x) + delimiter + std::to_string(f3.y) + delimiter + std::to_string(f3.z);
}

#endif // !STRINGUTILITIES_H