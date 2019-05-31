//
// ЗАГОЛОВОК: SOCK.CPP
//
// ОПИСАНИЕ: абстрактный класс Socket
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//
// TODO: описания
//
#include "sock.h"
#include <windows.h>
#define closesocket(socket) close(socket)



Socket::Socket() {
	socket = consock = 0;
}



Socket::~Socket() {
	if ( IsOpen() )
		Close();
}



bool Socket::OpenRand() {
	int x;
	// TODO: realize
	return Open(x);
}



void Socket::Close() {
	if ( socket != 0 ) {
		closesocket( socket );
		socket = 0;
	}
}



bool Socket::IsOpen() {
	return ( socket != 0 );
}



bool Socket::Send(Address dest, void *data, DWORD sz) {
	assert( data );
	assert( size > 0 );
	if ( !IsOpen() )
		return false;
	assert( dest.addr != 0 );
	assert( dest.port != 0 );
	sockaddr_in to;
	socklen_t toLength = sizeof( to );
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl( dest.addr );
	address.sin_port = htons( (unsigned short)dest.port );

	int sent_bytes = sendto( socket, (char *)data, size, 0,
	                         (sockaddr *)&address, toLength );

	return sent_bytes == size;
}



DWORD Socket::Receive(Address src, void *data, DWORD sz) {
	assert( data );
	assert( size > 0 );
	if ( !IsOpen() )
		return 0;
	sockaddr_in from;
	socklen_t fromLength = sizeof( from );
	int received_bytes = recvfrom( socket, (char *)data, size, 0,
	                               (sockaddr *)&from, &fromLength );
	if ( received_bytes <= 0 )
		return 0;
	src.addr = ntohl( from.sin_addr.s_addr );
	src.port = ntohl( from.sin_port );
	return received_bytes;
}
