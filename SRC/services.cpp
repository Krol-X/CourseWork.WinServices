#include "services.h"


void *SVC_getEnum(DWORD &sz, DWORD &num) {
	DWORD Access = SC_MANAGER_CONNECT
	               | SC_MANAGER_ENUMERATE_SERVICE
	               | SC_MANAGER_QUERY_LOCK_STATUS
	               ;
	SC_HANDLE hSCM;
	hSCM = OpenSCManager(0, 0, Access);
	sz = num = 0;
	if ( hSCM == 0 )
		return 0;
	DWORD cbNeeded;
	DWORD dwResumeHandle = 0;
	char buf1[65536], tmp[65536];
	LPENUM_SERVICE_STATUS stat = (LPENUM_SERVICE_STATUS) buf1;
	if (!EnumServicesStatus(hSCM, SERVICE_WIN32, SERVICE_STATE_ALL,
	                        stat, 65536, &cbNeeded,
	                        &num, &dwResumeHandle))
		return 0;
	sz = 0;
#define put(x) tmp[sz++] = x;
#define puts(x) strcpy(tmp+sz, x); sz+=strlen(x)+1;
	for (DWORD i=0; i<num; i++, stat++) {
		char buf[20];
		itoa( sz, buf, 10 );
		int x = SVC_GetStatus(stat->lpServiceName);
		put(x);
		puts(stat->lpServiceName);
		puts(stat->lpDisplayName);
	}
	char *buf = new char[sz];
	memcpy(buf, tmp, sz);
	return buf;
}


int SVC_GetStatus(char *name) {
	int r = 0;
	SC_HANDLE hSCM, hService;
	DWORD Access = SC_MANAGER_CONNECT
	               | SC_MANAGER_ENUMERATE_SERVICE
	               ;
	hSCM = OpenSCManager(0, 0, Access);
	if ( hSCM == 0 )
		return -1;
	hService = OpenService(hSCM, name,
	                       SERVICE_QUERY_STATUS
	                       | SERVICE_QUERY_CONFIG);
	if ( hService == 0 ) {
		_VERIFY(CloseServiceHandle(hService));
		return -1;
	}
	// 1. Get status
	SERVICE_STATUS stat;
	if (!QueryServiceStatus(hService, &stat)) {
		r |= 0x40;
	}
	// 2. Get service run config
	DWORD cbNeeded;
	char buf[8096];
	LPQUERY_SERVICE_CONFIG conf = (LPQUERY_SERVICE_CONFIG) buf;
	if (!QueryServiceConfig(hService, conf, 8096, &cbNeeded)) {
		r |= 0x80;
	}
	// tsTTTSSS; T - start type, S - state,
	// t - err get type, s - err get state
	r = (conf->dwStartType << 3) + stat.dwCurrentState;
	delete[] conf;
	_VERIFY(CloseServiceHandle(hService));
	_VERIFY(CloseServiceHandle(hSCM));
	return r;
}


//---------------------------------------------------------------------------
// ControlServiceAndWait
//
//  Sends a control request to a service and waits until the service reaches
//	the specified state.
//
//  Parameters:
//	  hService  - service handle
//	  dwControl - control code
//	  dwState	- service state to wait for
//	  dwTimeout - timeout in milliseconds
//
//  Returns:
//	  TRUE, if successful, FALSE - otherwise.
//
static BOOL ControlServiceAndWait(
    IN SC_HANDLE hService,
    IN DWORD dwControl,
    IN DWORD dwState,
    IN DWORD dwTimeout
) {
	_VERIFY(hService != NULL);

	SERVICE_STATUS Status;
	DWORD dwStart;
	DWORD dwCheckPoint = (DWORD)-1;
	DWORD dwWait;

	// send control code
	if (!ControlService(hService, dwControl, &Status))
		return FALSE;

	// remember when the operation was started
	dwStart = GetTickCount();

	// wait until the service reaches the specified state
	while (Status.dwCurrentState != dwState &&
	        Status.dwCurrentState != SERVICE_STOPPED) {
		// check if timeout has expired
		if (dwTimeout != INFINITE) {
			if (GetTickCount() - dwStart >= dwTimeout)
				return SetLastError(ERROR_TIMEOUT), FALSE;
		}

		// determine how long to wait
		if (dwCheckPoint != Status.dwCheckPoint) {
			dwCheckPoint = Status.dwCheckPoint;
			dwWait = Status.dwWaitHint;
		} else {
			dwWait = 1000;
		}

		// do wait
		Sleep(dwWait);

		// query service status
		if (!QueryServiceStatus(hService, &Status))
			return FALSE;
	}

	if (Status.dwCurrentState == SERVICE_STOPPED &&
	        Status.dwWin32ExitCode != ERROR_SUCCESS) {
		SetLastError(Status.dwWin32ExitCode);
		return FALSE;
	}

	return TRUE;
}


int SVC_SetStatus(char *name, int state) {
	BYTE st;
	int r = -1;
	SC_HANDLE hSCM, hService;
	DWORD Access = SC_MANAGER_CONNECT
	               | SC_MANAGER_ENUMERATE_SERVICE
	               ;
	hSCM = OpenSCManager(0, 0, Access);
	if ( hSCM == 0 )
		return -1;
	st = state & 7;
	switch (st) {
		case SERVICE_CONTROL_STOP:
			hService = OpenService(hSCM, name,
			                       SERVICE_START
			                       | SERVICE_QUERY_STATUS );
			if ( hService == 0 )
				return -1;
			r = ControlServiceAndWait( hService,
			                           SERVICE_CONTROL_STOP,
			                           SERVICE_STOPPED,
			                           SVC_TIMEOUT );
			break;
		case SERVICE_CONTROL_PAUSE:
			hService = OpenService(hSCM, name,
			                       SERVICE_PAUSE_CONTINUE
			                       | SERVICE_QUERY_STATUS );
			if ( hService == 0 )
				return -1;
			r = ControlServiceAndWait( hService,
			                           SERVICE_CONTROL_PAUSE,
			                           SERVICE_PAUSED,
			                           SVC_TIMEOUT );
			break;
		case SERVICE_CONTROL_CONTINUE:
			hService = OpenService(hSCM, name,
			                       SERVICE_PAUSE_CONTINUE
			                       | SERVICE_QUERY_STATUS );
			if ( hService == 0 )
				return -1;
			r = ControlServiceAndWait( hService,
			                           SERVICE_CONTROL_CONTINUE,
			                           SERVICE_RUNNING,
			                           SVC_TIMEOUT );
			break;
		case SERVICE_CONTROL_START:
			hService = OpenService( hSCM, name,
			                        SERVICE_START
			                        | SERVICE_QUERY_STATUS );
			if ( hService == 0 )
				return -1;
			r = StartService( hService, 0, 0 );
			break;
	}

	st = (state >> 3) & 7;
	if (2 <= st && st <= 4) {
		DWORD cbNeeded = 0;
		char buf[65536];
		LPQUERY_SERVICE_CONFIG conf = (LPQUERY_SERVICE_CONFIG) buf;
		QueryServiceConfig(hService, conf, 65536, &cbNeeded);
		ChangeServiceConfig(
		    hService, SERVICE_NO_CHANGE, st,
		    SERVICE_NO_CHANGE, 0, 0, 0, 0, 0, 0, 0
		);
	}
	_VERIFY( CloseServiceHandle( hService ) );
	if ( r == 0 )
		return -1;
	return SVC_GetStatus(name);
}

