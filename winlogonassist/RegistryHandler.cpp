#include "RegistryHandler.h"

RegistryHandler::RegistryHandler(LPCTSTR path)
{
	HKEY head_key = HKEY_CURRENT_USER;	//try using RegOpenCurrentUser 
	sub_key = path;

	LONG retval = RegCreateKeyEx(head_key, sub_key, 0, NULL, REG_OPTION_NON_VOLATILE, 
		KEY_ALL_ACCESS, NULL, &reg_handle, NULL);

	if (retval != ERROR_SUCCESS) {
		throw RegistryError();
	}

}

RegistryHandler::RegistryHandler(const RegistryHandler& other)
{
	HKEY head_key = HKEY_CURRENT_USER;	//try using RegOpenCurrentUser 
	sub_key = other.sub_key;
	LONG retval = RegOpenKeyEx(head_key, sub_key, 0, KEY_ALL_ACCESS, &reg_handle);

	if (retval != ERROR_SUCCESS) {
		throw RegistryError();
	}
}

RegistryHandler& RegistryHandler::operator=(const RegistryHandler& other) {
	if (this != &other) {
		RegCloseKey(reg_handle);
		HKEY head_key = HKEY_CURRENT_USER;	//try using RegOpenCurrentUser
		sub_key = other.sub_key;
		LONG retval = RegOpenKeyEx(head_key, other.sub_key, 0, KEY_ALL_ACCESS, &reg_handle);

		if (retval != ERROR_SUCCESS) {
			throw RegistryError();
		}
	}

	return *this;
}

RegistryHandler::~RegistryHandler()
{
	RegCloseKey(reg_handle);
}

void RegistryHandler::Write(LPCTSTR value_name, LPCTSTR data)
{
	size_t data_len = strlen(data);
	BYTE * byte_data = new BYTE[data_len + 1];
	
	for (size_t i = 0; i < data_len; ++i) {
		byte_data[i] = data[i];
	}
	byte_data[data_len] = '\0';

	LONG retval = RegSetValueEx(reg_handle, value_name, 0, REG_SZ, byte_data, data_len + 1);
	if (retval != ERROR_SUCCESS) {
		throw RegistryError();
	}

}

LPCTSTR RegistryHandler::getSubKey() const
{
	return this->sub_key;
}

bool operator==(const RegistryHandler& obj1, const RegistryHandler& obj2)
{
	return obj1.getSubKey() == obj2.getSubKey();
}