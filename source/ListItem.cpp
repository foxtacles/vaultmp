#include "ListItem.hpp"

using namespace std;
using namespace RakNet;

ListItem::ListItem() : Base(), list(0), selected(DEFAULT_SELECTED)
{
	initialize();
}

ListItem::ListItem(const pPacket& packet) : Base(PacketFactory::Pop<pPacket>(packet))
{
	initialize();

	PacketFactory::Access<pTypes::ID_LISTITEM_NEW>(packet, list, text, selected);
}

ListItem::~ListItem() noexcept {}

void ListItem::initialize()
{

}

pPacket ListItem::toPacket() const
{
	pPacket pBaseNew = Base::toPacket();
	pPacket packet = PacketFactory::Create<pTypes::ID_LISTITEM_NEW>(pBaseNew, list, text, selected);

	return packet;
}
