//
// ЗАГОЛОВОК: LOG.H
//
// ОПИСАНИЕ: лог (реализация для C++)
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
