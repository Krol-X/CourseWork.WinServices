//
// ÇÀÃÎËÎÂÎÊ: TCPSOCK.H
//
// ÎÏÈÑÀÍÈÅ: êëàññ TcpSocket
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
		bool Send(void *data, int size);
		int Receive(void *data, int size);
		void Disconnect();
};

#endif

