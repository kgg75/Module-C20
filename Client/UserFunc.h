#pragma once
#include <string>


bool SetName(std::string& name);	// выполняет ввод и проверку имени
bool SetLogin(std::string& login);	// выполняет ввод и проверку login
bool SetPassword(std::string& password);	// выполняет ввод и проверку пароля
bool NewUser(std::string& name, std::string& login, std::string& password);	// собирает сведения для регистрации нового пользователя
bool SignIn(std::string& login, std::string& password);	// собирает сведения для входа пользователя в чат
bool ChooseUser(std::string& receiver);	// выбирает получателя для отправки сообщения
bool AddMessage(std::string& text);	// добавляет сообщение