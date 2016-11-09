#include <iostream>
#include <Shobjidl.h>
#include <Shlobj.h>
#include <string>

#include "ShellHandler.h"

BOOL RunCommand(LPCTSTR cmd)
{
	(void)cmd;
	return TRUE;
}

bool SaveBMPFile(char *filename, HBITMAP bitmapDC, int width, int height);
HBITMAP GetThumbnail(LPWSTR Folder, LPWSTR FileName)
{
	// Initialize COM.
	HRESULT hrComInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	// Ignore RPC_E_CHANGED_MODE; this just means that COM has already been
    // initialized with a different mode. Since we don't care what the mode is,
    // we'll just use the existing mode.
    if (FAILED(hrComInit))
    {
		if (hrComInit != RPC_E_CHANGED_MODE)
		{
			std::cout << "CoInitializeEx failed: 0x%08lx\n" << hrComInit;
		}
    }
	//-----------------------

	IShellFolder* pDesktop = NULL;
	IShellFolder* pSub = NULL;
	IExtractImage* pIExtract = NULL;
	LPITEMIDLIST pidl = NULL;

	HRESULT hr;
	hr = SHGetDesktopFolder(&pDesktop);
	if(FAILED(hr)) return NULL;
	hr = pDesktop->ParseDisplayName(NULL, NULL, Folder, NULL, &pidl, NULL);
	if(FAILED(hr)) return NULL;
	hr = pDesktop->BindToObject(pidl, NULL, IID_IShellFolder, (void**)&pSub);
	if(FAILED(hr)) return NULL;
	hr = pSub->ParseDisplayName(NULL, NULL, FileName, NULL, &pidl, NULL);
	if(FAILED(hr)) return NULL;
	hr = pSub ->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST *)&pidl, IID_IExtractImage, NULL, (void**)& pIExtract);
	if(FAILED(hr)) return NULL;

	SIZE size;
	size.cx =300;
	size.cy =300;

	DWORD dwFlags = IEIFLAG_ORIGSIZE | IEIFLAG_QUALITY;

	HBITMAP hThumbnail = NULL;

	// Set up the options for the image
	OLECHAR pathBuffer[MAX_PATH];
	hr = pIExtract->GetLocation(pathBuffer, MAX_PATH, NULL, &size,32, &dwFlags);

	// Get the image
	hr = pIExtract ->Extract(&hThumbnail);

	pDesktop->Release();
	pSub->Release(); 

	SaveBMPFile("d:\\downloads\\000_file.bmp", hThumbnail, size.cx, size.cy);

	return hThumbnail;


}

// Screenshot
//    -> FileName: Name of file to save screenshot to
//    -> lpDDS: DirectDraw surface to capture
//    <- Result: success
//
bool SaveBMPFile(char *filename, HBITMAP bitmap, int width, int height) {
    bool success = false;
    
	(void)width;
	(void)height;
	//HDC SurfDC = NULL;        // GDI-compatible device context for the surface
    //HBITMAP OffscrBmp = NULL; // bitmap that is converted to a DIB
    HDC OffscrDC = NULL;      // offscreen DC that we can select OffscrBmp into
    LPBITMAPINFO lpbi=NULL; // bitmap format info; used by GetDIBits
    LPVOID lpvBits=NULL;    // pointer to bitmap bits array
    HANDLE BmpFile=INVALID_HANDLE_VALUE;    // destination .bmp file
    BITMAPFILEHEADER bmfh;  // .bmp file header

  //  // We need an HBITMAP to convert it to a DIB:
  //  if ((OffscrBmp = CreateCompatibleBitmap(bitmapDC, width, height)) == NULL)
  //      goto cleanup;

  //  // The bitmap is empty, so let's copy the contents of the surface to it.
  //  // For that we need to select it into a device context. We create one.
  //  if ((OffscrDC = CreateCompatibleDC(bitmapDC)) == NULL)
		//goto cleanup;

  //  // Select OffscrBmp into OffscrDC:
  //  HBITMAP OldBmp = (HBITMAP)SelectObject(OffscrDC, OffscrBmp);

  //  // Now we can copy the contents of the surface to the offscreen bitmap:
  //  BitBlt(OffscrDC, 0, 0, width, height, bitmapDC, 0, 0, SRCCOPY);

    // GetDIBits requires format info about the bitmap. We can have GetDIBits
    // fill a structure with that info if we pass a NULL pointer for lpvBits:
    // Reserve memory for bitmap info (BITMAPINFOHEADER + largest possible
    // palette):
    if ((lpbi = (LPBITMAPINFO)(new char[sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)])) == NULL) 
		goto cleanup;


    ZeroMemory(&lpbi->bmiHeader, sizeof(BITMAPINFOHEADER));
    lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  //  // Get info but first de-select OffscrBmp because GetDIBits requires it:
  //  SelectObject(OffscrDC, OldBmp);
  //  if (!GetDIBits(OffscrDC, OffscrBmp, 0, height, NULL, lpbi, DIB_RGB_COLORS))
  //      goto cleanup;

    // Reserve memory for bitmap bits:
    if ((lpvBits = new char[lpbi->bmiHeader.biSizeImage]) == NULL)
        goto cleanup;
	//----------------------------------------------------
	// Have GetDIBits convert OffscrBmp to a DIB (device-independent bitmap):
	if (!GetDIBits(OffscrDC, bitmap, 0, height, lpvBits, lpbi, DIB_RGB_COLORS))
		goto cleanup;

    // Create a file to save the DIB to:
    if ((BmpFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) {
		goto cleanup;
	}

    DWORD Written;    // number of bytes written by WriteFile
    
    // Write a file header to the file:
    bmfh.bfType = 19778;        // 'BM'
    // bmfh.bfSize = ???        // we'll write that later
    bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
    // bmfh.bfOffBits = ???     // we'll write that later
    if (!WriteFile(BmpFile, &bmfh, sizeof(bmfh), &Written, NULL))
        goto cleanup;

    if (Written < sizeof(bmfh)) 
		goto cleanup; 

    // Write BITMAPINFOHEADER to the file:
    if (!WriteFile(BmpFile, &lpbi->bmiHeader, sizeof(BITMAPINFOHEADER), &Written, NULL)) 
		goto cleanup;
	
    if (Written < sizeof(BITMAPINFOHEADER)) 
			goto cleanup;

    // Calculate size of palette:
    int PalEntries;
    // 16-bit or 32-bit bitmaps require bit masks:
    if (lpbi->bmiHeader.biCompression == BI_BITFIELDS) 
		PalEntries = 3;
    else
        // bitmap is palettized?
        PalEntries = (lpbi->bmiHeader.biBitCount <= 8) ?
            // 2^biBitCount palette entries max.:
            (int)(1 << lpbi->bmiHeader.biBitCount)
        // bitmap is TrueColor -> no palette:
        : 0;
    // If biClrUsed use only biClrUsed palette entries:
    if(lpbi->bmiHeader.biClrUsed) 
		PalEntries = lpbi->bmiHeader.biClrUsed;

    // Write palette to the file:
    if(PalEntries){
        if (!WriteFile(BmpFile, &lpbi->bmiColors, PalEntries * sizeof(RGBQUAD), &Written, NULL)) 
			goto cleanup;

        if (Written < PalEntries * sizeof(RGBQUAD)) 
			goto cleanup;
		}

    // The current position in the file (at the beginning of the bitmap bits)
    // will be saved to the BITMAPFILEHEADER:
    bmfh.bfOffBits = SetFilePointer(BmpFile, 0, 0, FILE_CURRENT);

    // Write bitmap bits to the file:
    if (!WriteFile(BmpFile, lpvBits, lpbi->bmiHeader.biSizeImage, &Written, NULL)) 
		goto cleanup;
	
    if (Written < lpbi->bmiHeader.biSizeImage) 
		goto cleanup;

    // The current pos. in the file is the final file size and will be saved:
    bmfh.bfSize = SetFilePointer(BmpFile, 0, 0, FILE_CURRENT);

    // We have all the info for the file header. Save the updated version:
    SetFilePointer(BmpFile, 0, 0, FILE_BEGIN);
    if (!WriteFile(BmpFile, &bmfh, sizeof(bmfh), &Written, NULL))
        goto cleanup;

    if (Written < sizeof(bmfh)) 
		goto cleanup;
	
	FlushFileBuffers(BmpFile);
	success = true;
cleanup:
	delete[] lpvBits;
    return success;
}


BOOL CreateThumbnails2(LPCTSTR path)
{
	SHELLEXECUTEINFO shell_info;
	DWORD com_initialize_calls = 0;
	
	// Initialize COM.
	HRESULT hrComInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	// Ignore RPC_E_CHANGED_MODE; this just means that COM has already been
    // initialized with a different mode. Since we don't care what the mode is,
    // we'll just use the existing mode.
    if (FAILED(hrComInit))
    {
		if (hrComInit != RPC_E_CHANGED_MODE)
		{
			std::cout << "CoInitializeEx failed: 0x%08lx\n" << hrComInit;
		}
    } else {
		++com_initialize_calls;
	}

	shell_info.hwnd = NULL;
	shell_info.lpFile = path;
	BOOL retval = ShellExecuteEx(&shell_info);
	return retval;
}