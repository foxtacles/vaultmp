#ifndef WINDOWGUI_H
#define WINDOWGUI_H

#include "vaultmp.hpp"
#include "Base.hpp"
#include "ReferenceTypes.hpp"
#include "GameFactory.hpp"

#include <unordered_set>

/**
 * \brief Represents a GUI frame window
 */

class Window : public Base
{
		friend class GameFactory;

	private:
		typedef std::unordered_map<RakNet::NetworkID, std::unordered_set<RakNet::NetworkID>> WindowChilds;

		static WindowChilds childs;

		RakNet::NetworkID parent;
		std::string label;
		std::tuple<float, float, float, float> pos, size;
		bool locked, visible;
		std::string text;

		void initialize();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

	protected:
		Window();
		Window(double posX, double posY, double sizeX, double sizeY, bool visible, bool locked, const char* text);
		Window(const pPacket& packet);
		Window(pPacket&& packet) : Window(packet) {};

	public:
		virtual ~Window() noexcept;

		static constexpr const char* GUI_MAIN_LABEL = "Main Window";
		static constexpr const char* GUI_MAIN_TEXT = "Chat Box";
		static constexpr std::tuple<float, float> GUI_MAIN_POS{0.01f, 0.01f};
		static constexpr std::tuple<float, float> GUI_MAIN_SIZE{0.35f, 0.3f};

		static const WindowChilds& GetChilds() { return childs; }
		static void CollectChilds(RakNet::NetworkID root, std::vector<RakNet::NetworkID>& dest);

		void SetParentWindow(Window* parent);
		void SetLabel(const std::string& label) { this->label = label; }
		bool SetPos(float X, float Y, float offset_X, float offset_Y);
		bool SetSize(float X, float Y, float offset_X, float offset_Y);
		void SetLocked(bool locked) { this->locked = locked; }
		void SetVisible(bool visible) { this->visible = visible; }
		void SetText(const std::string& text) { this->text = text; }

		RakNet::NetworkID GetParentWindow() const { return parent; }
		const std::string& GetLabel() const { return label; }
		const std::tuple<float, float, float, float>& GetPos() const { return pos; }
		const std::tuple<float, float, float, float>& GetSize() const { return size; }
		bool GetLocked() const { return locked; }
		bool GetVisible() const { return visible; }
		const std::string& GetText() const { return text; }

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

GF_TYPE_WRAPPER(Window, Base, ID_WINDOW, ALL_WINDOWS)

PF_MAKE(ID_WINDOW_NEW, pGeneratorReferenceExtend, RakNet::NetworkID, std::string, std::tuple<float, float, float, float>, std::tuple<float, float, float, float>, bool, bool, std::string)
template<>
inline const typename pTypesMap<pTypes::ID_WINDOW_NEW>::type* PacketFactory::Cast_<pTypes::ID_WINDOW_NEW>::Cast(const pPacket* packet) {
	pTypes type = packet->type();
	return (
		type == pTypes::ID_WINDOW_NEW ||
		type == pTypes::ID_BUTTON_NEW ||
		type == pTypes::ID_TEXT_NEW ||
		type == pTypes::ID_EDIT_NEW ||
		type == pTypes::ID_CHECKBOX_NEW
	) ? static_cast<const typename pTypesMap<pTypes::ID_WINDOW_NEW>::type*>(packet) : nullptr;
}
PF_MAKE_E(ID_WINDOW_REMOVE, pGeneratorReference)
PF_MAKE(ID_UPDATE_WPOS, pGeneratorReference, std::tuple<float, float, float, float>)
PF_MAKE(ID_UPDATE_WSIZE, pGeneratorReference, std::tuple<float, float, float, float>)
PF_MAKE(ID_UPDATE_WLOCKED, pGeneratorReference, bool)
PF_MAKE(ID_UPDATE_WVISIBLE, pGeneratorReference, bool)
PF_MAKE(ID_UPDATE_WTEXT, pGeneratorReference, std::string)
PF_MAKE_E(ID_UPDATE_WCLICK, pGeneratorReference)
PF_MAKE(ID_UPDATE_WMODE, pGeneratorDefault, bool)
PF_MAKE_E(ID_UPDATE_WRETURN, pGeneratorReference)

#endif
