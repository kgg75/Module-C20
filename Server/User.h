#pragma once
#include <string>
#include "../SHA1.h"


struct User {
	int _id;	// уникальный идентификатор
	std::string _name;	// полное имя
	std::string _login;	// учётное имя (login)
	SHA1PwdArray _pass_sha1_hash;	// хэш пароля
	bool _locked = false;	// флаг блокировки пользователя (зарезервирован)
	bool _logged = false;	// флаг, указывающий на вход пользователя

	User(
		const int id,
		const std::string& name,
		const std::string& login,
		const SHA1PwdArray& pass_sha1_hash,
		const bool locked = false,
		const bool logged = false
	);
};

