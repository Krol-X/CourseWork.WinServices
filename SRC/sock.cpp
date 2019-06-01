//
// ���������: SOCK.CPP
//
// ��������: ����������� ����� Socket
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//
#include "sock.h"
#include <assert.h>
#include <windows.h>


//
// �����������: Socket::Socket()
//
Socket::Socket() {
	sock = consock = 0;
}


//
// ����������: Socket::~Socket()
//
Socket::~Socket() {
	if ( IsOpen() )
		Close();
}


//
// �����: bool Socket::OpenRand()
//
// ����������: ������� ����� �� ����� �� ��������� ������
//
bool Socket::OpenRand() {
	return Open(0);
}


//
// �����: void Socket::Close()
//
// ����������: ������� �����
//
void Socket::Close() {
	if ( sock != 0 ) {
		closesocket( sock );
		sock = 0;
	}
}


//
// �����: bool Socket::IsOpen()
//
// �������: ��������� ������
//
bool Socket::IsOpen() {
	return ( sock != 0 );
}


//
// �����: bool Socket::Send(Address dest, void *data, int size)
//
// ����������: ��������� ������
//
// ����������: ���� ������ ��������
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
// �����: int Socket::Receive(Address src, void *data, int size)
//
// ����������: �������� ������
//
// ����������: ���������� �������� ����
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
// �����: Socket::Send(void *data, int size)
//
// ����������: ��������� ������ ����� ������������� ����������
//
// ����������: ���� ������ ��������
//
bool Socket::Send(void *data, int size) {
	if ( IsConnected() )
		return Send( addr, data, size );
	else
		return false;
}


//
// �����: int Socket::Receive(void *data, int size)
//
// ����������: �������� ������ ����� ������������� ����������
//
// ����������: ���������� �������� ����
//
int Socket::Receive(void *data, int size) {
	if ( IsConnected() )
		return Receive( addr, data, size );
	else
		return false;
}
