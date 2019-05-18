//
// ЗАГОЛОВОК: RESOURCE.H
//
// ОПИСАНИЕ: содержит определения идентификаторов ресурсов и элементов
//
// Copyright [C] 2019 Alex Kondratenko krolmail@list.ru
//

#ifndef resouce_h_
#define resouce_h_

#ifndef IDC_STATIC
#define IDC_STATIC                     -1
#endif

#define IDS_WNDCLASS                   1000

#define IDS_START                      2000
#define IDS_CONNECT                    2001
#define IDS_STOP                       2002
#define IDS_ERROR                      2003
#define IDS_ERRCONNECT                 2004
#define IDS_ERRSTART                   2005
#define IDS_UNKNOWN                    2006

#define IDS_STATE                      3000
#define IDS_STATE_STOPPED              3001
#define IDS_STATE_STARTING             3002
#define IDS_STATE_STOPING              3003
#define IDS_STATE_RUNNING              3004
#define IDS_STATE_CONTINUING           3005
#define IDS_STATE_PAUSING              3006
#define IDS_STATE_PAUSED               3007

#define IDS_RUNTYPE                    4000
#define IDS_RUNTYPE_BOOT               4000
#define IDS_RUNTYPE_SYSTEM             4001
#define IDS_RUNTYPE_AUTO               4002
#define IDS_RUNTYPE_DEMAND             4003
#define IDS_RUNTYPE_DISABLED           4004

#define IDD_CHOOSE                     1
#define IDD1_SERVER                    100
#define IDD1_CLIENT                    101
#define IDD1_IP                        102
#define IDD1_PORT                      103
#define IDD1_START                     104
#define IDD1_STOP                      105

#define IDC_LISTVIEW                   2
#define IDS_COL_num                    4
#define IDS_COL                        200
#define IDS_COL_NAME                   200
#define IDS_COL_FULLNAME               201
#define IDS_COL_STATUS                 202
#define IDS_COL_RUNTYPE                203

#define IDM_MAINMENU                   3
#define IDM_REFRESH                    300

//#define IDR_CLIENTMENU                 4
#define IDM_START                      400
#define IDM_PAUSE                      401
#define IDM_RESUME                     402
#define IDM_STOP                       403
#define IDM_AUTO                       404
#define IDM_DEMAND                     405
#define IDM_DISABLED                   406

#endif
