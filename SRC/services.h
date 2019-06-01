//
// ÇÀÃÎËÎÂÎÊ-ÌÎÄÓËÜ: SERVICES.H
//
// ÎÏÈÑÀÍÈÅ: èíòåðôåéñ óïðàâëåíèÿ ñëóæáàìè Windows
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//
// TODO: îïèñàíèÿ
//
#include "include.h"


#define SVC_TIMEOUT     10000
/*
#define SERVICE_STOPPED          0x00000001
#define SERVICE_START_PENDING    0x00000002
#define SERVICE_STOP_PENDING     0x00000003
#define SERVICE_RUNNING          0x00000004
#define SERVICE_CONTINUE_PENDING 0x00000005
#define SERVICE_PAUSE_PENDING    0x00000006
#define SERVICE_PAUSED           0x00000007

#define SERVICE_CONTROL_STOP     0x00000001
#define SERVICE_CONTROL_PAUSE    0x00000002
#define SERVICE_CONTROL_CONTINUE 0x00000003

#define SERVICE_BOOT_START       0x00000000
#define SERVICE_SYSTEM_START     0x00000001
#define SERVICE_AUTO_START       0x00000002
#define SERVICE_DEMAND_START     0x00000003
#define SERVICE_DISABLED         0x00000004
*/
#define SERVICE_CONTROL_START    0x00000004



class __SVCObj {
	protected:
		SC_HANDLE hSCM;
		bool err;
	public:
		bool &lastErr() {
			return err;
		}
};



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



class ServiceObj : __SVCObj {
	private:
		char *name;
	public:

		extends(Status, ServiceObj)
	public:
		operator int() {
			int r = 0;
			SC_HANDLE hService;
			hService = OpenService(self->hSCM, self->name,
			                       SERVICE_QUERY_STATUS|SERVICE_QUERY_CONFIG);
			self->err = (hService == NULL);
			if (self->err)
				return -1;
			// 1. Get status
			LPSERVICE_STATUS stat = new SERVICE_STATUS;
			if (!QueryServiceStatus(hService, stat)) {
				self->err = true;
				r|=0x40;
			}
			// 2. Get service run config
			DWORD cbNeeded;
			if (!QueryServiceConfig(hService, NULL, 0, &cbNeeded)) {
				if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
					self->err = true;
					r|=0x80;
				}
			}
			LPQUERY_SERVICE_CONFIG conf = (LPQUERY_SERVICE_CONFIG)
			                              new char[cbNeeded];
			if (!QueryServiceConfig(hService, conf, cbNeeded, &cbNeeded)) {
				self->err = true;
				r|=0x80;
			}
			// tsTTTSSS; T - start type, S - state,
			// t - err get type, s - err get state
			r = (conf->dwStartType << 3) + stat->dwCurrentState;
			delete stat;
			delete [] conf;
			_VERIFY(CloseServiceHandle(hService));
			return r;
		}


		int operator =(int flags) {
			SC_HANDLE hService;
			bool r;
			_VERIFY(flags);
			switch (flags & 0x07) {
				case 0:
					break;
				case SERVICE_CONTROL_START:
					hService = OpenService(self->hSCM, self->name,
					                       SERVICE_START|SERVICE_QUERY_STATUS);
					self->err = (hService == NULL);
					if (self->err)
						return -1;
					if (!StartService(hService, 0, NULL)) {
						self->err = true;
						_VERIFY(CloseServiceHandle(hService));
						return -1;
					}
					SERVICE_STATUS SvcStatus;
					for (;;) {
						if (!QueryServiceStatus(hService, &SvcStatus)) {
							self->err = true;
							_VERIFY(CloseServiceHandle(hService));
							return -1;
						}
						if (SvcStatus.dwCurrentState != SERVICE_START_PENDING)
							break;
						DWORD dwWait = SvcStatus.dwWaitHint;
						if (dwWait == 0)
							dwWait = 1000;
						Sleep(dwWait);
					}
					_VERIFY(CloseServiceHandle(hService));
					if (SvcStatus.dwCurrentState != SERVICE_RUNNING) {
						self->err = true;
						return -1;
					}
					break;
				case SERVICE_CONTROL_STOP:
					hService = OpenService(self->hSCM, self->name,
					                       SERVICE_START|SERVICE_QUERY_STATUS);
					self->err = (hService == NULL);
					if (self->err)
						return -1;
					r = ControlServiceAndWait(hService,
					                          SERVICE_CONTROL_STOP,
					                          SERVICE_STOPPED, SVC_TIMEOUT);
					_VERIFY(CloseServiceHandle(hService));
					if (r==0) {
						self->err = true;
						return -1;
					}
					break;
				case SERVICE_CONTROL_PAUSE:
					hService = OpenService(self->hSCM, self->name,
					                       SERVICE_START|SERVICE_QUERY_STATUS);
					self->err = (hService == NULL);
					if (self->err)
						return -1;
					r = ControlServiceAndWait(hService,
					                          SERVICE_CONTROL_PAUSE,
					                          SERVICE_PAUSED, SVC_TIMEOUT);
					_VERIFY(CloseServiceHandle(hService));
					if (r==0) {
						self->err = true;
						return -1;
					}
					break;
				case SERVICE_CONTROL_CONTINUE:
					hService = OpenService(self->hSCM, self->name,
					                       SERVICE_START|SERVICE_QUERY_STATUS);
					self->err = (hService == NULL);
					if (self->err)
						return -1;
					r = ControlServiceAndWait(hService,
					                          SERVICE_CONTROL_CONTINUE,
					                          SERVICE_RUNNING, SVC_TIMEOUT);
					_VERIFY(CloseServiceHandle(hService));
					if (r==0) {
						self->err = true;
						return -1;
					}
					break;
				default:
					_VERIFY(false);
			}
			flags = (flags >> 3) & 7;
			if (flags) {
				_VERIFY(flags<=4);
				DWORD cbNeeded;
				if (!QueryServiceConfig(hService, NULL, 0, &cbNeeded)) {
					if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
						self->err = true;
						return -1;
					}
				}
				LPQUERY_SERVICE_CONFIG conf = (LPQUERY_SERVICE_CONFIG)
				                              new char[cbNeeded];
				if (!QueryServiceConfig(hService, conf, cbNeeded, &cbNeeded)) {
					self->err = true;
					return -1;
				}
				ChangeServiceConfig(
				    hService, SERVICE_NO_CHANGE, flags,
				    SERVICE_NO_CHANGE, 0, 0, 0, 0, 0, 0, 0
				);
				delete conf;
			}
			return (int)(*this);
		}
		extends_end(Status)



		ServiceObj(SC_HANDLE hSCM, LPCTSTR Name) {
			extends_init(Status);
			_VERIFY(hSCM!=0);
			name = new char[strlen(Name)];
			strcpy(name, Name);
			this->hSCM = hSCM;
		}
};



class SCMObj : __SVCObj {
	public:
		SCMObj() {
			hSCM = 0;
			err = false;
		}


		bool Init(LPCTSTR MachineName = 0,
		          LPCTSTR DBName = 0,
		          DWORD Access = SC_MANAGER_CONNECT |
		                         SC_MANAGER_ENUMERATE_SERVICE |
		                         SC_MANAGER_QUERY_LOCK_STATUS) {
			if (hSCM) {
				CloseServiceHandle(hSCM);
			}
			hSCM = OpenSCManager(MachineName, DBName, Access);
			err = (hSCM == NULL);
			return err;
		}


		bool Inited() {
			return (hSCM != 0);
		}


		void *getEnum(DWORD &size, DWORD &num) {
			if (!Inited())
				return 0;
			DWORD cbNeeded;
			DWORD dwResumeHandle = 0;
			char buf1[65536], tmp[65536];
			LPENUM_SERVICE_STATUS stat = (LPENUM_SERVICE_STATUS) buf1;
			if (!EnumServicesStatus(hSCM, SERVICE_WIN32, SERVICE_STATE_ALL,
			                        stat, 65536, &cbNeeded,
			                        &num, &dwResumeHandle))
				return 0;
			size = 0;
#define put(x) tmp[size++] = x;
#define puts(x) strcpy(tmp+size, x); size+=strlen(x)+1;
			for (int i=0; i<num; i++, stat++) {
				int x = ServiceObj(hSCM, stat->lpServiceName).Status;
				put(x);
				puts(stat->lpServiceName);
				puts(stat->lpDisplayName);
			}
			char *buf = new char[size];
			memcpy(buf, tmp, size);
			return buf;
		}


		ServiceObj getService(LPCTSTR name) {
			return ServiceObj(hSCM, name);
		}


		~SCMObj() {
			if (hSCM != NULL)
				_VERIFY(CloseServiceHandle(hSCM));
		}
};

