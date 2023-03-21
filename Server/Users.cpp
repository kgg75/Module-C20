#include <iostream>
#include <fstream>
#include <exception>
#include <nlohmann/json.hpp>
#include "Users.h"
#include "../ServiceFunc.h"
#include "Converter.h"
#include "../Constants.h"


using json = nlohmann::json;
using namespace std;


//private:
void Users::CheckArraySize() {	// проверка наличия свободного места в векторе и его увеличение при необходимости
	if (user.capacity() <= usersCount)
		user.reserve(user.capacity() * 2);	//	при заполнении вектора каждый раз увеличиваем его ёмкость вдвое
}


uint32_t Users::GetIndex(const uint32_t id) const {	// возвращает порядковый номер пользователя по его id
	for (auto i = 1; i <= usersCount; i++)
		if (user[i]._id == id)
			return i;
	return 0;	// ошибка!
}


int32_t Users::FindLogin(const string& login) const {	// проверяет соответствие введённого login существующим; возвращает порядковый номер или код ошибки
	for (auto i = 1; i <= usersCount; i++)
		if (login == user[i]._login)
			return i;

	return ERR_USER_NOT_FOUND;
}


//public:
Users::Users() {
	user.reserve(INITIAL_COUNT);
	user.emplace_back(0, SERVICE_NAME, SERVICE_LOGIN, shaPwdArray);	//	передаём в конструктор пустой shaPwdArray
}


int32_t Users::NewUser(const string& name, const string& login, const SHA1PwdArray& pass_sha1_hash) {	// регистрирует нового пользователя; возвращает его id или код ошибки
	CheckArraySize();

	if (FindLogin(login) > 0)
		return ERR_ALREADY_EXIST;

	if (lowercase_s(login) == SERVICE_LOGIN)	// попытка регистрации под служебным login
		return ERR_TRY_USE_SERVICE_NAME;

	idsCount++;
	usersCount++;
	user.emplace_back(idsCount, name, login, shaPwdArray, false, true);
	isChanged = true;

	return idsCount;
}


int32_t Users::Login(const string& login, const SHA1PwdArray& pass_sha1_hash) {	// выполняет вход пользователя в чат; возвращает его id или код ошибки
	auto findedUser = FindLogin(login);
	if (findedUser == ERR_USER_NOT_FOUND)
		return ERR_USER_NOT_FOUND;

	if (user[findedUser]._logged)
		return ERR_ALREADY_LOGGED;

	if (user[findedUser]._pass_sha1_hash == pass_sha1_hash) {
		user[findedUser]._logged = true;
		return user[findedUser]._id;
	}

	return ERR_WRONG_PASSWORD;
}


void Users::Logout(const uint32_t id) {	// выполняет выход пользователя из чата
	user[GetIndex(id)]._logged = false;
}


void Users::DeleteUser(const uint32_t id) {	// удаляет учётную запись пользователя
	user.erase(user.begin() + GetIndex(id));
	usersCount--;
	isChanged = true;
}


int32_t Users::ChangeRegData(const uint32_t id, const char* name, const char* login, const SHA1PwdArray* shaPwdArray, const uint8_t regDataType) {	// изменение учётной записи пользователя
	auto index = GetIndex(id);
	switch (regDataType) {
		case SLCMD_CHANGE_NAME:
			user[index]._name = name;
			break;
		case SLCMD_CHANGE_LOGIN:
			if (lowercase_s(login) == SERVICE_LOGIN)	// попытка регистрации под служебным login
				return ERR_TRY_USE_SERVICE_NAME;

			if (FindLogin(login) > 0)
				return ERR_ALREADY_EXIST;

			user[index]._login = login;
			break;
		case SLCMD_CHANGE_PASSWORD:
			user[index]._pass_sha1_hash = (SHA1PwdArray&)*shaPwdArray;	// копируем хэш нового пароля
			break;
	}
	isChanged = true;
	return 1;
}


bool Users::Open() {	// загружает данные пользователей из внешнего файла
	fstream users_data;
	users_data.open(USER_DATA_FILE_NAME, ios::in);
	if (!users_data.is_open()) {
		cout << "Неизвестная ошибка при загрузке данных пользователей!\n";
		return false;
	}
	bool result = false;
	json jsonObj;
	CheckArraySize();
	try {	// попытка создания json-объекта и чтения из него
		users_data >> jsonObj;
		for (auto i = 0; i < jsonObj.size(); i++) {
			for (auto j = 0; j < SHA1_HASH_LENGTH_UINTS; j++)
				shaPwdArray[j] = jsonObj[i]["sha1"][j];	// загрузка хэша пароля в переменную shaPwdArray
			user.emplace_back(	// помещение учётных данных в vector
				jsonObj[i]["id"],
				utf8_to_cp1251(jsonObj[i]["name"]),	// перекодирование строк из UTF-8
				utf8_to_cp1251(jsonObj[i]["login"]),	// перекодирование строк из UTF-8
				shaPwdArray,
				jsonObj[i]["locked"],
				false
			);
		}
		usersCount = (int)jsonObj.size();
		idsCount = user[usersCount]._id;
		result = true;
	}
	catch (exception& e) {
		cout << e.what() << '\n';
		cout << "Ошибка чтения файла данных!\n";	// ошибка будет при первом запуске чата, т.к. данные ещё не ни разу сохранялись
	}
	users_data.close();
	return result;
}


bool Users::Save() {	// сохраняет данные пользователей во внешний файл
	fstream users_data;
	users_data.open(USER_DATA_FILE_NAME, ios::out);
	if (!users_data.is_open()) {
		cout << "Неизвестная ошибка при сохранении данных пользователей!\n";
		return false;
	}
	bool result = false;
	json jsonObj;
	try {	// попытка создания json-объекта и записи в него
		for (auto i = 0; i < usersCount; i++) {
			jsonObj[i]["id"] = user[i + 1]._id;
			jsonObj[i]["name"] = cp1251_to_utf8(user[i + 1]._name);	// перекодирование строк в UTF-8
			jsonObj[i]["login"] = cp1251_to_utf8(user[i + 1]._login);	// перекодирование строк в UTF-8
			jsonObj[i]["sha1"] = user[i + 1]._pass_sha1_hash.data;	// jsonObj работает только с публичными data
			jsonObj[i]["locked"] = user[i + 1]._locked;
		}
		users_data << setw(4) << jsonObj;
		result = true;
	}
	catch (exception& e) {
		cout << e.what() << '\n';
		cout << "Ошибка записи данных в файл!\n";
	}
	users_data.close();
	return result;
}


size_t Users::Info(const uint32_t id, string& output) const {	// отображает данных указанного пользователя, возвращает длину строки output
	auto index = GetIndex(id);
	output = "            id = " + to_string(user[index]._id) +
		"\n           имя = " + user[index]._name +
		"\n         login = " + user[index]._login +
		"\n  заблокирован = " + BoolToStrR(user[index]._locked) +
		"\n     подключен = " + BoolToStrR(user[index]._logged) + '\n';
	return output.length();
}


size_t Users::ListByOne(const size_t user_index, string& output) const {	// выводит краткие сведения по указанному пользователю; возвращает длину строки output
	if (user_index < usersCount)
		output = "    " + to_string(user_index) + ")\t" + user[user_index]._login + " (" + user[user_index]._name + ")\n";
	else
		output = "";

	return output.length();
}


int Users::GetUsersQty() const  {
	return usersCount;
}


int Users::GetCurrentUser() const {	// порядковый номер текущего пользователя
	return currentUser;
}


int Users::GetUserId(const std::string& login) const {	// возвращает id указанного пользователя
	auto index = FindLogin(login);
	return (index > 0 ? user[index]._id : 0);	// 0 - нет такого пользователя
}


string Users::GetUserLogin(const uint32_t id) const {	// возвращает login пользователя, указанного по id
	for (auto i = 1; i < usersCount; i++)
		if (user[i]._id == id)
			return user[i]._login;
	return "";
}


bool Users::IsLogged(const string& login) const {	// проверяет, подключен ли пользователь с указанным login
	auto index = FindLogin(login);
	return (index > 0 ? user[index]._logged : false);
}


bool Users::IsChanged() const {	// возвращает флаг наличия изменений в данных пользователей
	return isChanged;
}