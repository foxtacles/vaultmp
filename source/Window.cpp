#include "Window.hpp"

using namespace std;
using namespace RakNet;

Window::WindowChilds Window::childs;
constexpr const char* Window::GUI_MAIN_TEXT;
constexpr tuple<float, float> Window::GUI_MAIN_SIZE;
constexpr tuple<float, float> Window::GUI_MAIN_POS;

Window::Window() : Base(), parent(0), locked(false), visible(true)
{
	initialize();
}

Window::Window(double posX, double posY, double sizeX, double sizeY, bool visible, bool locked, const char* text) : Base(), parent(0), locked(locked), visible(visible), text(text)
{
	initialize();

	if (!SetPos(posX, posY, 0.0f, 0.0f))
		throw VaultException("Invalid position values");

	if (!SetSize(sizeX, sizeY, 0.0f, 0.0f))
		throw VaultException("Invalid size values");
}

Window::Window(const pPacket& packet) : Base(PacketFactory::Pop<pPacket>(packet))
{
	initialize();

	PacketFactory::Access<pTypes::ID_WINDOW_NEW>(packet, parent, label, pos, size, locked, visible, text);

	if (parent)
		childs[parent].emplace(this->GetNetworkID());
}

Window::~Window() noexcept
{
	if (parent)
		childs[parent].erase(this->GetNetworkID());
}

void Window::initialize()
{

}

void Window::CollectChilds(NetworkID root, vector<NetworkID>& dest)
{
	dest.emplace_back(root);

	for (auto child : childs[root])
		CollectChilds(child, dest);
}

void Window::SetParentWindow(Window* parent)
{
	NetworkID id = this->GetNetworkID();

	if (this->parent)
		childs[this->parent].erase(id);

	if (parent)
	{
		this->parent = parent->GetNetworkID();
		childs[this->parent].emplace(id);
	}
	else
		this->parent = 0;
}

bool Window::SetPos(float X, float Y, float offset_X, float offset_Y)
{
	if (X >= 0.0f && X <= 1.0f && Y >= 0.0f && Y <= 1.0f)
	{
		pos = decltype(pos){X, Y, offset_X, offset_Y};
		return true;
	}

	return false;
}

bool Window::SetSize(float X, float Y, float offset_X, float offset_Y)
{
	if (X >= 0.0f && X <= 1.0f && Y >= 0.0f && Y <= 1.0f)
	{
		size = decltype(size){X, Y, offset_X, offset_Y};
		return true;
	}

	return false;
}

pPacket Window::toPacket() const
{
	pPacket pBaseNew = Base::toPacket();

	pPacket packet = PacketFactory::Create<pTypes::ID_WINDOW_NEW>(pBaseNew, parent, label, pos, size, locked, visible, text);

	return packet;
}
