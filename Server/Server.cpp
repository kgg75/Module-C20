#include <iostream>
#include <string>
#include <map>
#include "Users.h"
#include "Messages.h"
#include "Server.h"
#include "../Constants.h"
#include "../ServiceFunc.h"


#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	//#include <winsock.h>
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <stdlib.h>
	#include <stdio.h>

	#pragma comment(lib, "Ws2_32.lib")
	#pragma comment(lib, "Mswsock.lib")
	#pragma comment(lib, "AdvApi32.lib")

	typedef unsigned __int64    ssize_t;
	
	#if defined(max)
	#undef max
	#endif

	// Макросы для выражений зависимых от OS    
	#define WIN(exp) exp
	#define NIX(exp)
#elif defined(__linux__)
	#include <unistd.h>
	#include <sys/socket.h> // Библиотека для работы с сокетами
	//#include <sys/types.h>
	#include <netinet/in.h>
	// Макросы для выражений зависимых от OS    
	#define WIN(exp)
	#define NIX(exp) exp
#endif


using namespace std;


Users users;
Messages messages;
map <SOCKET, uint32_t> users_map;	// связка сокет - идентификатор (id) пользователя
char request_msg[BUFFER_LENGTH], response_msg[BUFFER_LENGTH];
SOCKET socket_descriptor;
fd_set descriptors_set{ 0 };
const timeval timeout = { 0, 0 };
bool loop = true;	// флаг продолжения/остановки ChatServer()


void print_help(const CommandSpace* _commandSpace) {	// печать справки по командам
	for (int i = 0; _commandSpace[i].command[0] != 0; i++)
		cout << "\t" << _commandSpace[i].command << "\t- " << _commandSpace[i].help << '\n';
}


void AtStart() {	// начальные сообщения
	cout.clear();
	cout << SEPARATOR;
	cout << "---=== Чат-сервер версия 1.0 ===---\n";
	cout << SEPARATOR;
	cout << "Сервер запущен " << GetTime() << '\n';
	cout << SEPARATOR;
	cout << "Справка по командам:\n";
	print_help(SERVER_COMMANDS);
}


int server_start() {	// инициализация сервера
	struct sockaddr_in server_address;
	int bind_status, connection_status;

	// Создадим сокет
	WIN(WSADATA wsaData = {0}; WSAStartup(MAKEWORD(2, 2), &wsaData);)

	socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_descriptor WIN(== INVALID_SOCKET)NIX(< 0)) {
		cout << "Сокет не может быть создан!\n";
		WIN(cout << "Код ошибки: " << WSAGetLastError() << '\n';)
		return -1;
	}

	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(PORT_NUMBER);  // Зададим номер порта для связи
	server_address.sin_family = AF_INET;    // Используем IPv4

	// Привяжем сокет
	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(BOOL);
	int iResult = setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&bOptVal, bOptLen);
	iResult = setsockopt(socket_descriptor, SOL_SOCKET, SO_BROADCAST, (char*)&bOptVal, bOptLen);

	bind_status = bind(socket_descriptor, (struct sockaddr*)&server_address, sizeof(server_address));
	if (bind_status WIN(== SOCKET_ERROR)NIX(!= 0)) {
		cout << "Сокет не может быть привязан!\n";
		WIN(cout << "Код ошибки: " << WSAGetLastError() << '\n';)
		return -1;
	}

	// Поставим сервер на прием данных 
	connection_status = listen(socket_descriptor, 5);
	if (connection_status WIN(== SOCKET_ERROR)NIX(!= 0)) {
		cout << "Сервер не может установить новое соединение!\n";
		WIN(cout << "Код ошибки: " << WSAGetLastError() << '\n';)
		return -1;
	}

	return 0;
}


void server_stop() {	// закрываем сокет, завершаем соединение
	WIN(closesocket)NIX(close)(socket_descriptor);
	WIN(WSACleanup());
}


void ConsoleWaitMessage() {
	cin.clear();
	//cin.ignore(numeric_limits<streamsize>::max(), '\n');	// очистка буфера консоли
	cout << "Введите команду: ";
}


int32_t SendResponse(const SOCKET connection_descriptor, const char* local_response_msg, const int32_t local_response_msg_length) {
	int32_t iResult =	WIN(send(connection_descriptor, local_response_msg, local_response_msg_length, 0))	// отправляем ответ
						NIX(write(connection_descriptor, local_response_msg, local_response_msg_length));

	if (iResult WIN(== SOCKET_ERROR)NIX(!= 0)) {	// Если передали > 0  байт, значит пересылка прошла успешно
		cout << "Ошибка отправки данных клиенту!\n";
		WIN(cout << "Код ошибки: " << WSAGetLastError() << '\n';)
	}

	return iResult;
}


int32_t ReadRequest(const SOCKET connection_descriptor) {
	int32_t iResult =	WIN(recv(connection_descriptor, request_msg, BUFFER_LENGTH, 0))	// получаем ответ
						NIX(read(connection_descriptor, request_msg, BUFFER_LENGTH));

	if (iResult WIN(== SOCKET_ERROR)NIX(!= 0)) {	// Если получили > 0  байт, значит приём прошёл успешно
		cout << "Ошибка получения данных от клиента!\n";
		WIN(cout << "Код ошибки: " << WSAGetLastError() << '\n';)
	}

	return iResult;
}


void MainRequestHandler(const SOCKET connection_descriptor) {
	string login, name, receiver, text;
	int32_t iResult, response_msg_length = 1;
	uint32_t count;
	uint8_t command_code = request_msg[0];

	WIN(memset(response_msg, 0, BUFFER_LENGTH))NIX(bzero(response_msg, BUFFER_LENGTH));
	response_msg[0] = 1;	// устанавливаем значение ответа по умолчанию

	switch (command_code) {
		case GCMD_REG:	// команда reg - зарегистрироваться
			name = &request_msg[1];	// создаём нового пользователя
			login = &request_msg[1 + login.length()  + 1];
			iResult = users.NewUser(name, login, (SHA1PwdArray&)request_msg[1 + name.length() + 1 + login.length() + 1]);
			if (iResult > 0) {	// регистрация прошла успешно
				users_map.find(connection_descriptor)->second = iResult;	// добавляем id пользователя в словарь
				iResult = 1;
			}
			response_msg[0] = (int8_t)iResult;	// подготовили ответ
			break;
		case GCMD_LOGIN:	// команда login - вход
			login = &request_msg[1];

			if (users.IsLogged(login)) {
				response_msg[0] = ERR_ALREADY_LOGGED;	// пользователь с таким именем уже вошёл
				break;
			}

			iResult = users.Login(login, (SHA1PwdArray&)request_msg[1 + login.length() + 1]);	// передаём ссылку на фрагмент буфeра с данными хэша
			if (iResult > 0) {	// login прошёл успешно
				users_map.find(connection_descriptor)->second = iResult;	// добавляем id пользователя в словарь
				iResult = 1;
			}
			response_msg[0] = (int8_t)iResult;	// подготовили ответ
			cout << '\n' << GetTime() << " на сокете #" << connection_descriptor << " вошёл в чат пользователь '" << login << "'\n";
			ConsoleWaitMessage();
			break;
		case LCMD_SEND:
			receiver = &request_msg[1];
			text = &request_msg[1 + receiver.length() + 1];
			iResult = messages.Add(users_map.at(connection_descriptor),
					users.GetUserLogin(users_map.at(connection_descriptor)),	// получаем login отправителя по его id
					users.GetUserId(receiver),	// получаем id получателя
					receiver,
					text);
			break;
		case LCMD_READ_ALL:
		case LCMD_READ_UNREADED: {
				bool showUnReadedOnly = (command_code == LCMD_READ_UNREADED ? true : false);
				receiver = &request_msg[1];
				uint32_t receiverId = users.GetUserId(receiver);	// получаем id получателя
				for (auto i = 0; i < messages.GetLastMsgId(); i++)	// перебираем все соообщения
					if (messages.ReadByOne(text, receiverId, i, showUnReadedOnly)) {
						iResult = SendResponse(connection_descriptor, text.c_str(), text.length() + 1);	// отправляем ответы
						ReadRequest(connection_descriptor);
					}
				response_msg[0] = 0;
			}
			break;
		case LCMD_USER_INFO:
			iResult = users.Info(users_map.at(connection_descriptor), text);
			memcpy(&response_msg[1], text.c_str(), iResult);
			response_msg_length += iResult + 1;
			break;
		case LCMD_USERS_LIST: {
				for (auto i = 1; i < users.GetUsersQty(); i++)	// перебираем всех пользователей, кроме служебного (№0)
					if (users.ListByOne(i, text)) {
						iResult = SendResponse(connection_descriptor, text.c_str(), text.length() + 1);	// отправляем ответы
						ReadRequest(connection_descriptor);
					}
				response_msg[0] = 0;
			}
			break;
		case LCMD_USER_DELETE:
			users.DeleteUser(users_map.at(connection_descriptor));	// аргумент - id пользователя
			users_map.find(connection_descriptor)->second = 0;
			break;
		case SLCMD_CHANGE_NAME:
			name = &request_msg[1];
			users.ChangeRegData(users_map.at(connection_descriptor), &request_msg[1], nullptr, nullptr, command_code);
			break;
		case SLCMD_CHANGE_LOGIN:
			login = &request_msg[1];
			iResult = users.ChangeRegData(users_map.at(connection_descriptor), nullptr, &request_msg[1], nullptr, command_code);
			response_msg[0] = (int8_t)iResult;
			break;
		case SLCMD_CHANGE_PASSWORD:
			users.ChangeRegData(users_map.at(connection_descriptor), nullptr, nullptr, (SHA1PwdArray*)&request_msg[1], command_code);
			break; 
		case LCMD_LOGOUT:	// локальная команда logout - выход
			users.Logout(users_map.at(connection_descriptor));	// аргумент - id пользователя
			cout << '\n' << GetTime() << " на сокете #" << connection_descriptor << " вышел из чата пользователь '" << users.GetUserLogin(users_map.at(connection_descriptor)) << "'\n";
			ConsoleWaitMessage();
			users_map.find(connection_descriptor)->second = 0;
			break;
		case LCMD_IS_USER_EXIST:
			iResult = users.GetUserId(&request_msg[1]);
			response_msg[0] = (int8_t)(iResult > 0 ? 1 : 0);
			break;
		case LCMD_MESSAGES_COUNT:	// все сообщения
		case LCMD_UNREADED_COUNT:	// только непрочитанные
			count = messages.GetMessagesQty(users_map.at(connection_descriptor), command_code == LCMD_UNREADED_COUNT);
			response_msg[0] = (int8_t)(count > 0 ? 1 : 0);
			memcpy(&response_msg[1], &count, sizeof(count));	// помещаем в буфер кол-во непрочитанных сообщений
			response_msg_length += sizeof(count);
			break;
		default:
			;
	}

	iResult = SendResponse(connection_descriptor, response_msg, response_msg_length);	// отправляем ответ
}


int InitializeServer() {
	cout << "Инициализация сервера чата... ";
	if (server_start() == 0)
		cout << "выполнена!\n";
	else {
		cout << "Ошибка инициализации сервера чата; программа будет закрыта.\n";
		loop = false;
		return 1;
	}

	cout << "Загрузка базы данных пользователей из внешнего файла... ";
	if (users.Open())	// загружаем данные пользователей
		cout << "выполнена!\n";
	else if (QuestionYN("Закрыть сервер?")) {
		loop = false;
		return 2;	// аварийный выход по желанию пользователя
	}

	cout << "Загрузка базы данных сообщений из внешнего файла... ";
	if (messages.Open())	// загружаем сообщения
		cout << "выполнена!\n";
	else if (QuestionYN("Закрыть сервер?")) {
		loop = false;
		return 3;	// аварийный выход по желанию пользователя
	}

	AtStart();
	return 0;
}


int ChatCommandsHandler() {	// обработчик консольных команд чата
	string command, text;

	while (loop) {
		ConsoleWaitMessage();
		getline(cin, command);

		if (!command.empty()) {
			switch (get_command_code(SERVER_COMMANDS, command)) {	// аназизируем серверную команду
				case SCMD_SAVE:
					if (users.Save() && messages.Save())
						cout << "Данные пользователей и сообщений сохранены.\n";
					break;
				case SCMD_CON:
					if (!users_map.empty()) {
						cout << "Список подключений:\n   сокет : учётное имя\n";
						for (map <SOCKET, uint32_t>::iterator it = users_map.begin(); it != users_map.end(); it++) {
							text = it->second != 0 ? users.GetUserLogin(users_map.at(it->first)) : "<вход не выполнен>";
							cout << "     " << it->first << " : " << text << "\n";
						}
					}
					else
						cout << "Нет подключений.\n";
					break;
				case SCMD_USERS:
					cout << "Список пользователей:\n";
					for (auto i = 1; i < users.GetUsersQty(); i++)	// перебираем всех пользователей, кроме служебного (№0)
						if (users.ListByOne(i, text))
							cout << text;
					break;
				case SCMD_HELP:
					print_help(SERVER_COMMANDS);
					break;
				case SCMD_EXIT:
					if (users.IsChanged())	// если есть изменения, сохраняем данные пользователей во внешний файл
						users.Save();
					if (messages.IsChanged())	// если есть изменения, сохраняем сообщения во внешний файл
						messages.Save();
					if (!users_map.empty()) {
						if (QuestionYN("Есть действующие подключения. Всё равно закрыть сервер?"))
							loop = false;	// инициализируем остановка сервера
					}
					else if (QuestionYN("Подтвердите остановку сервера"))
						loop = false;	// инициализируем остановка сервера

					break;
				default:
					cout << MSG_ERR_CMD;
			}
		}
		WIN(Sleep(DELAY_MS))NIX(usleep(DELAY_MCS));
	}

	return 0;
}


int ChatServer() {	// главный обработчик чата
	while (loop) {	// Communication Establishment
		FD_ZERO(&descriptors_set);
		FD_SET(socket_descriptor, &descriptors_set);

		int iResult = select(0, &descriptors_set, nullptr, nullptr, &timeout);	// проверяем готовность сокета на новые подключения

		if (iResult > 0 && users_map.size() < FD_SETSIZE) {
			struct sockaddr_in client_address;
			socklen_t length = sizeof(client_address);
			SOCKET socket = accept(socket_descriptor, (struct sockaddr*)&client_address, &length);
			if (socket WIN(== INVALID_SOCKET)NIX(< 0)) {
				cout << "\nСервер не может принимать данные от клиента!\n";
				WIN(cout << "Код ошибки: " << WSAGetLastError() << '\n';)
			}
			else {
				users_map.emplace(socket, 0);
				cout << '\n' << GetTime() << " подключился клиент #" << socket << '\n';
			}
			ConsoleWaitMessage();
		}
		else {
			if (iResult WIN(== SOCKET_ERROR)NIX(!= 0)) {
				WIN(cout << "Код ошибки: " << WSAGetLastError() << '\n';);
				ConsoleWaitMessage();
			}

			WIN(Sleep(DELAY_MS))NIX(usleep(DELAY_MCS));

			if (users_map.size() == 0)
				continue;
		}

		for (map <SOCKET, uint32_t>::iterator it = users_map.begin(); it != users_map.end(); ) {
			FD_ZERO(&descriptors_set);
			FD_SET(it->first, &descriptors_set);

			iResult = select(0, nullptr, nullptr, &descriptors_set, &timeout);	// проверяем сокет на ошибки
			if (iResult > 0) {
				WIN(cout << "\nЕсть ошибки в подключении клиента\t" << it->first <<
					"\nКод ошибки : " << WSAGetLastError() << '\n';);
				ConsoleWaitMessage();
			}

			descriptors_set.fd_count = 1;
			iResult = select(0, &descriptors_set, nullptr, nullptr, &timeout);	// проверяем готовность сокета на чтение
			if (iResult > 0) {
				WIN(memset(request_msg, 0, BUFFER_LENGTH))NIX(bzero(request_msg, BUFFER_LENGTH));
				iResult = recv(it->first, request_msg, BUFFER_LENGTH, 0);	// принимаем запрос
				
				if (iResult > 0) {
					if (strcmp(request_msg, CLIENT_STOP_MESSAGE) == 0) {
						cout << '\n' << GetTime() << " клиент #" << it->first << " отключился\n";
						ConsoleWaitMessage();
						it = users_map.erase(it);
						continue;
					}

					MainRequestHandler(it->first);	// обрабатывает входящие данные из буфера и отправляет ответы
				}
				else if (iResult WIN(== SOCKET_ERROR)NIX(!= 0)) {	// клиент "отвалился"
					cout << '\n' << GetTime() << " связь с клиентом #" << it->first << " потеряна\n";
					ConsoleWaitMessage();
					users.Logout(users_map.at(it->first));	// помечаем пользователя как отключённого
					it = users_map.erase(it);
					continue;
				}
			}
			it++;
		}
		WIN(Sleep(DELAY_MS))NIX(usleep(DELAY_MCS));
	}

	// остановка сервера и выход
	server_stop();

	return 0;
}

