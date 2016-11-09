#include "BMPHandler.h"

BMPHandler::BMPHandler()
{
	hbmScreen = NULL;
	dst_dc = NULL;
}

BMPHandler::~BMPHandler()
{
	DeleteObject(dst_dc);
	DeleteObject(hbmScreen);
}

VOID BMPHandler::SetHeaders(RECT img_size)
{
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;
	bi.biWidth = img_size.right - img_size.left;
	bi.biHeight = img_size.bottom - img_size.top;

	//bfType must always be BM for Bitmaps
	bmfHeader.bfType = 0x4D42; //BM   

	DWORD dwBmpSize = ((bi.biWidth * bi.biBitCount + 31) / 32) * 4 * bi.biHeight;

	//Offset to where the actual bitmap bits start.
	bmfHeader.bfOffBits = sizeof(bmfHeader)+sizeof(bi);

	//Size of the file, includes: size of headers and size of bitmap
	bmfHeader.bfSize = bmfHeader.bfOffBits + dwBmpSize;
}

BOOL BMPHandler::Save(LPCTSTR path)
{
	DWORD bmp_size = bmfHeader.bfSize - bmfHeader.bfOffBits;
	LPVOID lpbitmap = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bmp_size);
	if (lpbitmap == NULL) {
		throw BMPHandlerError(__LINE__);
	}

	// Gets the "bits" from the bitmap and copies them into a buffer 
	// which is pointed to by lpbitmap.
	if (GetDIBits(dst_dc, hbmScreen, 0,
		bi.biHeight,
		lpbitmap,
		(BITMAPINFO *)&bi, DIB_RGB_COLORS) == 0) {
		throw BMPHandlerError(__LINE__);
	}

	SaveToFile(path, lpbitmap, bmp_size);

	//Unlock and Free the DIB from the heap
	HeapFree(GetProcessHeap(), 0, lpbitmap);

	return TRUE;
}

BOOL BMPHandler::SaveToFile(LPCTSTR path, LPVOID lpbitmap, DWORD dwBmpSize)
{
	// A file is created, this is where we will save the screen capture.
	HANDLE hFile = CreateFile(path,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE) {
		throw BMPHandlerError(__LINE__);
	}

	DWORD dwBytesWritten = 0;
	WriteFile(hFile, &bmfHeader, sizeof(bmfHeader), &dwBytesWritten, NULL);
	WriteFile(hFile, &bi, sizeof(bi), &dwBytesWritten, NULL);
	WriteFile(hFile, lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

	//Close the handle for the file that was created
	CloseHandle(hFile);

	return TRUE;
}
