//
// ���������: SOCK.H
//
// ��������: ����������� ����� Socket
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//
#define MAX_CLIENTS 5


#define PLATFORM_WINDOWS  1
#define PLATFORM_MAC      2
#define PLATFORM_UNIX     3

#if defined(_WIN32)
#define PLATFORM PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM PLATFORM_MAC
#else
#define PLATFORM PLATFORM_UNIX
#endif

#if PLATFORM == PLATFORM_WINDOWS
#include <winsock2.h>
#pragma comment( lib, "wsock32.lib" )
#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#define closesocket(socket) close(socket)
#else
#error unknown platform!
#endif



//
// �����-���������: Address
//
// ����������: ����� � ����
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



//
// �����: Socket
//
// ����������: ���������� ������
//
class Socket {
	protected:
		int sock;
		int consock; // ����� ����������
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
		bool Send(Address dest, void *data, int size);
		int Receive(Address src, void *data, int size);
};

