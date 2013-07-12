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

	PacketFactory::Access<pTypes::ID_WINDOW_NEW>(packet, id, parent, label, pos, size, locked, visible, text);

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

bool Window::SetPos(double X, double Y)
{
	if (X >= 0.0 && X <= 1.0 && Y >= 0.0 && Y <= 1.0)
	{
		pos.first = X;
		pos.second = Y;
		return true;
	}

	return false;
}

bool Window::SetSize(double X, double Y)
{
	if (X >= 0.0 && X <= 1.0 && Y >= 0.0 && Y <= 1.0)
	{
		size.first = X;
		size.second = Y;
		return true;
	}

	return false;
}

pPacket Window::toPacket() const
{
	pPacket packet = PacketFactory::Create<pTypes::ID_WINDOW_NEW>(const_cast<Window*>(this)->GetNetworkID(), parent, label, pos, size, locked, visible, text);

	return packet;
}
