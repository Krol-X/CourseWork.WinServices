#include "engine.h"



inline bool InitializeSockets() {
#if PLATFORM == PLATFORM_WINDOWS
	WSADATA WsaData;
	return WSAStartup( MAKEWORD(2,2), &WsaData ) == NO_ERROR;
#else
	return true;
#endif
}

inline void ShutdownSockets() {
#if PLATFORM == PLATFORM_WINDOWS
	WSACleanup();
#endif
}