#pragma once
#include <vector>
#include "Message.h"
#include "User.h"


class Messages {
	public:
		Messages();
		bool Add(const int senderId, const std::string& senderLogin, const int receiverId, const std::string& receiverLogin, const std::string& text);
		int FindUnReaded(const int receiverId) const;	// подсчёт непрочитанных сообщений для указанного получателя; возвращает количество
		uint32_t GetMessagesQty(const int receiverId, const bool UnReadedOnly) const;	// подсчёт всех сообщений для указанного получателя; возвращает количество
		bool ReadByOne(std::string& output, const uint32_t  receiverId, const uint32_t messageId, const bool showUnReadedOnly);	// выводит сообщения по одному для указанного по id пользователя
		bool Open();	// загружает сообщения из внешнего файла; true в случае успеха
		bool Save();	// сохраняет сообщения во внешний файл; true в случае успеха
		int GetLastMsgId() const;	// возвращает id последнего сообщения 
		bool IsChanged() const;	// возвращает флаг наличия изменений в сообщениях
	private:
		std::vector <Message> message;	// массив с сообщениями
		int msgsCount = 0;	// количество сообщений
		bool isChanged = false;	// флаг наличия изменений
		void CheckArraySize();	// проверяет наличие свободного места в векторе и увеличивает его при необходимости
};

