//
// ЗАГОЛОВОК: CPLOG.H
//
// ОПИСАНИЕ: лог (реализация для C++)
//
// ВЕРСИЯ: 12.12.18
//
// Copyright [C] 2018 Alex Kondratenko krolmail@list.ru
//

#ifndef log_h_
#define log_h_

#include <stdio.h>



class LogObj {
	protected:
		char *name;
	public:
		LogObj(char *);
		bool New(char *, bool = true);
		bool Exists();
		bool Write(char *);
		bool Write(char *, char *);
		bool WriteInt(char *, int);
		~LogObj();
};

#endif
