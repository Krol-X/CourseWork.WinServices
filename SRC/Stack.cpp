#include "Stack.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////PUBLIC///////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
Stack::Stack()
{
    Size = 0;
    Current = nullptr;
    Temp = nullptr;
    First = nullptr;
}
Stack::~Stack()
{
    if (First->Next == nullptr)
    {
        delete First;
        First = nullptr;
        Size--;
    }
    do {
        Temp = First;
        while (Temp->Next->Next != nullptr)
            Temp = Temp->Next;
        delete Temp->Next;
        Temp->Next = nullptr;
    } while (First->Next->Next != nullptr);
    Size = 0;
}
bool Stack::Empty()
{
    return (Size == 0 ? 1 : 0);
}
void Stack::operator+=(const double Val)
{
    EntryS *Ptr = new EntryS[1];
    if (Size == 0)
    {
        First = Ptr;
        Temp = First;
        Ptr->Next = nullptr;
        Ptr->Value = Val;
        Current = Ptr;
        Size++;
        return;
    }
    Temp = First;
    while (Temp->Next != nullptr)
        Temp = Temp->Next;
    Ptr->Next = Temp->Next;
    Temp->Next = Ptr;
    Ptr->Value = Val;
    Current = Ptr;
    Size++;
}
void Stack::operator-=(const double Val)
{
    if (Empty()) return;
    Temp = First;
    if (Size == 1)
    {
        delete First;
        First = nullptr;
        Size--;
        return;
    }
    if (First->Next == nullptr)
    {
        delete First;
        First = nullptr;
        Size--;
    }
    while (Temp->Next->Next != nullptr)
        Temp = Temp->Next;
    if (Current == Temp->Next) Current = Temp;
    Temp->Next->Value = Val;
    delete Temp->Next;
    Temp->Next = nullptr;
    Size--;
}
void Stack::operator+()
{
    if (Current->Next == nullptr)
        Current = First;
    else Current = Current->Next;
}
void Stack::operator-()
{
        Temp = First;
    if (Current == First)
    {
        while (Temp->Next != nullptr)
            Temp = Temp->Next;
        Current = Temp;
    }
    else
    {
        while (Temp->Next != Current)
            Temp = Temp->Next;
        Current = Temp;
    }
}
void Stack::SetCurrent(const double Val)
{
    Current->Value = Val;
}
void Stack::Print()
{
    if (Size == 0)
    {
        cout << "Контейнер пуст.\n";
        return;
    }
    Temp = First;
    if (Size == 1)
    {
        SetCol;
        cout << Temp->Value << endl;
        SetDefCol;
    }
    else
    {
        do{
            SetCol;
            cout << Temp->Value;
            SetDefCol;
            cout << " -> ";
            Temp = Temp->Next;
        } while (Temp->Next != nullptr);
        SetCol;
        cout << Temp->Value << endl;
        SetDefCol;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////PRIVATE//////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////