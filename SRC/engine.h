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
#define waitStack(stack) while ( stack.empty() ) sleep(SLEEP_TIME);


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


#define RESERVED_BYTES 4+4+4+1
#define BUF_SIZE RESERVED_BYTES+65536
struct Datagram {
	char hdr[4];
	DWORD sz;
	DWORD id;
	BYTE cmd;
	char data[];
};
#define MAX_WAIT 3.0
#define MAX_TRY 3
#define SLEEP_TIME 10
#define DATAGRAMM_HDR "\17VS\3"
#define CMD_ANY      0x00
#define CMD_CHECKOUT 0x80
#define CMD_LIST     0x1A
#define CMD_SET      0x2B

#include "services.h"


typedef struct _Datagram2 {
	Datagram* dg;
	int sz;
} Datagram2;


typedef struct _Postponed {
	Address sender;
	vector<Datagram2> dg;
	DWORD id = 0;

	Datagram2 *findDatagram(BYTE reqcmd, DWORD minid) {
		if (dg.empty())
			return 0;
		for (vector<Datagram2>::iterator i = dg.begin(); i != dg.end(); i++)
			if (i->dg->cmd == reqcmd || reqcmd == CMD_ANY)
				if (i->dg->id >= minid) {
					Datagram2 *dg2 = &(*i);
					dg.erase(i);
					return dg2;
				}
		// erase
		return 0;
	}
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

		Postponed *findPostpone(Address addr) {
			for (vector<Postponed>::iterator i = postponed.begin();
			        i != postponed.end(); i++)
				if (i->sender == addr)
					return &(*i);
			return 0;
		}

	public:
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
		Datagram2 *NextDatagram( Address sender, BYTE reqcmd ) {
			Address _sender;
			char buf[BUF_SIZE];
			Datagram *dg = (Datagram *) buf;
			Datagram2 *dg2 = 0;
			Postponed *p = findPostpone(sender);
			if (p) {
				dg2 = p->findDatagram(reqcmd, p->id);
				if (dg2)
					return dg2;
			}
			DWORD sz = Receive(_sender, dg, BUF_SIZE);
			if (sz<RESERVED_BYTES || !chkhdr(dg->hdr)) {
				return 0;
			}
			dg2 = new Datagram2;
			dg2->dg = (Datagram *) new char[sz];
			memcpy(dg2->dg, buf, sz);
			dg2->sz = sz;
			if (_sender != sender && reqcmd != CMD_ANY) {
				p = findPostpone(sender);
				if (!p) {
					p = new Postponed;
					p->sender = _sender;
					p->id = 0;
					p->dg.insert(p->dg.end(), *dg2);
					postponed.insert(postponed.end(), *p);
					return 0;
				}
				p->dg.insert(p->dg.end(), *dg2);
				return 0;
			}
			return dg2;
		}


//
// МЕТОД: Datagram *Requere(Address, BYTE)
//
// НАЗНАЧЕНИЕ: берёт следующую датаграмму с данной коммандой
//
		Datagram2 *Requere(Address sender, BYTE reqcmd = CMD_ANY) {
			int sz = 0;
			for (int trycount = 0; trycount < MAX_TRY; trycount++) {
				Datagram2 *dg2;
				clock_t start = clock();
				while ( !(dg2 = NextDatagram(sender, reqcmd)) ) {
					clock_t end = clock();
					double seconds = (double)(end - start) / CLOCKS_PER_SEC;
					if (seconds>=MAX_WAIT)
						break;
					_sleep(SLEEP_TIME);
				}
				if ( dg2 != 0 )
					return dg2;
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
	DWORD size, num;
	Datagram *data;
	Datagram2 *dg2;
	SCMObj scm;
	while ( Server.state != Server.State::STOPING ) {
		if ( (dg2 = Server.Requere(sender)) != 0 ) {
			for (int trycount = 0; trycount < MAX_TRY; trycount++) {
				switch (dg2->dg->cmd) {
					case CMD_LIST:
						scm.Init();
						data = (Datagram *) scm.getEnum(size, num);
						if (!data) {
							Log.Write("FAIL");
							break;
						}
						Log.Write("Make package...");
						memcpy(data->hdr, DATAGRAMM_HDR, 4);
						data->cmd = CMD_LIST;
						data->id = dg2->dg->id;
						data->sz = size;
						_VERIFY(Server.Send(sender, data, size+RESERVED_BYTES));
						Log.Write("OK");
						delete data;
						break;
					case CMD_SET:
						Log.Write("Cmd - set");
						ServiceObj *srv;
						*srv = SCMObj().getService(&data->data[1]); // ???
						srv->Status = (int) &data->data[0];
						// ...
						// Check out datagram?
						break;
					default:
						Log.WriteInt("Unknown cmd: ", dg2->dg->cmd);
						break;
				}
				if (Server.Requere(sender)->dg->cmd ==
				        dg2->dg->cmd | CMD_CHECKOUT)
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
				_sleep(SLEEP_TIME);
				clock_t start = clock();
				while ( ( sz = Receive(a, buf, BUF_SIZE) ) == 0
				        && (!chkhdr(buf->hdr)) ) {
					clock_t end = clock();
					double seconds = (double)(end - start) / CLOCKS_PER_SEC;
					if (seconds>=MAX_WAIT)
						break;
					_sleep(SLEEP_TIME);
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

