//
// ���������: SOCK.CPP
//
// ��������: ���������� ������������ ������ Socket
//
#include "sock.h"
#include <assert.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>

#define MAX_WAIT 1.0
#define MAX_TRY  10
#define DELAY    0.1

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
// �����: bool Socket::Bind(Address addr)
//
// ����������: ��������� ����� � ����� (������� ��������)
//
// ����������: ���� ������ ��������
//
bool Socket::Bind(Address addr) {
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
// �����: void Socket::Wait()
//
void Socket::Wait() {
    wait(DELAY);
}


//
// �����: bool Socket::SetNonBlocking()
//
// ����������: ��������� ����� � ������������� �����
//
bool Socket::SetNonBlocking() {
	unsigned long nonBlocking = 1;
	bool r;
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
	r = ( fcntl( sock, F_SETFL, O_NONBLOCK, nonBlocking ) != -1 );
#elif PLATFORM == PLATFORM_WINDOWS
	r = ( ioctlsocket( sock, FIONBIO, &nonBlocking ) == 0 );
#endif
	return r;
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
// �����: bool Socket::IsServer()
//
// �������: ��� ������
//
bool Socket::IsServer() {
	return isserver;
}


//
// �������: void wait( float seconds )
//
// ����������: ������������� ��������� �� ��������� �����
//
void wait( float seconds ) {
#if PLATFORM == PLATFORM_WINDOWS
	Sleep( (int) ( seconds * 1000.0f ) );
#else
	usleep( (int) ( seconds * 1000000.0f ) );
#endif
}

