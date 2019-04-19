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
const TCHAR *szWindowClass = TEXT("KSvcCtl");


//
// ���������� ����������
//
HINSTANCE hInst;
HWND hClientWnd;
HWND hListView;



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
	int r;
	do {
		r = DialogBox(hInst, MAKEINTRESOURCE(IDD_CHOOSE), 0, ChooseDlgProc);
		if (r) {
			if (!hClientWnd) {
				if (!RegisterWndClass()) {
					MessageBoxA(0, "Cannot register windows class!",
					            "Error!", MB_ICONERROR | MB_OK);
					return EXIT_FAILURE;
				}
			}
			hClientWnd = InitClientWnd();

			MSG msg;
			while (GetMessage(&msg, NULL, 0, 0)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if (msg.message == WM_COMMAND &&
				        LOWORD(msg.wParam) == WM_DESTROY)
					break;
			}
		}
	} while (r);
	return EXIT_SUCCESS;
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
			//SendMessage(GetDlgItem(hDlg, IDD1_STOP), WS_DISABLED, 0L, 0L);
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDD1_START:
					if (SendMessage(GetDlgItem(hDlg, IDD1_CLIENT),
					                BM_GETCHECK, 0L, 0L))
						EndDialog(hDlg, -1);
					if (SendMessage(GetDlgItem(hDlg, IDD1_SERVER),
					                BM_GETCHECK, 0L, 0L)) {
						// Trying to start a server...
					}
					break;
				case IDD1_STOP:

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
	wcex.lpszClassName   = szWindowClass;
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
		wc.lpszClassName  = szWindowClass;
		aReturn = RegisterClass(&wc);
	}
	return aReturn;
}



//
// �������: WHND InitClientWnd()
//
// ����������: ������ ���� � �������� ����������, ���������� ����
//
// ����������: ������������ ���� / NULL
//
HWND InitClientWnd() {
	HWND hWnd = CreateWindow(szWindowClass, "",
	                         WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,	0,
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
		    hListView,
		    LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES
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
			break;

		case WM_SIZE:
			//RefreshWindow(hWnd);
			break;
		case WM_DESTROY:
			//PostQuitMessage(0);
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

