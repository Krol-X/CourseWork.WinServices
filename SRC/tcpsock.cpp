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
#include <time.h>

#define MAX_WAIT 1.0
#define MAX_TRY  10
#define DELAY    0.1

//
// МЕТОД: bool TcpSocket::Open()
//
// НАЗНАЧЕНИЕ: открыть сокет
//
// ВОЗРАЩАЕТ: флаг успеха операции
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
// ВОЗРАЩАЕТ: флаг успеха операции
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
// ВОЗРАЩАЕТ: флаг успеха операции
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
// ВОЗРАЩАЕТ: флаг успеха операции
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
// ВОЗРАЩАЕТ: состояние соединения
//
bool TcpSocket::IsConnected() {
	return ( consock != 0 );
}


//
// МЕТОД: bool TcpSocket::Send(void *data, int size)
//
// НАЗНАЧЕНИЕ: Отправить данные через установленное соединение
//
// ВОЗВРАЩАЕТ: флаг успеха операции
//
bool TcpSocket::Send(void *data, int size) {
	assert( data );
	assert( size > 0 );
	if ( !IsOpen() )
		return false;
	int _sock;
	if ( IsServer() )
		_sock = consock;
	else
		_sock = sock;
	int sent_bytes = send( _sock, (char *)data, size, 0 );
	return sent_bytes == size;
}


//
// МЕТОД: int TcpSocket::Receive(void *data, int size)
//
// НАЗНАЧЕНИЕ: Получить данные через установленное соединение
//
// ВОЗВРАЩАЕТ: количество принятых байт
//
int TcpSocket::Receive(void *data, int size) {
	assert( data );
	assert( size > 0 );
	if ( !IsOpen() )
		return 0;
	int _sock;
	if ( IsServer() )
		_sock = consock;
	else
		_sock = sock;
	int received_bytes;
	clock_t start = clock();
	for ( int trycount = 0; trycount < MAX_TRY; trycount++ ) {
		double seconds;
		do {
			received_bytes = recv( _sock, (char *)data, size, 0 );
			if ( received_bytes > 0 )
				break;
			clock_t end = clock();
			seconds = (double)( end - start ) / CLOCKS_PER_SEC;
			wait(DELAY);
		} while ( seconds < MAX_WAIT );
		if ( received_bytes > 0 )
			break;
	}
	if ( received_bytes <= 0 )
		return 0;
	return received_bytes;
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

