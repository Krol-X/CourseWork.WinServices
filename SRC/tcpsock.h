//
// ÇÀÃÎËÎÂÎÊ: TCPSOCK.H
//
// ÎÏÈÑÀÍÈÅ: êëàññ TcpSocket
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//

#include "sock.h"

class TcpSocket : public Socket {
	public:
		bool Open(unsigned short port);
		bool Accept();
		bool Connect(Address addr);
		bool IsConnected();
		void Disconnect();
};

