#include "include.h"
#include "engine.h"
#include "services.h"

////////////////////////////////////////////////////////////////////////////////
// СЕКЦИЯ: Настройки и константы
//

#define ProtocolId *((DWORD *)"\17VS\3")
#define CMD_ANY      0
#define CMD_CHECKOUT 0x80000000
#define CMD_LIST     0x0С000000
#define CMD_SET      0x18000000

////////////////////////////////////////////////////////////////////////////////
// СЕКЦИЯ: Сервер
//


Server::Server() {
	
}


Server::~Server() {

}


bool Server::Start(unsigned int port) {

}


bool Server::IsWorking() {

}


void Server::Stop() {

}

////////////////////////////////////////////////////////////////////////////////
// СЕКЦИЯ: Клиент
//

Client::Client() {

}


Client::~Client() {

}


bool Client::Init(Address addr) {

}


void Client::Done() {

}


bool Client::GetList() {

}


unsigned int Client::ListSize() {

}


ListItem* Client::ListItem(unsigned int idx) {
	
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

