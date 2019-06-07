//
// ���������: UDPSOCK.CPP
//
// ��������: ����� UdpSocket
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//
#include "udpsock.h"
#include <assert.h>
#include <windows.h>

#define MAX_WAIT 3.0
#define MAX_TRY  5
#define DELAY    0.1

#include "cplog.h"

char *genLogFName();
LogObj Log(genLogFName());

//
// �������: char *genLogFName()
//
// ����������: ���������� ��� ����� ����
//
char *genLogFName() {
	char *buf = new char[25];
	time_t t = time(0);
	struct tm* aTm = localtime(&t);
	sprintf(buf, "ksvc %02d%02d%04d_%02d%02d%02d.log",
	        aTm->tm_mday, aTm->tm_mon+1, aTm->tm_year+1900,
	        aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
	return buf;
}


//
// �����: void UdpSocket::chktimeout()
//
// ����������: �������� �������� ����������
//
bool UdpSocket::chktimeout() {
	if ( timeout < 0 )
		return false;
	clock_t end = clock();
	timeout += (double)(end - start) / CLOCKS_PER_SEC;
	start = end;
	if ( timeout > MAX_WAIT ) {
		timeout = -1;
		Log.WriteDateF("Timeout TRUE\n");
		return true;
	}
	Log.WriteDateF("Timeout: %ld\n", timeout);
	return false;
}


//
// �����: bool UdpSocket::Open()
//
// ����������: ������� �����
//
// ���������: ���� ������ ��������
//
bool UdpSocket::Open() {
	assert( !IsOpen() );
	isserver = false;
	tmpsz = 0;
	consock = 0;
	// create socket
	sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( sock <= 0 ) {
		sock = 0;
		Log.WriteDateF("Open FALSE\n");
		return false;
	}
	Log.WriteDateF("Open TRUE\n");
	return true;
}


//
// �����: bool UdpSocket::Accept()
//
// ����������:  ������ �������, ���������� � ���
//
// ���������: ���� ������ ��������
//
bool UdpSocket::Accept() {
	timeout = -1;
	consock = 1; // �������� ��� IsConnected()
	tmpsz = Receive( tmpbuf, 1024 );
	Log.WriteDateF("Accept %d\n", tmpsz > 0);
	return ( tmpsz > 0 );
}


//
// �����: bool UdpSocket::Connect(Address address)
//
// ����������: ���������� � �������� (��������)
//
// ���������: ���� ������ ��������
//
bool UdpSocket::Connect(Address address) {
	timeout = -2;
	consock = -1; // �������� ��� IsConnected()
	conaddr = address;
	Log.WriteDateF("Connect\n");
	return true;
}


//
// �����: bool UdpSocket::IsConnected()
//
// ���������: ��������� ����������
//
bool UdpSocket::IsConnected() {
	return ( consock != 0 );
}


#include <stdio.h>
#ifndef countof
#   define countof(a)	    (sizeof(a)/sizeof(a[0]))
#endif
void PrintError(IN DWORD dwErrorCode) {
	TCHAR szErrorText[512];
	if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorCode, 0,
	                   szErrorText, countof(szErrorText), NULL))
		sprintf(szErrorText, "Error %ld", dwErrorCode);
	MessageBox(0, szErrorText, "Error", MB_OK | MB_ICONERROR);
}


//
// �����: bool UdpSocket::Send(void *data, int size)
//
// ����������: ��������� ������ ����� ������������� ����������
//
// ���������: ���� ������ ��������
//
bool UdpSocket::Send(void *data, int size) {
	assert( data );
	assert( size > 0 );
	if ( !IsOpen() )
		return false;
	int _sock;
	_sock = sock;
	int sent_bytes;
	sockaddr_in adr;
	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = htonl( conaddr.addr );
	adr.sin_port = htons( (unsigned short) conaddr.port );
	if (timeout == -1)
		sent_bytes = send( _sock, (char *)data, size, 0 );
	else
		sent_bytes = sendto( _sock, (char *)data, size, 0,
		                     (const sockaddr *)&adr, sizeof(adr) );
    Log.WriteDateF("Sending %d\n", sent_bytes);
	if ( sent_bytes == -1 ) {
		PrintError( GetLastError() );
	}
	return sent_bytes == size;
}


//
// �����: int UdpSocket::Receive(void *data, int size)
//
// ����������: �������� ������ ����� ������������� ����������
//
// ���������: ���������� �������� ����
//
int UdpSocket::Receive(void *data, int size) {
	assert( data );
	assert( size > 0 );
	if ( !IsOpen() )
		return 0;
	int _sock;
	_sock = sock;
	int received_bytes;
	sockaddr_in from;
	int fromLength = sizeof( from );
	clock_t start = clock();
	if ( tmpsz <= 0 ) {
		for ( int trycount = 0; trycount < MAX_TRY; trycount++ ) {
			double seconds;
			do {
				received_bytes = recvfrom( _sock, (char *)data, size, 0,
				                           (sockaddr*)&from, &fromLength );
				if ( received_bytes > 0 )
					break;
				clock_t end = clock();
				seconds = (double)( end - start ) / CLOCKS_PER_SEC;
				wait(DELAY);
			} while ( seconds < MAX_WAIT );
			if ( received_bytes > 0 )
				break;
		}
	} else {
		received_bytes = (tmpsz < received_bytes)? tmpsz: received_bytes;
		memcpy( data, tmpbuf, received_bytes );
		tmpsz = 0;
		return received_bytes;
	}
	Log.WriteDateF("Receiving %d\n", received_bytes);
	if ( received_bytes <= 0 )
		return 0;
	if ( timeout == -1 ) {
		conaddr.addr = ntohl( from.sin_addr.s_addr );
		conaddr.port = ntohs( from.sin_port );
		start = clock();
		timeout = 0;
		return received_bytes;
	} else if ( conaddr.addr != ntohl( from.sin_addr.s_addr )
	            || conaddr.port != ntohs( from.sin_port ) )
		return 0;
	if ( chktimeout() )
		return 0;
	return received_bytes;
}


//
// �����: void UdpSocket::Disconnect()
//
// ����������: ��������� �����, ������� ����� (��������)
//
void UdpSocket::Disconnect() {
	if ( IsConnected() ) {
		timeout = -1;
		consock = 0;
		Log.WriteDateF("Disconnect\n");
	}
}
