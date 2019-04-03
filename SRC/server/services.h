//
// ЗАГОЛОВОК МОДУЛЯ: SERVICES.C
//
// ОПИСАНИЕ: упрощённый интерфейс управления службами Windows NT
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//
#include <windows.h>

void SvcInit(DWORD Access = SC_MANAGER_CONNECT |
                            SC_MANAGER_ENUMERATE_SERVICE |
                            SC_MANAGER_QUERY_LOCK_STATUS);
//
// НАЗНАЧЕНИЕ: Инициализировать модуль для работы с SCM
//
// ПРИМЕЧАНИЕ: Заданы аргументы по умолчанию для Everyone (Authenticated Users).
//
// ВОЗВРАТИТЬ: Error = 0(Успех)/-1(Неуспех)
//



int SvcInited();
//
// ВОЗВРАТИТЬ: состояние модуля
//



int SvcError();
//
// ВОЗВРАТИТЬ: номер последней ошибки
//



LPENUM_SERVICE_STATUS SvcEnum(DWORD &n);
//
// ВОЗВРАТИТЬ: Список сервисов заданного размера;
//             n = количество элементов списка.
//
// ПРИМЕЧАНИЕ: Если Error == ERROR_MORE_DATA, то это значит, что
//             буфер не вместил часть элементов списка.
//



LPSERVICE_STATUS SvcStatus()
