#include "Window.h"
#include "PacketFactory.h"
#include "GameFactory.h"

using namespace std;
using namespace RakNet;

Window::Window() : Reference(0x00000000, 0x00000000)
{
	initialize();
}

Window::Window(const pDefault* packet) : Reference(0x00000000, 0x00000000)
{
	initialize();

	NetworkID id;

	PacketFactory::Access<pTypes::ID_WINDOW_NEW>(packet, id, parent, label, pos, size, locked, visible);

	this->SetNetworkID(id);
}

Window::Window(pPacket&& packet) : Window(packet.get())
{

}

Window::~Window()
{

}

void Window::initialize()
{
	parent = 0;
	locked = false;
	visible = true;
}

bool Window::SetPos(double x, double y)
{
	if (x >= 0.0 && x <= 1.0 && y >= 0.0 && y <= 1.0)
	{
		pos.first = x;
		pos.second = y;
		return true;
	}

	return false;
}

bool Window::SetSize(double x, double y)
{
	if (x >= 0.0 && x <= 1.0 && y >= 0.0 && y <= 1.0)
	{
		size.first = x;
		size.second = y;
		return true;
	}

	return false;
}

pPacket Window::toPacket() const
{
	pPacket packet = PacketFactory::Create<pTypes::ID_WINDOW_NEW>(const_cast<Window*>(this)->GetNetworkID(), parent, label, pos, size, locked, visible);

	return packet;
}
