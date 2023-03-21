#include <iostream>
#include "UserFunc.h"
#include "../ServiceFunc.h"
#include "../Constants.h"


using namespace std;


bool SetName(string& name) {	// ��������� ���� � �������� �����
	cout << "�������, ��� (����� " << MIN_NAME_LENGTH << "-" << MAX_NAME_LENGTH << " ��������, ������� ���������): ";
	name = get_string_from_console();
	return (check_name(name, "���", MIN_NAME_LENGTH, MAX_NAME_LENGTH));
}


bool SetLogin(string& login) {	// ��������� ���� � �������� login
	cout << "������� ��� (login) (����� " << MIN_LOGIN_LENGTH << "-" << MAX_LOGIN_LENGTH << " ��������, ������� �� �����������): ";
	login = get_string_from_console();
	return (check_name(login, "login", MIN_LOGIN_LENGTH, MAX_LOGIN_LENGTH) && check_spaces(login, "login"));	// �������� ����������
}


bool SetPassword(string& password) {	// ��������� ���� � �������� ������
	cout << "������: ";
	password = get_value_from_console();
	if (!check_name(password, "������", MIN_PASSWORD_LENGTH, MAX_PASSWORD_LENGTH) || !check_spaces(password, "������")) {
		password = "";
		return false;
	}
	cout << "��������� ������: ";
	string password2 = get_value_from_console();
	if (password != password2) {
		cout << "������ �� ���������!\n";
		password = "";
		return false;
	}

	return true;
}


bool NewUser(string& name, string& login, string& password) {	// �������� �������� ��� ����������� ������ ������������
	cout << "��� ����������� ������� ��������� ������ ������������:\n";
	return (SetName(name) && SetLogin(login) && SetPassword(password));
}


bool SignIn(string& login, string& password) {	// �������� �������� ��� ����� ������������ � ���
	cout << "������� �������� ��� (login): ";
	cin >> login;
	if (!check_empty_name(login, "���"))
		return false;

	cout << "������� ������: ";
	cin >> password;

	return true;
}


bool ChooseUser(string& receiver) {	// �������� ���������� ��� �������� ���������
	cout << "������� �������� ��� (login) ����������: ";
	cin >> receiver;
	return (check_empty_name(receiver, "���"));
}


bool AddMessage(string& text) {	// ��������� ���������
	cout << "������� ����� ���������:";
	text = get_string_from_console();
	return (check_name(text, "���������", 1, MESSAGE_LENGTH));
}