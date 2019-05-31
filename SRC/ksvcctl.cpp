//
// ОСНОВНОЙ МОДУЛЬ: KSVCCTL.CPP
//
// ОПИСАНИЕ ПРОГРАММЫ:
//   Программа позволяет управлять службами на удалённом компьютере (в режиме
// клиента) или предоставлять доступ другим программам с поддержкой специального
// протокола (в режиме сервера).
//
// ЦЕЛЕВАЯ ПЛАТФОРМА: WIN32
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//
#include "include.h"
#include "resource.h"

TCHAR szError[MAX_LOADSTRING];
#include "engine.h"


//
// Прототипы функций модуля
//
INT_PTR CALLBACK ChooseDlgProc(HWND, UINT, WPARAM, LPARAM);
ATOM RegisterWndClass();
HWND InitClientWnd();
bool InitListView(HWND);
LRESULT CALLBACK ClientWndProc(HWND, UINT, WPARAM, LPARAM);
void RefreshWindow(HWND);


//
// Глобальные константы
//
const TCHAR *szNull = TEXT("");
const TCHAR *szConfigFile = TEXT("ksvcctl.cfg");


//
// Глобальные переменные
//
HINSTANCE hInst;
HWND hClientWnd;
HWND hListView;
Address addr;
TCHAR szWndClass[MAX_LOADSTRING];
TCHAR szStart[MAX_LOADSTRING];
TCHAR szConnect[MAX_LOADSTRING];
TCHAR szStop[MAX_LOADSTRING];
TCHAR szErrStart[MAX_LOADSTRING];
//TCHAR szErrConnect[MAX_LOADSTRING];
TCHAR szErrLdList[MAX_LOADSTRING];
Server server;
Client client;


//
// ФУНКЦИЯ: void LoadSettings()
//
// НАЗНАЧЕНИЕ: загружает настройки из файла конфигурации
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
// ФУНКЦИЯ: void SaveSettings()
//
// НАЗНАЧЕНИЕ: сохраняет настройки в файл конфигурации
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
// ФУНКЦИЯ: int WinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
//
// НАЗНАЧЕНИЕ: точка входа для приложения
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
//	LoadString(hInstance, IDS_ERRCONNECT, szErrConnect, MAX_LOADSTRING);
	LoadString(hInstance, IDS_ERRLDLIST, szErrLdList, MAX_LOADSTRING);
	LoadSettings();
	int r;
	do {
		r = DialogBox(hInst, MAKEINTRESOURCE(IDD_CHOOSE), 0, ChooseDlgProc);
		if (r) {
			if ( !(hClientWnd = InitClientWnd()) )
				break;
			ShowWindow(hClientWnd, SW_SHOW);
			UpdateWindow(hClientWnd);
			RefreshWindow(hClientWnd);
			MSG msg;
			while (GetMessage(&msg, NULL, 0, 0)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	} while (r);
	server.Stop();
	ShutdownSockets();
	SaveSettings();
	return EXIT_SUCCESS;
}


//
// ФУНКЦИЯ: ATOM RegisterWndClass()
//
// НАЗНАЧЕНИЕ: регистрирует класс окна
//
// ВОЗВРАТИТЬ: флаг успеха выполнения операции
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
	// Поддержка для предыдущих версий Windows(NT 4.5? =)
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
// ФУНКЦИЯ: INT_PTR CALLBACK ChooseDlgProc(HWND, UINT, WPARAM, LPARAM)
//
// ВОЗВРАТИТЬ:  0 - закрытие диалога,
//             -1 - переключение в режим клиента (при успешном соединении)
//
INT_PTR CALLBACK ChooseDlgProc(HWND hDlg, UINT msg, WPARAM wParam,
                               LPARAM lParam) {
	BOOL fError;
	switch (msg) {
		case WM_INITDIALOG:
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
					                BM_GETCHECK, 0L, 0L))
						EndDialog(hDlg, -1);
					if (SendMessage(GetDlgItem(hDlg, IDD1_SERVER),
					                BM_GETCHECK, 0L, 0L)) {
						if (server.Start(addr.port)) {
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
					Server();
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
					             !server.IsWorking());
					EnableWindow(GetDlgItem(hDlg, IDD1_CLIENT),
					             !server.IsWorking());
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
// ФУНКЦИЯ: WHND InitClientWnd()
//
// НАЗНАЧЕНИЕ: создаёт окно и элементы управление, отображает окно
//
// ВОЗВРАТИТЬ: идетификатор окна / NULL
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
// ФУНКЦИЯ: bool InitListView(HWND)
//
// НАЗНАЧЕНИЕ: создаёт элемент управления ListView
//
// ВОЗВРАТИТЬ: флаг успеха выполнения операции
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
// ФУНКЦИЯ: LRESULT CALLBACK ClientWndProc(HWND, UINT, WPARAM, LPARAM)
//
// НАЗНАЧЕНИЕ: обрабатывает сообщения от окна клиента
//
// WM_COMMAND	- обработка меню приложения
// WM_NOTIFY	- обработка запросов отображения базы данных в ListView
// WM_SIZE      - обновить отображение окна
// WM_DESTROY	- ввести сообщение о выходе и вернуться
//
LRESULT CALLBACK ClientWndProc(HWND hWnd, UINT message, WPARAM wParam,
                               LPARAM lParam) {
	TCHAR buf[MAX_LOADSTRING];
	ListItem *item;
	switch (message) {
		case WM_CREATE:
			client.Init(addr);
			if (!client.GetList())
				MessageBox(hWnd, szErrLdList, szError, MB_ICONERROR | MB_OK);
			//RefreshWindow(hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDM_REFRESH:
					if (!client.GetList())
						MessageBox(hWnd, szErrLdList,
						           szError, MB_ICONERROR | MB_OK);
					RefreshWindow(hWnd);
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
							if ( client.ListSize() == 0 ) {
								//lpdi->item.pszText = (LPSTR) szNull;
								break;
							}
							item = client.ListItem( itemid );
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
						HMENU hmenuPopup;
						hmenuPopup = GetSubMenu(
						                 LoadMenu(hInst, "IDM_POPUPMENU"), 0);
						if (!hmenuPopup)
							break;
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
			RefreshWindow(hWnd);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


//
// ФУНКЦИЯ: RefreshWindow(HWND)
//
// НАЗНАЧЕНИЕ: обновляет элементы окна клиента
//
void RefreshWindow(HWND hWnd) {
	// 1. Устанавливаем количество записей ListView
	ListView_SetItemCount( hListView, client.ListSize() );
	// 2. Изменяем размеры ListView
	RECT rc, sbrc;
	GetClientRect(hWnd, &rc);
	LONG sbheight = (sbrc.bottom - sbrc.top);
	MoveWindow(hListView,
	           rc.left,
	           rc.top,
	           rc.right - rc.left,
	           rc.bottom - rc.top - sbheight,
	           true);
	for (int i=0; i<IDS_COL_num; i++)
		ListView_SetColumnWidth(hListView, i, LVSCW_AUTOSIZE_USEHEADER);
}

