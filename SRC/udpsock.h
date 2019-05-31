//
// ÇÀÃÎËÎÂÎÊ: UDPSOCK.H
//
// ÎÏÈÑÀÍÈÅ: êëàññ UdpSocket
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//

#include "sock.h"

class UdpSocket : Socket {
	public:
		UdpSocket();
		~UdpSocket();
		bool Open(unsigned short port);
		bool Accept();
		bool Connect(Address addr);
		bool IsConnected();
		void Disconnect();
}
