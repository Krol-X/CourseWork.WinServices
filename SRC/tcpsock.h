//
// ���������: TCPSOCK.H
//
// ��������: ����� TcpSocket
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//

#include "sock.h"

class TcpSocket : Socket {
	public:
		TcpSocket();
		~TcpSocket();
		bool Open(unsigned short port);
		bool Accept();
		bool Connect(Address addr);
		bool IsConnected();
		void Disconnect();
};

