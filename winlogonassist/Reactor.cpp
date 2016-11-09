#include "Reactor.h"

Reactor::Reactor()
{

}

void Reactor::Unregister(HANDLE handle)
{
	this->handlers.remove(handle);
}

void Reactor::Run()
{

}