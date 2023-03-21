#include "Converter.h"

// конвертация строк UTF-8 <-> 1251
std::string cp_convert(const std::string& _str, const int& _sourceCP, const int& _destCP) { // этот код не мой!
    std::string result;
    WCHAR* ures = nullptr;
    char* cres = nullptr;

    int result_u = MultiByteToWideChar(_sourceCP, 0, &_str[0], -1, 0, 0);
    if (result_u != 0) {
        ures = new WCHAR[result_u];
        if (MultiByteToWideChar(_sourceCP, 0, &_str[0], -1, ures, result_u)) {
            int result_c = WideCharToMultiByte(_destCP, 0, ures, -1, 0, 0, 0, 0);
            if (result_c != 0) {
                cres = new char[result_c];
                if (WideCharToMultiByte(_destCP, 0, ures, -1, cres, result_c, 0, 0))
                    result = cres;
                delete[] cres;
            }
        }
        delete[] ures;
    }
    return result;
}


std::string utf8_to_cp1251(const std::string& _str) {
    return cp_convert(_str, CP_UTF8, 1251);
}

std::string cp1251_to_utf8(const std::string& _str) {
    return cp_convert(_str, 1251, CP_UTF8);
}