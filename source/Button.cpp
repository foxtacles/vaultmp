#include "Button.hpp"

using namespace std;
using namespace RakNet;

Button::Button() : Window()
{
	initialize();
}

Button::Button(const pPacket& packet) : Window(PacketFactory::Pop<pPacket>(packet))
{
	initialize();

	PacketFactory::Access<pTypes::ID_BUTTON_NEW>(packet);
}

Button::~Button() noexcept {}

void Button::initialize()
{

}

pPacket Button::toPacket() const
{
	pPacket pWindowNew = Window::toPacket();
	pPacket packet = PacketFactory::Create<pTypes::ID_BUTTON_NEW>(pWindowNew);

	return packet;
}
