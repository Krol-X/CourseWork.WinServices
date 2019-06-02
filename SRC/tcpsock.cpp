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
// МЕТОД: bool TcpSocket::Open()
//
// НАЗНАЧЕНИЕ: открыть сокет
//
// ВОЗВРАЩАЕТ: флаг успеха операции
//
bool TcpSocket::Open() {
	assert( !IsOpen() );
	isserver = false;
	// create socket
	sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( sock <= 0 ) {
		sock = 0;
		return false;
	}
	return true;
}


//
// МЕТОД: bool TcpSocket::Bind(Address addr)
//
// НАЗНАЧЕНИЕ: привязать сокет к порту (сделать сервером)
//
// ВОЗВРАЩАЕТ: флаг успеха операции
//
bool TcpSocket::Bind(Address addr) {
	assert( IsOpen() );
	// bind to port
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = addr.addr;
	address.sin_port = htons( addr.port );
	if ( bind( sock, (const sockaddr*) &address, sizeof(sockaddr_in) ) < 0 ) {
		Close();
		return false;
	}
	isserver = true;
	return true;
}


//
// МЕТОД: bool TcpSocket::Accept()
//
// НАЗНАЧЕНИЕ:  нового клиента, соединится с ним
//
// ВОЗВРАЩАЕТ: флаг успеха операции
//
bool TcpSocket::Accept() {
	sockaddr cAddr;
	int cAddrSz = sizeof(cAddr);
	consock = -1;
	if ( listen( sock, 1 ) == 0 )
		consock = accept( sock, &cAddr, &cAddrSz );
	if( consock < 0 ) {
		consock = 0;
		return false;
	}
	return true;
}


//
// МЕТОД: bool TcpSocket::Connect(Address address)
//
// НАЗНАЧЕНИЕ: соединится с сервером
//
// ВОЗВРАЩАЕТ: флаг успеха операции
//
bool TcpSocket::Connect(Address address) {
	struct sockaddr_in adr;
	adr.sin_family = AF_INET;
	adr.sin_port = htons( address.port );
	adr.sin_addr.s_addr = htonl( address.addr );
	int i = connect( sock, (struct sockaddr *)&adr, sizeof(adr) );
	if( i < 0 ) {
		consock = 0;
		return false;
	}
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
// НАЗНАЧЕНИЕ: разорвать связь, закрыть сокет (если сервер, то сокет соединения)
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

