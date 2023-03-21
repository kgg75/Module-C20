#include <iostream>
#include "Client.h"
#include "UserFunc.h"
#include "../Constants.h"
#include "../ServiceFunc.h"
#include "../SHA1.h"

#if defined(_WIN32)
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
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
	//typedef unsigned __int64	socklen_t;

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


char request_msg[BUFFER_LENGTH], response_msg[BUFFER_LENGTH];
uint32_t request_msg_length, response_msg_length;
SOCKET socket_descriptor;
fd_set descriptors_set{ 0 };
const timeval timeout = { 0, 0 };


int client_start() {
	int32_t iResult;

	// Создадим сокет
	WIN(
		WSADATA wsaData = {0};
		WSAStartup(MAKEWORD(2, 2), &wsaData);
	)

	socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_descriptor WIN(== INVALID_SOCKET)NIX(< 0)) {
		cout << "Сокет не может быть создан!\n";
		WIN(cout << "Код ошибки: " << WSAGetLastError() << '\n';)
		return -1;
	}

	struct sockaddr_in server_address;
	server_address.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);    // Установим адрес сервера
	server_address.sin_port = htons(PORT_NUMBER);    // Зададим номер порта
	server_address.sin_family = AF_INET;    // Используем IPv4

	// Установим соединение с сервером
	iResult = connect(socket_descriptor, (struct sockaddr*)&server_address, sizeof(server_address));
	if (iResult WIN(== SOCKET_ERROR)NIX(!= 0)) {
		cout << "Соединение с сервером не установлено!\n";
		WIN(cout << "Код ошибки: " << WSAGetLastError() << '\n';)
		return -1;
	}
	return 0;
}

void client_stop() {	// закрываем сокет, завершаем соединение
	WIN(send(socket_descriptor, CLIENT_STOP_MESSAGE, sizeof(CLIENT_STOP_MESSAGE), 0))	// информируем сервер о своём завершении
	NIX(write(socket_descriptor, CLIENT_STOP_MESSAGE, sizeof(CLIENT_STOP_MESSAGE)));

	(closesocket)NIX(close)(socket_descriptor);
	WIN(WSACleanup());
}


void print_help(const CommandSpace* commandSpace) {	// печать справки по командам
	for (int i = 0; commandSpace[i].command[0] != 0; i++)
		cout << "\t" << commandSpace[i].command << "\t- " << commandSpace[i].help << '\n';
}


void AtStart() {	// начальные сообщения
	cout.clear();
	cout << SEPARATOR;
	cout << "* * *  Чат-клиент версия 1.0  * * *\n";
	cout << SEPARATOR;
	cout << "Чат запущен " << GetTime() << '\n';
	cout << SEPARATOR;
	cout << "Выберите действие:\n";
	print_help(GLOBAL_COMMANDS);
	cout << SEPARATOR;
}


int32_t MessagesExchange(const uint8_t command_code) {
	request_msg[0] = command_code;	// размещаем код команды в ячейку [0];

	int32_t iResult =	WIN(send(socket_descriptor, request_msg, request_msg_length, 0))
						NIX(write(socket_descriptor, request_msg, request_msg_length));

	if (iResult WIN(== SOCKET_ERROR)NIX(!= 0)) {	// Если передали >= 0  байт, значит пересылка прошла успешно
		cout << "Ошибка отправки данных на сервер!\n";
		WIN(cout << "Код ошибки: " << WSAGetLastError() << '\n';)
	}

	iResult =	WIN(recv(socket_descriptor, response_msg, BUFFER_LENGTH, 0))	// получаем ответ
				NIX(read(socket_descriptor, response_msg, BUFFER_LENGTH));

	if (iResult WIN(== SOCKET_ERROR)NIX(!= 0)) {	// Если получили >= 0  байт, значит приём прошёл успешно
		cout << "Ошибка получения данных с сервера!\n";
		WIN(cout << "Код ошибки: " << WSAGetLastError() << '\n';)
	}

	return iResult;
}


uint32_t RequestMessagesQty(const string& receiver, const uint8_t flag_UnReadedOrAll) {	// запрос числа непрочитанных сообщений
	memcpy(&request_msg[1], receiver.c_str(), receiver.length());	// копируем login
	request_msg_length = 1 + receiver.length() + 1;
	MessagesExchange(flag_UnReadedOrAll);	// получаем количество сообщений

	uint32_t unReadedMsgs;
	memcpy(&unReadedMsgs, &response_msg[1], sizeof(unReadedMsgs));
	return unReadedMsgs;
}


void PrintMessages(const uint8_t command_code, const string& login) {
	memcpy(&request_msg[1], login.c_str(), login.length());	// копируем login
	request_msg_length = 1 + login.length() + 1;

	while (true) {
		MessagesExchange(command_code);	// инициализируем передачу и подтверждаем продолжение передачи сообщений
		if (response_msg[0] != 0)
			cout << response_msg;
		else
			break;
	}
}


void PrintErrorMessage(const int8_t error_code) {
	auto i = 0;
	while (ERROR_SPACE[i].code != error_code && ERROR_SPACE[i].code != 0)
		i++;
	cout << ERROR_SPACE[i].text << '\n';
}


int Chat() {	// главный обработчик чата
	string login, new_login, name, password, receiver;
	SHA1PwdArray pass_sha1_hash;	// хэш пароля
	bool doing = true;
	AtStart();

	while (doing) {
		WIN(memset(request_msg, 0, BUFFER_LENGTH))NIX(bzero(request_msg, BUFFER_LENGTH));
		switch (auto g_command_code = get_command(GLOBAL_COMMANDS, "Введите команду >> ")) {	// получаем глобальную команду с консоли
			case GCMD_REG:	// команда reg - зарегистрироваться
				cout << "Для регистрации введите следующие данные пользователя:\n";
				if (NewUser(name, login, password)) {
					sha1(pass_sha1_hash, password.c_str(), password.length());
					memcpy(&request_msg[1], name.c_str(), name.length());	// копируем name
					memcpy(&request_msg[1 + name.length() + 1], login.c_str(), login.length());	// копируем login
					memcpy(&request_msg[1 + name.length() + 1 + login.length() + 1], &pass_sha1_hash, SHA1_HASH_LENGTH_BYTES);	// копируем хэш с учётом '0' в конце login
					request_msg_length = 1 + name.length() + 1 + login.length() + 1 + SHA1_HASH_LENGTH_BYTES;
					MessagesExchange(g_command_code);

					if ((int8_t)response_msg[0] == 1)	// пользователь успешно зарегистрировался
						cout << "Вы успешно зарегистрированы!\n";
					else PrintErrorMessage((int8_t)response_msg[0]);
				}
				break;
			case GCMD_LOGIN:	// команда login - вход
				if (SignIn(login, password)) {
					sha1(pass_sha1_hash, password.c_str(), password.length());
					memcpy(&request_msg[1], login.c_str(), login.length());	// копируем login
					memcpy(&request_msg[1 + login.length() + 1], &pass_sha1_hash, SHA1_HASH_LENGTH_BYTES);	// копируем хэш с учётом '0' в конце login
					request_msg_length = 1 + login.length() + 1 + SHA1_HASH_LENGTH_BYTES;
					MessagesExchange(g_command_code);

					if ((int8_t)response_msg[0] == 1) {	// пользователь успешно авторизовался
						cout << "Пользователь '" << login << "' вошёл в чат.\n";

						uint32_t unReadedMsgs = RequestMessagesQty(login, LCMD_UNREADED_COUNT);
						if (unReadedMsgs > 0) {
							cout << "Есть непрочитанные сообщения (" << unReadedMsgs << "). ";
							if (QuestionYN("Показать?"))	// вывод непрочитанных сообщений
								PrintMessages(LCMD_READ_UNREADED, login);
						}

						while (!login.empty()) {	// пока пользователь не вышел из чата
							WIN(memset(request_msg, 0, BUFFER_LENGTH))NIX(bzero(request_msg, BUFFER_LENGTH));
							// начало локального пространства команд
							switch (auto l_command_code = get_command(LOCAL_COMMANDS, ", введите команду >> ", login)) {	// получаем локальную команду с консоли
								case LCMD_SEND: {	// локальная команда send - отправить сообщение
										if (!ChooseUser(receiver))	// выбор получателя
											break;
										memcpy(&request_msg[1], receiver.c_str(), receiver.length());	// копируем receiver
										request_msg_length = 1 + receiver.length() + 1;
										MessagesExchange(LCMD_IS_USER_EXIST);	// проверяем существование получателя

										if ((int8_t)response_msg[0] == 1) {
											if (receiver != login) {
												string text;
												if (AddMessage(text)) {
													memcpy(&request_msg[1], receiver.c_str(), receiver.length());	// копируем имя получателя
													memcpy(&request_msg[1 + receiver.length() + 1], text.c_str(), text.length());	// копируем текст сообщения
													request_msg_length = 1 + receiver.length() + 1 + text.length() + 1;
													MessagesExchange(l_command_code);

													if (response_msg[0] != 0)
														cout << "Сообщение для '" << receiver << "' отправлено.\n";
												}
											} else
												cout << "Нельзя отправлять сообщения самому себе.\n";
										}
									}
									break;
								case LCMD_READ: {	// локальная команда read - прочитать сообщения
										bool answer = QuestionYN("Показать только непрочитанные (y) или все сообщения (n)?");
										uint32_t messsagesQty = RequestMessagesQty(login, answer ? LCMD_UNREADED_COUNT : LCMD_MESSAGES_COUNT);	// получаем количество сообщений
										
										if (messsagesQty > 0)
											PrintMessages(answer ? LCMD_READ_UNREADED : LCMD_READ_ALL, login);
										else
											cout << "Нет " << (answer ? "непрочитанных " : "") << "сообщений.\n";
									}
									break;
								case LCMD_USER_INFO:	// локальная команда info - вывести информацию о пользователе
									MessagesExchange(l_command_code);
									cout << "Информация о пользователе:\n" << &response_msg[1];
									break;
								case LCMD_USERS_LIST:	// локальная команда list - вывести список всех пользователей
									cout << "Список пользователей:\n";
									PrintMessages(l_command_code, login);
									break;
								case LCMD_USER_DELETE:	// локальная команда delete - удалить учётную запись пользователя
									if (QuestionYN("Информация о пользователе будет навсегда удалена. Вы уверены?") && 
										QuestionYN("Это действие нельзя будет отменить. Подтвердите удаление ещё раз."))
										MessagesExchange(l_command_code);
										login.clear();
									break;
								case LCMD_USER_CHANGE: {	// локальная команда change - изменить учётные данные пользователя
										bool loop = true;
										cout << "Вы находитесь в меню изменения учётных данных пользователя '" << login << "'\n";
										print_help(SUBLOCAL_COMMANDS);
										while (loop) {
											switch (auto sl_command_code = get_command(SUBLOCAL_COMMANDS, ", выберите действие или команду >> ", login)) {	// получаем локальную команду с консоли
												case SLCMD_CHANGE_NAME:	// локальная команда name - изменить имя пользователя
													if (SetName(name)) {
														memcpy(&request_msg[1], name.c_str(), name.length());
														request_msg_length = 1 + name.length() + 1;
														MessagesExchange(sl_command_code);
														
														if ((int8_t)response_msg[0] == 1)
														cout << "Имя пользователя '" << login << "' было изменено.\n\n";
													}
													break;
												case SLCMD_CHANGE_LOGIN:	// локальная команда log - изменить учётное имя (login) пользователя
													if (SetLogin(new_login)) {
														memcpy(&request_msg[1], new_login.c_str(), new_login.length());
														request_msg_length = 1 + new_login.length() + 1;
														MessagesExchange(sl_command_code);

														if ((int8_t)response_msg[0] == 1) {
															login = new_login;
															cout << "Учётное имя (login) пользователя было изменено на '" << login << "'.\n\n";
														}
														else PrintErrorMessage((int8_t)response_msg[0]);
													}
													break;
												case SLCMD_CHANGE_PASSWORD:	// локальная команда pwd - изменить пароль пользователя
													if (SetPassword(password)) {
														sha1(pass_sha1_hash, password.c_str(), password.length());
														memcpy(&request_msg[1], &pass_sha1_hash, SHA1_HASH_LENGTH_BYTES);	// копируем новый хэш
														request_msg_length = 1 + SHA1_HASH_LENGTH_BYTES;
														MessagesExchange(sl_command_code);
	
														if ((int8_t)response_msg[0] == 1)
															cout << "Пароль пользователя '" << login << "' был изменён.\n\n";
													}
													break;
												case SLCMD_HELP:	// локальная команда help - вывести справку по командам
													print_help(SUBLOCAL_COMMANDS);
													break;
												case SLCMD_EXIT:	// локальная команда exit - выйти из этого меню
													loop = false;
													cout << "Вы вышли из меню изменения учётных данных.\n\n";
													break;
												default:
													cout << MSG_ERR_CMD;
											}
										}
									}	// конец локального пространства команды change - изменить учётные данные пользователя
									break;
								case LCMD_HELP:	// локальная команда help - вывести справку по командам
									print_help(LOCAL_COMMANDS);
									break;
								case LCMD_LOGOUT:	// локальная команда logout - выход
									request_msg_length = 1;
									MessagesExchange(l_command_code);
									login.clear();
									break;
								default:
									cout << MSG_ERR_CMD;
							}
						}	// конец локального пространства команд
					}
					else PrintErrorMessage((int8_t)response_msg[0]);	// ошибки авторизации
				}
				break;	// продолжаем запрос команд
			case GCMD_HELP:	// команда help - вывести справку по командам
				print_help(GLOBAL_COMMANDS);
				break;
			case GCMD_QUIT:	// команда quit - закрыть чат
				doing = false;
				break;
			default:
				cout << MSG_ERR_CMD;
		}
	}
	return 0;
}
