//
// ??????: ENGINE.CPP
//
// ????????: ?????????? ?????? ??????? ? ???????
//
#include "include.h"
#include "engine.h"
#include "services.h"
#include <stdio.h>

extern pthread_mutex_t mutex;

////////////////////////////////////////////////////////////////////////////////
// ??????: ???
//

#include "log.h"

char *genLogFName();
LogObj Log(genLogFName());

//
// ???????: char *genLogFName()
//
// ??????????: ?????????? ??? ????? ????
//
char *genLogFName() {
	char *buf = new char[25];
	time_t t = time(0);
	struct tm* aTm = localtime(&t);
	sprintf(buf, "mysvclog %02d%02d%04d_%02d%02d%02d.log",
	        aTm->tm_mday, aTm->tm_mon+1, aTm->tm_year+1900,
	        aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
	return buf;
}

////////////////////////////////////////////////////////////////////////////////
// ??????: ?????????, ????????? ? ????????? ??????????
//

#define PROTOCOLID *((DWORD *)"\17VS\2")
#define CMD_ANY      0
#define CMD_LIST     0x0C000000
#define CMD_SET      0x18000000

//
// ?????????: Datagram
//
// ??????????: ???????????? ???????? ? ??????
//
struct Datagram {
	DWORD id;
	DWORD cmd_cou;
	char data[];
};
#define RESERVED_BYTES 4+4
#define BUF_SIZE RESERVED_BYTES+65536



////////////////////////////////////////////////////////////////////////////////
// ??????: ??????
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
// ???????: static void *Server_main(void *)
//
// ??????????: ????? ? ?????????? ?????????
//
// ???????: ??? ?????? ?????????? ???????? ?????? ????????? ?????????
//
static void *Server_main(void *_param) {
	bool x;
	ServerParams *param = (ServerParams *)_param;
	Socket *sock = &param->sock;
	unsigned char inbuf[RESERVED_BYTES+260], outbuf[BUF_SIZE];
	Datagram *indg = (Datagram *) inbuf;
	Datagram *outdg = (Datagram *) outbuf;
	void *data;
	Log.WriteDateF("Server: started\n");
	do {
		if ( sock->Accept() ) {
			Log.WriteDateF("Server: connected with client\n");
			outdg->id = PROTOCOLID;
			if ( sock->Receive( inbuf, RESERVED_BYTES+260 )
			        && indg->id == PROTOCOLID ) {
				Log.WriteDateF("Server: giving command %d\n", indg->cmd_cou>>24);
				switch ( indg->cmd_cou ) {
					case CMD_LIST:
						DWORD sz, num;
						sz = num = 0;
						data = SVC_getEnum(sz, num);
						memcpy( outdg->data, data, sz );
						outdg->cmd_cou = CMD_LIST + num;
						Log.WriteDateF("Server: sending list\n");
						sock->Send(outdg, sz + RESERVED_BYTES);
						break;
					case CMD_SET:
						SVC_SetStatus( (char *)(&indg->data[1]),
						               indg->data[0] );
						Log.WriteDateF("Server: set service - ok\n");
						break;
					default:
						Log.WriteDateF("Server error: unknown command\n");
				}
			} else
				Log.WriteDateF("Server error: no packet or unknown id\n");
			sock->Wait();
			sock->Disconnect();
			Log.WriteDateF("Server: disconnected\n");
		}
		GetParam1( x, active );
	} while ( x );
	Log.WriteDateF("Server: stoped\n");
	return 0;
}


//
// ?????: bool Server::Start(unsigned int port)
//
// ??????????: ????????? ?????? ? ????? ????????? ?????????
//
// ?????????: ???? ?????? ????????
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
// ?????: bool Server::IsWorking()
//
// ?????????: ????????? ???????
//
// ?????????: ???? ?????? ????????
//
bool Server::IsWorking() {
	GetParam( bool r, active );
	return r;
}


//
// ?????: void Server::Stop()
//
// ??????????: ?????????? ?????? ? ????? ????????? ?????????
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
// ??????: ??????
//


Client::Client() {
	param.list.clear();
}


//
// ?????: bool Client::GetList()
//
// ??????????: ???????? ?????? ????? ???????
//
// ?????????: ???? ?????? ????????
//
bool Client::GetList(Address addr) {
	pthread_mutex_lock(&mutex);
	bool r;
	Socket *sock = &param.sock;
	unsigned char inbuf[BUF_SIZE], outbuf[RESERVED_BYTES];
	Datagram *indg = (Datagram *) inbuf;
	Datagram *outdg = (Datagram *) outbuf;
	outdg->id = PROTOCOLID;
	Log.WriteDateF("Client: getting list\n");
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
		} else
			Log.WriteDateF("Client error: cannot connect\
or setup non-blocking mode\n");
		if ( sock->IsOpen() )
			sock->Close();
	} else
		Log.WriteDateF("Client error: socket is not opened\n");
	pthread_mutex_unlock(&mutex);
	return r;
}


//
// ?????: void Client::SetSvc(Address addr, unsigned int idx, BYTE state)
//
// ??????????: ???????? ??????? ??? ?????????? ???????
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
	Log.WriteDateF("Client: set service \"%s\" %d\n", item->name, state);
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
		} else
			Log.WriteDateF("Client error: cannot connect\
or setup non-blocking mode\n");
		if ( sock->IsOpen() )
			sock->Close();
	} else
		Log.WriteDateF("Client error: socket is not opened\n");
	pthread_mutex_unlock(&mutex);
}


//
// ?????: unsigned int Client::ListSize()
//
// ?????????: ?????? ??????
//
unsigned int Client::ListSize() {
	//unsigned int r;
	//GetParam( r, list.size() );
	return param.list.size();
}


//
// ?????: ListItem* Client::GetItem(unsigned int idx)
//
// ?????????: ????????? ?? ??????? ??????
//
ListItem* Client::GetItem(unsigned int idx) {
	struct ListItem* r;
	pthread_mutex_lock(&mutex);
	r = param.list[idx];
	pthread_mutex_unlock(&mutex);
	return r;
}


////////////////////////////////////////////////////////////////////////////////
// ??????: ??????????????? ???????
//

//
// ???????: bool InitializeSockets()
//
// ??????????: ???????????????? ?????? ? ????????
//
// ?????????: ???? ?????? ????????
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
// ???????: void ShutdownSockets()
//
// ??????????: ????????? ?????? ? ????????
//
void ShutdownSockets() {
#if PLATFORM == PLATFORM_WINDOWS
	WSACleanup();
#endif
}
