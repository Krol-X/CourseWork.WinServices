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
#include <stdlib.h>
#include <time.h>

#define MAX_WAIT 1.0
#define MAX_TRY  10
#define DELAY    0.1

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
// МЕТОД: void Socket::Wait()
//
void Socket::Wait() {
    wait(DELAY);
}


//
// МЕТОД: bool Socket::SetNonBlocking()
//
// НАЗНАЧЕНИЕ: перевести сокет в неблокирующий режим
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
// МЕТОД: bool Socket::IsServer()
//
// ВЕРНУТЬ: тип сокета
//
bool Socket::IsServer() {
	return isserver;
}


//
// МЕТОД: bool Socket::Send(void *data, int size)
//
// НАЗНАЧЕНИЕ: Отправить данные через установленное соединение
//
// ВОЗВРАЩАЕТ: флаг успеха операции
//
bool Socket::Send(void *data, int size) {
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
// МЕТОД: int Socket::Receive(void *data, int size)
//
// НАЗНАЧЕНИЕ: Получить данные через установленное соединение
//
// ВОЗВРАЩАЕТ: количество принятых байт
//
int Socket::Receive(void *data, int size) {
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
// ФУНКЦИЯ: void wait( float seconds )
//
// НАЗНАЧЕНИЕ: приостановить программу на некоторое время
//
void wait( float seconds ) {
#if PLATFORM == PLATFORM_WINDOWS
	Sleep( (int) ( seconds * 1000.0f ) );
#else
	usleep( (int) ( seconds * 1000000.0f ) );
#endif
}

