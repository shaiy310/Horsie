#ifndef __THUMBNAILS_H__
#define __THUMBNAILS_H__

#include <Windows.h>
#include <Vfw.h>

#include "BMPHandler.h"

class Thumbnails : public BMPHandler
{
public:
	Thumbnails();
	~Thumbnails();
	
	VOID Create(LPCTSTR img_file, LPCTSTR out_file);

private:
};

#endif