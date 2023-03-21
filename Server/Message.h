#pragma once
#include <string>


struct Message {
	int id;	// ���������� ������������� ���������
	int senderId;	// ������������� �����������
	std::string senderLogin;	// login �����������
	int receiverId;	// ������������� ����������
	std::string receiverLogin;	// login ����������
	std::string date_time;	// ����� ��������	//������������ time_t?
	std::string text;	// ����� ���������
	bool wasReaded;	// ���� ���������

	Message(
		const int _id,
		const int _senderId,
		const std::string& _senderLogin,
		const int _receiverId,
		const std::string& _receiverLogin,
		const std::string& _date_time,
		const std::string& _text,
		const bool _wasReaded = false
	);
};

