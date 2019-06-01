//
// МОДУЛЬ: ENGINE.CPP
//
// ОПИСАНИЕ: реализация работы сервера и клиента
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//
#include "include.h"
#include "engine.h"
#include "services.h"


////////////////////////////////////////////////////////////////////////////////
// СЕКЦИЯ: Настройки, константы и структура датаграммы
//

#define ProtocolId *((DWORD *)"\17VS\3")
#define CMD_ANY      0
#define CMD_CHECKOUT 0x80000000
#define CMD_LIST     0x0С000000
#define CMD_SET      0x18000000

//
// СТРУКТУРА: Datagram
//
// СОДЕРЖИМОЕ: передоваемая комманда и данные
//
struct Datagram {
	DWORD cmd_cou;
	char data[];
};
#define RESERVED_BYTES 4
#define BUF_SIZE RESERVED_BYTES+65536



////////////////////////////////////////////////////////////////////////////////
// СЕКЦИЯ: Сервер
//

#define GetParam(x, y) pthread_mutex_lock(&param.mutex); \
	x = param.y; pthread_mutex_unlock(&param.mutex);
#define SetParam(x, y) pthread_mutex_lock(&param.mutex); \
	param.x = y; pthread_mutex_unlock(&param.mutex);
#define GetParam1(x, y) pthread_mutex_lock(&param->mutex); \
	x = param->y; pthread_mutex_unlock(&param->mutex);
#define SetParam1(x, y) pthread_mutex_lock(&param->mutex); \
	param->x = y; pthread_mutex_unlock(&param->mutex);


//
// ФУНКЦИЯ: static void *Server_main(void *)
//
// НАЗНАЧЕНИЕ: приём и обработкаа сообщений
//
// ТАКТИКА: при приёме датаграммы создание потока обработки сообщения
//
static void *Server_main(void *_param) {
	bool x;
	ServerParams *param = (ServerParams *)_param;
	Socket *sock = &param->sock;
	do {
		if ( sock->Accept() ) {
			// ...
			sock->Disconnect();
		}
		GetParam1( x, active );
	} while ( x );
}


//
// МЕТОД: bool Server::Start(unsigned int port)
//
// НАЗНАЧЕНИЕ: запустить сервер и поток обработки сообщений
//
// ВОЗВРАЩАЕТ: флаг успеха операции
//
bool Server::Start(unsigned int port) {
	assert( !IsWorking() );
	bool r;
	if (port < 1024)
		r = param.sock.OpenRand();
	else
		r = param.sock.Open( port );
	if (r) {
		r = ( pthread_create( &thread, 0, Server_main, &param ) == 0 );
		if (!r)
			param.sock.Close();
	}
	SetParam( active, r );
	return r;
}


//
// МЕТОД: bool Server::IsWorking()
//
// ВОЗВРАЩАЕТ: состояние сервера
//
// ВОЗВРАЩАЕТ: флаг успеха операции
//
bool Server::IsWorking() {
	GetParam( bool r, active );
	return r;
}


//
// МЕТОД: void Server::Stop()
//
// НАЗНАЧЕНИЕ: остановить сервер и поток обработки сообщений
//
void Server::Stop() {
	if ( IsWorking() ) {
		SetParam( active, false );
		pthread_join( thread, 0 );
		if ( param.sock.IsOpen() )
			param.sock.Close();
	}
}



////////////////////////////////////////////////////////////////////////////////
// СЕКЦИЯ: Клиент
//

//
// МЕТОД: bool Client::GetList()
//
// НАЗНАЧЕНИЕ: получить список служб сервера
//
// ВОЗВРАЩАЕТ: флаг успеха операции
//
bool Client::GetList(Address addr) {
	bool r;
	assert( !param.sock.IsOpen() );
	r = param.sock.OpenRand();
	if (r) {
		r = param.sock.Connect( addr );
		if (r) {

			param.sock.Disconnect();
		}
		if ( param.sock.IsOpen() )
			param.sock.Close();
	}
	return r;
}


//
// МЕТОД: unsigned int Client::ListSize()
//
// ВОЗВРАЩАЕТ: размер списка
//
unsigned int Client::ListSize() {
	unsigned int r;
	GetParam( r, list.size() );
	return r;
}


//
// МЕТОД: ListItem* Client::ListItem(unsigned int idx)
//
// ВОЗВРАЩАЕТ: указатель на элемент списка
//
ListItem* Client::ListItem(unsigned int idx) {
	struct ListItem* r;
	pthread_mutex_lock(&param.mutex);
	r = &param.list[idx];
	pthread_mutex_unlock(&param.mutex);
	return r;
}



////////////////////////////////////////////////////////////////////////////////
// СЕКЦИЯ: Вспомогательные функции
//

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


//
// ФУНКЦИЯ: bool InitializeSockets()
//
// НАЗНАЧЕНИЕ: инициализировать работу с сокетами
//
// ВОЗВРАЩАЕТ: флаг успеха операции
//
bool InitializeSockets() {
#if PLATFORM == PLATFORM_WINDOWS
	WSADATA WsaData;
	return WSAStartup( MAKEWORD(2,2), &WsaData ) == NO_ERROR;
#else
	return true;
#endif
}


//
// ФУНКЦИЯ: void ShutdownSockets()
//
// НАЗНАЧЕНИЕ: завершить работу с сокетами
//
void ShutdownSockets() {
#if PLATFORM == PLATFORM_WINDOWS
	WSACleanup();
#endif
}

