//
// ЗАГОЛОВОК-МОДУЛЬ: ENGINE.H
//
// ОПИСАНИЕ: реализация работы сервера и клиента
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//

// Основной поток сервера/клиента
pthread_t thr_main;

////////////////////////////////////////////////////////////////////////////////
// СЕКЦИЯ: Настройки и константы
//
const DWORD ProtocolId = *((DWORD *)"\17VS\3");
const float DeltaTime = 0.001f;
const float TimeOut = 0.1f;

#define RESERVED_BYTES 4+1
typedef struct Datagram {
	DWORD sz;
	BYTE cmd;
	char data[];
} Datagram;
#define BUF_SIZE RESERVED_BYTES+65536

// Комманды
#define CMD_ANY      0x00
#define CMD_CHECKOUT 0x80
#define CMD_LIST     0x1A
#define CMD_SET      0x2B

// Элемент распакованного списка клиента
typedef struct ListItem {
	BYTE state;
	char *name, *viewname;
	char data[];
} ListItem, *pListItem;



////////////////////////////////////////////////////////////////////////////////
// СЕКЦИЯ: Лог
//
char *genLogFName();
LogObj Log(genLogFName());

//
// ФУНКЦИЯ: char *genLogFName()
//
// НАЗНАЧЕНИЕ: генерирует имя файла лога
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

////////////////////////////////////////////////////////////////////////////////
// СЕКЦИЯ: Сервер
//

Connection Server( ProtocolId, TimeOut );

//
// ФУНКЦИЯ: static void *Server_main(void *)
//
// НАЗНАЧЕНИЕ: приём и обработкаа сообщений
//
// ТАКТИКА: при приёме датаграммы создание потока обработки сообщения
//          + уведомление клиента о новом сокете
//
static void *Server_main(void *) {

	return 0;
}


//
// ФУНКЦИЯ: bool StartServer(WORD)
//
// НАЗНАЧЕНИЕ: запуск сервера и потока обработки сообщений
//
bool StartServer(WORD port) {
	bool b = Server.Start(port);
	b &= (pthread_create(&thr_main, 0, Server_main, 0) != 0);
	return b;
}


//
// ФУНКЦИЯ: void StopServer()
//
// НАЗНАЧЕНИЕ: остановка сервера и потока обработки сообщений
//
void StopServer() {
	if (Server.IsListening() || Server.IsConnected()) {
		// TODO: остановка/ожидание остальных потоков
		Server.Stop(); // TODO: внутри семафор
		pthread_join(thr_main, 0);
	}
}



////////////////////////////////////////////////////////////////////////////////
// СЕКЦИЯ: Клиент
//

Connection Client( ProtocolId, TimeOut );

vector<ListItem> list;

//
// ФУНКЦИЯ: bool Client_list(WORD, Address)
//
// НАЗНАЧЕНИЕ: получить список служб сервера
//
bool Client_list(WORD port, Address addr) {
	bool connected = Client.Start(port);
	if (connected) {
		Client.Connect(addr);
		connected = false;
		unsigned char buf[BUF_SIZE];
		int sz = 0;

		while ( true ) {
			if ( !connected && Client.IsConnected() ) {
				printf( "client connected to server\n" );
				connected = true;
			}

			if ( !connected && Client.ConnectFailed() ) {
				printf( "connection failed\n" );
				break;
			}

			unsigned char buf1[RESERVED_BYTES];
			Datagram *dg = (Datagram *) buf1;
			dg->cmd = CMD_LIST;
			dg->sz = 0;
			Client.SendPacket( buf1, RESERVED_BYTES );

			while ( true ) {
				int bytes_read = Client.ReceivePacket( buf, BUF_SIZE );
				if ( bytes_read == 0 )
					break;
				sz = bytes_read;
				printf( "received packet from server\n" );
			}
			Client.Update( DeltaTime );
			wait( DeltaTime );
		}
		char *str1, *str2;
		pListItem item;
		for (int i=0; i<sz; i++) { // FIXME
			str1 = (char *) &buf[1];
			str2 = (char *) &buf[strlen(str1)+1];
			int len1 = strlen(str1);
			int len2 = strlen(str2);
			item = (pListItem) new char[1+4+4+len1+len2+2];
			item->state = buf[0];
			item->name = (char *) &item->data;
			item->viewname = (char *) &item->data[len1];
			strcpy(item->name, str1);
			strcpy(item->viewname, str2);
			list.insert(list.end(), *item);
		}
	}
	return connected;
}

bool Client_set() {

}

