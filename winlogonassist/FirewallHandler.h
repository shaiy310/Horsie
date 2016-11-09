#ifndef __FIREWALL_HANDLER_H__
#define __FIREWALL_HANDLER_H__

#include <Windows.h>
#include <netfw.h>

#pragma comment( lib, "ole32.lib" )
#pragma comment( lib, "oleaut32.lib" )

class FirewallHandler
{
public:
	FirewallHandler();
	~FirewallHandler();

	void InitializeCom();
	BOOL AddRule(BSTR name, BSTR description, BSTR group, BSTR apllication, BSTR port);

private:
	UINT com_initialize_calls;

	HRESULT WFCOMInitialize(INetFwPolicy2** ppNetFwPolicy2);

};


#endif