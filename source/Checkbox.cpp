#include "Checkbox.hpp"

using namespace std;
using namespace RakNet;

Checkbox::Checkbox() : Window(), selected(DEFAULT_SELECTED)
{
	initialize();
}

Checkbox::Checkbox(const pPacket& packet) : Window(PacketFactory::Pop<pPacket>(packet))
{
	initialize();

	PacketFactory::Access<pTypes::ID_CHECKBOX_NEW>(packet, selected);
}

Checkbox::~Checkbox() noexcept {}

void Checkbox::initialize()
{

}

pPacket Checkbox::toPacket() const
{
	pPacket pWindowNew = Window::toPacket();
	pPacket packet = PacketFactory::Create<pTypes::ID_CHECKBOX_NEW>(pWindowNew, selected);

	return packet;
}
