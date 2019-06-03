//
// ÇÀÃÎËÎÂÎÊ: TCPSOCK.H
//
// ÎÏÈÑÀÍÈÅ: êëàññ TcpSocket
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//

#ifndef tcpsock_h_
#define tcpsock_h_

#include "sock.h"

class TcpSocket : public Socket {
	public:
		bool Open();
		bool Bind(Address addr);
		bool Accept();
		bool Connect(Address addr);
		bool IsConnected();
		void Disconnect();
};

#endif

