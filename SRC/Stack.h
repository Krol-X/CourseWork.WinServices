#pragma once

#include "Header.h"

struct EntryS
{
    double Value;
    EntryS *Next;
};

class Stack
{
    int Size;
    EntryS *First, *Current, *Temp;
public:
    //methods
    Stack();
    ~Stack();
    bool Empty();
    void Print();
    void SetCurrent(const double);
    //operators
    void operator +=(const double);
    void operator -=(const double);
    void operator +();
    void operator -();
};