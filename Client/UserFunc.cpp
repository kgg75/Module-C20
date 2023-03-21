#include <iostream>
#include "UserFunc.h"
#include "../ServiceFunc.h"
#include "../Constants.h"


using namespace std;


bool SetName(string& name) {	// выполняет ввод и проверку имени
	cout << "Фамилия, имя (длина " << MIN_NAME_LENGTH << "-" << MAX_NAME_LENGTH << " символов, пробелы разрешены): ";
	name = get_string_from_console();
	return (check_name(name, "имя", MIN_NAME_LENGTH, MAX_NAME_LENGTH));
}


bool SetLogin(string& login) {	// выполняет ввод и проверку login
	cout << "Учётное имя (login) (длина " << MIN_LOGIN_LENGTH << "-" << MAX_LOGIN_LENGTH << " символов, пробелы не допускаются): ";
	login = get_string_from_console();
	return (check_name(login, "login", MIN_LOGIN_LENGTH, MAX_LOGIN_LENGTH) && check_spaces(login, "login"));	// проверка синтаксиса
}


bool SetPassword(string& password) {	// выполняет ввод и проверку пароля
	cout << "Пароль: ";
	password = get_value_from_console();
	if (!check_name(password, "пароль", MIN_PASSWORD_LENGTH, MAX_PASSWORD_LENGTH) || !check_spaces(password, "пароль")) {
		password = "";
		return false;
	}
	cout << "Повторите пароль: ";
	string password2 = get_value_from_console();
	if (password != password2) {
		cout << "Пароли не совпадают!\n";
		password = "";
		return false;
	}

	return true;
}


bool NewUser(string& name, string& login, string& password) {	// собирает сведения для регистрации нового пользователя
	cout << "Для регистрации введите следующие данные пользователя:\n";
	return (SetName(name) && SetLogin(login) && SetPassword(password));
}


bool SignIn(string& login, string& password) {	// собирает сведения для входа пользователя в чат
	cout << "Укажите условное имя (login): ";
	cin >> login;
	if (!check_empty_name(login, "имя"))
		return false;

	cout << "Укажите пароль: ";
	cin >> password;

	return true;
}


bool ChooseUser(string& receiver) {	// выбирает получателя для отправки сообщения
	cout << "Укажите условное имя (login) получателя: ";
	cin >> receiver;
	return (check_empty_name(receiver, "имя"));
}


bool AddMessage(string& text) {	// добавляет сообщение
	cout << "Введите текст сообщения:";
	text = get_string_from_console();
	return (check_name(text, "сообщение", 1, MESSAGE_LENGTH));
}