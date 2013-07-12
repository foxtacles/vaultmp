#ifndef WINDOWGUI_H
#define WINDOWGUI_H

#include <string>
#include <utility>

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
		RakNet::NetworkID parent;
		std::string label;
		std::pair<double, double> pos, size;
		bool locked, visible;

		void initialize();

		Window(const Window&);
		Window& operator=(const Window&);

	protected:
		Window();
		Window(const pDefault* packet);
		Window(pPacket&& packet);

	public:
		virtual ~Window();

		void SetParentWindow(Window& parent) { this->parent = parent.GetNetworkID(); }
		void SetLabel(const std::string& label) { this->label = label; }
		bool SetPos(double x, double y);
		bool SetSize(double x, double y);
		void SetLocked(bool locked) { this->locked = locked; }
		void SetVisible(bool visible) { this->visible = visible; }

		RakNet::NetworkID GetParentWindow() const { return parent; }
		const std::string& GetLabel() const { return label; }
		const std::pair<double, double>& GetPos() const { return pos; }
		const std::pair<double, double>& GetSize() const { return size; }
		bool GetLocked() const { return locked; }
		bool GetVisible() const { return visible; }

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

#endif
