//
// ЗАГОЛОВОК: SOCK.CPP
//
// ОПИСАНИЕ: абстрактный класс Socket
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//
#include "sock.h"
#include <assert.h>
#include <windows.h>


//
// КОНСТРУКТОР: Socket::Socket()
//
Socket::Socket() {
	sock = consock = 0;
}


//
// ДЕСТРУКТОР: Socket::~Socket()
//
Socket::~Socket() {
	if ( IsOpen() )
		Close();
}


//
// МЕТОД: bool Socket::OpenRand()
//
// НАЗНАЧЕНИЕ: открыть сокет на одном из свободных портов
//
bool Socket::OpenRand() {
	return Open(0);
}


//
// МЕТОД: void Socket::Close()
//
// НАЗНАЧЕНИЕ: закрыть сокет
//
void Socket::Close() {
	if ( sock != 0 ) {
		closesocket( sock );
		sock = 0;
	}
}


//
// МЕТОД: bool Socket::IsOpen()
//
// ВЕРНУТЬ: состояние сокета
//
bool Socket::IsOpen() {
	return ( sock != 0 );
}


//
// МЕТОД: bool Socket::Send(Address dest, void *data, int size)
//
// НАЗАНЧЕНИЕ: Отправить данные
//
// ВОЗВРАЩАЕТ: флаг успеха операции
//
bool Socket::Send(Address dest, void *data, int size) {
	assert( data );
	assert( size > 0 );
	if ( !IsOpen() )
		return false;
	assert( dest.addr != 0 );
	assert( dest.port != 0 );
	sockaddr_in to;
	to.sin_family = AF_INET;
	to.sin_addr.s_addr = htonl( dest.addr );
	to.sin_port = htons( (unsigned short)dest.port );
	int sent_bytes = sendto( sock, (char *)data, size, 0,
	                         (sockaddr *)&to, sizeof(to) );
	return sent_bytes == size;
}


//
// МЕТОД: int Socket::Receive(Address src, void *data, int size)
//
// НАЗАНЧЕНИЕ: Получить данные
//
// ВОЗВРАЩАЕТ: количество принятых байт
//
int Socket::Receive(Address src, void *data, int size) {
	assert( data );
	assert( size > 0 );
	if ( !IsOpen() )
		return 0;
	sockaddr_in from;
	int fromLength = sizeof(from);
	int received_bytes = recvfrom( sock, (char *)data, size, 0,
	                               (sockaddr *)&from, &fromLength );
	if ( received_bytes <= 0 )
		return 0;
	src.addr = ntohl( from.sin_addr.s_addr );
	src.port = ntohl( from.sin_port );
	return received_bytes;
}


//
// МЕТОД: Socket::Send(void *data, int size)
//
// НАЗАНЧЕНИЕ: Отправить данные через установленное соединение
//
// ВОЗВРАЩАЕТ: флаг успеха операции
//
bool Socket::Send(void *data, int size) {
	if ( IsConnected() )
		return Send( addr, data, size );
	else
		return false;
}


//
// МЕТОД: int Socket::Receive(void *data, int size)
//
// НАЗАНЧЕНИЕ: Получить данные через установленное соединение
//
// ВОЗВРАЩАЕТ: количество принятых байт
//
int Socket::Receive(void *data, int size) {
	if ( IsConnected() )
		return Receive( addr, data, size );
	else
		return false;
}
