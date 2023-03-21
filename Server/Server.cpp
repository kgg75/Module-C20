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


Users users;
Messages messages;
map <SOCKET, uint32_t> users_map;	// ������ ����� - ������������� (id) ������������
char request_msg[BUFFER_LENGTH], response_msg[BUFFER_LENGTH];
SOCKET socket_descriptor;
fd_set descriptors_set{ 0 };
const timeval timeout = { 0, 0 };
bool loop = true;	// ���� �����������/��������� ChatServer()


void print_help(const CommandSpace* _commandSpace) {	// ������ ������� �� ��������
	for (int i = 0; _commandSpace[i].command[0] != 0; i++)
		cout << "\t" << _commandSpace[i].command << "\t- " << _commandSpace[i].help << '\n';
}


void AtStart() {	// ��������� ���������
	cout.clear();
	cout << SEPARATOR;
	cout << "---=== ���-������ ������ 1.0 ===---\n";
	cout << SEPARATOR;
	cout << "������ ������� " << GetTime() << '\n';
	cout << SEPARATOR;
	cout << "������� �� ��������:\n";
	print_help(SERVER_COMMANDS);
}


int server_start() {	// ������������� �������
	struct sockaddr_in server_address;
	int bind_status, connection_status;

	// �������� �����
	WIN(WSADATA wsaData = {0}; WSAStartup(MAKEWORD(2, 2), &wsaData);)

	socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_descriptor WIN(== INVALID_SOCKET)NIX(< 0)) {
		cout << "����� �� ����� ���� ������!\n";
		WIN(cout << "��� ������: " << WSAGetLastError() << '\n';)
		return -1;
	}

	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(PORT_NUMBER);  // ������� ����� ����� ��� �����
	server_address.sin_family = AF_INET;    // ���������� IPv4

	// �������� �����
	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(BOOL);
	int iResult = setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&bOptVal, bOptLen);
	iResult = setsockopt(socket_descriptor, SOL_SOCKET, SO_BROADCAST, (char*)&bOptVal, bOptLen);

	bind_status = bind(socket_descriptor, (struct sockaddr*)&server_address, sizeof(server_address));
	if (bind_status WIN(== SOCKET_ERROR)NIX(!= 0)) {
		cout << "����� �� ����� ���� ��������!\n";
		WIN(cout << "��� ������: " << WSAGetLastError() << '\n';)
		return -1;
	}

	// �������� ������ �� ����� ������ 
	connection_status = listen(socket_descriptor, 5);
	if (connection_status WIN(== SOCKET_ERROR)NIX(!= 0)) {
		cout << "������ �� ����� ���������� ����� ����������!\n";
		WIN(cout << "��� ������: " << WSAGetLastError() << '\n';)
		return -1;
	}

	return 0;
}


void server_stop() {	// ��������� �����, ��������� ����������
	WIN(closesocket)NIX(close)(socket_descriptor);
	WIN(WSACleanup());
}


void ConsoleWaitMessage() {
	cin.clear();
	//cin.ignore(numeric_limits<streamsize>::max(), '\n');	// ������� ������ �������
	cout << "������� �������: ";
}


int32_t SendResponse(const SOCKET connection_descriptor, const char* local_response_msg, const int32_t local_response_msg_length) {
	int32_t iResult =	WIN(send(connection_descriptor, local_response_msg, local_response_msg_length, 0))	// ���������� �����
						NIX(write(connection_descriptor, local_response_msg, local_response_msg_length));

	if (iResult WIN(== SOCKET_ERROR)NIX(!= 0)) {	// ���� �������� > 0  ����, ������ ��������� ������ �������
		cout << "������ �������� ������ �������!\n";
		WIN(cout << "��� ������: " << WSAGetLastError() << '\n';)
	}

	return iResult;
}


int32_t ReadRequest(const SOCKET connection_descriptor) {
	int32_t iResult =	WIN(recv(connection_descriptor, request_msg, BUFFER_LENGTH, 0))	// �������� �����
						NIX(read(connection_descriptor, request_msg, BUFFER_LENGTH));

	if (iResult WIN(== SOCKET_ERROR)NIX(!= 0)) {	// ���� �������� > 0  ����, ������ ���� ������ �������
		cout << "������ ��������� ������ �� �������!\n";
		WIN(cout << "��� ������: " << WSAGetLastError() << '\n';)
	}

	return iResult;
}


void MainRequestHandler(const SOCKET connection_descriptor) {
	string login, name, receiver, text;
	int32_t iResult, response_msg_length = 1;
	uint32_t count;
	uint8_t command_code = request_msg[0];

	WIN(memset(response_msg, 0, BUFFER_LENGTH))NIX(bzero(response_msg, BUFFER_LENGTH));
	response_msg[0] = 1;	// ������������� �������� ������ �� ���������

	switch (command_code) {
		case GCMD_REG:	// ������� reg - ������������������
			name = &request_msg[1];	// ������ ������ ������������
			login = &request_msg[1 + login.length()  + 1];
			iResult = users.NewUser(name, login, (SHA1PwdArray&)request_msg[1 + name.length() + 1 + login.length() + 1]);
			if (iResult > 0) {	// ����������� ������ �������
				users_map.find(connection_descriptor)->second = iResult;	// ��������� id ������������ � �������
				iResult = 1;
			}
			response_msg[0] = (int8_t)iResult;	// ����������� �����
			break;
		case GCMD_LOGIN:	// ������� login - ����
			login = &request_msg[1];

			if (users.IsLogged(login)) {
				response_msg[0] = ERR_ALREADY_LOGGED;	// ������������ � ����� ������ ��� �����
				break;
			}

			iResult = users.Login(login, (SHA1PwdArray&)request_msg[1 + login.length() + 1]);	// ������� ������ �� �������� ���e�� � ������� ����
			if (iResult > 0) {	// login ������ �������
				users_map.find(connection_descriptor)->second = iResult;	// ��������� id ������������ � �������
				iResult = 1;
			}
			response_msg[0] = (int8_t)iResult;	// ����������� �����
			cout << '\n' << GetTime() << " �� ������ #" << connection_descriptor << " ����� � ��� ������������ '" << login << "'\n";
			ConsoleWaitMessage();
			break;
		case LCMD_SEND:
			receiver = &request_msg[1];
			text = &request_msg[1 + receiver.length() + 1];
			iResult = messages.Add(users_map.at(connection_descriptor),
					users.GetUserLogin(users_map.at(connection_descriptor)),	// �������� login ����������� �� ��� id
					users.GetUserId(receiver),	// �������� id ����������
					receiver,
					text);
			break;
		case LCMD_READ_ALL:
		case LCMD_READ_UNREADED: {
				bool showUnReadedOnly = (command_code == LCMD_READ_UNREADED ? true : false);
				receiver = &request_msg[1];
				uint32_t receiverId = users.GetUserId(receiver);	// �������� id ����������
				for (auto i = 0; i < messages.GetLastMsgId(); i++)	// ���������� ��� ����������
					if (messages.ReadByOne(text, receiverId, i, showUnReadedOnly)) {
						iResult = SendResponse(connection_descriptor, text.c_str(), text.length() + 1);	// ���������� ������
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
				for (auto i = 1; i < users.GetUsersQty(); i++)	// ���������� ���� �������������, ����� ���������� (�0)
					if (users.ListByOne(i, text)) {
						iResult = SendResponse(connection_descriptor, text.c_str(), text.length() + 1);	// ���������� ������
						ReadRequest(connection_descriptor);
					}
				response_msg[0] = 0;
			}
			break;
		case LCMD_USER_DELETE:
			users.DeleteUser(users_map.at(connection_descriptor));	// �������� - id ������������
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
		case LCMD_LOGOUT:	// ��������� ������� logout - �����
			users.Logout(users_map.at(connection_descriptor));	// �������� - id ������������
			cout << '\n' << GetTime() << " �� ������ #" << connection_descriptor << " ����� �� ���� ������������ '" << users.GetUserLogin(users_map.at(connection_descriptor)) << "'\n";
			ConsoleWaitMessage();
			users_map.find(connection_descriptor)->second = 0;
			break;
		case LCMD_IS_USER_EXIST:
			iResult = users.GetUserId(&request_msg[1]);
			response_msg[0] = (int8_t)(iResult > 0 ? 1 : 0);
			break;
		case LCMD_MESSAGES_COUNT:	// ��� ���������
		case LCMD_UNREADED_COUNT:	// ������ �������������
			count = messages.GetMessagesQty(users_map.at(connection_descriptor), command_code == LCMD_UNREADED_COUNT);
			response_msg[0] = (int8_t)(count > 0 ? 1 : 0);
			memcpy(&response_msg[1], &count, sizeof(count));	// �������� � ����� ���-�� ������������� ���������
			response_msg_length += sizeof(count);
			break;
		default:
			;
	}

	iResult = SendResponse(connection_descriptor, response_msg, response_msg_length);	// ���������� �����
}


int InitializeServer() {
	cout << "������������� ������� ����... ";
	if (server_start() == 0)
		cout << "���������!\n";
	else {
		cout << "������ ������������� ������� ����; ��������� ����� �������.\n";
		loop = false;
		return 1;
	}

	cout << "�������� ���� ������ ������������� �� �������� �����... ";
	if (users.Open())	// ��������� ������ �������������
		cout << "���������!\n";
	else if (QuestionYN("������� ������?")) {
		loop = false;
		return 2;	// ��������� ����� �� ������� ������������
	}

	cout << "�������� ���� ������ ��������� �� �������� �����... ";
	if (messages.Open())	// ��������� ���������
		cout << "���������!\n";
	else if (QuestionYN("������� ������?")) {
		loop = false;
		return 3;	// ��������� ����� �� ������� ������������
	}

	AtStart();
	return 0;
}


int ChatCommandsHandler() {	// ���������� ���������� ������ ����
	string command, text;

	while (loop) {
		ConsoleWaitMessage();
		getline(cin, command);

		if (!command.empty()) {
			switch (get_command_code(SERVER_COMMANDS, command)) {	// ����������� ��������� �������
				case SCMD_SAVE:
					if (users.Save() && messages.Save())
						cout << "������ ������������� � ��������� ���������.\n";
					break;
				case SCMD_CON:
					if (!users_map.empty()) {
						cout << "������ �����������:\n   ����� : ������� ���\n";
						for (map <SOCKET, uint32_t>::iterator it = users_map.begin(); it != users_map.end(); it++) {
							text = it->second != 0 ? users.GetUserLogin(users_map.at(it->first)) : "<���� �� ��������>";
							cout << "     " << it->first << " : " << text << "\n";
						}
					}
					else
						cout << "��� �����������.\n";
					break;
				case SCMD_USERS:
					cout << "������ �������������:\n";
					for (auto i = 1; i < users.GetUsersQty(); i++)	// ���������� ���� �������������, ����� ���������� (�0)
						if (users.ListByOne(i, text))
							cout << text;
					break;
				case SCMD_HELP:
					print_help(SERVER_COMMANDS);
					break;
				case SCMD_EXIT:
					if (users.IsChanged())	// ���� ���� ���������, ��������� ������ ������������� �� ������� ����
						users.Save();
					if (messages.IsChanged())	// ���� ���� ���������, ��������� ��������� �� ������� ����
						messages.Save();
					if (!users_map.empty()) {
						if (QuestionYN("���� ����������� �����������. �� ����� ������� ������?"))
							loop = false;	// �������������� ��������� �������
					}
					else if (QuestionYN("����������� ��������� �������"))
						loop = false;	// �������������� ��������� �������

					break;
				default:
					cout << MSG_ERR_CMD;
			}
		}
		WIN(Sleep(DELAY_MS))NIX(usleep(DELAY_MCS));
	}

	return 0;
}


int ChatServer() {	// ������� ���������� ����
	while (loop) {	// Communication Establishment
		FD_ZERO(&descriptors_set);
		FD_SET(socket_descriptor, &descriptors_set);

		int iResult = select(0, &descriptors_set, nullptr, nullptr, &timeout);	// ��������� ���������� ������ �� ����� �����������

		if (iResult > 0 && users_map.size() < FD_SETSIZE) {
			struct sockaddr_in client_address;
			socklen_t length = sizeof(client_address);
			SOCKET socket = accept(socket_descriptor, (struct sockaddr*)&client_address, &length);
			if (socket WIN(== INVALID_SOCKET)NIX(< 0)) {
				cout << "\n������ �� ����� ��������� ������ �� �������!\n";
				WIN(cout << "��� ������: " << WSAGetLastError() << '\n';)
			}
			else {
				users_map.emplace(socket, 0);
				cout << '\n' << GetTime() << " ����������� ������ #" << socket << '\n';
			}
			ConsoleWaitMessage();
		}
		else {
			if (iResult WIN(== SOCKET_ERROR)NIX(!= 0)) {
				WIN(cout << "��� ������: " << WSAGetLastError() << '\n';);
				ConsoleWaitMessage();
			}

			WIN(Sleep(DELAY_MS))NIX(usleep(DELAY_MCS));

			if (users_map.size() == 0)
				continue;
		}

		for (map <SOCKET, uint32_t>::iterator it = users_map.begin(); it != users_map.end(); ) {
			FD_ZERO(&descriptors_set);
			FD_SET(it->first, &descriptors_set);

			iResult = select(0, nullptr, nullptr, &descriptors_set, &timeout);	// ��������� ����� �� ������
			if (iResult > 0) {
				WIN(cout << "\n���� ������ � ����������� �������\t" << it->first <<
					"\n��� ������ : " << WSAGetLastError() << '\n';);
				ConsoleWaitMessage();
			}

			descriptors_set.fd_count = 1;
			iResult = select(0, &descriptors_set, nullptr, nullptr, &timeout);	// ��������� ���������� ������ �� ������
			if (iResult > 0) {
				WIN(memset(request_msg, 0, BUFFER_LENGTH))NIX(bzero(request_msg, BUFFER_LENGTH));
				iResult = recv(it->first, request_msg, BUFFER_LENGTH, 0);	// ��������� ������
				
				if (iResult > 0) {
					if (strcmp(request_msg, CLIENT_STOP_MESSAGE) == 0) {
						cout << '\n' << GetTime() << " ������ #" << it->first << " ����������\n";
						ConsoleWaitMessage();
						it = users_map.erase(it);
						continue;
					}

					MainRequestHandler(it->first);	// ������������ �������� ������ �� ������ � ���������� ������
				}
				else if (iResult WIN(== SOCKET_ERROR)NIX(!= 0)) {	// ������ "���������"
					cout << '\n' << GetTime() << " ����� � �������� #" << it->first << " ��������\n";
					ConsoleWaitMessage();
					users.Logout(users_map.at(it->first));	// �������� ������������ ��� ������������
					it = users_map.erase(it);
					continue;
				}
			}
			it++;
		}
		WIN(Sleep(DELAY_MS))NIX(usleep(DELAY_MCS));
	}

	// ��������� ������� � �����
	server_stop();

	return 0;
}

