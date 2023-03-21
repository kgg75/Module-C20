#pragma once
#include <Windows.h>
#include <string>

// конвертация строк UTF-8 <-> 1251
std::string cp_convert(const std::string& _str, const int& _sourceCP, const int& _destCP);
std::string utf8_to_cp1251(const std::string& _str);
std::string cp1251_to_utf8(const std::string& _str);