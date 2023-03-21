#pragma once
#include <string>
#include "../SHA1.h"


struct User {
	int _id;	// ���������� �������������
	std::string _name;	// ������ ���
	std::string _login;	// ������� ��� (login)
	SHA1PwdArray _pass_sha1_hash;	// ��� ������
	bool _locked = false;	// ���� ���������� ������������ (��������������)
	bool _logged = false;	// ����, ����������� �� ���� ������������

	User(
		const int id,
		const std::string& name,
		const std::string& login,
		const SHA1PwdArray& pass_sha1_hash,
		const bool locked = false,
		const bool logged = false
	);
};

