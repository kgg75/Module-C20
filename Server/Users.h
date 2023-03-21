#pragma once
#include <string>
#include <vector>
#include "User.h"


class Users {	// ��� ����� bool ������������ �������� true � ������ ������, false ��� ��������� �����������
	public:
		Users();
		int32_t NewUser(const std::string& name, const std::string& login, const SHA1PwdArray& pass_sha1_hash);	// ������������ ������ ������������; ���������� ��� id ��� ��� ������
		int32_t Login(const std::string& login, const SHA1PwdArray& pass_sha1_hash);	// ��������� ���� ������������ � ���; ���������� ��� id ��� ��� ������
		void Logout(const uint32_t id);	// ��������� ����� ������������ �� ����
		bool Open();	// ��������� ������ ������������� �� �������� �����
		bool Save();	// ��������� ������ ������������� �� ������� ����
		void DeleteUser(const uint32_t id);	// ������� ������� ������ ������������
		int32_t ChangeRegData(const uint32_t id, const char* name, const char* login, const SHA1PwdArray* shaPwdArray, const uint8_t regDataType);	// �������� ������� ������ ������������; ���������� ��� ������
		size_t Info(const uint32_t id, std::string& output) const;	// ���������� ������ ���������� ������������, ���������� ����� ������ output
		size_t ListByOne(const size_t user_index, std::string& output) const;	// ������� ������� �������� �� ���������� ������������; ���������� ����� ������ output
// getters
		int GetUsersQty() const;	// ���������� ���������� �������������
		int GetCurrentUser() const;	// ���������� ���������� ����� �������� ������������
		//int GetUserId(const int user_index = -1) const;	// id ���������� ������������; �� ��������� - ��������
		int GetUserId(const std::string& login) const;	// ���������� id ���������� ������������
		std::string GetUserLogin(const uint32_t id) const;	// ���������� login ������������, ���������� �� id
		bool IsLogged(const std::string& login) const;	// ���������, ��������� �� ������������ � ��������� login
		bool IsChanged() const;	// ���������� ���� ������� ��������� � ������ �������������
	private:
		std::vector<User> user;	// ������ � ������� �������������
		int usersCount = 0;	// ���������� �������������
		int idsCount = 0; // ���������� �������������� ���������������
		int currentUser = idsCount; // ���������� ����� �������� ������������ (���������� � 1)
		bool isChanged = false;	// ���� ������� ��������� � ������ �������������
		SHA1PwdArray shaPwdArray;	// ��������� ��������� ���� ������
		// �������
		void CheckArraySize();	// ��������� ������� ���������� ����� � ������� � ��� ���������� ��� �������������
		uint32_t GetIndex(const uint32_t id) const;	// ���������� ���������� ����� ������������ �� ��� id
		int32_t FindLogin(const std::string& login) const;	// ��������� ������������ ��������� login ������������; ���������� ���������� ����� ��� ��� ������
};

