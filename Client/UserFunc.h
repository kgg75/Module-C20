#pragma once
#include <string>


bool SetName(std::string& name);	// ��������� ���� � �������� �����
bool SetLogin(std::string& login);	// ��������� ���� � �������� login
bool SetPassword(std::string& password);	// ��������� ���� � �������� ������
bool NewUser(std::string& name, std::string& login, std::string& password);	// �������� �������� ��� ����������� ������ ������������
bool SignIn(std::string& login, std::string& password);	// �������� �������� ��� ����� ������������ � ���
bool ChooseUser(std::string& receiver);	// �������� ���������� ��� �������� ���������
bool AddMessage(std::string& text);	// ��������� ���������