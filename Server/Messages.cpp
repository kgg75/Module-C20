#include <iostream>
#include <fstream>
#include <exception>
#include <../../json-develop/include/nlohmann/json.hpp>
#include <string>
#include "Messages.h"
#include "Message.h"
#include "../ServiceFunc.h"
#include "Converter.h"
#include "../Constants.h"


using json = nlohmann::json;
using namespace std;


//private:
void Messages::CheckArraySize() {	// проверяет наличие свободного места в векторе и увеличивает его при необходимости
	if (message.capacity() <= msgsCount)
		message.reserve(message.capacity() * 2);	//	при заполнении вектора каждый раз увеличиваем его ёмкость вдвое
}


//public:
Messages::Messages() {
	message.reserve(INITIAL_MSG_COUNT);
}


bool Messages::Add(const int senderId, const string& senderLogin, const int receiverId, const string& receiverLogin, const string& text) {	// добавление сообщения
	CheckArraySize();

	message.emplace_back(msgsCount, senderId, senderLogin, receiverId, receiverLogin, GetTime(), text);	// вставка сообщения в конец вектора
	msgsCount++;
	isChanged = true;	// установка флага наличия изменений в массиве сообщений
	return true;
}


int Messages::FindUnReaded(const int receiverId) const {	// подсчёт непрочитанных сообщений для указанного получателя
	int count = 0;
	for (auto i = 0; i < msgsCount; i++)
		if ((message[i].receiverId == receiverId) && (!message[i].wasReaded))
			count++;
	return count;
}


uint32_t Messages::GetMessagesQty(const int receiverId, const bool UnReadedOnly) const {	// подсчёт непрочитанных сообщений для указанного получателя
	uint32_t count = 0;
	for (auto i = 0; i < msgsCount; i++)
		if ((message[i].receiverId == receiverId) && (!UnReadedOnly || !message[i].wasReaded))
			count++;
	return count;
}


bool Messages::ReadByOne(string& output, const uint32_t receiverId, const uint32_t messageId, const bool showUnReadedOnly) {	// выводит сообщения по одному для указанного по id пользователя
	if (messageId < msgsCount && message[messageId].receiverId == receiverId && (!showUnReadedOnly || !message[messageId].wasReaded)) {
		output = "#" + to_string(message[messageId].id) + " " + message[messageId].date_time + " от '" + message[messageId].senderLogin + "': " + message[messageId].text + '\n';
		message[messageId].wasReaded = true;	// пометка прочитанным
		isChanged = true;	// установка флага наличия изменений в массиве сообщений
		return true;
	}
	else
		return false;
}


bool Messages::Open() {	// загружает сообщения из внешнего файла; true в случае успеха
	fstream msgs_data;
	msgs_data.open(MSG_DATA_FILE_NAME, ios::in);
	if (!msgs_data.is_open()) {
		cout << "Неизвестная ошибка при загрузке сообщений!\n";
		return false;
	}
	bool result = false;
	json jsonObj;
	CheckArraySize();
	try {	// попытка создания json-объекта и чтения из него
		msgs_data >> jsonObj;
		for (auto i = 0; i < jsonObj.size(); i++)
			message.emplace_back(
				jsonObj[i]["id"],
				jsonObj[i]["senderId"],
				utf8_to_cp1251(jsonObj[i]["senderLogin"]),	// перекодирование строк из UTF-8
				jsonObj[i]["receiverId"],
				utf8_to_cp1251(jsonObj[i]["receiverLogin"]),	// перекодирование строк из UTF-8
				utf8_to_cp1251(jsonObj[i]["date_time"]),	// перекодирование строк из UTF-8
				utf8_to_cp1251(jsonObj[i]["text"]),	// перекодирование строк из UTF-8
				jsonObj[i]["wasReaded"]
			);
		msgsCount = (int)jsonObj.size();
		result = true;
	}
	catch (exception& e) {
		cout << e.what() << '\n';
	}
	msgs_data.close();
	return result;
}


bool Messages::Save() {	// сохраняет сообщения во внешний файл; true в случае успеха
	fstream msgs_data;
	msgs_data.open(MSG_DATA_FILE_NAME, ios::out);
	if (!msgs_data.is_open()) {
		cout << "Неизвестная ошибка при сохранении сообщений!\n";
		return false;
	}
	bool result = false;
	json jsonObj;
	try {	// попытка создания json-объекта и записи в него
		for (auto i = 0; i < msgsCount; i++) {
			jsonObj[i]["id"] = message[i].id;
			jsonObj[i]["senderId"] = message[i].senderId;
			jsonObj[i]["senderLogin"] = cp1251_to_utf8(message[i].senderLogin);	// перекодирование строк в UTF-8
			jsonObj[i]["receiverId"] = message[i].receiverId;
			jsonObj[i]["receiverLogin"] = cp1251_to_utf8(message[i].receiverLogin);	// перекодирование строк в UTF-8
			jsonObj[i]["date_time"] = cp1251_to_utf8(message[i].date_time);	// перекодирование строк в UTF-8
			jsonObj[i]["text"] = cp1251_to_utf8(message[i].text);	// перекодирование строк в UTF-8
			jsonObj[i]["wasReaded"] = message[i].wasReaded;
		}
		msgs_data << setw(4) << jsonObj;
		result = true;
	}
	catch (exception& e) {
		cout << e.what() << '\n';
	}
	msgs_data.close();
	return result;
}


int Messages::GetLastMsgId() const {	// возвращает id последнего сообщения 
	return msgsCount;
}


bool Messages::IsChanged() const {	// возвращает флаг наличия изменений в сообщениях
	return isChanged;
}