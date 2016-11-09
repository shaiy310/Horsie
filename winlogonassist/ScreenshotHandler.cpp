#include <iostream>
#include "ScreenshotHandler.h"

ScreenshotHandler::ScreenshotHandler(DOUBLE ratio) : BMPHandler()
{
	// Adjust the source properties to the screen 
	src_dc = GetDC(NULL);
	if (src_dc == NULL) throw ScreenshotException(__LINE__);

	// Full Screen size
	src_size.top = 0;
	src_size.bottom = GetSystemMetrics(SM_CYSCREEN);
	src_size.left = 0;
	src_size.right = GetSystemMetrics(SM_CXSCREEN);

	// Create a compatible DC which is used in a BitBlt from the window DC
	dst_dc = CreateCompatibleDC(src_dc);
	if (dst_dc == NULL) throw ScreenshotException(__LINE__);

	// Destination size
	dst_size.top = 0;
	dst_size.bottom = static_cast<LONG>(ratio * (src_size.bottom - src_size.top));
	dst_size.left = 0;
	dst_size.right = static_cast<LONG>(ratio * (src_size.right - src_size.left));

	// Create a compatible bitmap from the Window DC
	hbmScreen = CreateCompatibleBitmap(src_dc, dst_size.right - dst_size.left, dst_size.bottom - dst_size.top);
	if (hbmScreen == NULL) throw ScreenshotException(__LINE__);

	// Select the compatible bitmap into the compatible memory DC.
	SelectObject(dst_dc, hbmScreen);

	// This is the best stretch mode
	SetStretchBltMode(src_dc, HALFTONE);

	SetHeaders(dst_size);
}

ScreenshotHandler::~ScreenshotHandler()
{
	ReleaseDC(NULL, src_dc);
}

int ScreenshotHandler::CaptureAnImage(LPCTSTR file_name)
{
	//The source DC is the entire screen and the destination DC is the current window (HWND)
	if (!StretchBlt(dst_dc,
		dst_size.left, dst_size.top,
		dst_size.right - dst_size.left, dst_size.bottom - dst_size.top,
		src_dc,
		src_size.left, src_size.top,
		src_size.right - src_size.left, src_size.bottom - src_size.top,
		SRCCOPY)) {
		throw ScreenshotException(__LINE__);
	}

	Save(file_name);

	//Clean up
	return 0;
}