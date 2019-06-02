//
// ���������: ENGINE.H
//
// ��������: ���������� ������ ������� � �������
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//
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
};


//
// ���������: ClientParams
//
// ����������: ����� ��������� ��� ������� �������
//
struct ClientParams {
	SocketType sock;
	vector<ListItem> list;
	pthread_mutex_t mutex;
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
		bool GetList(Address addr);
		unsigned int ListSize();
		ListItem* GetItem(unsigned int idx);
		//ListItem *SetSvc();
};

// ���������������� ������ � ��������
bool InitializeSockets();

// ��������� ������ � ��������
void ShutdownSockets();

