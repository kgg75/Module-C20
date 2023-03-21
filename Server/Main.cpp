#include <windows.h>
#include "Server.h"


using namespace std;


int main() {
	system("chcp 1251");

	int iResult = InitializeServer();
	if (iResult != 0)
		return iResult;

	const auto THREADS_NUMBER = 2;
	HANDLE hThreads[THREADS_NUMBER];
	hThreads[0] = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ChatCommandsHandler, 0, 0, 0);	// обработчик команд чата
	hThreads[1] = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ChatServer, 0, 0, 0);	// главный обработчик чата

	if (hThreads[0] == 0 || hThreads[1] == 0)
		return 1;	// ошибка!

	WaitForMultipleObjects(THREADS_NUMBER, hThreads, TRUE, INFINITE);

	DWORD iRetVal[THREADS_NUMBER];
	if (GetExitCodeThread(hThreads[0], &iRetVal[0]))
		CloseHandle(hThreads[0]);
	if (GetExitCodeThread(hThreads[1], &iRetVal[1]))
		CloseHandle(hThreads[1]);

	return iRetVal[1];	// результат ChatServer()
}
