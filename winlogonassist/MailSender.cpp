#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <winsock2.h>
#include <string>

#include "MailSender.h"

static const char BASE64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char PAD64 = '=';
short verbose = 0;
char *rcsid = "$Id: yamp.cpp,v 1.9 2003/08/29 11:03:53 wroos Exp $";
char *release = "1.2";

MailSender::MailSender(LPCTSTR smtp_server) : smtp_server(smtp_server)
{
	DWORD len = 32;
	WSADATA wsaptr;
	WSAStartup(MAKEWORD(2, 0), &wsaptr);

	fprintf(stdout, "[%s:%d] Getting host and username\n", __FILE__, __LINE__);
	if (gethostname(domain, 64) == -1) {
		fprintf(stderr, "[%s:%d] Error %s calling gethostname\n", __FILE__,
			__LINE__, WSAGetLastError());
		WSACleanup();
	}

	user = new TCHAR[len];
	if (GetUserName(user, &len) == 0) {
		fprintf(stderr, "[%s:%d] Error %s calling GetUsername\n", __FILE__,
			__LINE__, GetLastError());
		WSACleanup();
	}

	fprintf(stdout, "Mail from %s@%s...\n", user, domain);
}

MailSender::~MailSender()
{
	delete[] user;
}

/* ------------------------------------------------------------------------ *
* main()
* ------------------------------------------------------------------------ */
int MailSender::func_main(LPCTSTR subject, LPCTSTR body)
{
	short attachments = 0;
	int i, j, a, skip, numrcpts = 0;
	char *smtpsrv = NULL, *afilename = NULL;
	char buf[BUFLEN];
	char ipaddress[16];
	FILE *fp;
	time_t ltime;
	struct tm *now;
	struct hostent *he = NULL;
	struct sockaddr_in addr;
	struct _stat sb;

	//s for server					var: smtpsrv
	//u for subject					var: subject
	//b for body					var: body
	//t for reply to				var: replytoroot	| replyto
	//r for destination emails		var: rcptroot		| rcpts
	//a for attachments				var: attroot		| atts

	if (verbose) fprintf(stdout, "Resolving smtp server %s...\n", smtpsrv);

	he = gethostbyname(smtpsrv);
	if (!he) {
		fprintf(stderr, "[%s:%d] Error %d calling gethostbyname\n", __FILE__,
			__LINE__, WSAGetLastError());
		WSACleanup();
		return -1;
	}

	//strcpy(ipaddress, inet_ntoa(*((struct in_addr *)he->h_addr)));

#ifdef DEBUG
	fprintf(stdout, "[%s:%d] Connecting to %s\n", __FILE__, __LINE__,
		ipaddress);
	fflush(stdout);
#endif

	s = socket(PF_INET, SOCK_STREAM, 0);

	if (s == INVALID_SOCKET) {
		fprintf(stderr, "[%s:%d] Error %d creating socket\n", __FILE__, __LINE__,
			WSAGetLastError());
		return -1;
	}

	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(25);
	addr.sin_addr.s_addr = inet_addr(ipaddress);

	if (verbose) fprintf(stdout, "Connecting to %s:25\n", ipaddress);

	if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
		fprintf(stderr, "[%s:%d] Error %d connecting to %s:%d\n", __FILE__,
			__LINE__, WSAGetLastError(), smtpsrv, 25);
		return -1;
	}

#ifdef DEBUG
	fprintf(stdout, "[%s:%d] Establishing SMTP connection\n", __FILE__,
		__LINE__);
	fflush(stdout);
#endif
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ end of setting socket
	if (verbose) fprintf(stdout, "Sending mail...\n", ipaddress);

	if (rx(s, buf) == -1 || strncmp(buf, "220", 3) != 0) {
		fprintf(stderr, "[%s:%d] %s returned %s\n", __FILE__, __LINE__, smtpsrv,
			buf);
		closesocket(s);
		WSACleanup();
		return -1;
	}

	sprintf_s(buf, BUFLEN, "helo %s\r\n", domain);
	tx(s, buf, strlen(buf));
	if (rx(s, buf) == -1 || strncmp(buf, "250", 3) != 0) {
		fprintf(stderr, "[%s:%d] %s returned %s\n", __FILE__, __LINE__, smtpsrv,
			buf);
		closesocket(s);
		WSACleanup();
		return -1;
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ end of connection establishment
#ifdef DEBUG
	fprintf(stdout, "[%s:%d] Sending SMTP header\n", __FILE__, __LINE__);
	fflush(stdout);
#endif

//	/*replyto = replytoroot;
//	if (replyto) sprintf(buf, "mail from:%s\r\n", replyto->item);
//	else */sprintf(buf, "mail from:<%s@%s>\r\n", user, domain);
//	tx(s, buf, strlen(buf));
//
//	if (rx(s, buf) == -1 || strncmp(buf, "250", 3) != 0) {
//		fprintf(stderr, "[%s:%d] %s returned %s\n", __FILE__, __LINE__, smtpsrv,
//			buf);
//		closesocket(s);
//		WSACleanup();
//		return -1;
//	}
//
//#ifdef DEBUG
//	fprintf(stdout, "[%s:%d] Recipients\n", __FILE__, __LINE__);
//	fflush(stdout);
//#endif
//
//	while (1) {
//		//sprintf(buf, "rcpt to:<%s>\r\n", rcpts->item);
//
//#ifdef DEBUG
//		fprintf(stdout, "[%s:%d] %s", __FILE__, __LINE__, buf);
//		fflush(stdout);
//#endif
//
//		tx(s, buf, strlen(buf));
//		if (rx(s, buf) == -1 || strncmp(buf, "250", 3) != 0) {
//			fprintf(stderr, "[%s:%d] %s returned %s\n", __FILE__, __LINE__,
//				smtpsrv, buf);
//			closesocket(s);
//			WSACleanup();
//			return -1;
//		}
//
//		//if (rcpts->next) rcpts = rcpts->next;
//		else break;
//	}
//
//	sprintf(buf, "data\r\n");
//	tx(s, buf, strlen(buf));
//	if (rx(s, buf) == -1 || strncmp(buf, "354", 3) != 0) {
//		fprintf(stderr, "[%s:%d] %s returned %s\n", __FILE__, __LINE__, smtpsrv,
//			buf);
//		closesocket(s);
//		WSACleanup();
//		return -1;
//	}
//
//#ifdef DEBUG
//	fprintf(stdout, "[%s:%d] RFC 822 message body\n", __FILE__, __LINE__);
//	fflush(stdout);
//#endif
//
//	//rcpts = rcptroot;
//	if (rcpts) {
//		sprintf(buf, "To:%s", rcpts->item);
//		tx(s, buf, strlen(buf));
//
//		while (1) {
//			if (verbose) fprintf(stdout, "To %s\n", rcpts->item);
//			if (rcpts->next) rcpts = rcpts->next;
//			else break;
//			sprintf(buf, ",\n\t%s", rcpts->item);
//			tx(s, buf, strlen(buf));
//		}
//
//		sprintf(buf, "\r\n");
//		tx(s, buf, strlen(buf));
//	}
//
//	replyto = replytoroot;
//	if (replyto) sprintf(buf, "From:%s\r\n", replyto->item);
//	else sprintf(buf, "From:%s@%s\r\n", user, domain);
//	tx(s, buf, strlen(buf));
//
//	replyto = replytoroot;
//	if (replyto) sprintf(buf, "Return-Path:%s\r\n", replyto->item);
//	else sprintf(buf, "Return-Path:%s@%s\r\n", user, domain);
//	tx(s, buf, strlen(buf));
//
//	replyto = replytoroot;
//	if (replyto) {
//		sprintf(buf, "Reply-To:%s", replyto->item);
//		tx(s, buf, strlen(buf));
//
//		while (1) {
//			if (verbose) fprintf(stdout, "Reply to %s\n", replyto->item);
//			if (replyto->next) replyto = replyto->next;
//			else break;
//			sprintf(buf, ",\r\n\t%s", replyto->item);
//			tx(s, buf, strlen(buf));
//		}
//
//		sprintf(buf, "\r\n");
//		tx(s, buf, strlen(buf));
//	}
//
//	sprintf(buf, "Subject:%s\r\n", subject);
//	tx(s, buf, strlen(buf));
//
//	time(&ltime);
//	now = localtime(&ltime);
//	strftime(buf, BUFLEN - 1, "Date:%a, %d %b %y %H:%M:%S +0200\r\n", now);
//	tx(s, buf, strlen(buf));
//
//	sprintf(buf, "MIME-Version: 1.0\r\n"); // is that important?
//	tx(s, buf, strlen(buf));
//
//	if (attachments) {
//		sprintf(buf, "Content-type: multipart/mixed; boundary=abc\r\n");
//		tx(s, buf, strlen(buf));
//	}
//	else {
//		sprintf(buf, "Content-type: text/plain\r\n");
//		tx(s, buf, strlen(buf));
//	}
//
//#ifdef DEBUG
//	fprintf(stdout, "[%s:%d] Message body\n", __FILE__, __LINE__);
//	fflush(stdout);
//#endif
//
//	sprintf(buf, "\r\n");
//	tx(s, buf, strlen(buf));
//
//	if (body) {
//		if (attachments) {
//			sprintf(buf, "This is a multi-part message in MIME format.\r\n");
//			tx(s, buf, strlen(buf));
//
//			sprintf(buf, "\r\n");
//			tx(s, buf, strlen(buf));
//
//			sprintf(buf, "--abc\r\n");
//			tx(s, buf, strlen(buf));
//
//			sprintf(buf, "\r\n");
//			tx(s, buf, strlen(buf));
//		}
//
//		if (_stat(body, &sb) == 0) /* File exists */
//		{
//			fp = fopen(body, "r");
//			if (!fp) {
//				fprintf(stderr, "[%s:%d] Error opening file %s for reading\n",
//					__FILE__, __LINE__, body);
//				return -1;
//			}
//			else {
//				while (1) {
//					j = 0;
//					while (j < BUFLEN - 1) {
//						i = fgetc(fp);
//						if (feof(fp)) break;
//						buf[j] = i;
//						buf[j + 1] = 0;
//						j++;
//					}
//
//					tx(s, buf, strlen(buf));
//
//					if (feof(fp)) break;
//				}
//			}
//			fclose(fp);
//		}
//		else {
//			/*
//			* -b flag is text to be included directly
//			*/
//			tx(s, body, strlen(body));
//		}
//	}
//	else if (fseek(stdin, 1, SEEK_SET) == 0)
//		/*
//		* Check if there is something to read on stdin, other wise you wait
//		*/
//	{
//		fseek(stdin, 0, SEEK_SET); /* Rewind again */
//
//		if (attachments) {
//			sprintf(buf, "This is a multi-part message in MIME format.\r\n");
//			tx(s, buf, strlen(buf));
//
//			sprintf(buf, "\r\n");
//			tx(s, buf, strlen(buf));
//
//			sprintf(buf, "--abc\r\n");
//			tx(s, buf, strlen(buf));
//
//			sprintf(buf, "\r\n");
//			tx(s, buf, strlen(buf));
//		}
//
//		while (1) {
//			j = 0;
//			while (j < BUFLEN - 1) {
//				i = fgetc(stdin);
//				if (feof(stdin)) break;
//				buf[j] = i;
//				buf[j + 1] = 0;
//				j++;
//			}
//
//			tx(s, buf, strlen(buf));
//
//			if (feof(stdin)) break;
//		}
//	}
//	else {
//		sprintf(buf, "%s", "No message body\r\n");
//		tx(s, buf, strlen(buf));
//	}
//
//	if (attachments) {
//		atts = attroot;
//		while (atts) {
//			if (verbose) fprintf(stdout, "Attaching file %s...\n", atts->item);
//
//			sprintf(buf, "\r\n");
//			tx(s, buf, strlen(buf));
//
//			sprintf(buf, "--abc\r\n");
//			tx(s, buf, strlen(buf));
//
//			sprintf(buf, "Content-type: application/octet-stream; name=%s\r\n",
//				atts->item);
//			tx(s, buf, strlen(buf));
//
//			sprintf(buf, "Content-transfer-encoding: base64\r\n");
//			tx(s, buf, strlen(buf));
//
//			sprintf(buf, "\r\n");
//			tx(s, buf, strlen(buf));
//
//			fp = fopen(atts->path, "rb");
//			if (!fp) {
//				fprintf(stderr, "[%s:%d] Error opening file %s for reading\n",
//					__FILE__, __LINE__, atts->path);
//				closesocket(s);
//				WSACleanup();
//				return -1;
//			}
//			else {
//				while (1) {
//					unsigned char binbuf[BUFLEN];
//					char b64buf[BUFLEN * 2], b64_76buf[78];
//					int k = 0;
//
//					j = 0;
//					i = fgetc(fp);
//					while (!feof(fp)) {
//						binbuf[j] = i;
//						binbuf[j + 1] = 0;
//						j++;
//						if (j >= BUFLEN - 1) break;
//						i = fgetc(fp);
//					}
//
//					j = tob64(binbuf, b64buf, j);
//
//					for (k = 0; k < j; k += 76) {
//						int l = (j - k < 76) ? j - k : 76;
//						strncpy(b64_76buf, &b64buf[k], l);
//						b64_76buf[l++] = '\n';
//						b64_76buf[l] = 0;
//						tx(s, b64_76buf, l);
//					}
//
//					if (feof(fp)) break;
//				}
//			}
//
//			fclose(fp);
//
//			atts = atts->next;
//		}
//
//		sprintf(buf, "\r\n");
//		tx(s, buf, strlen(buf));
//
//		sprintf(buf, "--abc--\r\n");
//		tx(s, buf, strlen(buf));
//	}
//
//#ifdef DEBUG
//	fprintf(stdout, "[%s:%d] EOM\n", __FILE__, __LINE__);
//	fflush(stdout);
//#endif
//
//	sprintf(buf, "\r\n.\r\n");
//	tx(s, buf, strlen(buf));
//	if (rx(s, buf) == -1) {
//		fprintf(stderr, "[%s:%d] %s returned %s\n", __FILE__, __LINE__, smtpsrv,
//			buf);
//		closesocket(s);
//		WSACleanup();
//		return -1;
//	}
//
//#ifdef DEBUG
//	fprintf(stdout, "[%s:%d] Closing\n", __FILE__, __LINE__);
//	fflush(stdout);
//#endif
//
//	sprintf(buf, "quit\r\n");
//	tx(s, buf, strlen(buf));
//	if (rx(s, buf) == -1 || strncmp(buf, "221", 3) != 0) {
//		fprintf(stderr, "[%s:%d] %s returned %s\n", __FILE__, __LINE__, smtpsrv,
//			buf);
//		closesocket(s);
//		WSACleanup();
//		return -1;
//	}
//
//#ifdef DEBUG
//	fprintf(stdout, "[%s:%d] Tearing down connection\n", __FILE__, __LINE__);
//	fflush(stdout);
//#endif
//
//	closesocket(s);
//	WSACleanup();
//
//	if (verbose) fprintf(stdout, "Done\n");
//
//	return 1;
//}
//
///* ------------------------------------------------------------------------- *
//* tx()
//* ------------------------------------------------------------------------- */
//int tx(SOCKET s, char *str, int packetlen)
//{
//	size_t wbytes;
//	int n;
//
//	for (wbytes = 0; wbytes < packetlen; wbytes += n) {
//		n = send(s, str + wbytes, packetlen - wbytes, 0);
//		if (n == SOCKET_ERROR) {
//			fprintf(stderr, "[%s:%d] Error %d sending\n", __FILE__, __LINE__,
//				WSAGetLastError());
//			return -1;
//		}
//	}
	return 1;
}

/* ------------------------------------------------------------------------- *
* rx()
* ------------------------------------------------------------------------- */
int rx(SOCKET s, char *str)
{
	int r;

	memset(str, 0, BUFLEN);
	r = recv(s, str, BUFLEN - 1, 0);
	if (r == SOCKET_ERROR) {
		fprintf(stderr, "[%s:%d] Error %d receiving\n", __FILE__, __LINE__,
			WSAGetLastError());
		return -1;
	}

	return r;
}

/* ------------------------------------------------------------------------ *
* tob64()
* ------------------------------------------------------------------------ */
int tob64(unsigned char *source, char *target, int sourcelen)
{
	int targetlen = 0;
	unsigned char in[3], out[4], *ptr;

#ifdef DEBUG
	fprintf(stdout, "[%s:%d] tob64 source: [%s], length: %d\n", __FILE__,
		__LINE__, source, sourcelen);
	fflush(stdout);
#endif

	ptr = source;
	while (sourcelen > 2) {
		in[0] = *ptr++;
		in[1] = *ptr++;
		in[2] = *ptr++;
		sourcelen -= 3;

		out[0] = in[0] >> 2;
		out[1] = ((in[0] & 0x03) << 4) + (in[1] >> 4);
		out[2] = ((in[1] & 0x0f) << 2) + (in[2] >> 6);
		out[3] = in[2] & 0x3f;

		if (out[0] >= 64) fprintf(stderr, "[%s:%d]\n", __FILE__, __LINE__);
		if (out[1] >= 64) fprintf(stderr, "[%s:%d]\n", __FILE__, __LINE__);
		if (out[2] >= 64) fprintf(stderr, "[%s:%d]\n", __FILE__, __LINE__);
		if (out[3] >= 64) fprintf(stderr, "[%s:%d]\n", __FILE__, __LINE__);
		target[targetlen++] = BASE64[out[0]];
		target[targetlen++] = BASE64[out[1]];
		target[targetlen++] = BASE64[out[2]];
		target[targetlen++] = BASE64[out[3]];
	}

	/*
	* Now worry about padding
	*/
	if (sourcelen != 0) {
		int i;

		in[0] = in[1] = in[2] = 0;

		for (i = 0; i < sourcelen; i++) {
			in[i] = *ptr++;
		}

		out[0] = in[0] >> 2;
		out[1] = ((in[0] & 0x03) << 4) + (in[1] >> 4);
		out[2] = ((in[1] & 0x0f) << 2) + (in[2] >> 6);

		target[targetlen++] = BASE64[out[0]];
		target[targetlen++] = BASE64[out[1]];

		if (sourcelen == 1) {
			target[targetlen++] = PAD64;
		}
		else {
			target[targetlen++] = BASE64[out[2]];
		}
		target[targetlen++] = PAD64;
	}
	target[targetlen] = 0;

#ifdef DEBUG
	fprintf(stdout, "[%s:%d] tob64 target: [%s], length: %d\n", __FILE__,
		__LINE__, target, targetlen);
	fflush(stdout);
#endif

	return targetlen;
}