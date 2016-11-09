#ifndef __SCREENSHOT_HANDLER_H__
#define __SCREENSHOT_HANDLER_H__

#include <Windows.h>
#include "BaseException.h"
#include "BMPHandler.h"

class ScreenshotHandler : public BMPHandler
{
public:
	class ScreenshotException : public BaseException {
	public: ScreenshotException(DWORD line) : BaseException(SCREENSHOT_HANDLER, line, NORMAL) { }
	};
	ScreenshotHandler(DOUBLE ratio);
	/*ScreenshotHandler(const ScreenshotHandler& other);
	ScreenshotHandler& operator=(const ScreenshotHandler& other);*/
	~ScreenshotHandler();

	int CaptureAnImage(LPCTSTR file_name);

private:
	HDC src_dc;	// DC of the source (screen or window)
	RECT src_size;	// the size of the screen image
	
	RECT dst_size;	// the size of the output image
};

#endif