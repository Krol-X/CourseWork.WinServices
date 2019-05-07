//
// ЗАГОЛОВОК: EXTENDS.H
//
// ОПИСАНИЕ: реализация встроенных классов в C++
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//
#pragma once

#define extends(name, obj) class { private: obj *self; public: friend obj;
#define extends_end(name) } name;
#define extends_init(name) name.self = this;
