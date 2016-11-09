#ifndef __SHELL_HANDLER_H__
#define __SHELL_HANDLER_H__

#include <Windows.h>
#include <netfw.h>

#pragma comment( lib, "ole32.lib" )
#pragma comment( lib, "oleaut32.lib" )

BOOL RunCommand(LPCTSTR cmd);

HBITMAP GetThumbnail(LPWSTR Folder, LPWSTR FileName);

#endif