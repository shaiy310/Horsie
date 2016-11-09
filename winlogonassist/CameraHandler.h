#ifndef __CAMERA_HANDLER_H__
#define __CAMERA_HANDLER_H__

#include <Windows.h>
#include <Vfw.h>

#include "BMPHandler.h"

class CameraHandler : public BMPHandler
{
public:
	CameraHandler();
	~CameraHandler();

	VOID TakePicture(LPCTSTR path);

private:
	HWND cam_window;
};

#endif