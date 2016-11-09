

// link with vfw32.lib
#pragma comment (lib, "vfw32.lib")
#pragma comment (lib, "gdi32.lib")

#include "CameraHandler.h"

CameraHandler::CameraHandler() : BMPHandler()
{
	cam_window = capCreateCaptureWindow("cam.era", WS_CHILD | WS_MINIMIZE | WS_VISIBLE, 0, 0, 0, 0, GetConsoleWindow(), 0);
	if (cam_window == NULL) {
		// TODO: error
	}
	capDriverConnect(cam_window, 0);
	//SendMessage(cam_window, WM_CAP_DLG_VIDEOSOURCE, 0, 0);
}

CameraHandler::~CameraHandler()
{
	capDriverDisconnect(cam_window);
	DestroyWindow(cam_window);
}

VOID CameraHandler::TakePicture(LPCTSTR path)
{
	//Grab a Frame
	capGrabFrame(cam_window);
	//Copy the frame we have just grabbed to the clipboard
	capEditCopy(cam_window);

	HDC hdc;
	PAINTSTRUCT ps;

	//Copy the clipboard image data to a HBITMAP object called hbm
	hdc = BeginPaint(cam_window, &ps);
	dst_dc = CreateCompatibleDC(hdc);
	if (dst_dc != NULL) {
		if (OpenClipboard(cam_window)) {
			hbmScreen = (HBITMAP)GetClipboardData(CF_BITMAP);
			SelectObject(dst_dc, hbmScreen);
			CloseClipboard();
		}
	}
	
	//Save hbm to a .bmp file called Frame.bmp
	//~~~~~~
	BITMAP bmp;
	// Retrieve the bitmap color format, width, and height.
	GetObject(hbmScreen, sizeof(BITMAP), (LPSTR)&bmp);
	//~~~~~
	RECT r;
	r.top = 0;
	r.left = 0;
	r.bottom = bmp.bmHeight;
	r.right = bmp.bmWidth;

	SetHeaders(r);


	Save(path);

	//PBITMAPINFO pbi = CreateBitmapInfoStruct(hbm);
	//CreateBMPFile(path, pbi, hbm, hdcMem);
}
