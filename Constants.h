#pragma once


struct CommandSpace {	// ��������� ��������� ������ ������� - ���������
	const uint8_t code;
	const char* command;
	const char* command2;	// ����������� ������� ������ ��� ������� - ����������������!
	const char* help;
};


struct ErrorSpace {	// ��������� ��������� ������ ��� ������ - ���������
	const int8_t code;
	const char* text;
};


// ���� ������
const uint8_t
	// ���������
		SCMD_SAVE = 1,
		SCMD_CON = 2,
		SCMD_USERS = 3,
		SCMD_HELP = 4,
		SCMD_EXIT = 5,
	// ����������
		GCMD_REG = 20,
		GCMD_LOGIN = 21,
		GCMD_HELP = 22,
		GCMD_QUIT = 23,
	// ���������
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
	// ������������
		SLCMD_CHANGE_NAME = 90,
		SLCMD_CHANGE_LOGIN = 91,
		SLCMD_CHANGE_PASSWORD = 92,
		SLCMD_HELP = 93,
		SLCMD_EXIT = 94;


const CommandSpace SERVER_COMMANDS[]{	// ������������ ��������� ������ � �����������
	{SCMD_SAVE, "save", "s", "��������� ������ ������������� � ���������"},
	{SCMD_CON, "con", "c", "������� ������ �����������"},
	{SCMD_USERS, "users", "u", "������� ������ �������������"},
	{SCMD_HELP, "help", "?", "������� ������� �� ��������"},
	{SCMD_EXIT, "exit", "e", "������� ������"},
	{0, "", "", ""}
};

const CommandSpace GLOBAL_COMMANDS[] {	// ������������ ���������� ������ � �����������
	{GCMD_REG, "reg", "r", "������������������"},
	{GCMD_LOGIN, "login", "l+", "����� � ���"},
	{GCMD_HELP, "help", "?", "������� ������� �� ��������"},
	{GCMD_QUIT, "quit", "q", "������� ���"},
	{0, "", "", ""}
};

const CommandSpace LOCAL_COMMANDS[] {	// ������������ ��������� ������ � �����������
	{LCMD_SEND, "send", "s", "��������� ���������"},
	{LCMD_READ, "read", "r", "��������� ���������"},
	{LCMD_USER_INFO, "info", "i", "������� ���������� � ������������"},
	{LCMD_USERS_LIST, "list", "l", "������� ������ ���� �������������"},
	{LCMD_USER_DELETE, "delete", "del", "������� ������� ������ ������������"},
	{LCMD_USER_CHANGE, "change", "ch", "�������� ������� ������ ������������"},
	{LCMD_HELP, "help", "?", "������� ������� �� ��������"},
	{LCMD_LOGOUT, "logout", "l-", "����� �� ����"},
	{0, "", "", ""}
};

const CommandSpace SUBLOCAL_COMMANDS[] {	// ��������������� ��������� ������ ��� ��������� ��������������� ������ � �����������
	{SLCMD_CHANGE_NAME, "name", "n", "�������� ��� ������������"},
	{SLCMD_CHANGE_LOGIN, "login", "l", "�������� ������� ��� (login) ������������"},
	{SLCMD_CHANGE_PASSWORD, "pwd", "p", "�������� ������ ������������"},
	{SLCMD_HELP, "help", "?", "������� ������� �� ��������"},
	{SLCMD_EXIT, "exit", "e", "����� �� ����� ����"},
	{0, "", "", ""}
};

// ���� ������
const int8_t
	ERR_USER_NOT_FOUND = -1,
	ERR_WRONG_PASSWORD = -2,
	ERR_ALREADY_LOGGED = -3,
	ERR_ALREADY_EXIST = -4,
	ERR_TRY_USE_SERVICE_NAME = -5;


const ErrorSpace ERROR_SPACE[]{
	{ERR_USER_NOT_FOUND, "������������ � ����� login �� ����������!"},
	{ERR_WRONG_PASSWORD, "�������� ������!"},
	{ERR_ALREADY_LOGGED, "������������ � ����� login ��� �������� ���� � ��� � ������� ������!"},
	{ERR_ALREADY_EXIST, "������������ � ����� login ��� ���������������!"},
	{ERR_TRY_USE_SERVICE_NAME, "���������������� ��� ����� ������ ������!"},
	{0, ""}
};


const int
	PORT_NUMBER = 7777, // ����� ������������ ���� ����� �����
	DELAY_MS = 400,	// �������� ��� ������� � ��
	DELAY_MCS = DELAY_MS * 1'000;	// �������� ��� ������� � ���


const size_t
	MIN_NAME_LENGTH = 2,	// ����������� ����� ����� ������������
	MAX_NAME_LENGTH = 32,	// ������������ ����� ����� ������������
	MIN_LOGIN_LENGTH = 2,	// ����������� ����� login
	MAX_LOGIN_LENGTH = 16,	// ������������ ����� login
	MIN_PASSWORD_LENGTH = 3,	// ����������� ����� ������
	MAX_PASSWORD_LENGTH = 8,	// ������������ ����� ������
	BUFFER_LENGTH = 1024, // ������������ ������ ������ ��� ������
	MESSAGE_LENGTH = 512, // ������������ ������ ���������
	INITIAL_COUNT = 2,	// ��������� ����� ������� � ������� �������������; ����������� ��������=2 ������ ������ ��� ������������!!
	INITIAL_MSG_COUNT = 2;	// ��������� ����� ������� � �����������; ����������� ��������=2 ������ ������ ��� ������������!!


const char
	SERVER_ADDRESS[]{ "127.0.0.1" },
	CLIENT_STOP_MESSAGE[]{ "stop" },
	SERVICE_NAME[]{ "��� ������������" },	// ��� ���������� ������������ � �������� 0
	SERVICE_LOGIN[]{ "all" },	// login ���������� ������������ � �������� 0
	USER_DATA_FILE_NAME[]{ "users.json" },	// ��� ����� ��� �������� ������ �������������
	MSG_DATA_FILE_NAME[]{ "msgs.json" },	// ��� ����� ��� �������� ���������
	SEPARATOR[]{ "-------------------------------------------------------------------------\n" },
	MSG_ERR_CMD[]{ "������� �� ����������, ��������� ����.\n" },
	DATE_TIME_FORMAT[]{ "[%Y-%m-%d %H:%M:%S]" };
