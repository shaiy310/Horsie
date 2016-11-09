#include <WinSock2.h>
#include <cstdio>
#include <string>
#include <ctime>

#include "CommandProtocol.h"

// help method
BYTE * copy_data(const BYTE * data, const DWORD data_len);
BYTE * copy_data(const LPCTSTR data, const DWORD data_len);

CMDPacket::CMDPacket(const CMDType type, const DataProcessing data_processing, 
	const BYTE data_processing_param, const BYTE* data, const DWORD data_len)
{
	this->header.timestamp = static_cast<DWORD>(time(NULL));
	this->header.type = type;
	this->header.data_processing = data_processing;
	this->header.data_processing_param = data_processing_param;
	memset(this->header.reserved, 0x00, sizeof(this->header.reserved));
	this->raw_data_len = data_len;
	this->raw_data = copy_data(data, data_len);
	this->process_data();
	this->header.checksum = calculate_checksum(this->data, this->header.data_len);
}

CMDPacket::CMDPacket(const CMDType type, const DataProcessing data_processing,
	const BYTE data_processing_param, const LPCTSTR data)
	
{
	this->header.timestamp = static_cast<DWORD>(time(NULL));
	this->header.type = type;
	this->header.data_processing = data_processing;
	this->header.data_processing_param = data_processing_param;
	memset(this->header.reserved, 0x00, sizeof(this->header.reserved));
	this->raw_data_len = strlen(data);
	this->raw_data = copy_data(data, this->raw_data_len);
	this->process_data();
	this->header.checksum = calculate_checksum(this->data, this->header.data_len);
}

CMDPacket::CMDPacket(const CMDHeader psudo_header, const BYTE * data, const DWORD data_len)
{
	if (psudo_header.data_len != data_len) {
		throw InvalidHeaderError();
	}

	this->header = psudo_header;
	this->data = copy_data(data, this->header.data_len);
	this->deprocess_data();
}

CMDPacket::CMDPacket(const CMDPacket& other)
{
	*this = other;
}

CMDPacket& CMDPacket::operator=(const CMDPacket& other)
{
	if (this != &other) {
		delete[] this->data;

		this->header.type = other.header.type;
		this->header.data_processing = other.header.data_processing;
		this->header.data_processing_param = other.header.data_processing_param;
		this->header.data_len = other.header.data_len;
		this->data = copy_data(other.data, other.header.data_len);
		this->raw_data_len = other.raw_data_len;
		this->raw_data = copy_data(other.raw_data, other.raw_data_len);
	}

	return *this;
}

CMDPacket::~CMDPacket()
{
	delete[] this->data;
	delete[] this->raw_data;
}

BYTE * CMDPacket::ToBinary(DWORD * length)
{
	BYTE * buffer = new BYTE[HEADER_SIZE + this->header.data_len];
	CMDHeader header = this->header;

	DWORD magic = htonl(PROTOCOL_MAGIC);
	header.checksum = htonl(calculate_checksum(this->data, this->header.data_len));
	header.timestamp = htonl(header.timestamp);
	header.data_len  = htonl(header.data_len);

	DWORD offset = 0;
	memmove(buffer + offset, &magic, sizeof(magic));
	offset += sizeof(magic);
	memmove(buffer + offset, &(header), sizeof(header));
	offset += sizeof(header);
	memmove(buffer + offset, this->data, this->header.data_len);
	*length = offset + this->header.data_len;

	return buffer;
}

CMDPacket::CMDHeader CMDPacket::FromBinary(const BYTE * binary, const DWORD binary_len)
{
	if (binary_len < HEADER_SIZE) {
		throw InvalidHeaderError();
	}

	DWORD magic;
	memmove(&magic, binary, sizeof(magic));
	if (ntohl(magic) != PROTOCOL_MAGIC) {
		throw InvalidHeaderError();
	}

	CMDHeader header;
	memmove(&header, binary + 4, sizeof(header));
	for (int i = 0; i < RESERVED_SIZE; ++i) {
		if (header.reserved[i] != 0) {
			throw InvalidHeaderError();
		}
	}

	header.checksum = ntohl(header.checksum);
	header.timestamp = ntohl(header.timestamp);
	header.data_len = ntohl(header.data_len);

	return header;
}


void CMDPacket::process_data()
{
	BYTE temp;
	this->header.data_len = this->raw_data_len;
	this->data = new BYTE[this->header.data_len];

	switch (this->header.data_processing) {
	case XOR:
		for (DWORD i = 0; i < this->header.data_len; ++i) {
			this->data[i] = this->raw_data[i] ^ this->header.data_processing_param;
		}
		break;

	case ROLLED_XOR:
		temp = this->header.data_processing_param;
		for (DWORD i = 0; i < this->header.data_len; ++i) {
			temp ^= this->raw_data[i];
			this->data[i] = temp;
		}
		break;

	default:	// NONE
		memmove(this->data, this->raw_data, this->header.data_len);
		break;
	}
}

void CMDPacket::deprocess_data()
{
	BYTE temp;
	this->raw_data_len = this->header.data_len;
	this->raw_data = new BYTE[this->raw_data_len];

	switch (this->header.data_processing) {
	case XOR:
		for (DWORD i = 0; i < this->raw_data_len; ++i) {
			this->raw_data[i] = this->raw_data[i] ^ this->header.data_processing_param;
		}
		break;

	case ROLLED_XOR:
		temp = this->header.data_processing_param;
		for (DWORD i = 0; i < this->raw_data_len; ++i) {
			this->raw_data[i] = temp ^ this->data[i];
			temp = this->data[i];
			
		}
		break;

	default:	// NONE
		memmove(this->raw_data, this->data, this->raw_data_len);
		break;
	}
}

bool operator==(CMDPacket& p1, CMDPacket& p2)
{
	return ((p1.header == p2.header) &&
		(memcmp(p1.data, p2.data, p1.header.data_len) == 0));
}

bool operator==(CMDPacket::CMDHeader& h1, CMDPacket::CMDHeader& h2)
{
	return ((h1.checksum == h2.checksum) &&
		(h1.timestamp == h2.timestamp) &&
		(h1.type == h2.type) &&
		(h1.data_processing == h2.data_processing) &&
		(h1.data_processing_param == h2.data_processing_param) &&
		(h1.data_len == h2.data_len));
}

DWORD calculate_checksum(const BYTE * data, const DWORD data_len)
{
	DWORD checksum = 0;
	for (DWORD i = 0; i < data_len; ++i) {
		checksum += data[i];
	}

	return checksum;
}

// help method
BYTE * copy_data(const BYTE * data, const DWORD data_len)
{
	BYTE * new_data = new BYTE[data_len];
	for (DWORD i = 0; i < data_len; ++i) {
		new_data[i] = data[i];
	}

	return new_data;
}

BYTE * copy_data(const LPCTSTR data, const DWORD data_len)
{
	BYTE * new_data = new BYTE[data_len];
	for (DWORD i = 0; i < data_len; ++i) {
		new_data[i] = static_cast<BYTE>(data[i]);
	}

	return new_data;
}