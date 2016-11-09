#ifndef __BASE_EXCEPTION_H__
#define __BASE_EXCEPTION_H__

#include <exception>
#include <Windows.h>

enum ModulesMagic : BYTE {
	SCREENSHOT_HANDLER = 0xf1,
	INFO_COLLECTOR = 0xf2,
	BMP_HANDLER = 0xf3,
	CAMERA_HANDLER = 0xf4,
};

class BaseException : public std::exception
{
public:
	enum ErrorType { NORMAL = 0, WSA = 1 };
	BaseException(ModulesMagic magic, DWORD line = 0, ErrorType error_type = NORMAL) :
		magic(magic), line(line) {
		if (error_type == NORMAL) {
			last_error = GetLastError();
		}
		else {
			last_error = WSAGetLastError();
		}
	}

	DWORD GetMagic() const { return this->magic; }
	DWORD GetLine() const { return this->line; }
	DWORD GetError() const { return this->last_error; }
	
protected:

	ModulesMagic magic;
	DWORD last_error;
	DWORD line;
	std::exception e;
};

#endif