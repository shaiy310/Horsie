#ifndef __REGISTRY_HANDLER_H__
#define __REGISTRY_HANDLER_H__

#include <Windows.h>
#include <exception>

class RegistryHandler
{
public:
	class RegistryError : public std::exception { };

	RegistryHandler(LPCTSTR path);
	RegistryHandler(const RegistryHandler&);
	RegistryHandler& operator=(const RegistryHandler& other);
	~RegistryHandler();

	LPCTSTR getSubKey() const;

	void Write(LPCTSTR value_name, LPCTSTR data);

private:
	HKEY reg_handle;
	LPCTSTR sub_key;
};

bool operator==(const RegistryHandler& obj1, const RegistryHandler& obj2);

#endif