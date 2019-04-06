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
ATOM RegisterWndClass();
WHND InitWnd();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


//
// Глобальные константы
//
const TCHAR *szNull = TEXT("");
const TCHAR *szConfigFile = TEXT("ksvcctl.cfg");



//
// Глобальные переменные
//
HINSTANCE hInst;
HWND hWnd;
TCHAR szWindowClass[MAX_LOADSTRING];



//
//   ФУНКЦИЯ: int wWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
//
//   НАЗНАЧЕНИЕ: точка входа для приложения
//
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                      LPTSTR    lpCmdLine, int       nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	// 0. Инициализация глобальных переменных, загрузка некоторых ресурсов
	LoadString(hInstance, IDS_WNDCLASS, szWindowClass, MAX_LOADSTRING);

	// 1. Проверяем, не было ли это приложение запущено ранее
	HWND hWnd = FindWindow(szWindowClass, NULL);
	if (hWnd) {
		if(IsIconic(hWnd))
			ShowWindow(hWnd, SW_RESTORE);
		SetForegroundWindow(hWnd);
		return 0;
	}

	// 2. Регистрация класса окна
	if (!RegisterWndClass())
		return 0;

	// 3. Создание окна
	hWnd = InitWnd();
	if (!hWnd)
		return false;

	MSG msg;
	// 5. Цикл обработки сообщений приложения
	while (GetMessage(&msg, NULL, 0, 0)) {
		//if (!TranslateAccelerator(hWnd, hAccelTable, &msg)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		//}
	}
	return (int) msg.wParam;
}



//
// ФУНКЦИЯ: ATOM RegisterWndClass()
//
// НАЗНАЧЕНИЕ: регистрирует класс окна
//
// ВОЗВРАТИТЬ: флаг успеха выполнения операции
//
ATOM RegisterWndClass() {
	// Регистрируем класс окна
	WNDCLASSEX  wcex;
	ATOM        aReturn;


	wcex.cbSize          = sizeof(WNDCLASSEX);
	wcex.style           = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc     = (WNDPROC)WndProc;
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
	// Поддержка для предыдущих версий Windows(NT 4.5? =)
	if (!aReturn) {
		WNDCLASS wc;
		wc.style          = 0;
		wc.lpfnWndProc    = (WNDPROC)WndProc;
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
// ФУНКЦИЯ: WHND InitWnd()
//
// НАЗНАЧЕНИЕ: создаёт окно и элементы управление, отображает окно
//
// ВОЗВРАТИТЬ: идетификатор окна / NULL
//
HWND InitWnd() {
	HWND hWnd = CreateWindow(szWindowClass, szNull/*FIXME*/,
	                         WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,	0,
	                         640, 480, NULL, NULL, hInst, NULL);
	if (hWnd) {
		//if (!InitStatusBar(hWnd))
		//	return NULL;
		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);
		RefreshWindow(hWnd);
		return hWnd;
	} else
		return NULL;
}



//
// ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
// НАЗНАЧЕНИЕ: обрабатывает сообщения в главном окне
//
// WM_COMMAND	- обработка меню приложения
// WM_DESTROY	- ввести сообщение о выходе и вернуться
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
                         LPARAM lParam) {
	TCHAR buf[MAX_LOADSTRING*2], buf2[MAX_LOADSTRING];
	switch (message) {
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}



