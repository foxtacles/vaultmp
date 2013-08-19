#include "GUIHelper.h"

namespace GUIHelper
{
	bool onMouseEnter(const CEGUI::EventArgs& e)
	{
		CEGUI::WindowEventArgs *ev=(CEGUI::WindowEventArgs*)&e;
		return true;
	}

	bool onMouseLeave(const CEGUI::EventArgs& e)
	{
		CEGUI::WindowEventArgs *ev=(CEGUI::WindowEventArgs*)&e;
		return true;
	}

	CEGUI::Window* newFramedWindow(string wname)
	{
		CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
		CEGUI::DefaultWindow* root = (CEGUI::DefaultWindow*)winMgr.getWindow("Root");
		CEGUI::FrameWindow * wnd = (CEGUI::FrameWindow*)winMgr.createWindow("TaharezLook/FrameWindow", wname);
		root->addChildWindow(wnd);

		return wnd;
	}

	CEGUI::Window* newWindow(string parent,string wname,string type)
	{
		CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();

		CEGUI::FrameWindow *w = ((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow(parent));
		CEGUI::Window* wnd=(CEGUI::Window*)winMgr.createWindow(type, wname);

		w->addChildWindow(wnd);

		//Exceptions for adding mouseenter/mouseleave listener?
		wnd->subscribeEvent(CEGUI::Window::EventMouseEnters,onMouseEnter);
		wnd->subscribeEvent(CEGUI::Window::EventMouseLeaves,onMouseLeave);

		/*if(wname.compare("st")==0)
		{
			
			wnd->setProperty("FrameEnabled","True");
			wnd->setProperty("BackgroundColours","tl:FF000000 tr:FF000000 bl:FF000000 br:FF000000");
			wnd->setProperty("BackgroundEnabled","True");
		}*/

		return wnd;
	}

	CEGUI::Window* getWindow(string name)
	{
		return CEGUI::WindowManager::getSingleton().getWindow(name);
	}

	void destroyWindow(string name)
	{
		CEGUI::WindowManager::getSingleton().destroyWindow(name);
	}

	void setTextColour(CEGUI::Window* w,string t)
	{
		w->setProperty("TextColours","tl:"+t+" tr:"+t+" bl:"+t+" br:"+t);
	}
};