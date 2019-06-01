//
// ЗАГОЛОВОК: TCPSOCK.CPP
//
// ОПИСАНИЕ: класс TcpSocket
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//
#include "tcpsock.h"
#include <assert.h>
#include <windows.h>


//
// МЕТОД: bool TcpSocket::Open(unsigned short port)
//
// НАЗАНЧЕНИЕ: открыть сокет
//
// ВОЗВРАЩАЕТ: флаг успеха операции
//
bool TcpSocket::Open(unsigned short port) {
	assert( !IsOpen() );
	// create socket
	sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( sock <= 0 ) {
		sock = 0;
		return false;
	}
	// bind to port
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( port );
	if ( bind( sock, (const sockaddr*) &address, sizeof(sockaddr_in) ) < 0 ) {
		Close();
		return false;
	}
	// set non-blocking io
	unsigned long nonBlocking = 1;
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
	if ( fcntl( sock, F_SETFL, O_NONBLOCK, nonBlocking ) == -1 ) {
#elif PLATFORM == PLATFORM_WINDOWS
	if ( ioctlsocket( sock, FIONBIO, &nonBlocking ) != 0 ) {
#endif
		Close();
		return false;
	}
	return true;
}


//
// МЕТОД: bool TcpSocket::Accept()
//
// НАЗАНЧЕНИЕ: ожидать нового клиента, соединится с ним
//
// ВОЗВРАЩАЕТ: флаг успеха операции
//
bool TcpSocket::Accept() {
	listen( sock, MAX_CLIENTS );
	consock = accept( sock, 0, 0 );
	if( consock < 0 ) {
		consock = 0;
		return false;
	}
	return true;
}


//
// МЕТОД: bool TcpSocket::Connect(Address address)
//
// НАЗАНЧЕНИЕ: соединится с сервером
//
// ВОЗВРАЩАЕТ: флаг успеха операции
//
bool TcpSocket::Connect(Address address) {
	struct sockaddr_in adr;
	adr.sin_family = AF_INET;
	adr.sin_port = htons( address.port );
	adr.sin_addr.s_addr = htonl( address.addr );
	if( connect( sock, (struct sockaddr *)&adr, sizeof(adr) ) < 0 ) {
		consock = 0;
		return false;
	}
	addr = address;
	consock = -1; // заглушка для IsConnected()
	return true;
}


//
// МЕТОД: bool TcpSocket::IsConnected()
//
// ВОЗВРАЩАЕТ: состояние соединения
//
bool TcpSocket::IsConnected() {
	return ( consock != 0 );
}


//
// МЕТОД: void TcpSocket::Disconnect()
//
// НАЗАНЧЕНИЕ: разорвать связь, закрыть сокет (если сервер, то сокет соединения)
//
void TcpSocket::Disconnect() {
	if ( IsConnected() ) {
		if ( consock == -1 )
			Close();
		else
			closesocket( consock );
		consock = 0;
	}
}

