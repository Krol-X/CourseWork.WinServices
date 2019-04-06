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

//
// ���������� ���������
//
const TCHAR *szNull = TEXT("");
const TCHAR *szConfigFile = TEXT("ksvcctl.cfg");



//
// ���������� ����������
//
HINSTANCE hInst;



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
	int r;
	do {
		r = DialogBox(hInst, MAKEINTRESOURCE(IDD_CHOOSE), 0, ChooseDlgProc);
/*		switch (r) {
			case 1:
				break;
			case 2:
				break;
		}*/
	} while (r);
	return 0;
}



//
// �������: INT_PTR CALLBACK ChooseDlgProc(HWND, UINT, WPARAM, LPARAM)
//
// ����������:  0 - �������� �������,
//             -1 - ������������ � ����� ������� (��� �������� ����������)
//
INT_PTR CALLBACK ChooseDlgProc(HWND hDlg, UINT msg, WPARAM wParam,
                              LPARAM lParam) {
	char buf[MAX_LOADSTRING];
	switch (msg) {
		case WM_INITDIALOG:
			break;
		case WM_COMMAND:
			switch(wParam) {
			}
			return true;
		case WM_CLOSE:
			EndDialog(hDlg, 0);
			break;
	}
	return false;
}

