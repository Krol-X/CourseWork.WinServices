//
// ЗАГОЛОВОК: SERVICES.H
//
// ОПИСАНИЕ: интерфейс управления службами Windows
//

#ifndef services_h_
#define services_h_

#include "include.h"

#define SVC_TIMEOUT     10000
/*
#define SERVICE_STOPPED          0x00000001
#define SERVICE_START_PENDING    0x00000002
#define SERVICE_STOP_PENDING     0x00000003
#define SERVICE_RUNNING          0x00000004
#define SERVICE_CONTINUE_PENDING 0x00000005
#define SERVICE_PAUSE_PENDING    0x00000006
#define SERVICE_PAUSED           0x00000007

#define SERVICE_CONTROL_STOP     0x00000001
#define SERVICE_CONTROL_PAUSE    0x00000002
#define SERVICE_CONTROL_CONTINUE 0x00000003

#define SERVICE_BOOT_START       0x00000000
#define SERVICE_SYSTEM_START     0x00000001
#define SERVICE_AUTO_START       0x00000002
#define SERVICE_DEMAND_START     0x00000003
#define SERVICE_DISABLED         0x00000004
*/
#define SERVICE_CONTROL_START    0x00000004

void *SVC_getEnum(DWORD &sz, DWORD &num);
int SVC_GetStatus(char *name);
int SVC_SetStatus(char *name, int flags);

#endif

