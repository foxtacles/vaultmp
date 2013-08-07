#ifndef WINDOWGUI_H
#define WINDOWGUI_H

#include "vaultmp.h"
#include "Reference.h"

/**
 * \brief Represents a GUI frame window
 */

class Window : public Reference
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

		Window(const Window&);
		Window& operator=(const Window&);

	protected:
		Window();
		Window(const pDefault* packet);
		Window(pPacket&& packet);

	public:
		virtual ~Window();

#ifdef VAULTSERVER
		static const char* GUI_MAIN_LABEL;
		static const char* GUI_MAIN_TEXT;
		static const std::tuple<double, double, double, double> GUI_MAIN_POS;
		static const std::tuple<double, double, double, double> GUI_MAIN_SIZE;
#endif

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

#endif
