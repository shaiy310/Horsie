#ifndef	__REACTOR_H__
#define __REACTOR_H__

#include <Windows.h>
#include <cstdio>
#include <list>

class Reactor
{
public:
	Reactor();

	void Register(HANDLE handle, void(*callback)(HANDLE handle, PVOID param), PVOID PARAM);
	void Unregister(HANDLE handle);

	void Run();

private:
	std::list<HANDLE> handlers;
};

#endif