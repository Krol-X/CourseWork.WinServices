//
// ???????? ??????: MYSVCCTL.CPP
//
// ???????? ?????????:
//   ????????? ????????? ????????? ???????? ?? ????????? ?????????? (? ??????
// ???????) ??? ????????????? ?????? ?????? ?????????? ? ?????????? ????????????
// ????????? (? ?????? ???????).
//
// ??????? ?????????: WIN32
//
#include "include.h"
#include "resource.h"
#include "engine.h"
#include "services.h"

pthread_mutex_t mutex;

//
// ????????? ??????? ??????
//
INT_PTR CALLBACK ChooseDlgProc(HWND, UINT, WPARAM, LPARAM);
ATOM RegisterWndClass();
HWND InitClientWnd();
bool InitListView(HWND);
LRESULT CALLBACK ClientWndProc(HWND, UINT, WPARAM, LPARAM);
void RefreshWindow(HWND);
void UpdateClientMenu(HMENU hMenu, BYTE state);
UINT LV_Selection();


//
// ?????????? ?????????
//
const TCHAR *szNull = TEXT("");
const TCHAR *szConfigFile = TEXT("mysvcctl.cfg");


//
// ?????????? ??????????
//
HINSTANCE hInst;
HWND hClientWnd;
HWND hListView;
Address addr;
TCHAR szWndClass[MAX_LOADSTRING];
TCHAR szStart[MAX_LOADSTRING];
TCHAR szConnect[MAX_LOADSTRING];
TCHAR szStop[MAX_LOADSTRING];
TCHAR szError[MAX_LOADSTRING];
TCHAR szErrStart[MAX_LOADSTRING];
TCHAR szErrLdList[MAX_LOADSTRING];
Server *server;
Client *client;


//
// ???????: void LoadSettings()
//
// ??????????: ????????? ????????? ?? ????? ????????????
//
void LoadSettings() {
	FILE *f = fopen(szConfigFile, "rb");
	if (f) {
		struct {
			DWORD a;
			WORD p;
		} adr;
		fread(&adr, sizeof(adr), 1, f);
		fclose(f);
		addr = Address(adr.a, adr.p);
	}
}


//
// ???????: void SaveSettings()
//
// ??????????: ????????? ????????? ? ???? ????????????
//
void SaveSettings() {
	FILE *f = fopen(szConfigFile, "wb");
	if (f) {
		struct {
			DWORD a = addr.addr;
			WORD p = addr.port;
		} adr;
		fwrite(&addr, sizeof(addr), 1, f);
		fclose(f);
	}
}


//
// ???????: int WinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
//
// ??????????: ????? ????? ??? ??????????
//
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine, int       nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	hInst = hInstance;
	InitCommonControls();
	hClientWnd = 0;
	LoadString(hInstance, IDS_WNDCLASS, szWndClass, MAX_LOADSTRING);
	if (!RegisterWndClass()) {
		MessageBox(0, "Cannot register windows class!",
		           szError, MB_ICONERROR | MB_OK);
		return EXIT_FAILURE;
	}
	if (!InitializeSockets()) {
		MessageBox(0, "Cannot initialize sockets!",
		           szError, MB_ICONERROR | MB_OK);
		return EXIT_FAILURE;
	}
	LoadString(hInstance, IDS_START, szStart, MAX_LOADSTRING);
	LoadString(hInstance, IDS_CONNECT, szConnect, MAX_LOADSTRING);
	LoadString(hInstance, IDS_STOP, szStop, MAX_LOADSTRING);
	LoadString(hInstance, IDS_ERROR, szError, MAX_LOADSTRING);
	LoadString(hInstance, IDS_ERRSTART, szErrStart, MAX_LOADSTRING);
	LoadString(hInstance, IDS_ERRLDLIST, szErrLdList, MAX_LOADSTRING);
	LoadSettings();
	int r;
	do {
		r = DialogBox(hInst, MAKEINTRESOURCE(IDD_CHOOSE), 0, ChooseDlgProc);
		if (r) {
			client = new Client();
			if ( !(hClientWnd = InitClientWnd()) )
				break;
			ShowWindow(hClientWnd, SW_SHOW);
			UpdateWindow(hClientWnd);
			MSG msg;
			while (GetMessage(&msg, NULL, 0, 0)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			client->~Client();
			delete client;
		}
	} while (r);
	ShutdownSockets();
	SaveSettings();
	return EXIT_SUCCESS;
}


//
// ???????: ATOM RegisterWndClass()
//
// ??????????: ???????????? ????? ????
//
// ??????????: ???? ?????? ?????????? ????????
//
ATOM RegisterWndClass() {
	WNDCLASSEX  wcex;
	ATOM        aReturn;

	wcex.cbSize          = sizeof(WNDCLASSEX);
	wcex.style           = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc     = (WNDPROC)ClientWndProc;
	wcex.cbClsExtra      = 0;
	wcex.cbWndExtra      = 0;
	wcex.hInstance       = hInst;
	wcex.hCursor         = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground   = (HBRUSH) (COLOR_WINDOW + 1);
	wcex.lpszMenuName    = MAKEINTRESOURCE(IDM_MAINMENU);
	wcex.lpszClassName   = szWndClass;
	wcex.hIcon           = LoadIcon(hInst, IDI_APPLICATION);
	wcex.hIconSm         = (HICON) LoadImage(hInst,
	                       IDI_APPLICATION, IMAGE_ICON,
	                       16, 16, 0);
	aReturn = RegisterClassEx(&wcex);
	// ????????? ??? ?????????? ?????? Windows(NT 4.5? =)
	if (!aReturn) {
		WNDCLASS wc;
		wc.style          = 0;
		wc.lpfnWndProc    = (WNDPROC)ClientWndProc;
		wc.cbClsExtra     = 0;
		wc.cbWndExtra     = 0;
		wc.hInstance      = hInst;
		wc.hIcon          = LoadIcon(hInst, IDI_APPLICATION);
		wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground  = (HBRUSH) (COLOR_WINDOW + 1);
		wc.lpszMenuName   = MAKEINTRESOURCE(IDM_MAINMENU);
		wc.lpszClassName  = szWndClass;
		aReturn = RegisterClass(&wc);
	}
	return aReturn;
}


//
// ???????: INT_PTR CALLBACK ChooseDlgProc(HWND, UINT, WPARAM, LPARAM)
//
// ??????????:  0 - ???????? ???????,
//             -1 - ???????????? ? ????? ??????? (??? ???????? ??????????)
//
INT_PTR CALLBACK ChooseDlgProc(HWND hDlg, UINT msg, WPARAM wParam,
                               LPARAM lParam) {
	BOOL fError;
	switch (msg) {
		case WM_INITDIALOG:
			server = new Server();
			DWORD adr;
			WORD port;
			SendMessage(GetDlgItem(hDlg, IDD1_IP),
			            IPM_SETADDRESS, 0, (LPARAM) addr.addr);
			SetDlgItemInt(hDlg, IDD1_PORT, addr.port, false);
			SendMessage(GetDlgItem(hDlg, IDD1_SERVER), BM_SETCHECK, TRUE, 0);
			SendMessage(GetDlgItem(hDlg, IDD1_CLIENT), BM_SETCHECK, FALSE, 0);
			EnableWindow(GetDlgItem(hDlg, IDD1_STOP), FALSE);
			goto idd1_server;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDD1_START:
					SendMessage(GetDlgItem(hDlg, IDD1_IP),
					            IPM_GETADDRESS, 0, (LPARAM) &adr);
					port = GetDlgItemInt(hDlg, IDD1_PORT, &fError, false);
					addr = Address(adr, port);
					SaveSettings();
					if (SendMessage(GetDlgItem(hDlg, IDD1_CLIENT),
					                BM_GETCHECK, 0, 0))
						EndDialog(hDlg, -1);
					if (SendMessage(GetDlgItem(hDlg, IDD1_SERVER),
					                BM_GETCHECK, 0, 0)) {
						if (server->Start(addr.port)) {
							EnableWindow(GetDlgItem(hDlg, IDD1_START), FALSE);
							EnableWindow(GetDlgItem(hDlg, IDD1_STOP), TRUE);
							EnableWindow(GetDlgItem(hDlg, IDD1_CLIENT), FALSE);
							EnableWindow(GetDlgItem(hDlg, IDD1_PORT), FALSE);
						} else {
							MessageBox(hDlg, szErrStart, szError,
							           MB_ICONERROR | MB_OK);
						}
					}
					break;
				case IDD1_STOP:
					server->Stop();
					EnableWindow(GetDlgItem(hDlg, IDD1_START), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDD1_STOP), FALSE);
					EnableWindow(GetDlgItem(hDlg, IDD1_CLIENT), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDD1_PORT), TRUE);
					break;
				case IDD1_SERVER:
idd1_server:
					EnableWindow(GetDlgItem(hDlg, IDD1_IP), FALSE);
					SetWindowText(GetDlgItem(hDlg, IDD1_START), szStart);
					SetWindowText(GetDlgItem(hDlg, IDD1_STOP), szStop);
					EnableWindow(GetDlgItem(hDlg, IDD1_START),
					             !server->IsWorking());
					EnableWindow(GetDlgItem(hDlg, IDD1_CLIENT),
					             !server->IsWorking());
					break;
				case IDD1_CLIENT:
					EnableWindow(GetDlgItem(hDlg, IDD1_IP), TRUE);
					SetWindowText(GetDlgItem(hDlg, IDD1_START), szConnect);
					SetWindowText(GetDlgItem(hDlg, IDD1_STOP), szNull);
					EnableWindow(GetDlgItem(hDlg, IDD1_STOP), FALSE);
					break;
			}
			return true;
		case WM_CLOSE:
			server->~Server();
			delete server;
			SendMessage(GetDlgItem(hDlg, IDD1_IP),
			            IPM_GETADDRESS, 0, (LPARAM) &adr);
			port = GetDlgItemInt(hDlg, IDD1_PORT, &fError, false);
			addr = Address(adr, port);
			EndDialog(hDlg, 0);
			break;
	}
	return false;
}


//
// ???????: WHND InitClientWnd()
//
// ??????????: ??????? ???? ? ???????? ??????????, ?????????? ????
//
// ??????????: ???????????? ???? / NULL
//
HWND InitClientWnd() {
	bool err = false;
	HWND hWnd = CreateWindow(szWndClass, szWndClass,
	                         WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0,
	                         640, 480, NULL, NULL, hInst, NULL);
	if (hWnd) {
		if (!InitListView(hWnd))
			err = true;
		if (err) {
			DestroyWindow(hWnd);
			hWnd = NULL;
		}
	}
	return hWnd;
}


//
// ???????: bool InitListView(HWND)
//
// ??????????: ??????? ??????? ?????????? ListView
//
// ??????????: ???? ?????? ?????????? ????????
//
bool InitListView(HWND hWnd) {
	INT dwStyle = WS_TABSTOP
	              | WS_CHILD
	              | WS_VISIBLE
	              | LVS_SHOWSELALWAYS
	              | LVS_SINGLESEL
	              | LVS_REPORT
	              | LVS_OWNERDATA
	              ;
	hListView = CreateWindowEx(WS_EX_CLIENTEDGE,
	                           WC_LISTVIEW,
	                           (LPCTSTR) NULL,
	                           dwStyle,
	                           0, 0, 10, 10,
	                           hWnd,
	                           (HMENU) IDC_LISTVIEW,
	                           hInst,
	                           NULL);
	if (hListView) {
		ListView_SetExtendedListViewStyle (
		    hListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES
		);
		LV_COLUMN lvColumn;
		TCHAR buf[MAX_LOADSTRING];
		lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvColumn.fmt = LVCFMT_LEFT;
		lvColumn.cx = 100;
		lvColumn.pszText = buf;
		for (UINT i = 0; i < IDS_COL_num; i++) {
			LoadString(hInst, IDS_COL+i, buf, MAX_LOADSTRING);
			ListView_InsertColumn(hListView, i, &lvColumn);
		}
	}
	return (hListView != NULL);
}


//
// ???????: LRESULT CALLBACK ClientWndProc(HWND, UINT, WPARAM, LPARAM)
//
// ??????????: ???????????? ????????? ?? ???? ???????
//
LRESULT CALLBACK ClientWndProc(HWND hWnd, UINT message, WPARAM wParam,
                               LPARAM lParam) {
	TCHAR buf[MAX_LOADSTRING];
	ListItem *item;
	switch (message) {
		case WM_CREATE:
			pthread_mutex_lock(&mutex);
			if (!client->GetList(addr))
				MessageBox(hWnd, szErrLdList,
				           szError, MB_ICONERROR | MB_OK);
			RefreshWindow(hWnd);
			pthread_mutex_unlock(&mutex);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDM_REFRESH:
					pthread_mutex_lock(&mutex);
					if (!client->GetList(addr))
						MessageBox(hWnd, szErrLdList,
						           szError, MB_ICONERROR | MB_OK);
					RefreshWindow(hWnd);
					pthread_mutex_unlock(&mutex);
					break;
				case IDM_START:
					pthread_mutex_lock(&mutex);
					client->SetSvc( addr, LV_Selection(),
					                SERVICE_CONTROL_START );
					pthread_mutex_unlock(&mutex);
					break;
				case IDM_PAUSE:
					pthread_mutex_lock(&mutex);
					client->SetSvc( addr, LV_Selection(),
					                SERVICE_CONTROL_PAUSE );
					pthread_mutex_unlock(&mutex);
					break;
				case IDM_RESUME:
					pthread_mutex_lock(&mutex);
					client->SetSvc( addr, LV_Selection(),
					                SERVICE_CONTROL_CONTINUE );
					pthread_mutex_unlock(&mutex);
					break;
				case IDM_STOP:
					pthread_mutex_lock(&mutex);
					client->SetSvc( addr, LV_Selection(),
					                SERVICE_CONTROL_STOP );
					pthread_mutex_unlock(&mutex);
					break;
				case IDM_AUTO:
					pthread_mutex_lock(&mutex);
					client->SetSvc( addr, LV_Selection(),
					                SERVICE_AUTO_START << 3 );
					pthread_mutex_unlock(&mutex);
					break;
				case IDM_DEMAND:
					pthread_mutex_lock(&mutex);
					client->SetSvc( addr, LV_Selection(),
					                SERVICE_DEMAND_START << 3 );
					pthread_mutex_unlock(&mutex);
					break;
				case IDM_DISABLED:
					pthread_mutex_lock(&mutex);
					client->SetSvc( addr, LV_Selection(),
					                SERVICE_DISABLED << 3 );
					pthread_mutex_unlock(&mutex);
					break;
			}
			break;

		case WM_NOTIFY:
			LV_DISPINFO *lpdi;
			lpdi = (LV_DISPINFO*) lParam;
			NMITEMACTIVATE *lpNMITEMACTIVATE;
			lpNMITEMACTIVATE = (NMITEMACTIVATE*) lParam;
			UINT itemid;
			if ((((LPNMHDR)lParam)->hwndFrom) == hListView) {
				switch (lpdi->hdr.code) {
					case LVN_GETDISPINFO:
						itemid = lpdi->item.iItem;
						if (lpdi->item.mask & LVIF_TEXT) {
							if ( client->ListSize() == 0 ) {
								break;
							}
							pthread_mutex_lock(&mutex);
							item = client->GetItem( itemid );
							pthread_mutex_unlock(&mutex);
							switch(lpdi->item.iSubItem) {
								case 0:
									lpdi->item.pszText = item->name;
									break;
								case 1:
									lpdi->item.pszText = item->viewname;
									break;
								case 2:
									if (item->state & 0x40)
										LoadString(hInst, IDS_UNKNOWN,
										           buf, MAX_LOADSTRING);
									else
										LoadString(hInst,
										           IDS_STATE+(item->state & 7),
										           buf, MAX_LOADSTRING);
									lpdi->item.pszText = buf;
									break;
								case 3:
									if (item->state & 0x80)
										LoadString(hInst, IDS_UNKNOWN,
										           buf, MAX_LOADSTRING);
									else
										LoadString(hInst,
										           IDS_RUNTYPE +
										           ((item->state>>3) & 7), buf,
										           MAX_LOADSTRING);
									lpdi->item.pszText = buf;
									break;
							}
						}
						break;
					case NM_RCLICK:
						pthread_mutex_lock(&mutex);
						HMENU hmenuPopup;
						hmenuPopup = GetSubMenu(
						                 LoadMenu(hInst, "IDM_POPUPMENU"), 0);
						if (!hmenuPopup)
							break;
						ListItem *item = client->GetItem( LV_Selection() );
						UpdateClientMenu( hmenuPopup, item->state );
						pthread_mutex_unlock(&mutex);
						POINT cursor;
						GetCursorPos(&cursor);
						TrackPopupMenu(hmenuPopup,
						               TPM_LEFTALIGN | TPM_RIGHTBUTTON,
						               cursor.x, cursor.y, 0, hWnd, NULL);
						DestroyMenu(hmenuPopup);
						break;
				}
			}
			break;

		case WM_SIZE:
			pthread_mutex_lock(&mutex);
			RefreshWindow(hWnd);
			pthread_mutex_unlock(&mutex);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


//
// ???????: void RefreshWindow(HWND)
//
// ??????????: ????????? ???????? ???? ???????
//
void RefreshWindow(HWND hWnd) {
	// 1. ????????????? ?????????? ??????? ListView
	ListView_SetItemCount( hListView, client->ListSize() );
	// 2. ???????? ??????? ListView
	RECT rc;
	GetClientRect(hWnd, &rc);
	MoveWindow(hListView,
	           rc.left,
	           rc.top,
	           rc.right - rc.left,
	           rc.bottom - rc.top,
	           true);
	for (int i=0; i<IDS_COL_num; i++)
		ListView_SetColumnWidth(hListView, i, LVSCW_AUTOSIZE_USEHEADER);
}


//
// ???????: void UpdateClientMenu(HMENU hMenu, BYTE state)
//
// ??????????: ????????? ??????????? ????????? ????
//
void UpdateClientMenu(HMENU hMenu, BYTE state) {
	const UINT mf_state[] = { MF_DISABLED, MF_ENABLED };
	BYTE st;
	st = ( state & 0x40 )? 0: state & 7;
	EnableMenuItem( hMenu, IDM_START,
	                mf_state[ (int)(st == SERVICE_STOPPED) ] );
	EnableMenuItem( hMenu, IDM_PAUSE,
	                mf_state[ (int)(st == SERVICE_RUNNING) ] );
	EnableMenuItem( hMenu, IDM_RESUME,
	                mf_state[ (int)(st == SERVICE_PAUSED) ] );
	EnableMenuItem( hMenu, IDM_STOP,
	                mf_state[ (int)( st == SERVICE_RUNNING
	                                 | st == SERVICE_PAUSED) ] );
	st = ( state & 0x80 )? 7: (state >> 3) & 7;
	EnableMenuItem( hMenu, IDM_AUTO,
	                mf_state[ (int)(st != SERVICE_AUTO_START
	                                         && st > 1) ] );
	EnableMenuItem( hMenu, IDM_DEMAND,
	                mf_state[ (int)(st != SERVICE_DEMAND_START
	                                         && st > 1) ] );
	EnableMenuItem( hMenu, IDM_DISABLED,
	                mf_state[ (int)(st != SERVICE_DISABLED
	                                         && st > 1) ] );
}


//
// ???????: UINT LV_Selection()
//
// ??????????: ???????? ????? ??????????? ????????
//
UINT LV_Selection() {
	INT index;
	index = ListView_GetNextItem(hListView,
	                             -1, LVNI_ALL | LVNI_SELECTED);
	if (index == -1)
		index = client->ListSize();
	return (UINT) index;
}

