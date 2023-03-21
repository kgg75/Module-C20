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

	// ������� ��� ��������� ��������� �� OS    
	#define WIN(exp) exp
	#define NIX(exp)
#elif defined(__linux__)
	#include <unistd.h>
	#include <sys/socket.h> // ���������� ��� ������ � ��������
	//#include <sys/types.h>
	#include <netinet/in.h>
	// ������� ��� ��������� ��������� �� OS    
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

	// �������� �����
	WIN(
		WSADATA wsaData = {0};
		WSAStartup(MAKEWORD(2, 2), &wsaData);
	)

	socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_descriptor WIN(== INVALID_SOCKET)NIX(< 0)) {
		cout << "����� �� ����� ���� ������!\n";
		WIN(cout << "��� ������: " << WSAGetLastError() << '\n';)
		return -1;
	}

	struct sockaddr_in server_address;
	server_address.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);    // ��������� ����� �������
	server_address.sin_port = htons(PORT_NUMBER);    // ������� ����� �����
	server_address.sin_family = AF_INET;    // ���������� IPv4

	// ��������� ���������� � ��������
	iResult = connect(socket_descriptor, (struct sockaddr*)&server_address, sizeof(server_address));
	if (iResult WIN(== SOCKET_ERROR)NIX(!= 0)) {
		cout << "���������� � �������� �� �����������!\n";
		WIN(cout << "��� ������: " << WSAGetLastError() << '\n';)
		return -1;
	}
	return 0;
}

void client_stop() {	// ��������� �����, ��������� ����������
	WIN(send(socket_descriptor, CLIENT_STOP_MESSAGE, sizeof(CLIENT_STOP_MESSAGE), 0))	// ����������� ������ � ���� ����������
	NIX(write(socket_descriptor, CLIENT_STOP_MESSAGE, sizeof(CLIENT_STOP_MESSAGE)));

	(closesocket)NIX(close)(socket_descriptor);
	WIN(WSACleanup());
}


void print_help(const CommandSpace* commandSpace) {	// ������ ������� �� ��������
	for (int i = 0; commandSpace[i].command[0] != 0; i++)
		cout << "\t" << commandSpace[i].command << "\t- " << commandSpace[i].help << '\n';
}


void AtStart() {	// ��������� ���������
	cout.clear();
	cout << SEPARATOR;
	cout << "* * *  ���-������ ������ 1.0  * * *\n";
	cout << SEPARATOR;
	cout << "��� ������� " << GetTime() << '\n';
	cout << SEPARATOR;
	cout << "�������� ��������:\n";
	print_help(GLOBAL_COMMANDS);
	cout << SEPARATOR;
}


int32_t MessagesExchange(const uint8_t command_code) {
	request_msg[0] = command_code;	// ��������� ��� ������� � ������ [0];

	int32_t iResult =	WIN(send(socket_descriptor, request_msg, request_msg_length, 0))
						NIX(write(socket_descriptor, request_msg, request_msg_length));

	if (iResult WIN(== SOCKET_ERROR)NIX(!= 0)) {	// ���� �������� >= 0  ����, ������ ��������� ������ �������
		cout << "������ �������� ������ �� ������!\n";
		WIN(cout << "��� ������: " << WSAGetLastError() << '\n';)
	}

	iResult =	WIN(recv(socket_descriptor, response_msg, BUFFER_LENGTH, 0))	// �������� �����
				NIX(read(socket_descriptor, response_msg, BUFFER_LENGTH));

	if (iResult WIN(== SOCKET_ERROR)NIX(!= 0)) {	// ���� �������� >= 0  ����, ������ ���� ������ �������
		cout << "������ ��������� ������ � �������!\n";
		WIN(cout << "��� ������: " << WSAGetLastError() << '\n';)
	}

	return iResult;
}


uint32_t RequestMessagesQty(const string& receiver, const uint8_t flag_UnReadedOrAll) {	// ������ ����� ������������� ���������
	memcpy(&request_msg[1], receiver.c_str(), receiver.length());	// �������� login
	request_msg_length = 1 + receiver.length() + 1;
	MessagesExchange(flag_UnReadedOrAll);	// �������� ���������� ���������

	uint32_t unReadedMsgs;
	memcpy(&unReadedMsgs, &response_msg[1], sizeof(unReadedMsgs));
	return unReadedMsgs;
}


void PrintMessages(const uint8_t command_code, const string& login) {
	memcpy(&request_msg[1], login.c_str(), login.length());	// �������� login
	request_msg_length = 1 + login.length() + 1;

	while (true) {
		MessagesExchange(command_code);	// �������������� �������� � ������������ ����������� �������� ���������
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


int Chat() {	// ������� ���������� ����
	string login, new_login, name, password, receiver;
	SHA1PwdArray pass_sha1_hash;	// ��� ������
	bool doing = true;
	AtStart();

	while (doing) {
		WIN(memset(request_msg, 0, BUFFER_LENGTH))NIX(bzero(request_msg, BUFFER_LENGTH));
		switch (auto g_command_code = get_command(GLOBAL_COMMANDS, "������� ������� >> ")) {	// �������� ���������� ������� � �������
			case GCMD_REG:	// ������� reg - ������������������
				cout << "��� ����������� ������� ��������� ������ ������������:\n";
				if (NewUser(name, login, password)) {
					sha1(pass_sha1_hash, password.c_str(), password.length());
					memcpy(&request_msg[1], name.c_str(), name.length());	// �������� name
					memcpy(&request_msg[1 + name.length() + 1], login.c_str(), login.length());	// �������� login
					memcpy(&request_msg[1 + name.length() + 1 + login.length() + 1], &pass_sha1_hash, SHA1_HASH_LENGTH_BYTES);	// �������� ��� � ������ '0' � ����� login
					request_msg_length = 1 + name.length() + 1 + login.length() + 1 + SHA1_HASH_LENGTH_BYTES;
					MessagesExchange(g_command_code);

					if ((int8_t)response_msg[0] == 1)	// ������������ ������� �����������������
						cout << "�� ������� ����������������!\n";
					else PrintErrorMessage((int8_t)response_msg[0]);
				}
				break;
			case GCMD_LOGIN:	// ������� login - ����
				if (SignIn(login, password)) {
					sha1(pass_sha1_hash, password.c_str(), password.length());
					memcpy(&request_msg[1], login.c_str(), login.length());	// �������� login
					memcpy(&request_msg[1 + login.length() + 1], &pass_sha1_hash, SHA1_HASH_LENGTH_BYTES);	// �������� ��� � ������ '0' � ����� login
					request_msg_length = 1 + login.length() + 1 + SHA1_HASH_LENGTH_BYTES;
					MessagesExchange(g_command_code);

					if ((int8_t)response_msg[0] == 1) {	// ������������ ������� �������������
						cout << "������������ '" << login << "' ����� � ���.\n";

						uint32_t unReadedMsgs = RequestMessagesQty(login, LCMD_UNREADED_COUNT);
						if (unReadedMsgs > 0) {
							cout << "���� ������������� ��������� (" << unReadedMsgs << "). ";
							if (QuestionYN("��������?"))	// ����� ������������� ���������
								PrintMessages(LCMD_READ_UNREADED, login);
						}

						while (!login.empty()) {	// ���� ������������ �� ����� �� ����
							WIN(memset(request_msg, 0, BUFFER_LENGTH))NIX(bzero(request_msg, BUFFER_LENGTH));
							// ������ ���������� ������������ ������
							switch (auto l_command_code = get_command(LOCAL_COMMANDS, ", ������� ������� >> ", login)) {	// �������� ��������� ������� � �������
								case LCMD_SEND: {	// ��������� ������� send - ��������� ���������
										if (!ChooseUser(receiver))	// ����� ����������
											break;
										memcpy(&request_msg[1], receiver.c_str(), receiver.length());	// �������� receiver
										request_msg_length = 1 + receiver.length() + 1;
										MessagesExchange(LCMD_IS_USER_EXIST);	// ��������� ������������� ����������

										if ((int8_t)response_msg[0] == 1) {
											if (receiver != login) {
												string text;
												if (AddMessage(text)) {
													memcpy(&request_msg[1], receiver.c_str(), receiver.length());	// �������� ��� ����������
													memcpy(&request_msg[1 + receiver.length() + 1], text.c_str(), text.length());	// �������� ����� ���������
													request_msg_length = 1 + receiver.length() + 1 + text.length() + 1;
													MessagesExchange(l_command_code);

													if (response_msg[0] != 0)
														cout << "��������� ��� '" << receiver << "' ����������.\n";
												}
											} else
												cout << "������ ���������� ��������� ������ ����.\n";
										}
									}
									break;
								case LCMD_READ: {	// ��������� ������� read - ��������� ���������
										bool answer = QuestionYN("�������� ������ ������������� (y) ��� ��� ��������� (n)?");
										uint32_t messsagesQty = RequestMessagesQty(login, answer ? LCMD_UNREADED_COUNT : LCMD_MESSAGES_COUNT);	// �������� ���������� ���������
										
										if (messsagesQty > 0)
											PrintMessages(answer ? LCMD_READ_UNREADED : LCMD_READ_ALL, login);
										else
											cout << "��� " << (answer ? "������������� " : "") << "���������.\n";
									}
									break;
								case LCMD_USER_INFO:	// ��������� ������� info - ������� ���������� � ������������
									MessagesExchange(l_command_code);
									cout << "���������� � ������������:\n" << &response_msg[1];
									break;
								case LCMD_USERS_LIST:	// ��������� ������� list - ������� ������ ���� �������������
									cout << "������ �������������:\n";
									PrintMessages(l_command_code, login);
									break;
								case LCMD_USER_DELETE:	// ��������� ������� delete - ������� ������� ������ ������������
									if (QuestionYN("���������� � ������������ ����� �������� �������. �� �������?") && 
										QuestionYN("��� �������� ������ ����� ��������. ����������� �������� ��� ���."))
										MessagesExchange(l_command_code);
										login.clear();
									break;
								case LCMD_USER_CHANGE: {	// ��������� ������� change - �������� ������� ������ ������������
										bool loop = true;
										cout << "�� ���������� � ���� ��������� ������� ������ ������������ '" << login << "'\n";
										print_help(SUBLOCAL_COMMANDS);
										while (loop) {
											switch (auto sl_command_code = get_command(SUBLOCAL_COMMANDS, ", �������� �������� ��� ������� >> ", login)) {	// �������� ��������� ������� � �������
												case SLCMD_CHANGE_NAME:	// ��������� ������� name - �������� ��� ������������
													if (SetName(name)) {
														memcpy(&request_msg[1], name.c_str(), name.length());
														request_msg_length = 1 + name.length() + 1;
														MessagesExchange(sl_command_code);
														
														if ((int8_t)response_msg[0] == 1)
														cout << "��� ������������ '" << login << "' ���� ��������.\n\n";
													}
													break;
												case SLCMD_CHANGE_LOGIN:	// ��������� ������� log - �������� ������� ��� (login) ������������
													if (SetLogin(new_login)) {
														memcpy(&request_msg[1], new_login.c_str(), new_login.length());
														request_msg_length = 1 + new_login.length() + 1;
														MessagesExchange(sl_command_code);

														if ((int8_t)response_msg[0] == 1) {
															login = new_login;
															cout << "������� ��� (login) ������������ ���� �������� �� '" << login << "'.\n\n";
														}
														else PrintErrorMessage((int8_t)response_msg[0]);
													}
													break;
												case SLCMD_CHANGE_PASSWORD:	// ��������� ������� pwd - �������� ������ ������������
													if (SetPassword(password)) {
														sha1(pass_sha1_hash, password.c_str(), password.length());
														memcpy(&request_msg[1], &pass_sha1_hash, SHA1_HASH_LENGTH_BYTES);	// �������� ����� ���
														request_msg_length = 1 + SHA1_HASH_LENGTH_BYTES;
														MessagesExchange(sl_command_code);
	
														if ((int8_t)response_msg[0] == 1)
															cout << "������ ������������ '" << login << "' ��� ������.\n\n";
													}
													break;
												case SLCMD_HELP:	// ��������� ������� help - ������� ������� �� ��������
													print_help(SUBLOCAL_COMMANDS);
													break;
												case SLCMD_EXIT:	// ��������� ������� exit - ����� �� ����� ����
													loop = false;
													cout << "�� ����� �� ���� ��������� ������� ������.\n\n";
													break;
												default:
													cout << MSG_ERR_CMD;
											}
										}
									}	// ����� ���������� ������������ ������� change - �������� ������� ������ ������������
									break;
								case LCMD_HELP:	// ��������� ������� help - ������� ������� �� ��������
									print_help(LOCAL_COMMANDS);
									break;
								case LCMD_LOGOUT:	// ��������� ������� logout - �����
									request_msg_length = 1;
									MessagesExchange(l_command_code);
									login.clear();
									break;
								default:
									cout << MSG_ERR_CMD;
							}
						}	// ����� ���������� ������������ ������
					}
					else PrintErrorMessage((int8_t)response_msg[0]);	// ������ �����������
				}
				break;	// ���������� ������ ������
			case GCMD_HELP:	// ������� help - ������� ������� �� ��������
				print_help(GLOBAL_COMMANDS);
				break;
			case GCMD_QUIT:	// ������� quit - ������� ���
				doing = false;
				break;
			default:
				cout << MSG_ERR_CMD;
		}
	}
	return 0;
}
