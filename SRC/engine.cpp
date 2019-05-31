#include "include.h"
#include "engine.h"
#include "services.h"

////////////////////////////////////////////////////////////////////////////////
// ������: ��������� � ���������
//

#define ProtocolId *((DWORD *)"\17VS\3")
#define CMD_ANY      0
#define CMD_CHECKOUT 0x80000000
#define CMD_LIST     0x0�000000
#define CMD_SET      0x18000000

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


//
// �������: bool InitializeSockets()
//
// ����������: ���������������� ������ � ��������
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
// �������: void ShutdownSockets()
//
// ����������: ��������� ������ � ��������
//
void ShutdownSockets() {
#if PLATFORM == PLATFORM_WINDOWS
	WSACleanup();
#endif
}






