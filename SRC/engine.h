#include "include.h"


#define BUF_SIZE 2048
#define DATAGRAMM_HDR "\17VS\3"
#define CMD_LIST   0x1A
#define CMD_INFO   0x2B
#define CMD_SET    0x3C
#define FLAG_START      0x01
#define FLAG_STOP       0x02
#define FLAG_PAUSE      0x04
#define FLAG_RESUME     0x08
#define FLAG_RUN_NO     0x10
#define FLAG_RUN_MAN    0x20 // manually
#define FLAG_RUN_AUTO   0x30
#define FLAG_DENY_PAUSE 0x40
#define FLAG_DENY_RESUM 0x80


inline bool InitializeSockets() {
#if PLATFORM == PLATFORM_WINDOWS
	WSADATA WsaData;
	return WSAStartup( MAKEWORD(2,2), &WsaData ) == NO_ERROR;
#else
	return true;
#endif
}



inline void ShutdownSockets() {
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
};



struct Datagramm {
	char hdr[4];
	WORD sz;
	BYTE flag;
	BYTE cmd;
	DWORD crc;
	char data[];
};



class Obj {
	protected:
		pthread_t thr;
		WORD port;
	public:
		enum State {
			Passive,
			Active,
			Starting,
			Stoping,
			StartError
		} state;
		HWND hwnd;
		int sock;


		int Receive(Address &sender, void *data, int size) {
			assert( data );
			assert( size > 0 );
			if ( socket == 0 )
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
			assert( data );
			assert( size > 0 );
			if ( socket == 0 )
				return false;
			assert( destination.addr != 0 );
			assert( destination.port != 0 );
			sockaddr_in address;
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = htonl( destination.addr );
			address.sin_port = htons( (unsigned short) destination.port );
			int sent_bytes = sendto( sock, (const char*)data, size, 0,
			                         (const sockaddr*)&address,
			                         sizeof(sockaddr_in) );
			return sent_bytes == size;
		}
};



static void *Server_main(void *);
class : public Obj {
	public:
		bool Start(WORD port) {
			state = State::Starting;
			this->sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
			if ( sock <= 0 ) {
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
				printf( "failed to bind socket\n" );
				Stop();
				return false;
			}
			// set non-blocking io
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
			int nonBlocking = 1;
			if ( fcntl( sock, F_SETFL, O_NONBLOCK, nonBlocking ) == -1 ) {
				printf( "failed to set non-blocking socket\n" );
				Stop();
				return false;
			}
#elif PLATFORM == PLATFORM_WINDOWS
			DWORD nonBlocking = 1;
			if ( ioctlsocket( sock, FIONBIO, &nonBlocking ) != 0 ) {
				printf( "failed to set non-blocking socket\n" );
				Stop();
				return false;
			}
#endif
			pthread_create(&thr, 0, Server_main, 0);
			state = State::Active;
			return true;
		}


		bool Active() {
			return state == State::Active;
		}


		void Stop() {
			state = State::Stoping;
			if ( sock != 0 ) {
				closesocket(sock);
				sock = 0;
			}
			if ( thr != 0 ) {
				pthread_join(thr, 0);
			}
			state = State::Passive;
		}
} Server;



struct forkParam {
	pthread_t thr;
	Address sender;
	stack <Datagramm *> dgst;
};
#define stackPop(stack) stack.top(); stack.pop();



static void *Server_fork(void *p) {
	forkParam *param = (forkParam *) p;
	assert( !param->dgst.empty() );
	Datagramm *data = stackPop(param->dgst);
	switch (data->cmd) {
		case CMD_LIST:
			break;
		case CMD_INFO:
			break;
		case CMD_SET:
			break;
	}
}



static void *Server_main(void *) {
	Address sender;
	char buf[BUF_SIZE];
	vector <forkParam> thrs;

	Datagramm *dg;
	int recived;
	while ( Server.state != Obj::State::Stoping ) {
		if ( (recived = Server.Receive(sender, &buf, BUF_SIZE)) ) {
			if ( strcmp(dg->hdr, DATAGRAMM_HDR) == 0 ) {
				dg = (Datagramm *) new char[recived];
				memcpy(dg, buf, recived);
				bool founded = false;
				for (auto &i: thrs) {
					if (i.sender == sender) {
						i.dgst.push(dg);
						founded = true;
						break;
					}
				}
				if (!founded) {
					forkParam *param = new forkParam;
					param->sender = sender;
					thrs.insert(thrs.end(), *param);
					pthread_create(&param->thr, 0, Server_fork, param);
				}
			}
		}
	}
}



////////////////////////////////////////////////////////////////////////////////
// Client
//
static void *Client_main(void *);
class : public Obj {
	private:
		UINT ip;
	public:
		bool Start(UINT ip, WORD port) {
			assert( !Active() );
			state = State::Starting;
			this->ip = ip;
			this->port = port;
			pthread_create(&thr, 0, Client_main, 0);
			state = State::Active;
			return true;
		}
		bool Active() {
			return state == State::Active;
		}
		void Stop() {
			state = State::Stoping;
			pthread_join(thr, 0);
			state = State::Passive;
		}
} Client;

static void *Client_main(void *) {
	while ( Client.state != Obj::State::Stoping ) {
	}
}

