#ifndef __MAIL_SENDER_H__
#define __MAIL_SENDER_H__

#include <Windows.h>
#include <WinSock2.h>
#include <list>

#define BUFLEN 256

typedef struct attached_file
{
	LPCTSTR item; //file name
	LPCTSTR path; //file path
} AttachedFile;

int tx(SOCKET, char *, int);
int rx(SOCKET, char *);
int tob64(unsigned char *, char *, int);

class MailSender
{
public:
	MailSender(LPCTSTR smtp_server);
	~MailSender();

	int func_main(LPCTSTR subject, LPCTSTR body);

private:
	LPCTSTR smtp_server;
	SOCKET s;
	LPTSTR user;
	LPTSTR domain;
	LPCTSTR from_addr;
	std::list<LPCTSTR> rrcptroot; // mail addresses
	std::list<AttachedFile> attroot;	// attached files
};


#endif
