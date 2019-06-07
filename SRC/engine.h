//
// ЗАГОЛОВОК: ENGINE.H
//
// ОПИСАНИЕ: реализация работы сервера и клиента
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//

#ifndef engine_h_
#define engine_h_

#include "tcpsock.h"
#define SocketType TcpSocket


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
// СТРУКТУРА: ServerParams
//
// СОДЕРЖИМОЕ: общие параметры для потоков сервера
//
struct ServerParams {
	SocketType sock;
	bool active;
	pthread_mutex_t mutex;
};


//
// КЛАСС: Server
//
// НАЗНАЧЕНИЕ: управление основным потоком сервера
//
class Server {
	private:
		pthread_t thread;
		ServerParams param;
	public:
		bool Start(unsigned short port = 0);
		bool IsWorking();
		void Stop();
		~Server();
};


//
// СТРУКТУРА: ClientParams
//
// СОДЕРЖИМОЕ: общие параметры для потоков клиента
//
struct ClientParams {
	SocketType sock;
	vector<ListItem *> list;
};


//
// КЛАСС: Client
//
// НАЗНАЧЕНИЕ: реализация клиента
//
class Client {
	private:
		ClientParams param;
	public:
	    Client();
		bool GetList(Address addr);
		void SetSvc(Address addr, unsigned int idx, BYTE state);
		unsigned int ListSize();
		ListItem* GetItem(unsigned int idx);
};

// Инициализировать работу с сокетами
bool InitializeSockets();

// Завершить работу с сокетами
void ShutdownSockets();

#endif

