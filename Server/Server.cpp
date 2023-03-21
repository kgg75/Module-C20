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

	// Ìàêðîñû äëÿ âûðàæåíèé çàâèñèìûõ îò OS    
	#define WIN(exp) exp
	#define NIX(exp)
#elif defined(__linux__)
	#include <unistd.h>
	#include <sys/socket.h> // Áèáëèîòåêà äëÿ ðàáîòû ñ ñîêåòàìè
	//#include <sys/types.h>
	#include <netinet/in.h>
	// Ìàêðîñû äëÿ âûðàæåíèé çàâèñèìûõ îò OS    
	#define WIN(exp)
	#define NIX(exp) exp
#endif


using namespace std;


Users users;
Messages messages;
map <SOCKET, uint32_t> users_map;	// ñâÿçêà ñîêåò - èäåíòèôèêàòîð (id) ïîëüçîâàòåëÿ
char request_msg[BUFFER_LENGTH], response_msg[BUFFER_LENGTH];
SOCKET socket_descriptor;
fd_set descriptors_set{ 0 };
const timeval timeout = { 0, 0 };
bool loop = true;	// ôëàã ïðîäîëæåíèÿ/îñòàíîâêè ChatServer()


void print_help(const CommandSpace* _commandSpace) {	// ïå÷àòü ñïðàâêè ïî êîìàíäàì
	for (int i = 0; _commandSpace[i].command[0] != 0; i++)
		cout << "\t" << _commandSpace[i].command << "\t- " << _commandSpace[i].help << '\n';
}


void AtStart() {	// íà÷àëüíûå ñîîáùåíèÿ
	cout.clear();
	cout << SEPARATOR;
	cout << "---=== ×àò-ñåðâåð âåðñèÿ 1.0 ===---\n";
	cout << SEPARATOR;
	cout << "Ñåðâåð çàïóùåí " << GetTime() << '\n';
	cout << SEPARATOR;
	cout << "Ñïðàâêà ïî êîìàíäàì:\n";
	print_help(SERVER_COMMANDS);
}


int server_start() {	// èíèöèàëèçàöèÿ ñåðâåðà
	struct sockaddr_in server_address;
	int bind_status, connection_status;

	// Ñîçäàäèì ñîêåò
	WIN(WSADATA wsaData = {0}; WSAStartup(MAKEWORD(2, 2), &wsaData);)

	socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_descriptor WIN(== INVALID_SOCKET)NIX(< 0)) {
		cout << "Ñîêåò íå ìîæåò áûòü ñîçäàí!\n";
		WIN(cout << "Êîä îøèáêè: " << WSAGetLastError() << '\n';)
		return -1;
	}

	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(PORT_NUMBER);  // Çàäàäèì íîìåð ïîðòà äëÿ ñâÿçè
	server_address.sin_family = AF_INET;    // Èñïîëüçóåì IPv4

	// Ïðèâÿæåì ñîêåò
	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(BOOL);
	int iResult = setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&bOptVal, bOptLen);
	iResult = setsockopt(socket_descriptor, SOL_SOCKET, SO_BROADCAST, (char*)&bOptVal, bOptLen);

	bind_status = bind(socket_descriptor, (struct sockaddr*)&server_address, sizeof(server_address));
	if (bind_status WIN(== SOCKET_ERROR)NIX(!= 0)) {
		cout << "Ñîêåò íå ìîæåò áûòü ïðèâÿçàí!\n";
		WIN(cout << "Êîä îøèáêè: " << WSAGetLastError() << '\n';)
		return -1;
	}

	// Ïîñòàâèì ñåðâåð íà ïðèåì äàííûõ 
	connection_status = listen(socket_descriptor, 5);
	if (connection_status WIN(== SOCKET_ERROR)NIX(!= 0)) {
		cout << "Ñåðâåð íå ìîæåò óñòàíîâèòü íîâîå ñîåäèíåíèå!\n";
		WIN(cout << "Êîä îøèáêè: " << WSAGetLastError() << '\n';)
		return -1;
	}

	return 0;
}


void server_stop() {	// çàêðûâàåì ñîêåò, çàâåðøàåì ñîåäèíåíèå
	WIN(closesocket)NIX(close)(socket_descriptor);
	WIN(WSACleanup());
}


void ConsoleWaitMessage() {
	cin.clear();
	//cin.ignore(numeric_limits<streamsize>::max(), '\n');	// î÷èñòêà áóôåðà êîíñîëè
	cout << "Ââåäèòå êîìàíäó: ";
}


int32_t SendResponse(const SOCKET connection_descriptor, const char* local_response_msg, const int32_t local_response_msg_length) {
	int32_t iResult =	WIN(send(connection_descriptor, local_response_msg, local_response_msg_length, 0))	// îòïðàâëÿåì îòâåò
						NIX(write(connection_descriptor, local_response_msg, local_response_msg_length));

	if (iResult WIN(== SOCKET_ERROR)NIX(!= 0)) {	// Åñëè ïåðåäàëè > 0  áàéò, çíà÷èò ïåðåñûëêà ïðîøëà óñïåøíî
		cout << "Îøèáêà îòïðàâêè äàííûõ êëèåíòó!\n";
		WIN(cout << "Êîä îøèáêè: " << WSAGetLastError() << '\n';)
	}

	return iResult;
}


int32_t ReadRequest(const SOCKET connection_descriptor) {
	int32_t iResult =	WIN(recv(connection_descriptor, request_msg, BUFFER_LENGTH, 0))	// ïîëó÷àåì îòâåò
						NIX(read(connection_descriptor, request_msg, BUFFER_LENGTH));

	if (iResult WIN(== SOCKET_ERROR)NIX(!= 0)) {	// Åñëè ïîëó÷èëè > 0  áàéò, çíà÷èò ïðè¸ì ïðîø¸ë óñïåøíî
		cout << "Îøèáêà ïîëó÷åíèÿ äàííûõ îò êëèåíòà!\n";
		WIN(cout << "Êîä îøèáêè: " << WSAGetLastError() << '\n';)
	}

	return iResult;
}


void MainRequestHandler(const SOCKET connection_descriptor) {
	string login, name, receiver, text;
	int32_t iResult, response_msg_length = 1;
	uint32_t count;
	uint8_t command_code = request_msg[0];

	WIN(memset(response_msg, 0, BUFFER_LENGTH))NIX(bzero(response_msg, BUFFER_LENGTH));
	response_msg[0] = 1;	// óñòàíàâëèâàåì çíà÷åíèå îòâåòà ïî óìîë÷àíèþ

	switch (command_code) {
		case GCMD_REG:	// êîìàíäà reg - çàðåãèñòðèðîâàòüñÿ
			name = &request_msg[1];	// ñîçäà¸ì íîâîãî ïîëüçîâàòåëÿ
			login = &request_msg[1 + login.length()  + 1];
			iResult = users.NewUser(name, login, (SHA1PwdArray&)request_msg[1 + name.length() + 1 + login.length() + 1]);
			if (iResult > 0) {	// ðåãèñòðàöèÿ ïðîøëà óñïåøíî
				users_map.find(connection_descriptor)->second = iResult;	// äîáàâëÿåì id ïîëüçîâàòåëÿ â ñëîâàðü
				iResult = 1;
			}
			response_msg[0] = (int8_t)iResult;	// ïîäãîòîâèëè îòâåò
			break;
		case GCMD_LOGIN:	// êîìàíäà login - âõîä
			login = &request_msg[1];

			if (users.IsLogged(login)) {
				response_msg[0] = ERR_ALREADY_LOGGED;	// ïîëüçîâàòåëü ñ òàêèì èìåíåì óæå âîø¸ë
				break;
			}

			iResult = users.Login(login, (SHA1PwdArray&)request_msg[1 + login.length() + 1]);	// ïåðåäà¸ì ññûëêó íà ôðàãìåíò áóôeðà ñ äàííûìè õýøà
			if (iResult > 0) {	// login ïðîø¸ë óñïåøíî
				users_map.find(connection_descriptor)->second = iResult;	// äîáàâëÿåì id ïîëüçîâàòåëÿ â ñëîâàðü
				iResult = 1;
				cout << '\n' << GetTime() << " íà ñîêåòå #" << connection_descriptor << " âîø¸ë â ÷àò ïîëüçîâàòåëü '" << login << "'\n";
				ConsoleWaitMessage();
			}
			response_msg[0] = (int8_t)iResult;	// ïîäãîòîâèëè îòâåò

			break;
		case LCMD_SEND:
			receiver = &request_msg[1];
			text = &request_msg[1 + receiver.length() + 1];
			iResult = messages.Add(users_map.at(connection_descriptor),
					users.GetUserLogin(users_map.at(connection_descriptor)),	// ïîëó÷àåì login îòïðàâèòåëÿ ïî åãî id
					users.GetUserId(receiver),	// ïîëó÷àåì id ïîëó÷àòåëÿ
					receiver,
					text);
			break;
		case LCMD_READ_ALL:
		case LCMD_READ_UNREADED: {
				bool showUnReadedOnly = (command_code == LCMD_READ_UNREADED ? true : false);
				receiver = &request_msg[1];
				uint32_t receiverId = users.GetUserId(receiver);	// ïîëó÷àåì id ïîëó÷àòåëÿ
				for (auto i = 0; i < messages.GetLastMsgId(); i++)	// ïåðåáèðàåì âñå ñîîîáùåíèÿ
					if (messages.ReadByOne(text, receiverId, i, showUnReadedOnly)) {
						iResult = SendResponse(connection_descriptor, text.c_str(), text.length() + 1);	// îòïðàâëÿåì îòâåòû
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
				for (auto i = 1; i < users.GetUsersQty(); i++)	// ïåðåáèðàåì âñåõ ïîëüçîâàòåëåé, êðîìå ñëóæåáíîãî (¹0)
					if (users.ListByOne(i, text)) {
						iResult = SendResponse(connection_descriptor, text.c_str(), text.length() + 1);	// îòïðàâëÿåì îòâåòû
						ReadRequest(connection_descriptor);
					}
				response_msg[0] = 0;
			}
			break;
		case LCMD_USER_DELETE:
			users.DeleteUser(users_map.at(connection_descriptor));	// àðãóìåíò - id ïîëüçîâàòåëÿ
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
		case LCMD_LOGOUT:	// ëîêàëüíàÿ êîìàíäà logout - âûõîä
			users.Logout(users_map.at(connection_descriptor));	// àðãóìåíò - id ïîëüçîâàòåëÿ
			cout << '\n' << GetTime() << " íà ñîêåòå #" << connection_descriptor << " âûøåë èç ÷àòà ïîëüçîâàòåëü '" << users.GetUserLogin(users_map.at(connection_descriptor)) << "'\n";
			ConsoleWaitMessage();
			users_map.find(connection_descriptor)->second = 0;
			break;
		case LCMD_IS_USER_EXIST:
			iResult = users.GetUserId(&request_msg[1]);
			response_msg[0] = (int8_t)(iResult > 0 ? 1 : 0);
			break;
		case LCMD_MESSAGES_COUNT:	// âñå ñîîáùåíèÿ
		case LCMD_UNREADED_COUNT:	// òîëüêî íåïðî÷èòàííûå
			count = messages.GetMessagesQty(users_map.at(connection_descriptor), command_code == LCMD_UNREADED_COUNT);
			response_msg[0] = (int8_t)(count > 0 ? 1 : 0);
			memcpy(&response_msg[1], &count, sizeof(count));	// ïîìåùàåì â áóôåð êîë-âî íåïðî÷èòàííûõ ñîîáùåíèé
			response_msg_length += sizeof(count);
			break;
		default:
			;
	}

	iResult = SendResponse(connection_descriptor, response_msg, response_msg_length);	// îòïðàâëÿåì îòâåò
}


int InitializeServer() {
	cout << "Èíèöèàëèçàöèÿ ñåðâåðà ÷àòà... ";
	if (server_start() == 0)
		cout << "âûïîëíåíà!\n";
	else {
		cout << "Îøèáêà èíèöèàëèçàöèè ñåðâåðà ÷àòà; ïðîãðàììà áóäåò çàêðûòà.\n";
		loop = false;
		return 1;
	}

	cout << "Çàãðóçêà áàçû äàííûõ ïîëüçîâàòåëåé èç âíåøíåãî ôàéëà... ";
	if (users.Open())	// çàãðóæàåì äàííûå ïîëüçîâàòåëåé
		cout << "âûïîëíåíà!\n";
	else if (QuestionYN("Çàêðûòü ñåðâåð?")) {
		loop = false;
		return 2;	// àâàðèéíûé âûõîä ïî æåëàíèþ ïîëüçîâàòåëÿ
	}

	cout << "Çàãðóçêà áàçû äàííûõ ñîîáùåíèé èç âíåøíåãî ôàéëà... ";
	if (messages.Open())	// çàãðóæàåì ñîîáùåíèÿ
		cout << "âûïîëíåíà!\n";
	else if (QuestionYN("Çàêðûòü ñåðâåð?")) {
		loop = false;
		return 3;	// àâàðèéíûé âûõîä ïî æåëàíèþ ïîëüçîâàòåëÿ
	}

	AtStart();
	return 0;
}


int ChatCommandsHandler() {	// îáðàáîò÷èê êîíñîëüíûõ êîìàíä ÷àòà
	string command, text;

	while (loop) {
		ConsoleWaitMessage();
		getline(cin, command);

		if (!command.empty()) {
			switch (get_command_code(SERVER_COMMANDS, command)) {	// àíàçèçèðóåì ñåðâåðíóþ êîìàíäó
				case SCMD_SAVE:
					if (users.Save() && messages.Save())
						cout << "Äàííûå ïîëüçîâàòåëåé è ñîîáùåíèé ñîõðàíåíû.\n";
					break;
				case SCMD_CON:
					if (!users_map.empty()) {
						cout << "Ñïèñîê ïîäêëþ÷åíèé:\n   ñîêåò : ó÷¸òíîå èìÿ\n";
						for (map <SOCKET, uint32_t>::iterator it = users_map.begin(); it != users_map.end(); it++) {
							text = it->second != 0 ? users.GetUserLogin(users_map.at(it->first)) : "<âõîä íå âûïîëíåí>";
							cout << "     " << it->first << " : " << text << "\n";
						}
					}
					else
						cout << "Íåò ïîäêëþ÷åíèé.\n";
					break;
				case SCMD_USERS:
					cout << "Ñïèñîê ïîëüçîâàòåëåé:\n";
					for (auto i = 1; i < users.GetUsersQty(); i++)	// ïåðåáèðàåì âñåõ ïîëüçîâàòåëåé, êðîìå ñëóæåáíîãî (¹0)
						if (users.ListByOne(i, text))
							cout << text;
					break;
				case SCMD_HELP:
					print_help(SERVER_COMMANDS);
					break;
				case SCMD_EXIT:
					if (users.IsChanged())	// åñëè åñòü èçìåíåíèÿ, ñîõðàíÿåì äàííûå ïîëüçîâàòåëåé âî âíåøíèé ôàéë
						users.Save();
					if (messages.IsChanged())	// åñëè åñòü èçìåíåíèÿ, ñîõðàíÿåì ñîîáùåíèÿ âî âíåøíèé ôàéë
						messages.Save();
					if (!users_map.empty()) {
						if (QuestionYN("Åñòü äåéñòâóþùèå ïîäêëþ÷åíèÿ. Âñ¸ ðàâíî çàêðûòü ñåðâåð?"))
							loop = false;	// èíèöèàëèçèðóåì îñòàíîâêà ñåðâåðà
					}
					else if (QuestionYN("Ïîäòâåðäèòå îñòàíîâêó ñåðâåðà"))
						loop = false;	// èíèöèàëèçèðóåì îñòàíîâêà ñåðâåðà

					break;
				default:
					cout << MSG_ERR_CMD;
			}
		}
		WIN(Sleep(DELAY_MS))NIX(usleep(DELAY_MCS));
	}

	return 0;
}


int ChatServer() {	// ãëàâíûé îáðàáîò÷èê ÷àòà
	while (loop) {	// Communication Establishment
		FD_ZERO(&descriptors_set);
		FD_SET(socket_descriptor, &descriptors_set);

		int iResult = select(0, &descriptors_set, nullptr, nullptr, &timeout);	// ïðîâåðÿåì ãîòîâíîñòü ñîêåòà íà íîâûå ïîäêëþ÷åíèÿ

		if (iResult > 0 && users_map.size() < FD_SETSIZE) {
			struct sockaddr_in client_address;
			socklen_t length = sizeof(client_address);
			SOCKET socket = accept(socket_descriptor, (struct sockaddr*)&client_address, &length);
			if (socket WIN(== INVALID_SOCKET)NIX(< 0)) {
				cout << "\nÑåðâåð íå ìîæåò ïðèíèìàòü äàííûå îò êëèåíòà!\n";
				WIN(cout << "Êîä îøèáêè: " << WSAGetLastError() << '\n';)
			}
			else {
				users_map.emplace(socket, 0);
				cout << '\n' << GetTime() << " ïîäêëþ÷èëñÿ êëèåíò #" << socket << '\n';
			}
			ConsoleWaitMessage();
		}
		else {
			if (iResult WIN(== SOCKET_ERROR)NIX(!= 0)) {
				WIN(cout << "Êîä îøèáêè: " << WSAGetLastError() << '\n';);
				ConsoleWaitMessage();
			}

			WIN(Sleep(DELAY_MS))NIX(usleep(DELAY_MCS));

			if (users_map.size() == 0)
				continue;
		}

		for (map <SOCKET, uint32_t>::iterator it = users_map.begin(); it != users_map.end(); ) {
			FD_ZERO(&descriptors_set);
			FD_SET(it->first, &descriptors_set);

			iResult = select(0, nullptr, nullptr, &descriptors_set, &timeout);	// ïðîâåðÿåì ñîêåò íà îøèáêè
			if (iResult > 0) {
				WIN(cout << "\nÅñòü îøèáêè â ïîäêëþ÷åíèè êëèåíòà\t" << it->first <<
					"\nÊîä îøèáêè : " << WSAGetLastError() << '\n';);
				ConsoleWaitMessage();
			}

			descriptors_set.fd_count = 1;
			iResult = select(0, &descriptors_set, nullptr, nullptr, &timeout);	// ïðîâåðÿåì ãîòîâíîñòü ñîêåòà íà ÷òåíèå
			if (iResult > 0) {
				WIN(memset(request_msg, 0, BUFFER_LENGTH))NIX(bzero(request_msg, BUFFER_LENGTH));
				iResult = recv(it->first, request_msg, BUFFER_LENGTH, 0);	// ïðèíèìàåì çàïðîñ
				
				if (iResult > 0) {
					if (strcmp(request_msg, CLIENT_STOP_MESSAGE) == 0) {
						cout << '\n' << GetTime() << " êëèåíò #" << it->first << " îòêëþ÷èëñÿ\n";
						ConsoleWaitMessage();
						it = users_map.erase(it);
						continue;
					}

					MainRequestHandler(it->first);	// îáðàáàòûâàåò âõîäÿùèå äàííûå èç áóôåðà è îòïðàâëÿåò îòâåòû
				}
				else if (iResult WIN(== SOCKET_ERROR)NIX(!= 0)) {	// êëèåíò "îòâàëèëñÿ"
					cout << '\n' << GetTime() << " ñâÿçü ñ êëèåíòîì #" << it->first << " ïîòåðÿíà\n";
					ConsoleWaitMessage();
					users.Logout(users_map.at(it->first));	// ïîìå÷àåì ïîëüçîâàòåëÿ êàê îòêëþ÷¸ííîãî
					it = users_map.erase(it);
					continue;
				}
			}
			it++;
		}
		WIN(Sleep(DELAY_MS))NIX(usleep(DELAY_MCS));
	}

	// îñòàíîâêà ñåðâåðà è âûõîä
	server_stop();

	return 0;
}

