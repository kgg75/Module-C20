#pragma once


struct CommandSpace {	// структура описывает связку команда - подсказка
	const uint8_t code;
	const char* command;
	const char* command2;	// дублирующий вариант набора для отладки - недокументирован!
	const char* help;
};


struct ErrorSpace {	// структура описывает связку код ошибки - сообщение
	const int8_t code;
	const char* text;
};


// коды команд
const uint8_t
	// серверные
		SCMD_SAVE = 1,
		SCMD_CON = 2,
		SCMD_USERS = 3,
		SCMD_HELP = 4,
		SCMD_EXIT = 5,
	// глобальные
		GCMD_REG = 20,
		GCMD_LOGIN = 21,
		GCMD_HELP = 22,
		GCMD_QUIT = 23,
	// локальные
		LCMD_SEND = 50,
		LCMD_READ = 51,
		LCMD_READ_ALL = 52,
		LCMD_READ_UNREADED = 53,
		LCMD_USER_INFO = 54,
		LCMD_USERS_LIST = 55,
		LCMD_USER_DELETE = 56,
		LCMD_USER_CHANGE = 57,
		LCMD_HELP = 58,
		LCMD_LOGOUT = 59,
		LCMD_IS_USER_EXIST = 70,
		LCMD_MESSAGES_COUNT = 71,
		LCMD_UNREADED_COUNT = 72,
	// сублокальные
		SLCMD_CHANGE_NAME = 90,
		SLCMD_CHANGE_LOGIN = 91,
		SLCMD_CHANGE_PASSWORD = 92,
		SLCMD_HELP = 93,
		SLCMD_EXIT = 94;


const CommandSpace SERVER_COMMANDS[]{	// пространство серверных команд с подсказками
	{SCMD_SAVE, "save", "s", "сохранить данные пользователей и сообщений"},
	{SCMD_CON, "con", "c", "вывести список подключений"},
	{SCMD_USERS, "users", "u", "вывести список пользователей"},
	{SCMD_HELP, "help", "?", "вывести справку по командам"},
	{SCMD_EXIT, "exit", "e", "закрыть сервер"},
	{0, "", "", ""}
};

const CommandSpace GLOBAL_COMMANDS[] {	// пространство глобальных команд с подсказками
	{GCMD_REG, "reg", "r", "зарегистрироваться"},
	{GCMD_LOGIN, "login", "l+", "войти в чат"},
	{GCMD_HELP, "help", "?", "вывести справку по командам"},
	{GCMD_QUIT, "quit", "q", "закрыть чат"},
	{0, "", "", ""}
};

const CommandSpace LOCAL_COMMANDS[] {	// пространство локальных команд с подсказками
	{LCMD_SEND, "send", "s", "отправить сообщение"},
	{LCMD_READ, "read", "r", "прочитать сообщения"},
	{LCMD_USER_INFO, "info", "i", "вывести информацию о пользователе"},
	{LCMD_USERS_LIST, "list", "l", "вывести список всех пользователей"},
	{LCMD_USER_DELETE, "delete", "del", "удалить учётную запись пользователя"},
	{LCMD_USER_CHANGE, "change", "ch", "изменить учётные данные пользователя"},
	{LCMD_HELP, "help", "?", "вывести справку по командам"},
	{LCMD_LOGOUT, "logout", "l-", "выйти из чата"},
	{0, "", "", ""}
};

const CommandSpace SUBLOCAL_COMMANDS[] {	// подпространство локальных команд для изменения регистрационных данных с подсказками
	{SLCMD_CHANGE_NAME, "name", "n", "изменить имя пользователя"},
	{SLCMD_CHANGE_LOGIN, "login", "l", "изменить учётное имя (login) пользователя"},
	{SLCMD_CHANGE_PASSWORD, "pwd", "p", "изменить пароль пользователя"},
	{SLCMD_HELP, "help", "?", "вывести справку по командам"},
	{SLCMD_EXIT, "exit", "e", "выйти из этого меню"},
	{0, "", "", ""}
};

// коды ошибок
const int8_t
	ERR_USER_NOT_FOUND = -1,
	ERR_WRONG_PASSWORD = -2,
	ERR_ALREADY_LOGGED = -3,
	ERR_ALREADY_EXIST = -4,
	ERR_TRY_USE_SERVICE_NAME = -5;


const ErrorSpace ERROR_SPACE[]{
	{ERR_USER_NOT_FOUND, "Пользователь с таким login не существует!"},
	{ERR_WRONG_PASSWORD, "Неверный пароль!"},
	{ERR_ALREADY_LOGGED, "Пользователь с таким login уже выполнил вход в чат с другого адреса!"},
	{ERR_ALREADY_EXIST, "Пользователь с таким login уже зарегистрирован!"},
	{ERR_TRY_USE_SERVICE_NAME, "Регистрироваться под таким именем нельзя!"},
	{0, ""}
};


const int
	PORT_NUMBER = 7777, // Будем использовать этот номер порта
	DELAY_MS = 400,	// задержка для сервера в мс
	DELAY_MCS = DELAY_MS * 1'000;	// задержка для сервера в мкс


const size_t
	MIN_NAME_LENGTH = 2,	// минимальная длина имени пользователя
	MAX_NAME_LENGTH = 32,	// максимальная длина имени пользователя
	MIN_LOGIN_LENGTH = 2,	// минимальная длина login
	MAX_LOGIN_LENGTH = 16,	// максимальная длина login
	MIN_PASSWORD_LENGTH = 3,	// минимальная длина пароля
	MAX_PASSWORD_LENGTH = 8,	// максимальная длина пароля
	BUFFER_LENGTH = 1024, // Максимальный размер буфера для данных
	MESSAGE_LENGTH = 512, // Максимальный размер сообщения
	INITIAL_COUNT = 2,	// начальная длина массива с данными пользователей; минимальное значение=2 задано только для тестирования!!
	INITIAL_MSG_COUNT = 2;	// начальная длина массива с сообщениями; минимальное значение=2 задано только для тестирования!!


const char
	SERVER_ADDRESS[]{ "127.0.0.1" },
	CLIENT_STOP_MESSAGE[]{ "stop" },
	SERVICE_NAME[]{ "все пользователи" },	// имя служебного пользователя с индексом 0
	SERVICE_LOGIN[]{ "all" },	// login служебного пользователя с индексом 0
	USER_DATA_FILE_NAME[]{ "users.json" },	// имя файла для хранения данных пользователей
	MSG_DATA_FILE_NAME[]{ "msgs.json" },	// имя файла для хранения сообщений
	SEPARATOR[]{ "-------------------------------------------------------------------------\n" },
	MSG_ERR_CMD[]{ "Команда не распознана, повторите ввод.\n" },
	DATE_TIME_FORMAT[]{ "[%Y-%m-%d %H:%M:%S]" };
