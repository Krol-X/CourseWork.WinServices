#include "include.h"


struct Address {
	union {
		struct {
			unsigned char a, b, c, d;
		};
		unsigned int value;
	} addr;
	unsigned short port;
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
};



static void *Server_main(void *);
class : public Obj {
	public:
		bool Start(WORD port) {
			state = State::Starting;
			this->port = port;
			pthread_create(&mainThread, 0, Server_main, 0);
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
} Server;



static void *Server_main(void *) {
	while (Server.state != Obj::State::Stoping) {
		MessageBox(0, "...", "...", MB_OK);
		Sleep(3000); // For test
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
	while (Client.state != Obj::State::Stoping) {
	}
}

