#include "RadioButton.hpp"

using namespace std;
using namespace RakNet;

RadioButton::RadioButton() : Window(), selected(DEFAULT_SELECTED), group(DEFAULT_GROUP)
{
	initialize();
}

RadioButton::RadioButton(const pPacket& packet) : Window(PacketFactory::Pop<pPacket>(packet))
{
	initialize();

	PacketFactory::Access<pTypes::ID_RADIOBUTTON_NEW>(packet, selected, group);
}

RadioButton::~RadioButton() noexcept {}

void RadioButton::initialize()
{

}

pPacket RadioButton::toPacket() const
{
	pPacket pWindowNew = Window::toPacket();
	pPacket packet = PacketFactory::Create<pTypes::ID_RADIOBUTTON_NEW>(pWindowNew, selected, group);

	return packet;
}
