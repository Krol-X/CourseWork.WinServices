//
// ���������: ENGINE.H
//
// ��������: ���������� ������ ������� � �������
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//
#include "tcpsock.h"


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
// ���������: Datagram
//
// ����������: ������������ �������� � ������
//
struct Datagram {
	DWORD cmd_cou;
	char data[];
};
#define RESERVED_BYTES 4
#define BUF_SIZE RESERVED_BYTES+65536


//
// �����: Server
//
// ����������: ���������� �������� ������� �������
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
// �����: Client
//
// ����������: ���������� �������
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


// ������������� ��������� �� ��������� �����
void wait( float seconds );

// ���������������� ������ � ��������
bool InitializeSockets();

// ��������� ������ � ��������
void ShutdownSockets();

