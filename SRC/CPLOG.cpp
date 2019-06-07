//
// ÃŒƒ”À‹: CPLOG.CPP
//
// Œœ»—¿Õ»≈: ÎÓ„ (Â‡ÎËÁ‡ˆËˇ ‰Îˇ C++)
//
// ¬≈–—»ﬂ: 06.06.19
//
// Copyright [C] 2018-2019 Alex Kondratenko krolmail@list.ru
//

#include "cplog.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>



LogObj::LogObj(char *n) {
	New(n);
}



bool LogObj::New(char *n, bool append) {
	FILE *f = fopen(n, (append)? "a+": "w");
	if (f) {
		if (name)
			delete[] name;
		name = n;
		fclose(f);
		return true;
	}
	return false;
}



bool LogObj::Exists() {
	return New(name);
}



bool LogObj::Write(const char *a, char *b) {
	char nul[] = "\0";
	if (b == 0)
		b = nul;
	char *c = new char[strlen(a)+strlen(b)];
	strcpy(c, a);
	strcat(c, b);
	FILE *f = fopen(name, "a+");
	bool op = ( f != 0 );
	if (op) {
		fputs(c, f);
		fputs("\n", f);
		fclose(f);
	}
	return op;
}



bool LogObj::WriteF(const char *format, ...) {
	va_list arglist;
	FILE *f = fopen(name, "a+");
	bool op = ( f != 0 );
	if (op) {
		va_start(arglist, format);
		vfprintf(f, format, arglist);
		va_end(arglist);
		fclose(f);
	}
	return op;
}



bool LogObj::WriteDateF(const char *format, ...) {
	va_list arglist;
	time_t dttm0;
	struct tm *dttm;
	FILE *f = fopen(name, "a+");
	bool op = ( f != 0 );
	if (op) {
		va_start(arglist, format);
		dttm0 = time(0);
		dttm = localtime(&dttm0);
		char *s = asctime((const tm *)dttm);
		s[strlen(s)-1] = '\0';
		fprintf(f, "%s  ", s);
		vfprintf(f, format, arglist);
		va_end(arglist);
		fclose(f);
	}
	return op;
}



bool LogObj::WriteInt(const char *s, int n) {
	char *c, buf[20];
	_itoa(n, buf, 10);
	c = new char[strlen(s)+strlen(buf)];
	strcpy(c, s);
	strcat(c, buf);
	return Write(c);
}



LogObj::~LogObj() {
	if (name)
		delete[] name;
}
