//
// ÇÀÃÎËÎÂÎÊ: SOCK.H
//
// ÎÏÈÑÀÍÈÅ: àáñòğàêòíûé êëàññ Socket
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//
#define MAX_CLIENTS 5


//
// ÊËÀÑÑ-ÑÒĞÓÊÒÓĞÀ: Address
//
// ÑÎÄÅĞÆÈÌÎÅ: àäğåñ è ïîğò
//
struct Address {
	union {
		struct {
			unsigned char a, b, c, d;
		};
		unsigned int addr;
	};
	unsigned short port;
	Address() {
		addr = port = 0;
	}
	Address(unsigned int addr, unsigned short port) {
		this->addr = addr;
		this->port = port;
	}
	bool operator == (Address &x) {
		return (x.addr == addr) && (x.port == port);
	}
	bool operator != (Address &x) {
		return !(*this == x);
	}
};



class Socket {
	protected:
		int socket;
		int consock; // ñîêåò ñîåäèíåíèÿ
	public:
		Socket();
		~Socket();
		virtual bool Open(unsigned short port) = 0;
		bool OpenRand();
		void Close();
		bool IsOpen();
		virtual bool Accept() = 0;
		virtual bool Connect(Address addr) = 0;
		virtual bool IsConnected() = 0;
		virtual void Disconnect() = 0;
		bool Send(Address dest, void *data, unsigned int sz);
		unsigned int Receive(Address src, void *data, unsigned int sz);
};

