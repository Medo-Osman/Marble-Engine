#ifndef STRINGUTILITIES_H
#define STRINGUTILITIES_H

#include <string>
#include <stdlib.h>
#include <DirectXMath.h>
#include <filesystem>

//using convert_t = std::codecvt_utf8<wchar_t>;
//static std::wstring_convert<convert_t, wchar_t> strconverter;
//
//static std::string to_string(std::wstring wstr)
//{
//    return strconverter.to_bytes(wstr);
//}
//
//static std::wstring to_wstring(std::string str)
//{
//    return strconverter.from_bytes(str);
//}

static std::wstring charToWchar(std::string oldString)
{
	if (oldString.size() == 0)
		return L"";

	size_t origsize = strlen(oldString.c_str()) + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t wcstring[newsize];
	mbstowcs_s(&convertedChars, wcstring, origsize, oldString.c_str(), _TRUNCATE);
	return wcstring;
}

static std::wstring extractFileName(std::wstring oldPath)
{
	std::wstring fileName;

	std::filesystem::path p(oldPath);

	fileName = p.filename();

	return fileName;
}

static std::string f3ToString(DirectX::XMFLOAT3 f3, std::string delimiter = ", ")
{
    return std::to_string(f3.x) + delimiter + std::to_string(f3.y) + delimiter + std::to_string(f3.z);
}

#endif // !STRINGUTILITIES_H