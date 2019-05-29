//
// ���������-������: ENGINE.H
//
// ��������: ���������� ������ ������� � �������
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//

// �������� ����� �������
pthread_t thr_main;

////////////////////////////////////////////////////////////////////////////////
// ������: ���
//
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

void WriteLog(char *s) {
	Log.Write(s);
}

////////////////////////////////////////////////////////////////////////////////
// ������: ��������� � ���������
//
const DWORD ProtocolId = *((DWORD *)"\17VS\3");
const float DeltaTime = 0.001f;
const float SendRate = 0.25f;
const float TimeOut = 0.1f;

#define RESERVED_BYTES 1+4
typedef struct Datagram {
	BYTE cmd;
	DWORD cou;
	char data[];
} Datagram;
#define BUF_SIZE RESERVED_BYTES+65536

// ��������
#define CMD_ANY      0x00
#define CMD_CHECKOUT 0x80
#define CMD_LIST     0x1A
#define CMD_SET      0x2B

// ������� �������������� ������ �������
typedef struct ListItem {
	BYTE state;
	char *name, *viewname;
	char data[];
} ListItem, *pListItem;

////////////////////////////////////////////////////////////////////////////////
// ������: ��������������� ������
//
#include "services.h"

////////////////////////////////////////////////////////////////////////////////
// ������: ������
//

Connection Server( ProtocolId, TimeOut );
bool Server_Active;

//
// �������: static void *Server_main(void *)
//
// ����������: ���� � ���������� ���������
//
// �������: ��� ����� ���������� �������� ������ ��������� ���������
//          + ����������� ������� � ����� ������
//
static void *Server_main(void *) {
	unsigned char buf[BUF_SIZE];
	Datagram *dg = (Datagram *) buf;
	SCMObj scm;
	DWORD size, num;

	while (Server_Active) {
		Server.Listen();

		bool skipnext = false;
		while ( Server_Active ) {
			unsigned char packet[RESERVED_BYTES];
			int bytes_read = Server.ReceivePacket( buf, RESERVED_BYTES );
			if ( bytes_read == 0 )
				break;
			WriteLog( "received packet from client\n" );
			size = 0;
			Server.Update( DeltaTime );
			wait( DeltaTime );
			if ( Server.IsConnected() ) {
				if (dg->cmd == CMD_LIST) {
					WriteLog("list cmd");
					scm.Init();
					dg = (Datagram *) scm.getEnum(size, num);
					if (!dg) {
						WriteLog( "failed to making service list" );
						break;
					}
					dg->cmd = CMD_LIST;
					dg->cou = num;
				} else if (dg->cmd == CMD_SET) {
					WriteLog("set cmd");
					// ...
				} else {
					WriteLog("unknown cmd\n");
				}
				if (size != 0) {
					Server.SendPacket((UCHAR *) dg, RESERVED_BYTES+size);
					WriteLog("ok");
				}
			}
		}

		Server.Update( DeltaTime );
		wait( DeltaTime );
	}
	WriteLog("server is stopped");
	return 0;
}


//
// �������: bool StartServer(WORD)
//
// ����������: ������ ������� � ������ ��������� ���������
//
void StopServer();
bool StartServer(WORD port) {
	WriteLog("Server starting:");
	Server_Active = true;
	bool b = Server.Start(port);
	b &= (pthread_create(&thr_main, 0, Server_main, 0) == 0);
	if (b)
		WriteLog("ok");
	else {
		WriteLog("fail");
		StopServer();
	}
	return b;
}


//
// �������: void StopServer()
//
// ����������: ��������� ������� � ������ ��������� ���������
//
void StopServer() {
	if (Server.IsListening() || Server.IsConnected()) {
		WriteLog("Server stoping");
		Server_Active = false; // TODO: �������?
		pthread_join(thr_main, 0);
		Server.Stop();
		WriteLog("ok");
	}
}



////////////////////////////////////////////////////////////////////////////////
// ������: ������
//

Connection Client( ProtocolId, TimeOut );

vector<ListItem> list;

//
// �������: bool Client_list(WORD, Address)
//
// ����������: �������� ������ ����� �������
//
bool Client_list(WORD port, Address addr) {
	bool connected = Client.Start(port);
	unsigned char buf[BUF_SIZE];
	Datagram *dg = (Datagram *) buf;
	if (connected) {
		Client.Connect(addr);
		connected = false;

		while ( true ) {
			if ( !connected && Client.IsConnected() ) {
				WriteLog( "client connected to server (list)\n" );
				connected = true;
			}

			if ( !connected && Client.ConnectFailed() ) {
				WriteLog( "connection failed (list)\n" );
				break;
			}

			unsigned char buf1[RESERVED_BYTES];
			Datagram *dg1 = (Datagram *) buf1;
			dg1->cmd = CMD_LIST;
			Client.SendPacket( buf1, RESERVED_BYTES );

			while ( true ) {
				int bytes_read = Client.ReceivePacket( buf, BUF_SIZE );
				if ( bytes_read == 0 )
					break;
				WriteLog( "received packet from server\n" );
				if (dg->cmd == CMD_LIST)
					break;
			}
			if (dg->cmd == CMD_LIST)
				break;
			Client.Update( DeltaTime );
			wait( DeltaTime );
		}
		Client.Stop();
		if (dg->cmd != CMD_LIST)
			return false;
		pListItem item;
		char *str1, *str2;
		int cou = dg->cou;
		char *pbuf = (char *) &buf[RESERVED_BYTES];
		for (int i=0; i<cou; i++) {
			str1 = (char *) &pbuf[1];
			str2 = (char *) str1+strlen(str1)+1;
			int len1 = strlen(str1);
			int len2 = strlen(str2);
			item = (pListItem) new char[1+2*sizeof(char *)+len1+len2+2];
			item->state = pbuf[0];
			item->name = (char *) &item->data;
			item->viewname = (char *) &item->data[len1+1];
			strcpy(item->name, str1);
			strcpy(item->viewname, str2);
			list.insert(list.end(), *item);
			pbuf = (char *) str2+strlen(str2)+1;
		}
		connected = true;
	}
	return connected;
}

bool Client_set() {

}

