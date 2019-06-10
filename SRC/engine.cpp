//
// МОДУЛЬ: ENGINE.CPP
//
// ОПИСАНИЕ: реализация работы сервера и клиента
//
#include "include.h"
#include "engine.h"
#include "services.h"
#include <stdio.h>

extern pthread_mutex_t mutex;

////////////////////////////////////////////////////////////////////////////////
// СЕКЦИЯ: Настройки, константы и структура датаграммы
//

#define PROTOCOLID *((DWORD *)"\17VS\2")
#define CMD_ANY      0
#define CMD_LIST     0x0C000000
#define CMD_SET      0x18000000

//
// СТРУКТУРА: Datagram
//
// СОДЕРЖИМОЕ: передаваемая комманда и данные
//
struct Datagram {
	DWORD id;
	DWORD cmd_cou;
	char data[];
};
#define RESERVED_BYTES 4+4
#define BUF_SIZE RESERVED_BYTES+65536



////////////////////////////////////////////////////////////////////////////////
// СЕКЦИЯ: Сервер
//

#define GetParam(x, y) pthread_mutex_lock(&mutex); \
	x = param.y; pthread_mutex_unlock(&mutex);
#define SetParam(x, y) pthread_mutex_lock(&mutex); \
	param.x = y; pthread_mutex_unlock(&mutex);
#define GetParam1(x, y) pthread_mutex_lock(&mutex); \
	x = param->y; pthread_mutex_unlock(&mutex);
#define SetParam1(x, y) pthread_mutex_lock(&mutex); \
	param->x = y; pthread_mutex_unlock(&mutex);


//
// ФУНКЦИЯ: static void *Server_main(void *)
//
// НАЗНАЧЕНИЕ: приём и обработкаа сообщений
//
// ТАКТИКА: при приёме датаграммы создание потока обработки сообщения
//
static void *Server_main(void *_param) {
	bool x;
	ServerParams *param = (ServerParams *)_param;
	Socket *sock = &param->sock;
	unsigned char inbuf[RESERVED_BYTES+260], outbuf[BUF_SIZE];
	Datagram *indg = (Datagram *) inbuf;
	Datagram *outdg = (Datagram *) outbuf;
	void *data;
	do {
		if ( sock->Accept() ) {
			outdg->id = PROTOCOLID;
			if ( sock->Receive( inbuf, RESERVED_BYTES+260 )
			        && indg->id == PROTOCOLID ) {
				switch ( indg->cmd_cou ) {
					case CMD_LIST:
						DWORD sz, num;
						sz = num = 0;
						data = SVC_getEnum(sz, num);
						memcpy( outdg->data, data, sz );
						outdg->cmd_cou = CMD_LIST + num;
						sock->Send(outdg, sz + RESERVED_BYTES);
						break;
					case CMD_SET:
						SVC_SetStatus( (char *)(&indg->data[1]),
						               indg->data[0] );
						break;
				}
			}
			sock->Wait();
			sock->Disconnect();
		}
		GetParam1( x, active );
	} while ( x );
	return 0;
}


//
// МЕТОД: bool Server::Start(unsigned int port)
//
// НАЗНАЧЕНИЕ: запустить сервер и поток обработки сообщений
//
// ВОЗРАЩАЕТ: флаг успеха операции
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
// МЕТОД: bool Server::IsWorking()
//
// ВОЗРАЩАЕТ: состояние сервера
//
// ВОЗРАЩАЕТ: флаг успеха операции
//
bool Server::IsWorking() {
	GetParam( bool r, active );
	return r;
}


//
// МЕТОД: void Server::Stop()
//
// НАЗНАЧЕНИЕ: остановить сервер и поток обработки сообщений
//
void Server::Stop() {
	if ( IsWorking() ) {
		SetParam( active, false );
		pthread_join( thread, 0 );
		if ( param.sock.IsOpen() )
			param.sock.Close();
	}
}


Server::~Server() {
	Stop();
}


////////////////////////////////////////////////////////////////////////////////
// СЕКЦИЯ: Клиент
//


Client::Client() {
	param.list.clear();
}


//
// МЕТОД: bool Client::GetList()
//
// НАЗНАЧЕНИЕ: получить список служб сервера
//
// ВОЗРАЩАЕТ: флаг успеха операции
//
bool Client::GetList(Address addr) {
	pthread_mutex_lock(&mutex);
	bool r;
	Socket *sock = &param.sock;
	unsigned char inbuf[BUF_SIZE], outbuf[RESERVED_BYTES];
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
					char *pbuf = indg->data, *st;
					ListItem *item;
					for (DWORD i=0; i < (indg->cmd_cou & 0xFFFFFF); i++) {
						st = pbuf;
						pbuf++;
						char *str1 = pbuf;
						int sz1 = strlen(str1);
						pbuf += strlen(pbuf) + 1;
						char *str2 = (char *) pbuf;
						int sz2 = strlen(str2);
						pbuf += strlen(pbuf) + 1;
						item = new ListItem;
						item->state = *st;
						item->name = new char[sz1+1];
						strcpy(item->name, str1);
						item->viewname = new char[sz2+1];
						strcpy(item->viewname, str2);
						param.list.insert(param.list.end(), item);
					}
					r = true;
				}
			}
			sock->Wait();
			sock->Disconnect();
		}
		if ( sock->IsOpen() )
			sock->Close();
	}
	pthread_mutex_unlock(&mutex);
	return r;
}


//
// МЕТОД: void Client::SetSvc(Address addr, unsigned int idx, BYTE state)
//
// НАЗНАЧЕНИЕ: посылает команду для управления службой
//
void Client::SetSvc(Address addr, unsigned int idx, BYTE state) {
	pthread_mutex_lock(&mutex);
	if ( param.list.size() <= idx )
		return;
	ListItem *item = GetItem( idx );
	bool r;
	Socket *sock = &param.sock;
	unsigned char outbuf[BUF_SIZE];
	Datagram *outdg = (Datagram *) outbuf;
	outdg->id = PROTOCOLID;
	assert( !sock->IsOpen() );
	r = sock->Open();
	if (r) {
		r = sock->Connect( addr );
		if ( r && sock->SetNonBlocking() ) {
			outdg->cmd_cou = CMD_SET;
			outdg->data[0] = state;
			strcpy( &outdg->data[1], item->name );
			r = false;
			sock->Wait();
			sock->Send( outdg, RESERVED_BYTES + 2 + strlen(&outdg->data[1]) );
			sock->Disconnect();
		}
		if ( sock->IsOpen() )
			sock->Close();
	}
	pthread_mutex_unlock(&mutex);
}


//
// МЕТОД: unsigned int Client::ListSize()
//
// ВОЗРАЩАЕТ: размер списка
//
unsigned int Client::ListSize() {
	//unsigned int r;
	//GetParam( r, list.size() );
	return param.list.size();
}


//
// МЕТОД: ListItem* Client::GetItem(unsigned int idx)
//
// ВОЗРАЩАЕТ: указатель на элемент списка
//
ListItem* Client::GetItem(unsigned int idx) {
	struct ListItem* r;
	pthread_mutex_lock(&mutex);
	r = param.list[idx];
	pthread_mutex_unlock(&mutex);
	return r;
}


////////////////////////////////////////////////////////////////////////////////
// СЕКЦИЯ: Вспомогательные функции
//

//
// ФУНКЦИЯ: bool InitializeSockets()
//
// НАЗНАЧЕНИЕ: инициализировать работу с сокетами
//
// ВОЗРАЩАЕТ: флаг успеха операции
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
// ФУНКЦИЯ: void ShutdownSockets()
//
// НАЗНАЧЕНИЕ: завершить работу с сокетами
//
void ShutdownSockets() {
#if PLATFORM == PLATFORM_WINDOWS
	WSACleanup();
#endif
}
