#pragma once
#include <string>


struct Message {
	int id;	// уникальный идентификатор сообщения
	int senderId;	// идентификатор отправителя
	std::string senderLogin;	// login отправителя
	int receiverId;	// идентификатор получателя
	std::string receiverLogin;	// login получателя
	std::string date_time;	// время создания	//использовать time_t?
	std::string text;	// текст сообщения
	bool wasReaded;	// флаг прочтения

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

