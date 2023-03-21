#include <windows.h>
#include <thread>
#include "Server.h"


using namespace std;


int main() {
	system("chcp 1251");

	int iResult = InitializeServer();
	if (iResult != 0)
		return iResult;

	thread chatCommandsThread(ChatCommandsHandler);
	iResult = ChatServer();

	if (chatCommandsThread.joinable())
		chatCommandsThread.join();

	return iResult;
}
