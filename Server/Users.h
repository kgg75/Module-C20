#pragma once
#include <string>
#include <vector>
#include "User.h"


class Users {	// для типов bool возвращаемое значение true в случае успеха, false при неудачном заваершении
	public:
		Users();
		int32_t NewUser(const std::string& name, const std::string& login, const SHA1PwdArray& pass_sha1_hash);	// регистрирует нового пользователя; возвращает его id или код ошибки
		int32_t Login(const std::string& login, const SHA1PwdArray& pass_sha1_hash);	// выполняет вход пользователя в чат; возвращает его id или код ошибки
		void Logout(const uint32_t id);	// выполняет выход пользователя из чата
		bool Open();	// загружает данные пользователей из внешнего файла
		bool Save();	// сохраняет данные пользователей во внешний файл
		void DeleteUser(const uint32_t id);	// удаляет учётную запись пользователя
		int32_t ChangeRegData(const uint32_t id, const char* name, const char* login, const SHA1PwdArray* shaPwdArray, const uint8_t regDataType);	// изменяет учётной записи пользователя; возвращает код ошибки
		size_t Info(const uint32_t id, std::string& output) const;	// отображает данных указанного пользователя, возвращает длину строки output
		size_t ListByOne(const size_t user_index, std::string& output) const;	// выводит краткие сведения по указанному пользователю; возвращает длину строки output
// getters
		int GetUsersQty() const;	// возвращает количество пользователей
		int GetCurrentUser() const;	// возвращает порядковый номер текущего пользователя
		//int GetUserId(const int user_index = -1) const;	// id указанного пользователя; по умолчанию - текущего
		int GetUserId(const std::string& login) const;	// возвращает id указанного пользователя
		std::string GetUserLogin(const uint32_t id) const;	// возвращает login пользователя, указанного по id
		bool IsLogged(const std::string& login) const;	// проверяет, подключен ли пользователь с указанным login
		bool IsChanged() const;	// возвращает флаг наличия изменений в данных пользователей
	private:
		std::vector<User> user;	// вектор с данными пользователей
		int usersCount = 0;	// количество пользователей
		int idsCount = 0; // количество использованных идентификаторов
		int currentUser = idsCount; // порядковый номер текущего пользователя (начинается с 1)
		bool isChanged = false;	// флаг наличия изменений в данных пользователей
		SHA1PwdArray shaPwdArray;	// временное хранилище хэша пароля
		// функции
		void CheckArraySize();	// проверяет наличия свободного места в векторе и его увеличение при необходимости
		uint32_t GetIndex(const uint32_t id) const;	// возвращает порядковый номер пользователя по его id
		int32_t FindLogin(const std::string& login) const;	// проверяет соответствие введённого login существующим; возвращает порядковый номер или код ошибки
};

