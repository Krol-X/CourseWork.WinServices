//
// ЗАГОЛОВОК: ENGINE.H
//
// ОПИСАНИЕ: реализация работы сервера и клиента
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//
#include "tcpsock.h"


//
// СТРУКТУРА: ListItem
//
// СОДЕРЖИМОЕ: элемент распакованного списка клиента
//
struct ListItem {
	BYTE state;
	char *name, *viewname;
	char data[];
};


//
// СТРУКТУРА: Datagram
//
// СОДЕРЖИМОЕ: передоваемая комманда и данные
//
struct Datagram {
	DWORD cmd_cou;
	char data[];
};
#define RESERVED_BYTES 4
#define BUF_SIZE RESERVED_BYTES+65536


//
// КЛАСС: Server
//
// НАЗНАЧЕНИЕ: управление основным потоком сервера
//
class Server {
	private:
		Address a;
		pthread_t thread;
		bool active;
	public:
		Server();
		~Server();
		bool Start();
		bool IsWorking();
		void Stop();
};


//
// КЛАСС: Client
//
// НАЗНАЧЕНИЕ: реализация клиента
//
class Client {
	private:
		Address a;
	public:
		Client();
		~Client();
		bool Init(char *addr);
		ListItem *GetList();
		//ListItem *SetSvc();
};


// Приостановить программу на некоторое время
void wait( float seconds );

// Инициализировать работу с сокетами
bool InitializeSockets();

// Завершить работу с сокетами
void ShutdownSockets();

