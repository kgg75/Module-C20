#pragma once
#include <iostream>
#include <string>
#include "Constants.h"


uint8_t get_command_code(const CommandSpace* commandSpace, std::string& command);	// ��������� ������ ������� � ���������� �������������� ��� ��������
uint8_t get_command(const CommandSpace* commandSpace, const std::string msg, const std::string& user_name = "");	// ��������� ��������� ������� � �������, ���������� �������������� ��� ��������
void lowercase(std::string&);	// ����������� ������ � ������ �������
std::string lowercase_s(const std::string& str);	// ����������� ������ � ������ �������
bool check_empty_name(const std::string& str, const std::string& subject);	// ��������� ������ �� �������
bool check_spaces(const std::string& str, const std::string& subject);	// ��������� ������ �� �������
bool check_name(const std::string& str, const std::string& subject, const size_t minLength, const size_t maxLength);	// ��������� ������ �� �������, �����������/������������ �����
std::string get_value_from_console();	// �������� �������� � �������
std::string get_string_from_console();	// �������� ������ � ������� � ������� � �� ��������� � �������� ��������
std::string GetTime();	// ���������� ������� �����
bool QuestionYN(const std::string& message);	// ����� ������ � ����� ���������� ������ y/n � ���������� �����
std::string BoolToStrR(bool value);
std::string BoolToStrE(bool value);