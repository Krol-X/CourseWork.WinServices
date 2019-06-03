#include "services.h"


//void tolog(char *s) {
//	FILE *f = fopen("log.txt", "a");
//	if (f) {
//		fputs(s, f);
//		fputs("\n", f);
//		fclose(f);
//	}
//}


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
//		tolog(buf);
//		tolog("Getting status of");
//		tolog(stat->lpServiceName);
//		tolog("Getting status");
		int x = SVC_GetStatus(stat->lpServiceName);
//		tolog("Put it");
		put(x);
		puts(stat->lpServiceName);
		puts(stat->lpDisplayName);
	}
//	tolog("s3");
	char *buf = new char[sz];
//	tolog("s4");
	memcpy(buf, tmp, sz);
//	tolog("s5");
	return buf;
}


int SVC_GetStatus(char *name) {
	int r = 0;
	SC_HANDLE hSCM, hService;
	DWORD Access = SC_MANAGER_CONNECT
	               | SC_MANAGER_ENUMERATE_SERVICE
	               ;
//	tolog("Open service");
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
//	tolog("Getting status");
	// 1. Get status
	SERVICE_STATUS stat;
	if (!QueryServiceStatus(hService, &stat)) {
		r |= 0x40;
	}
	// 2. Get service run config
//	tolog("Getting config");
	DWORD cbNeeded;
	char buf[8096];
	LPQUERY_SERVICE_CONFIG conf = (LPQUERY_SERVICE_CONFIG) buf;
	if (!QueryServiceConfig(hService, conf, 8096, &cbNeeded)) {
		r |= 0x80;
	}
//	tolog("ok");
	// tsTTTSSS; T - start type, S - state,
	// t - err get type, s - err get state
	r = (conf->dwStartType << 3) + stat.dwCurrentState;
//	tolog("Delete conf");
	delete[] conf;
	_VERIFY(CloseServiceHandle(hService));
	_VERIFY(CloseServiceHandle(hSCM));
//	tolog("ok");
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


int SVC_SetStatus(char *name, int flags) {
    // ...
	return SVC_GetStatus(name);
}
