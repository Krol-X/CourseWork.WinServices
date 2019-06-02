//
// ������: ENGINE.CPP
//
// ��������: ���������� ������ ������� � �������
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//
#include "include.h"
#include "engine.h"
#include "services.h"
#include <stdio.h>


////////////////////////////////////////////////////////////////////////////////
// ������: ���������, ��������� � ��������� ����������
//

#define PROTOCOLID *((DWORD *)"\17VS\3")
#define CMD_ANY      0
#define CMD_CHECKOUT 0x80000000
#define CMD_LIST     0x0C000000
#define CMD_SET      0x18000000

//
// ���������: Datagram
//
// ����������: ������������ �������� � ������
//
struct Datagram {
	DWORD id;
	DWORD cmd_cou;
	char data[];
};
#define RESERVED_BYTES 4+4
#define BUF_SIZE RESERVED_BYTES+65536



////////////////////////////////////////////////////////////////////////////////
// ������: ������
//

#define GetParam(x, y) pthread_mutex_lock(&param.mutex); \
	x = param.y; pthread_mutex_unlock(&param.mutex);
#define SetParam(x, y) pthread_mutex_lock(&param.mutex); \
	param.x = y; pthread_mutex_unlock(&param.mutex);
#define GetParam1(x, y) pthread_mutex_lock(&param->mutex); \
	x = param->y; pthread_mutex_unlock(&param->mutex);
#define SetParam1(x, y) pthread_mutex_lock(&param->mutex); \
	param->x = y; pthread_mutex_unlock(&param->mutex);


//
// �������: static void *Server_main(void *)
//
// ����������: ���� � ���������� ���������
//
// �������: ��� ����� ���������� �������� ������ ��������� ���������
//
static void *Server_main(void *_param) {
	bool x;
	ServerParams *param = (ServerParams *)_param;
	Socket *sock = &param->sock;
	unsigned char inbuf[BUF_SIZE], outbuf[BUF_SIZE];
	Datagram *indg = (Datagram *) inbuf;
	Datagram *outdg = (Datagram *) outbuf;
	SCMObj scm;
	void *data;
	do {
		if ( sock->Accept() ) {
			outdg->id = PROTOCOLID;
			if ( sock->Receive( inbuf, BUF_SIZE )
			        && indg->id == PROTOCOLID ) {
				switch ( indg->cmd_cou ) {
					case CMD_LIST:
						if ( scm.Init() ) {
							DWORD size, num;
							data = scm.getEnum(size, num);
							memcpy( &outdg->data, data, size );
							outdg->cmd_cou = CMD_LIST + num;
							sock->Send(outdg, size + RESERVED_BYTES);
						}
						break;
					case CMD_SET:
						// ...
						break;
				}
			}
			sock->Disconnect();
		}
		GetParam1( x, active );
	} while ( x );
	return 0;
}


//
// �����: bool Server::Start(unsigned int port)
//
// ����������: ��������� ������ � ����� ��������� ���������
//
// ����������: ���� ������ ��������
//
bool Server::Start(unsigned short port) {
	assert( !IsWorking() );
	bool r;
	r = param.sock.Open();
	if (r) {
		if (port < 1024)
			r = param.sock.Bind({INADDR_ANY, 0});
		else
			r = param.sock.Bind({INADDR_ANY, port});
	}
	if (r) {
		if ( (r = param.sock.SetNonBlocking()) == true )
			r = ( pthread_create( &thread, 0, Server_main, &param ) == 0 );
		if (!r)
			param.sock.Close();
	}
	SetParam( active, r );
	return r;
}


//
// �����: bool Server::IsWorking()
//
// ����������: ��������� �������
//
// ����������: ���� ������ ��������
//
bool Server::IsWorking() {
	GetParam( bool r, active );
	return r;
}


//
// �����: void Server::Stop()
//
// ����������: ���������� ������ � ����� ��������� ���������
//
void Server::Stop() {
	if ( IsWorking() ) {
		SetParam( active, false );
		pthread_join( thread, 0 );
		if ( param.sock.IsOpen() )
			param.sock.Close();
	}
}



////////////////////////////////////////////////////////////////////////////////
// ������: ������
//


Client::Client() {
	param.list.clear();
}

//
// �����: bool Client::GetList()
//
// ����������: �������� ������ ����� �������
//
// ����������: ���� ������ ��������
//
bool Client::GetList(Address addr) {
	pthread_mutex_lock(&param.mutex);
	bool r;
	Socket *sock = &param.sock;
	unsigned char inbuf[BUF_SIZE], outbuf[BUF_SIZE];
	Datagram *indg = (Datagram *) inbuf;
	Datagram *outdg = (Datagram *) outbuf;
	outdg->id = PROTOCOLID;
	assert( !sock->IsOpen() );
	r = sock->Open();
	if (r) {
		r = sock->Connect( addr );
		if ( r && sock->SetNonBlocking() ) {
			outdg->cmd_cou = CMD_LIST;
			r = false;
			sock->Wait();
			if ( sock->Send( outdg, RESERVED_BYTES ) ) {
				sock->Wait();
				if (sock->Receive( indg, BUF_SIZE ) > RESERVED_BYTES
				        && indg->id == PROTOCOLID ) {
					param.list.clear();
					char *pbuf = (char *) &indg->data;
					ListItem *item;
					for (DWORD i=0; i < (indg->cmd_cou & 0xFFFFFF); i++) {
						char *str1 = (char *) &pbuf[1];
						char *str2 = (char *) str1+strlen(str1)+1;
						int len1 = strlen(str1);
						int len2 = strlen(str2);
						item = (ListItem *) new
						       char[1+2*sizeof(char *)+len1+len2+2];
						item->state = pbuf[0];
						item->name = (char *) &item->data;
						item->viewname = (char *) &item->data[len1+1];
						strcpy(item->name, str1);
						strcpy(item->viewname, str2);
						param.list.insert(param.list.end(), *item);
						pbuf = (char *) str2+strlen(str2)+1;
					}
					r = true;
				}
			}
			sock->Disconnect();
		}
		if ( sock->IsOpen() )
			sock->Close();
	}
	pthread_mutex_unlock(&param.mutex);
	return r;
}


//
// �����: unsigned int Client::ListSize()
//
// ����������: ������ ������
//
unsigned int Client::ListSize() {
	unsigned int r;
	GetParam( r, list.size() );
	return r;
}


//
// �����: ListItem* Client::GetItem(unsigned int idx)
//
// ����������: ��������� �� ������� ������
//
ListItem* Client::GetItem(unsigned int idx) {
	struct ListItem* r;
	pthread_mutex_lock(&param.mutex);
	r = &param.list[idx];
	pthread_mutex_unlock(&param.mutex);
	return r;
}



////////////////////////////////////////////////////////////////////////////////
// ������: ��������������� �������
//

//
// �������: bool InitializeSockets()
//
// ����������: ���������������� ������ � ��������
//
// ����������: ���� ������ ��������
//
bool InitializeSockets() {
#if PLATFORM == PLATFORM_WINDOWS
	WSADATA WsaData;
	return WSAStartup( MAKEWORD(2,2), &WsaData ) == NO_ERROR;
#else
	return true;
#endif
}


//
// �������: void ShutdownSockets()
//
// ����������: ��������� ������ � ��������
//
void ShutdownSockets() {
#if PLATFORM == PLATFORM_WINDOWS
	WSACleanup();
#endif
}
