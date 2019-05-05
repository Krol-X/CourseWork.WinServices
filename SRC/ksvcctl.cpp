//
// �������� ������: KSVCCTL.CPP
//
// �������� ���������:
//   ��������� ��������� ��������� �������� �� �������� ���������� (� ������
// �������) ��� ������������� ������ ������ ���������� � ���������� ������������
// ��������� (� ������ �������).
//
// ������� ���������: WIN32
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//
#include "include.h"
#include "engine.h"
#include "resource.h"


//
// ��������� ������� ������
//
INT_PTR CALLBACK ChooseDlgProc(HWND, UINT, WPARAM, LPARAM);
ATOM RegisterWndClass();
HWND InitClientWnd();
bool InitListView(HWND hWnd);
LRESULT CALLBACK ClientWndProc(HWND, UINT, WPARAM, LPARAM);


//
// ���������� ���������
//
const TCHAR *szNull = TEXT("");
const TCHAR *szConfigFile = TEXT("ksvcctl.cfg");


//
// ���������� ����������
//
HINSTANCE hInst;
HWND hClientWnd;
HWND hListView;
TCHAR szWndClass[MAX_LOADSTRING];
TCHAR szStart[MAX_LOADSTRING];
TCHAR szConnect[MAX_LOADSTRING];
TCHAR szStop[MAX_LOADSTRING];
TCHAR szError[MAX_LOADSTRING];
TCHAR szErrStart[MAX_LOADSTRING];
TCHAR szErrConnect[MAX_LOADSTRING];



//
//   �������: int WinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
//
//   ����������: ����� ����� ��� ����������
//
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine, int       nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	hInst = hInstance;
	InitCommonControls();
	hClientWnd = 0;
	LoadString(hInstance, IDS_WNDCLASS, szWndClass, MAX_LOADSTRING);
	LoadString(hInstance, IDS_START, szStart, MAX_LOADSTRING);
	LoadString(hInstance, IDS_CONNECT, szConnect, MAX_LOADSTRING);
	LoadString(hInstance, IDS_STOP, szStop, MAX_LOADSTRING);
	LoadString(hInstance, IDS_ERROR, szError, MAX_LOADSTRING);
	LoadString(hInstance, IDS_ERRSTART, szErrStart, MAX_LOADSTRING);
	LoadString(hInstance, IDS_ERRCONNECT, szErrConnect, MAX_LOADSTRING);
	if (!RegisterWndClass()) {
		MessageBoxA(0, "Cannot register windows class!",
		            "Error!", MB_ICONERROR | MB_OK);
		return EXIT_FAILURE;
	}
	int r;
	do {
		r = DialogBox(hInst, MAKEINTRESOURCE(IDD_CHOOSE), 0, ChooseDlgProc);
		if (r) {
			hClientWnd = InitClientWnd();

			MSG msg;
			while (GetMessage(&msg, NULL, 0, 0)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	} while (r);
	return EXIT_SUCCESS;
}



//
// �������: ATOM RegisterWndClass()
//
// ����������: ������������ ����� ����
//
// ����������: ���� ������ ���������� ��������
//
ATOM RegisterWndClass() {
	// ������������ ����� ����
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
	wcex.lpszMenuName    = 0;
	wcex.lpszClassName   = szWndClass;
	wcex.hIcon           = LoadIcon(hInst, IDI_APPLICATION);
	wcex.hIconSm         = (HICON) LoadImage(hInst,
	                       IDI_APPLICATION, IMAGE_ICON,
	                       16, 16, 0);
	aReturn = RegisterClassEx(&wcex);
	// ��������� ��� ���������� ������ Windows(NT 4.5? =)
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
		wc.lpszMenuName   = 0;
		wc.lpszClassName  = szWndClass;
		aReturn = RegisterClass(&wc);
	}
	return aReturn;
}



//
// �������: INT_PTR CALLBACK ChooseDlgProc(HWND, UINT, WPARAM, LPARAM)
//
// ����������:  0 - �������� �������,
//             -1 - ������������ � ����� ������� (��� �������� ����������)
//
INT_PTR CALLBACK ChooseDlgProc(HWND hDlg, UINT msg, WPARAM wParam,
                               LPARAM lParam) {
	switch (msg) {
		case WM_INITDIALOG:
			SendMessage(GetDlgItem(hDlg, IDD1_SERVER), BM_SETCHECK, TRUE, 0);
			SendMessage(GetDlgItem(hDlg, IDD1_CLIENT), BM_SETCHECK, FALSE, 0);
			EnableWindow(GetDlgItem(hDlg, IDD1_STOP), FALSE);
			Server.hwnd = hDlg;
			goto idd1_server;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDD1_START:
					Address addr;
					BOOL fError;

					// addr.addr = ...
					addr.port = GetDlgItemInt(hDlg, IDD1_PORT, &fError, false);
					if (SendMessage(GetDlgItem(hDlg, IDD1_CLIENT),
					                BM_GETCHECK, 0L, 0L))
						EndDialog(hDlg, -1);
					if (SendMessage(GetDlgItem(hDlg, IDD1_SERVER),
					                BM_GETCHECK, 0L, 0L)) {
						if (Server.Start(addr.port)) {
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
					Server.Stop();
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
					             !Server.Active());
					EnableWindow(GetDlgItem(hDlg, IDD1_CLIENT),
					             !Server.Active());
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
			EndDialog(hDlg, 0);
			break;
	}
	return false;
}



//
// �������: WHND InitClientWnd()
//
// ����������: ������ ���� � �������� ����������, ���������� ����
//
// ����������: ������������ ���� / NULL
//
HWND InitClientWnd() {
	HWND hWnd = CreateWindow(szWndClass, szWndClass,
	                         WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0,
	                         640, 480, NULL, NULL, hInst, NULL);
	if (hWnd) {
		if (!InitListView(hWnd))
			return NULL;
		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);
		//RefreshWindow(hWnd);
		return hWnd;
	} else
		return NULL;
}



//
// �������: bool InitListView(HWND)
//
// ����������: ������ ������� ���������� ListView
//
// ����������: ���� ������ ���������� ��������
//
bool InitListView(HWND hWnd) {
	INT dwStyle = WS_TABSTOP |
	              WS_CHILD |
	              WS_VISIBLE |
	              LVS_SHOWSELALWAYS |
	              LVS_SINGLESEL |
	              LVS_REPORT |
	              LVS_OWNERDATA;
	hListView = CreateWindowEx(WS_EX_CLIENTEDGE,
	                           WC_LISTVIEW,
	                           (PCTSTR) NULL,
	                           dwStyle,
	                           0, 0, 0, 0,
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
	ListView_SetItemCount(hListView, 1);
	return (hListView != NULL);
}



//
// �������: LRESULT CALLBACK ClientWndProc(HWND, UINT, WPARAM, LPARAM)
//
// ����������: ������������ ��������� �� ���� �������
//
// WM_COMMAND	- ��������� ���� ����������
// WM_NOTIFY	- ��������� �������� ����������� ���� ������ � ListView
// WM_SIZE      - �������� ����������� ����
// WM_DESTROY	- ������ ��������� � ������ � ���������
//
LRESULT CALLBACK ClientWndProc(HWND hWnd, UINT message, WPARAM wParam,
                               LPARAM lParam) {
	switch (message) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {

			}
			break;

		case WM_NOTIFY:
			LV_DISPINFO *lpdi;
			lpdi = (LV_DISPINFO*) lParam;
			if (lpdi->item.mask & LVIF_TEXT) {
				switch(lpdi->item.iSubItem) {
					default:
						lpdi->item.pszText = "1";
						break;
				}
			}
			break;

		case WM_SIZE:
			//RefreshWindow(hWnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

