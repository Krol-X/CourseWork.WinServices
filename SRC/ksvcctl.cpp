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

//
// Прототипы функций модуля
//
INT_PTR CALLBACK ChooseDlgProc(HWND, UINT, WPARAM, LPARAM);

//
// Глобальные константы
//
const TCHAR *szNull = TEXT("");
const TCHAR *szConfigFile = TEXT("ksvcctl.cfg");



//
// Глобальные переменные
//
HINSTANCE hInst;



//
//   ФУНКЦИЯ: int WinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
//
//   НАЗНАЧЕНИЕ: точка входа для приложения
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
// ФУНКЦИЯ: INT_PTR CALLBACK ChooseDlgProc(HWND, UINT, WPARAM, LPARAM)
//
// ВОЗВРАТИТЬ:  0 - закрытие диалога,
//             -1 - переключение в режим клиента (при успешном соединении)
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

