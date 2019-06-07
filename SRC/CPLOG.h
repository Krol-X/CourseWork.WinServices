//
// ЗАГОЛОВОК: CPLOG.H
//
// ОПИСАНИЕ: лог (реализация для C++)
//
// ВЕРСИЯ: 06.06.19
//
// Copyright [C] 2018-2019 Alex Kondratenko krolmail@list.ru
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
		bool Write(const char *, char * = 0);
		bool WriteF(const char *format, ...);
		bool WriteDateF(const char *format, ...);
		bool WriteInt(const char *, int);
		~LogObj();
};

#endif
