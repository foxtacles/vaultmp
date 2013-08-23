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
		std::tuple<double, double, double, double> pos, size;
		bool locked, visible;
		std::string text;

		void initialize();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

	protected:
		Window();
		Window(const pDefault* packet);
		Window(pPacket&& packet) : Window(packet.get()) {};

	public:
		virtual ~Window() noexcept;

		static constexpr const char* GUI_MAIN_LABEL = "Main Window";
		static constexpr const char* GUI_MAIN_TEXT = "Chat Box";
		static constexpr std::tuple<double, double> GUI_MAIN_POS{0.01, 0.01};
		static constexpr std::tuple<double, double> GUI_MAIN_SIZE{0.35, 0.3};

		static const WindowChilds& GetChilds() { return childs; }
		static void CollectChilds(RakNet::NetworkID root, std::vector<RakNet::NetworkID>& dest);

		void SetParentWindow(Window* parent);
		void SetLabel(const std::string& label) { this->label = label; }
		bool SetPos(double X, double Y, double offset_X, double offset_Y);
		bool SetSize(double X, double Y, double offset_X, double offset_Y);
		void SetLocked(bool locked) { this->locked = locked; }
		void SetVisible(bool visible) { this->visible = visible; }
		void SetText(const std::string& text) { this->text = text; }

		RakNet::NetworkID GetParentWindow() const { return parent; }
		const std::string& GetLabel() const { return label; }
		const std::tuple<double, double, double, double>& GetPos() const { return pos; }
		const std::tuple<double, double, double, double>& GetSize() const { return size; }
		bool GetLocked() const { return locked; }
		bool GetVisible() const { return visible; }
		const std::string& GetText() const { return text; }

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

GF_TYPE_WRAPPER(Window, Base, ID_WINDOW, ALL_WINDOWS)

template<> struct pTypesMap<pTypes::ID_WINDOW_NEW> { typedef pGeneratorReferenceExtend<pTypes::ID_WINDOW_NEW, RakNet::NetworkID, std::string, std::tuple<double, double, double, double>, std::tuple<double, double, double, double>, bool, bool, std::string> type; };
template<>
inline const typename pTypesMap<pTypes::ID_WINDOW_NEW>::type* PacketFactory::Cast_<pTypes::ID_WINDOW_NEW>::Cast(const pDefault* packet) {
	pTypes type = packet->type();
	return (
		type == pTypes::ID_WINDOW_NEW ||
		type == pTypes::ID_BUTTON_NEW ||
		type == pTypes::ID_TEXT_NEW ||
		type == pTypes::ID_EDIT_NEW ||
		type == pTypes::ID_CHECKBOX_NEW
	) ? static_cast<const typename pTypesMap<pTypes::ID_WINDOW_NEW>::type*>(packet) : nullptr;
}
template<> struct pTypesMap<pTypes::ID_WINDOW_REMOVE> { typedef pGeneratorReference<pTypes::ID_WINDOW_REMOVE> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_WPOS> { typedef pGeneratorReference<pTypes::ID_UPDATE_WPOS, std::tuple<double, double, double, double>> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_WSIZE> { typedef pGeneratorReference<pTypes::ID_UPDATE_WSIZE, std::tuple<double, double, double, double>> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_WLOCKED> { typedef pGeneratorReference<pTypes::ID_UPDATE_WLOCKED, bool> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_WVISIBLE> { typedef pGeneratorReference<pTypes::ID_UPDATE_WVISIBLE, bool> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_WTEXT> { typedef pGeneratorReference<pTypes::ID_UPDATE_WTEXT, std::string> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_WCLICK> { typedef pGeneratorReference<pTypes::ID_UPDATE_WCLICK> type; };
template<> struct pTypesMap<pTypes::ID_UPDATE_WMODE> { typedef pGeneratorDefault<pTypes::ID_UPDATE_WMODE, bool> type; };

#endif
