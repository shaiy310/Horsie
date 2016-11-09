#ifndef __BMP_HANDLER_H__
#define __BMP_HANDLER_H__

#include <Windows.h>
#include "BaseException.h"

class BMPHandler
{
public:
	class BMPHandlerError : public BaseException {
	public: BMPHandlerError(DWORD line) : BaseException(BMP_HANDLER, line, NORMAL) { }
	};
	BMPHandler();
	~BMPHandler();

protected:
	HDC dst_dc;
	HBITMAP hbmScreen;

	BITMAPFILEHEADER bmfHeader;
	BITMAPINFOHEADER bi;

	VOID SetHeaders(RECT img_size);
	BOOL Save(LPCTSTR path);
	BOOL SaveToFile(LPCTSTR path, LPVOID lpbitmap, DWORD dwBmpSize);
};

#endif