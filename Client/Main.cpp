#include <iostream>
#include <string>
#include <windows.h>
#include "Client.h"


using namespace std;


int main() {
	system("chcp 1251");

	// инициализация клиента чата
	cout << "Подключение к серверу...";

	if (client_start() == 0)
		cout << "выполнено!\n";
	else {
		cout << "Ошибка подключения к серверу чата; программа будет закрыта.\n";
		return 1;
	}

	int result = Chat();	// запуск работы чата; можно получить результат (зарезервировано)
	
	// остановка клиента и выход
	client_stop();

	return result;
}