#include "Message.h"
#include <string>


Message::Message(const int _id,
	const int _senderId,
	const std::string& _senderLogin,
	const int _receiverId,
	const std::string& _receiverLogin,
	const std::string& _date_time,
	const std::string& _text,
	const bool _wasReaded)
	:
	id(_id),
	senderId(_senderId),
	senderLogin(_senderLogin),
	receiverId(_receiverId),
	receiverLogin(_receiverLogin),
	date_time(_date_time),
	text(_text),
	wasReaded(_wasReaded)
{}
