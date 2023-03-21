#pragma once
#include <iostream>
#include <string>
#include "Constants.h"


uint8_t get_command_code(const CommandSpace* commandSpace, std::string& command);	// выполняет анализ команды и возвращает соотвествующий код действия
uint8_t get_command(const CommandSpace* commandSpace, const std::string msg, const std::string& user_name = "");	// выполняет получение команды с консоли, возвращает соотвествующий код действия
void lowercase(std::string&);	// преобразует строку в нижний регистр
std::string lowercase_s(const std::string& str);	// преобразует строку в нижний регистр
bool check_empty_name(const std::string& str, const std::string& subject);	// проверяет строку на пустоту
bool check_spaces(const std::string& str, const std::string& subject);	// проверяет строку на пробелы
bool check_name(const std::string& str, const std::string& subject, const size_t minLength, const size_t maxLength);	// проверяет строку на пустоту, минимальную/максимальную длину
std::string get_value_from_console();	// получает значение с консоли
std::string get_string_from_console();	// получает строку с консоли и очищает её от начальных и конечных пробелов
std::string GetTime();	// возвращает текущее время
bool QuestionYN(const std::string& message);	// задаёт вопрос с двумя вариантами ответа y/n и возвращает ответ
std::string BoolToStrR(bool value);
std::string BoolToStrE(bool value);