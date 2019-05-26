//
// ЗАГОЛОВОК-МОДУЛЬ: ENGINE.H
//
// ОПИСАНИЕ: реализация сервера и клиента
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//
#include "include.h"


char *genLogFName();
LogObj Log(genLogFName());


//
// ФУНКЦИЯ: char *genLogFName()
//
// НАЗНАЧЕНИЕ: генерирует имя файла лога
//
char *genLogFName() {
	char *buf = new char[25];
	time_t t = time(0);
	struct tm* aTm = localtime(&t);
	sprintf(buf, "ksvc %02d%02d%04d_%02d%02d%02d.log",
	        aTm->tm_mday, aTm->tm_mon+1, aTm->tm_year+1900,
	        aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
	return buf;
}


#define stackPop(stack) stack.top(); stack.pop();
#define waitStack(stack) while ( stack.empty() ) sleep(1);


//
// МАКРОС: inline bool initializeSockets()
//
// ДЕЙСТВИЕ: инициализирует программу для работы с сокетами
//
inline bool initializeSockets() {
#if PLATFORM == PLATFORM_WINDOWS
	WSADATA WsaData;
	return WSAStartup( MAKEWORD(2,2), &WsaData ) == NO_ERROR;
#else
	return true;
#endif
}


//
// МАКРОС: inline void shutdownSockets()
//
// ДЕЙСТВИЕ: финализирует работу программы с сокетами
//
inline void shutdownSockets() {
#if PLATFORM == PLATFORM_WINDOWS
	WSACleanup();
#endif
}


struct Address {
	union {
		struct {
			unsigned char a, b, c, d;
		};
		unsigned int addr;
	};
	unsigned short port;
	bool operator == (Address &x) {
		return (x.addr == addr) && (x.port == port);
	}
	bool operator != (Address &x) {
		return !(*this == x);
	}
};


#define RESERVED_BYTES 4+4+2+1
#define BUF_SIZE RESERVED_BYTES+65536
struct Datagram {
	char hdr[4];
	DWORD sz;
	WORD id;
	BYTE cmd;
	char data[];
};
#define MAX_WAIT 3.0
#define MAX_TRY 3
#define SLEEP_TIME 50
#define DATAGRAMM_HDR "\17VS\3"
#define CMD_ANY      0x00
#define CMD_CHECKOUT 0x80
#define CMD_LIST     0x1A
#define CMD_SET      0x2B

#include "services.h"

typedef struct _Postponed {
	Address sender;
	Datagram* dg;
	int sz;
} Postponed, *pPostponed;


typedef struct _ListItem {
	BYTE state;
	char *name, *viewname;
	char data[];
} ListItem, *pListItem;



class Obj {
	protected:
		WORD port;
		vector<Postponed> postponed;
	public:
		HWND hwnd;
		int sock;

		int Receive(Address &sender, void *data, int size) {
			_VERIFY( data );
			_VERIFY( size > 0 );
			if ( sock == 0 )
				return false;
#if PLATFORM == PLATFORM_WINDOWS
			typedef int socklen_t;
#endif
			sockaddr_in from;
			socklen_t fromLength = sizeof( from );
			int received_bytes = recvfrom( sock, (char*)data, size, 0,
			                               (sockaddr*)&from, &fromLength );
			if ( received_bytes <= 0 )
				return 0;
			unsigned int address = ntohl( from.sin_addr.s_addr );
			unsigned short port = ntohs( from.sin_port );
			sender.addr=address;
			sender.port=port;
			return received_bytes;
		}


		bool Send(Address &destination, void *data, int size) {
			_VERIFY( data );
			_VERIFY( size > 0 );
			if ( sock == 0 )
				return false;
			_VERIFY( destination.addr != 0 );
			_VERIFY( destination.port != 0 );
			sockaddr_in address;
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = htonl( destination.addr );
			address.sin_port = htons( (unsigned short) destination.port );
			int sent_bytes = sendto( sock, (const char*)data, size, 0,
			                         (const sockaddr*)&address,
			                         sizeof(sockaddr_in) );
			return sent_bytes == size;
		}


#define chkhdr(hdr) memcmp(hdr, DATAGRAMM_HDR, 4) == 0
		Datagram *NextDatagram(Address sender, int &sz,
		                       BYTE reqcmd, DWORD minid ) {
			Address _sender;
			char buf[BUF_SIZE];
			Datagram *dg = (Datagram *) buf;
			Postponed postpone;

			if (postponed.empty()) {
				sz = Receive(_sender, dg, BUF_SIZE);
				if (sz<RESERVED_BYTES || !chkhdr(dg->hdr)) {
					sz = 0;
					return 0;
				}
				dg = (Datagram *) new char[sz];
				memcpy(dg, buf, sz);
				if (_sender != sender) {
					postpone.sender = _sender;
					postpone.sz = sz;
					postpone.dg = dg;
					postponed.insert(postponed.end(), postpone);
					dg = 0;
					sz = 0;
				}
			} else {
				for (vector<Postponed>::iterator i = postponed.begin();
				        i != postponed.end(); i++) {
					if (i->sender == sender && i->dg->cmd == reqcmd) {
						dg = i->dg;
						sz = i->sz;
						postponed.erase(i);
						if (dg->id < minid && minid != 0) {
							dg = 0;
							sz = 0;
						} else
							break;
					}
				}
			}
			return dg;
		}


		Datagram *Requere(Address sender, BYTE reqcmd = CMD_ANY,
		                  DWORD minid = 0) {
			int sz = 0;
			for (int trycount = 0; trycount < MAX_TRY; trycount++) {
				Datagram *dg = NextDatagram(sender, sz, reqcmd, minid);
				if ( dg != 0 )
					return dg;
				_sleep(SLEEP_TIME);
			}
			return 0;
		}
};


//
// ФУНКЦИЯ: bool nonBlockingIO(SOCKET)
//
// НАЗНАЧЕНИЕ: устанавливает неблокирующий
//
bool nonBlockingIO(SOCKET sock) {
	char errBlocking[] = "failed to set non-blocking socket\n";
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
	int nonBlocking = 1;
	if ( fcntl( sock, F_SETFL, O_NONBLOCK, nonBlocking ) == -1 ) {
#elif PLATFORM == PLATFORM_WINDOWS
	DWORD nonBlocking = 1;
	if ( ioctlsocket( sock, FIONBIO, &nonBlocking ) != 0 ) {
#endif
		Log.Write(errBlocking);
		printf(errBlocking);
		return false;
	} else
		return true;
}



static void *Server_main(void *);
class : public Obj {
	protected:
		pthread_t thr;
	public:
		enum State {
			PASSIVE,
			ACTIVE,
			STARTING,
			STOPING,
			STARTERROR
		} state;
		SCMObj SCM;

		bool Start(WORD port) {
			state = STARTING;
			Log.Write("Starting server...");
			Log.WriteInt("Port - ", port);
			this->sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
			if ( sock <= 0 ) {
				Log.Write("Failed to create socket");
				printf( "failed to create socket\n" );
				sock = 0;
				return false;
			}
			// bind to port
			sockaddr_in address;
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = INADDR_ANY;
			address.sin_port = htons( (unsigned short) port );
			if ( bind( sock, (const sockaddr*) &address,
			           sizeof(sockaddr_in) ) < 0 ) {
				Log.Write("Failed to bind socket");
				printf( "failed to bind socket\n" );
				Stop();
				return false;
			}
			// set non-blocking io
			if (!nonBlockingIO(sock)) {
				Stop();
				return false;
			}
			pthread_create(&thr, 0, Server_main, 0);
			state = ACTIVE;
			Log.Write("OK");
			return true;
		}


		bool Active() {
			return state == ACTIVE;
		}


		void Stop() {
			Log.Write("Stoping server...");
			state = STOPING;
			if ( sock != 0 ) {
				closesocket(sock);
				sock = 0;
			}
			if ( thr != 0 ) {
				pthread_join(thr, 0);
			}
			state = PASSIVE;
			Log.Write("OK");
		}
} Server;


//
// ФУНКЦИЯ: static void *Server_main(void *)
//
// НАЗНАЧЕНИЕ: приём и раздача сообщений дочерним потокам
//
static void *Server_main(void *) {
	Address sender;
	Datagram *dg;
	while ( Server.state != Server.State::STOPING ) {
		if ( (dg = Server.Requere(sender)) != 0 ) {
			for (int trycount = 0; trycount < MAX_TRY; trycount++) {
				switch (dg->cmd) {
					case CMD_LIST:

						break;
					case CMD_SET:

						break;
					default:
						Log.WriteInt("Unknown cmd: ", dg->cmd);
						break;
				}
				if (Server.Requere(sender)->cmd == dg->cmd | CMD_CHECKOUT)
					break;
			}
		}
	}
	return 0;
}



////////////////////////////////////////////////////////////////////////////////
// Client
//
class : public Obj {
	private:
		UINT ip;
		Address a;
	public:
		vector<pListItem> list;


		bool Init(Address adr) {
			a = adr;
			if (sock)
				Done();
			Log.Write("Client init...");
			char buf[24];
			sprintf(buf, "Addr - %u.%u.%u.%u", a.d, a.c, a.b, a.a);
			Log.Write(buf);
			Log.WriteInt("Port - ", a.port);
			this->sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
			if ( sock <= 0 ) {
				Log.Write("Failed to create socket");
				printf( "failed to create socket\n" );
				sock = 0;
				return false;
			}
			if (!nonBlockingIO(sock)) {
				Done();
				return false;
			}
			Log.Write("OK");
			return true;
		}


		bool getList() {
			Log.Write("Client: trying to get list");
			int sz = 0, _try = 0;
			Datagram *buf = (Datagram *) new char[BUF_SIZE];
			Datagram *dg = (Datagram *) new char[RESERVED_BYTES + sz];
			memcpy(dg->hdr, DATAGRAMM_HDR, 4);
			dg->cmd = CMD_LIST;
			dg->sz = sz;
			for (int i=0; i<MAX_TRY; i++) {
				Log.Write("Sending datagramm...");
				Send(a, dg, RESERVED_BYTES + sz);
				_sleep(50);
				clock_t start = clock();
				while ( ( sz = Receive(a, buf, BUF_SIZE) ) == 0
				        && (!chkhdr(buf->hdr)) ) {
					clock_t end = clock();
					double seconds = (double)(end - start) / CLOCKS_PER_SEC;
					if (seconds>=MAX_WAIT)
						break;
					_sleep(50);
				}
				if (sz)
					Log.Write("some datagram...");
				if ( chkhdr(buf->hdr) )
					break;
			}
			delete dg;
			if ( chkhdr(buf->hdr) && buf->cmd == CMD_LIST ) {
				Log.Write("Client: received");
				list.clear();
				BYTE *data = (BYTE *) &buf->data;
				char *str1, *str2;
				pListItem item;
				for (int i=0; i<buf->sz; i++) {
					str1 = (char *) &data[1];
					str2 = (char *) &data[strlen(str1)+1];
					int len1 = strlen(str1);
					int len2 = strlen(str2);
					item = (pListItem) new char[1+4+4+len1+len2+2];
					item->state = data[0];
					item->name = (char *) &item->data;
					item->viewname = (char *) &item->data[len1];
					strcpy(item->name, str1);
					strcpy(item->viewname, str2);
					list.insert(list.end(), item);
				}
				delete[] buf;
				Log.Write("Client: OK");
				return true;
			}
			Log.Write("Client: FAIL");
			return false;
		}


		void set(char *srv, BYTE state) {
			Log.Write("Client: trying to set service");
			Log.WriteInt(strcat(srv, " "), state);
			int sz = 1+strlen(srv)+1;
			Datagram *dg = (Datagram *) new char[RESERVED_BYTES + sz];
			dg->cmd = CMD_SET;
			dg->sz = sz;
			dg->data[0] = state;
			strcpy(&dg->data[1], srv);
			Send(a, dg, RESERVED_BYTES + sz);
			// Check out datagram?
		}


		void Done() {
			Log.Write("Client done...");
			if ( sock != 0 ) {
				closesocket(sock);
				sock = 0;
			}
			Log.Write("OK");
		}
} Client;

