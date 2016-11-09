#include <WinSock2.h>
#include <Ws2tcpip.h>

// link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#include "InfoCollector.h"

// depends on windows.h
#include <VersionHelpers.h>

const LPCSTR ServerIP = "78.47.139.102";
const WORD ServerPort = 80;
const LPSTR ServerRequest = "GET http://myexternalip.com/raw  HTTP/1.1\r\nHost: myexternalip.com\r\n\r\n"; 
//"GET http://ifconfig.me/ip.html  HTTP/1.1\r\nHost: www.ifconfig.me\r\n\r\n";
const LPSTR SuccessReply = "HTTP/1.1 200 OK";

InfoCollector::InfoCollector()
{
	WSADATA wdata;
	info = new TCHAR[1000];

	if (WSAStartup(MAKEWORD(2, 2), &wdata) != 0) {
		throw InfoCollectorException(WSAGetLastError());
	}
}

InfoCollector::~InfoCollector()
{
	WSACleanup();
	delete[] info;
}

LPCTSTR InfoCollector::ToString()
{
	LPCTSTR inIP = GetInternalIP(),
		exIP = GetExternalIP(),
		host_name = GetHostName(),
		user_name = GetUser(),
		os_version = GetOSVertion();

	StringCchPrintf(info, 1000,
		"Internal IP: %s\n"
		"External IP: %s\n"
		"Host Name:   %s\n"
		"User Name:   %s\n"
		"OS Version:  %s\n",
		inIP, exIP, host_name, user_name, os_version);

	/*delete exIP;*/
	delete inIP;
	delete host_name;
	delete user_name;

	return info;
}

LPCTSTR InfoCollector::GetInternalIP()
{
	DWORD length = 200;
	LPSTR message = new TCHAR[length + 1];
	memset(message, 0, length + 1);

	char ac[50];
	try {
		if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) {
			throw InfoCollectorException(__LINE__);
		}
		//cout << "Host name is " << ac << "." << endl;

		struct hostent *phe = gethostbyname(ac);
		if (phe == 0) {
			throw InfoCollectorException(__LINE__);
		}
	
		for (int i = 0; i < phe->h_length; ++i) {
			struct in_addr addr;
			memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
			StringCchPrintf(message, length, "%s%s\n", message, inet_ntoa(addr));//inet_ntoa()
		}
	}
	catch (const InfoCollectorException& e) {
		StringCchPrintf(message, length, "Error Code: %d line: %d", e.GetError(), e.GetLine());
	}

	return message;
}

LPSTR InfoCollector::GetExternalIP()
{
	DWORD bytes_transfer = 0;
	DWORD length = 200;
	LPSTR message = new TCHAR[length + 1];
	memset(message, 0, length + 1);
	SOCKET sock;

	try {
		sock = WSASocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
		if (sock == INVALID_SOCKET) {
			throw InfoCollectorException(__LINE__);
		}

		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = inet_addr(ServerIP);
		addr.sin_port = htons(ServerPort);

		// TODO: try to convert to: WSAConnectByList
		WSAConnect(sock, (struct sockaddr*)&addr, sizeof(addr), NULL, NULL, NULL, NULL);

		WSABUF buf;
		buf.len = strlen(ServerRequest);
		buf.buf = ServerRequest;

		if ((WSASend(sock, &buf, 1, &bytes_transfer, 0, NULL, NULL) != 0) ||
			(bytes_transfer != buf.len)) {
			throw InfoCollectorException(__LINE__);
		}

		buf.len = 300;	// "aaa.bbb.ccc.ddd"
		buf.buf = new CHAR[buf.len];
		DWORD flags = 0;
		if ((WSARecv(sock, &buf, 1, &bytes_transfer, &flags, NULL, NULL) != 0) ||
			(strncmp(buf.buf, SuccessReply, strlen(SuccessReply)) != 0)) {	// "x.x.x.x"
			throw InfoCollectorException(__LINE__);
		}

		buf.buf[bytes_transfer] = '\0';
		memmove(buf.buf, buf.buf + 213, 16);
		buf.buf[16] = '\0';
		
		return buf.buf;
	}
	catch (const InfoCollectorException& e)
	{
		DWORD params[3] = { e.GetError(), e.GetLine(), bytes_transfer };
		FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
			"Error Code: %1!u! line: %2!u! recv/send %3!u! bytes", 0, 0, message, length, (va_list*)params);
		return message;
	}
	closesocket(sock);
	return new CHAR[50];
}

LPCTSTR InfoCollector::GetHostName()
{
	DWORD length = 50;
	LPSTR name = new TCHAR[length + 1];
	
	GetComputerName(name, &length);

	return name;
}

LPCTSTR InfoCollector::GetUser()
{
	LPSTR name = new TCHAR[50];
	DWORD length = 50;
	GetUserName(name, &length);

	return name;
}

LPCTSTR InfoCollector::GetOSVertion()
{
	if (IsWindows7OrGreater()) {
		if (IsWindows7SP1OrGreater()) {
			if (IsWindows8OrGreater()) {
				if (IsWindows8Point1OrGreater()) {
					// FIXME: windows 10 is not define in windows.h
					if (IsWindowsVersionOrGreater(10, 0, 0)) {
						return TEXT("Windows 10");
					}
					return TEXT("Windows 8.1 and above");
				}
				return TEXT("Windows 8");
			}
			return TEXT("Windows 7 SP1 and above");
		}
		return TEXT("Windows 7 under SP1");
	}
	
	return TEXT("Before Windows7");
}

//SOCKET_ADDRESS_LIST * InfoCollector::GetAddressList()
//{
//	SOCKET_ADDRESS_LIST * AddressList = new SOCKET_ADDRESS_LIST[1];
//	GetAddrInfo(ServerIP, ServerPort, );
//	return AddressList;
//
//}