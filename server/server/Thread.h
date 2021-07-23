#pragma once
#include <windows.h>
extern ChatServer gServerObj;


//b
UINT  listenServerThread(LPVOID lParam);
UINT  recServerThread(LPVOID lParam);
