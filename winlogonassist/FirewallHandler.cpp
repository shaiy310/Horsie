#include "FirewallHandler.h"
#include <cstdio>
#include <iostream>

FirewallHandler::FirewallHandler()
{
	this->com_initialize_calls = 0;
	
}

FirewallHandler::~FirewallHandler()
{
	while (com_initialize_calls > 0) {
		CoUninitialize();
	}
}

void FirewallHandler::InitializeCom()
{
	// Initialize COM.
	HRESULT hrComInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	// Ignore RPC_E_CHANGED_MODE; this just means that COM has already been
    // initialized with a different mode. Since we don't care what the mode is,
    // we'll just use the existing mode.
    if (FAILED(hrComInit))
    {
		if (hrComInit != RPC_E_CHANGED_MODE)
		{
			std::cout << "CoInitializeEx failed: 0x%08lx\n" << hrComInit;
		}
    } else {
		++this->com_initialize_calls;
	}
}

BOOL FirewallHandler::AddRule(BSTR name, BSTR description, BSTR group, BSTR apllication, BSTR port)
{
    HRESULT hr = S_OK;

    INetFwPolicy2 *pNetFwPolicy2 = NULL;
    INetFwRules *pFwRules = NULL;
    INetFwRule *pFwRule = NULL;

    long CurrentProfilesBitMask = 0;

    BSTR bstrRuleName = name; //SysAllocString(L"OUTBOUND_RULE");
    BSTR bstrRuleDescription = description; //SysAllocString(L"Allow outbound network traffic from my Application over TCP port 4000");
    BSTR bstrRuleGroup = group; //SysAllocString(L"Sample Rule Group");
    BSTR bstrRuleApplication = apllication; //SysAllocString(L"%programfiles%\\MyApplication.exe");
    BSTR bstrRuleLPorts = port; //SysAllocString(L"4000");

    //com init

    // Retrieve INetFwPolicy2
    hr = this->WFCOMInitialize(&pNetFwPolicy2);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    // Retrieve INetFwRules
    hr = pNetFwPolicy2->get_Rules(&pFwRules);
    if (FAILED(hr))
    {
        std::cout << "get_Rules failed: 0x%08lx\n" << hr;
        goto Cleanup;
    }

    // Retrieve Current Profiles bitmask
    hr = pNetFwPolicy2->get_CurrentProfileTypes(&CurrentProfilesBitMask);
    if (FAILED(hr))
    {
        std::cout << "get_CurrentProfileTypes failed: 0x%08lx\n" << hr;
        goto Cleanup;
    }

    // When possible we avoid adding firewall rules to the Public profile.
    // If Public is currently active and it is not the only active profile, we remove it from the bitmask
    if ((CurrentProfilesBitMask & NET_FW_PROFILE2_PUBLIC) &&
        (CurrentProfilesBitMask != NET_FW_PROFILE2_PUBLIC))
    {
        CurrentProfilesBitMask ^= NET_FW_PROFILE2_PUBLIC;
    }

    // Create a new Firewall Rule object.
    hr = CoCreateInstance(
                __uuidof(NetFwRule),
                NULL,
                CLSCTX_INPROC_SERVER,
                __uuidof(INetFwRule),
                (void**)&pFwRule);
    if (FAILED(hr))
    {
        std::cout << "CoCreateInstance for Firewall Rule failed: 0x%08lx\n" << hr;
        goto Cleanup;
    }

    // Populate the Firewall Rule object
    pFwRule->put_Name(bstrRuleName);
    pFwRule->put_Description(bstrRuleDescription);
    pFwRule->put_ApplicationName(bstrRuleApplication);
    pFwRule->put_Protocol(NET_FW_IP_PROTOCOL_TCP);
    pFwRule->put_LocalPorts(bstrRuleLPorts);
    pFwRule->put_Direction(NET_FW_RULE_DIR_OUT);
    pFwRule->put_Grouping(bstrRuleGroup);
    pFwRule->put_Profiles(CurrentProfilesBitMask);
    pFwRule->put_Action(NET_FW_ACTION_ALLOW);
    pFwRule->put_Enabled(VARIANT_TRUE);

    // Add the Firewall Rule
    hr = pFwRules->Add(pFwRule);
    if (FAILED(hr))
    {
        std::cout << "Firewall Rule Add failed: 0x%08lx\n" << hr;
        goto Cleanup;
    }

Cleanup:

    // Free BSTR's
    SysFreeString(bstrRuleName);
    SysFreeString(bstrRuleDescription);
    SysFreeString(bstrRuleGroup);
    SysFreeString(bstrRuleApplication);
    SysFreeString(bstrRuleLPorts);

    // Release the INetFwRule object
    if (pFwRule != NULL)
    {
        pFwRule->Release();
    }

    // Release the INetFwRules object
    if (pFwRules != NULL)
    {
        pFwRules->Release();
    }

    // Release the INetFwPolicy2 object
    if (pNetFwPolicy2 != NULL)
    {
        pNetFwPolicy2->Release();
    }

    // Uninitialize COM.
   
    return 0;
}


// Instantiate INetFwPolicy2
HRESULT FirewallHandler::WFCOMInitialize(INetFwPolicy2** ppNetFwPolicy2)
{
    HRESULT hr = S_OK;

    hr = CoCreateInstance(
        __uuidof(NetFwPolicy2), 
        NULL, 
        CLSCTX_INPROC_SERVER, 
        __uuidof(INetFwPolicy2), 
        (void**)ppNetFwPolicy2);

    if (FAILED(hr))
    {
        std::cout << "CoCreateInstance for INetFwPolicy2 failed: 0x%08lx\n" << hr;
        goto Cleanup;        
    }

Cleanup:
    return hr;
}
