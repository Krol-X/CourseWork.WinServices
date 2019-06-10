//
// ÇÀÃÎËÎÂÎÊ: UDPSOCK.H
//
// ÎÏÈÑÀÍÈÅ: êëàññ UdpSocket
//
#include "sock.h"
#include <time.h>

class UdpSocket : public Socket {
	protected:
		double timeout;
		clock_t start;
		Address conaddr;
        char tmpbuf[1024];
        int tmpsz;
		bool chktimeout();
	public:
		bool Open();
		bool Accept();
		bool Connect(Address addr);
		bool IsConnected();
		bool Send(void *data, int size);
		int Receive(void *data, int size);
		void Disconnect();
};
