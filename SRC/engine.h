//
// ���������: ENGINE.H
//
// ��������: ���������� ������ ������� � �������
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//

#ifndef engine_h_
#define engine_h_

#include "tcpsock.h"
#define SocketType TcpSocket


//
// ���������: ListItem
//
// ����������: ������� �������������� ������ �������
//
struct ListItem {
	BYTE state;
	char *name, *viewname;
	char data[];
};


//
// ���������: ServerParams
//
// ����������: ����� ��������� ��� ������� �������
//
struct ServerParams {
	SocketType sock;
	bool active;
	pthread_mutex_t mutex;
};


//
// �����: Server
//
// ����������: ���������� �������� ������� �������
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
// ���������: ClientParams
//
// ����������: ����� ��������� ��� ������� �������
//
struct ClientParams {
	SocketType sock;
	vector<ListItem *> list;
};


//
// �����: Client
//
// ����������: ���������� �������
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

// ���������������� ������ � ��������
bool InitializeSockets();

// ��������� ������ � ��������
void ShutdownSockets();

#endif

