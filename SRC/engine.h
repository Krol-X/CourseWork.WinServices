#include "include.h"
#define BUF_SIZE 2048
#define DATAGRAMM_HDR "\17VS\3"


struct Address {
	union {
		struct {
			unsigned char a, b, c, d;
		};
		unsigned int value;
	} addr;
	unsigned short port;
};



struct Datagramm {
	char hdr[4];
	WORD sz;
	WORD cmd;
	DWORD crc;
	char data[];
};



class Obj {
	protected:
		pthread_t mainThread;
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
			sender.addr.value=address;
			sender.port=port;
			return received_bytes;
		}


		bool Send(Address &destination, void *data, int size) {
			assert( data );
			assert( size > 0 );
			if ( socket == 0 )
				return false;
			assert( destination.addr.value != 0 );
			assert( destination.port != 0 );
			sockaddr_in address;
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = htonl( destination.addr.value );
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
			pthread_create(&mainThread, 0, Server_main, 0);
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
			if ( mainThread != 0 ) {
				pthread_join(mainThread, 0);
			}
			state = State::Passive;
		}
} Server;



static void *Server_main(void *) {
	Address sender;
	char buf[BUF_SIZE];
	Datagramm *dg = (Datagramm *) buf;
	int recived;
	while ( Server.state != Obj::State::Stoping ) {
		if ( (recived = Server.Receive(sender, &buf, BUF_SIZE)) ) {
			if ( strcmp(dg->hdr, DATAGRAMM_HDR) == 0 ) {

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
			pthread_create(&mainThread, 0, Client_main, 0);
			state = State::Active;
			return true;
		}
		bool Active() {
			return state == State::Active;
		}
		void Stop() {
			state = State::Stoping;
			pthread_join(mainThread, 0);
			state = State::Passive;
		}
} Client;



static void *Client_main(void *) {
	while ( Client.state != Obj::State::Stoping ) {
	}
}

