#include "include.h"


inline bool initializeSockets() {
#if PLATFORM == PLATFORM_WINDOWS
	WSADATA WsaData;
	return WSAStartup( MAKEWORD(2,2), &WsaData ) == NO_ERROR;
#else
	return true;
#endif
}



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
		unsigned int value;
	} addr;
	unsigned short port;
};



struct Datagramm {
	char hdr[4]; // "\17VS\3"
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
};



static void *Server_main(void *);
class : public Obj {
	public:
		bool Start(WORD port) {
			state = State::Starting;
			this->port = port;
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
			if ( bind( sock, (const sockaddr*) &address, sizeof(sockaddr_in) ) < 0 ) {
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
				closesocket( sock );
				sock = 0;
			}
			if (mainThread != 0) {
				pthread_join(mainThread, 0);
			}
			state = State::Passive;
		}
} Server;



static void *Server_main(void *) {
	initializeSockets();
	while (Server.state != Obj::State::Stoping) {

	}
	shutdownSockets();
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
	initializeSockets();
	while (Client.state != Obj::State::Stoping) {
	}
	shutdownSockets();
}

