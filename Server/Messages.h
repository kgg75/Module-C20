#pragma once
#include <vector>
#include "Message.h"
#include "User.h"


class Messages {
	public:
		Messages();
		bool Add(const int senderId, const std::string& senderLogin, const int receiverId, const std::string& receiverLogin, const std::string& text);
		int FindUnReaded(const int receiverId) const;	// ������� ������������� ��������� ��� ���������� ����������; ���������� ����������
		uint32_t GetMessagesQty(const int receiverId, const bool UnReadedOnly) const;	// ������� ���� ��������� ��� ���������� ����������; ���������� ����������
		bool ReadByOne(std::string& output, const uint32_t  receiverId, const uint32_t messageId, const bool showUnReadedOnly);	// ������� ��������� �� ������ ��� ���������� �� id ������������
		bool Open();	// ��������� ��������� �� �������� �����; true � ������ ������
		bool Save();	// ��������� ��������� �� ������� ����; true � ������ ������
		int GetLastMsgId() const;	// ���������� id ���������� ��������� 
		bool IsChanged() const;	// ���������� ���� ������� ��������� � ����������
	private:
		std::vector <Message> message;	// ������ � �����������
		int msgsCount = 0;	// ���������� ���������
		bool isChanged = false;	// ���� ������� ���������
		void CheckArraySize();	// ��������� ������� ���������� ����� � ������� � ����������� ��� ��� �������������
};

