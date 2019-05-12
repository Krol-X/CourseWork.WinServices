//
// ÇÀÃÎËÎÂÎÊ-ÌÎÄÓËÜ: SERVICES.H
//
// ÎÏÈÑÀÍÈÅ: èíòåðôåéñ óïðàâëåíèÿ ñëóæáàìè Windows
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//
#include "include.h"

#define SVC_TIMEOUT     10000
/*
#define SERVICE_STOPPED 0x00000001
#define SERVICE_START_PENDING 0x00000002
#define SERVICE_STOP_PENDING 0x00000003
#define SERVICE_RUNNING 0x00000004
#define SERVICE_CONTINUE_PENDING 0x00000005
#define SERVICE_PAUSE_PENDING 0x00000006
#define SERVICE_PAUSED 0x00000007

#define SERVICE_CONTROL_STOP 0x00000001
#define SERVICE_CONTROL_PAUSE 0x00000002
#define SERVICE_CONTROL_CONTINUE 0x00000003

#define SERVICE_BOOT_START 0x00000000
#define SERVICE_SYSTEM_START 0x00000001
#define SERVICE_AUTO_START 0x00000002
#define SERVICE_DEMAND_START 0x00000003
#define SERVICE_DISABLED 0x00000004
*/


enum State {
	RESUMING = SERVICE_CONTINUE_PENDING,
	PAUSING = SERVICE_PAUSE_PENDING,
	PAUSED = SERVICE_PAUSED,
	ACTIVE = SERVICE_RUNNING,
	STARTING = SERVICE_START_PENDING,
	STOPING = SERVICE_STOP_PENDING,
	PASSIVE = SERVICE_STOPPED,
	UNKNOWN = 0
};



class __SVCObj {
	public:
		enum Error {
			OK = 0,
			NOT_INITED,
			NOT_GETSTATUS,
			NOT_GETCONFIG,
			NOT_STARTED,
			NOT_STOPPED
		};
	protected:
		SC_HANDLE hSCM;
		Error err;
	public:
		Error &lastErr() {
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
		operator State() {
			SC_HANDLE hService;
			LPSERVICE_STATUS stat = new SERVICE_STATUS;
			hService = OpenService(self->hSCM, self->name,
			                       SERVICE_QUERY_STATUS);
			self->err = (hService == NULL)? NOT_INITED: OK;
			if (self->err)
				return UNKNOWN;

			if (!QueryServiceStatus(hService, stat)) {
				self->err = NOT_GETSTATUS;
				return UNKNOWN;
			}

			State r;
			switch (stat->dwCurrentState) {
				case SERVICE_STOPPED:
					r = PASSIVE;
					break;
				case SERVICE_RUNNING:
					r = ACTIVE;
					break;
				case SERVICE_PAUSED:
					r = PAUSED;
					break;
				case SERVICE_STOP_PENDING:
					r = STOPING;
					break;
				case SERVICE_START_PENDING:
					r = STARTING;
					break;
				case SERVICE_PAUSE_PENDING:
					r = PAUSING;
					break;
				case SERVICE_CONTINUE_PENDING:
					r = RESUMING;
					break;
				default:
					_VERIFY(false);
					break;
			}
			_VERIFY(CloseServiceHandle(hService));
			return r;
		}


		operator int() {
			SC_HANDLE hService;
			hService = OpenService(self->hSCM, self->name,
			                       SERVICE_QUERY_CONFIG);
			self->err = (hService == NULL)? NOT_INITED: OK;
			if (self->err)
				return 0;

			DWORD cbNeeded;
			if (!QueryServiceConfig(hService, NULL, 0, &cbNeeded)) {
				if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
					self->err = NOT_GETCONFIG;
					return 0;
				}
			}
			LPQUERY_SERVICE_CONFIG conf = (LPQUERY_SERVICE_CONFIG)
			                              new char[cbNeeded];
			if (!QueryServiceConfig(hService, conf, cbNeeded, &cbNeeded)) {
				self->err = NOT_GETCONFIG;
				return 0;
			}

			int r = (int)((State)(*this));
			switch (conf->dwStartType) {
				case SERVICE_DISABLED:
					return r | FLAG_RUN_NO;
				case SERVICE_DEMAND_START:
					return r | FLAG_RUN_MAN;
				case SERVICE_AUTO_START:
					return r | FLAG_RUN_AUTO;
			}
		}


		int operator =(int flags) {
			SC_HANDLE hService;
			bool r;
			_VERIFY(flags);
			switch (flags & 0x0F) {
				case FLAG_START:
					hService = OpenService(self->hSCM, self->name,
					                       SERVICE_START|SERVICE_QUERY_STATUS);
					self->err = (hService == NULL)? NOT_INITED: OK;
					if (self->err)
						return 0;
					if (!StartService(hService, 0, NULL)) {
						self->err = NOT_STARTED;
						_VERIFY(CloseServiceHandle(hService));
						return 0;
					}
					SERVICE_STATUS SvcStatus;
					for (;;) {
						if (!QueryServiceStatus(hService, &SvcStatus)) {
							self->err = NOT_GETSTATUS;
							_VERIFY(CloseServiceHandle(hService));
							return 1;
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
						self->err = NOT_STARTED;
						return 0;
					}
					break;
				case FLAG_STOP:
					hService = OpenService(self->hSCM, self->name,
					                       SERVICE_START|SERVICE_QUERY_STATUS);
					self->err = (hService == NULL)? NOT_INITED: OK;
					if (self->err)
						return 0;
					r = ControlServiceAndWait(hService,
					                          SERVICE_CONTROL_STOP,
					                          SERVICE_STOPPED, SVC_TIMEOUT);
					_VERIFY(CloseServiceHandle(hService));
					if (r==0) {
						self->err = NOT_STOPPED;
						return 0;
					}
					break;
				case FLAG_PAUSE:
					hService = OpenService(self->hSCM, self->name,
					                       SERVICE_START|SERVICE_QUERY_STATUS);
					self->err = (hService == NULL)? NOT_INITED: OK;
					if (self->err)
						return 0;
					r = ControlServiceAndWait(hService,
					                          SERVICE_CONTROL_PAUSE,
					                          SERVICE_PAUSED, SVC_TIMEOUT);
					_VERIFY(CloseServiceHandle(hService));
					if (r==0) {
						self->err = NOT_STOPPED;
						return 0;
					}
					break;
				case FLAG_RESUME:
					hService = OpenService(self->hSCM, self->name,
					                       SERVICE_START|SERVICE_QUERY_STATUS);
					self->err = (hService == NULL)? NOT_INITED: OK;
					if (self->err)
						return 0;
					r = ControlServiceAndWait(hService,
					                          SERVICE_CONTROL_CONTINUE,
					                          SERVICE_RUNNING, SVC_TIMEOUT);
					_VERIFY(CloseServiceHandle(hService));
					if (r==0) {
						self->err = NOT_STOPPED;
						return 0;
					}
					break;
				default:
					_VERIFY(false);
			}
			switch (flags & 0x30) {
				// TODO: find and (paste) code there
				case FLAG_RUN_NO:
					break;
				case FLAG_RUN_MAN:
					break;
				case FLAG_RUN_AUTO:
					break;
				default:
					_VERIFY(false);
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
			err = NOT_INITED;
		}


		bool Init(PCTSTR MachineName = 0,
		          PCTSTR DBName = 0,
		          DWORD Access = SC_MANAGER_CONNECT |
		                         SC_MANAGER_ENUMERATE_SERVICE |
		                         SC_MANAGER_QUERY_LOCK_STATUS) {
			if (hSCM) {
				CloseServiceHandle(hSCM);
			}
			hSCM = OpenSCManager(MachineName, DBName, Access);
			err = (hSCM == NULL)? NOT_INITED: OK;
		}


		bool Inited() {
			return (hSCM != 0);
		}


		int getNum() {
			// TODO: code me / this method is necessary?
		}


		LPENUM_SERVICE_STATUS getEnum(int size, bool &isEnd) {
			// TODO: code me
		}


		ServiceObj getService(LPCTSTR name, DWORD Access) {
			return ServiceObj(hSCM, name);
		}


		~SCMObj() {
			if (hSCM != NULL)
				_VERIFY(CloseServiceHandle(hSCM));
		}
};

