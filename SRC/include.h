//
// ЗАГОЛОВОК: INCLUDE.H
//
// ОПИСАНИЕ: аналог stdafx.h из MSVS
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//

#ifndef include_h_
#define include_h_

#define PLATFORM_WINDOWS  1
#define PLATFORM_MAC      2
#define PLATFORM_UNIX     3

#if defined(_WIN32)
#define PLATFORM PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM PLATFORM_MAC
#else
#define PLATFORM PLATFORM_UNIX
#endif

// Для MSVS
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

// Конфигурация некоторых значений
#define MAX_LOADSTRING 64

// C RunTime
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
// C++ Runtime
#ifdef __cplusplus
#include <vector>
#include <stack>
using namespace std;
#endif

// Sockets Runtime
#if PLATFORM == PLATFORM_WINDOWS

#include <winsock2.h>

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#define closesocket(socket)  close(socket)

#endif

#if PLATFORM == PLATFORM_WINDOWS
#pragma comment( lib, "wsock32.lib" ) // for MSVS
#endif

// Windows Runtime
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>

#endif
