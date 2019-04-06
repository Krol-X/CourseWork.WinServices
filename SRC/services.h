//
// ЗАГОЛОВОК МОДУЛЯ: SERVICES.CPP
//
// ОПИСАНИЕ: интерфейс управления службами Windows NT
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//
#include <windows.h>

class ServiceObj {
	private:
	
	public:
		ServiceObj();
		~ServiceObj();
}

class SCMObj {
	private:
		bool inited;
		int err;
	public:
		SCMObj();
		bool Init(PCTSTR MachineName = 0,
		          PCTSTR DBName = 0,
		          DWORD Access = SC_MANAGER_CONNECT |
		                         SC_MANAGER_ENUMERATE_SERVICE |
		                         SC_MANAGER_QUERY_LOCK_STATUS);
		int lastErr();
		int getNum();
		LPENUM_SERVICE_STATUS getEnum(bool &isEnd);

		~SCMObj();
}
