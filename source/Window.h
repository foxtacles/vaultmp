#ifndef WINDOWGUI_H
#define WINDOWGUI_H

#include <string>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "vaultmp.h"
#include "RakNet.h"
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
		std::pair<double, double> pos, size;
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

		static const WindowChilds& GetChilds() { return childs; }
		static void CollectChilds(RakNet::NetworkID root, std::vector<RakNet::NetworkID>& dest);

		void SetParentWindow(Window* parent);
		void SetLabel(const std::string& label) { this->label = label; }
		bool SetPos(double X, double Y);
		bool SetSize(double X, double Y);
		void SetLocked(bool locked) { this->locked = locked; }
		void SetVisible(bool visible) { this->visible = visible; }
		void SetText(const std::string& text) { this->text = text; }

		RakNet::NetworkID GetParentWindow() const { return parent; }
		const std::string& GetLabel() const { return label; }
		const std::pair<double, double>& GetPos() const { return pos; }
		const std::pair<double, double>& GetSize() const { return size; }
		bool GetLocked() const { return locked; }
		bool GetVisible() const { return visible; }
		const std::string& GetText() const { return text; }

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

#endif
