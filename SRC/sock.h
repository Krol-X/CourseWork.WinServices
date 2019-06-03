//
// ЗАГОЛОВОК: SOCK.H
//
// ОПИСАНИЕ: абстрактный класс Socket
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//

#ifndef sock_h_
#define sock_h_

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
// КЛАСС-СТРУКТУРА: Address
//
// СОДЕРЖИМОЕ: адрес и порт
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
// КЛАСС: Socket
//
// НАЗНАЧЕНИЕ: реализация сокета
//
class Socket {
	protected:
		int sock;
		int consock; // сокет соединения
		bool isserver;
	public:
		Socket();
		~Socket();
		void Wait();
		bool SetNonBlocking();
		virtual bool Open() = 0;
		virtual bool Bind(Address addr) = 0;
		void Close();
		bool IsOpen();
		bool IsServer();
		virtual bool Accept() = 0;
		virtual bool Connect(Address addr) = 0;
		virtual bool IsConnected() = 0;
		virtual void Disconnect() = 0;
		bool Send(void *data, int size);
		int Receive(void *data, int size);
};


// Приостановить программу на некоторое время
void wait( float seconds );

#endif

