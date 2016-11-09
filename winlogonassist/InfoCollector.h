#ifndef __INFO_COLLECTOR_H__
#define __INFO_COLLECTOR_H__

#include <Windows.h>
#include <strsafe.h>
//#include <string>

#include "BaseException.h"

class InfoCollector
{
public:
	class InfoCollectorException : public BaseException {
	public: InfoCollectorException(DWORD line, ErrorType type = WSA) : BaseException(INFO_COLLECTOR, line, type) { }
	};
	InfoCollector();
	~InfoCollector();

	LPCTSTR ToString();

private:
	LPCTSTR GetInternalIP();
	LPSTR GetExternalIP();
	LPCTSTR GetHostName();
	LPCTSTR GetUser();
	LPCTSTR GetOSVertion();
	LPCTSTR GetA();
	LPCTSTR GetB();
	LPCTSTR GetC();
	LPCTSTR GetD();

//	SOCKET_ADDRESS_LIST * InfoCollector::GetAddressList();

	STRSAFE_LPSTR info;
};

#endif