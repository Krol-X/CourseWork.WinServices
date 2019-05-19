//
// МОДУЛЬ: CPLOG.CPP
//
// ОПИСАНИЕ: лог (реализация для C++)
//
// ВЕРСИЯ: 12.12.18
//
// Copyright [C] 2018 Alex Kondratenko krolmail@list.ru
//

#include "cplog.h"
#include <stdlib.h>
#include <string.h>



LogObj::LogObj(char *n) {
	New(n);
}



bool LogObj::New(char *n, bool append) {
	FILE *f = fopen(n, (append)? "a": "w");
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



bool LogObj::Write(char *s) {
	FILE *f = fopen(name, "a");
	bool b = (f!=NULL);
	if (b) {
		fputs(s, f);
		fputs("\n", f);
		fclose(f);
	}
	return b;
}



bool LogObj::Write(char *a, char *b) {
	char *c = new char[strlen(a)+strlen(b)];
	strcpy(c, a);
	strcat(c, b);
	return Write(c);
}



bool LogObj::WriteInt(char *s, int n) {
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
