//
// ЗАГОЛОВОК: TCPSOCK.CPP
//
// ОПИСАНИЕ: класс TcpSocket
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//
// TODO: описания, код
//
#include "tcpsock.h"


TcpSocket::TcpSocket() {
}



TcpSocket::~TcpSocket() {
	~Socket(); // ISTRUE?
}



bool TcpSocket::Open(unsigned short port) {
	assert( !IsOpen() );
	// create socket
	socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( socket <= 0 ) {
		socket = 0;
		return false;
	}
	// bind to port
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( port );
	if ( bind(socket, (const sockaddr*) &address, sizeof(sockaddr_in)) < 0 ) {
		Close();
		return false;
	}
	// set non-blocking io
	unsigned int nonBlocking = 1;
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
	if ( fcntl( socket, F_SETFL, O_NONBLOCK, nonBlocking ) == -1 ) {
#elif PLATFORM == PLATFORM_WINDOWS
	if ( ioctlsocket( socket, FIONBIO, &nonBlocking ) != 0 ) {
#endif
		Close();
		return false;
	}
	return true;
}



bool TcpSocket::Accept() {
	listen( listener, MAX_CLIENTS );
	consock = accept( listener, 0, 0 );
	if( sock < 0 ) {
		consock = 0;
		return false;
	}
	return true;
}



bool TcpSocket::Connect(Address address) {
	struct sockaddr_in adr;
	adr.sin_family = AF_INET;
	adr.sin_port = htons(address.port);
	adr.sin_addr.s_addr = htonl( address.addr );
	if(connect(socket, (struct sockaddr *)&adr, sizeof(adr)) < 0) {
		// ?
		return false;
	}
	consock = -1; // заглушка для IsConnected()
	return true;
}



bool IsConnected() {
	return ( consock != 0 );
}



void Disconnect() {
	if ( IsConnected() ) {
		if ( consock == -1 )
			Close();
		else
			closesocket(consock);
		consock = 0;
	}
}
