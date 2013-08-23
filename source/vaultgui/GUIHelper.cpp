#include "GUIHelper.h"

#include "Export.h"

namespace GUIHelper
{
	ChatboxHistory chatboxHistory;
	void chatboxHistory_Add(string s)
	{
		chatboxHistory.history.push_back(s);
		if(chatboxHistory.history.size()>=20)
			chatboxHistory.history.erase(chatboxHistory.history.begin());
		chatboxHistory.historyIndex=chatboxHistory.history.size();
	}

	string chatboxHistory_Up()
	{
		if(chatboxHistory.history.size()==0)
			return "";
		chatboxHistory.historyIndex--;
		if(chatboxHistory.historyIndex<0)
			chatboxHistory.historyIndex=0;
		return chatboxHistory.history[chatboxHistory.historyIndex];
	}

	string chatboxHistory_Down()
	{
		if(chatboxHistory.history.size()==0)
			return "";
		chatboxHistory.historyIndex++;
		if(chatboxHistory.historyIndex>=chatboxHistory.history.size())
		{
			chatboxHistory.historyIndex=chatboxHistory.history.size()-1;
		}
		return chatboxHistory.history[chatboxHistory.historyIndex];
	}

	

	/*
	*********************Handlers********************************
	*/
	void (*callbackPTR_OnClick)(char* name)=0;
	void (*callbackPTR_OnTextChange)(char* name,char* text)=0;
	void (*callbackPTR_OnListboxSelectionChange)(char* name,char** text)=0;
	void (*callbackPTR_OnCheckboxChange)(char* name,bool checked)=0;
	void (*callbackPTR_OnReturnDown)(char* name)=0;
	/*
	*********************Events********************************
	*/
	bool onMouseEnter(const CEGUI::EventArgs& e)
	{
		CEGUI::WindowEventArgs *ev=(CEGUI::WindowEventArgs*)&e;
		return false;
	}

	bool onMouseLeave(const CEGUI::EventArgs& e)
	{
		CEGUI::WindowEventArgs *ev=(CEGUI::WindowEventArgs*)&e;
		return false;
	}
	
	bool onKeyDown(const CEGUI::EventArgs& e)
	{
		CEGUI::KeyEventArgs *ev=(CEGUI::KeyEventArgs*)&e;
		if(ev->scancode==CEGUI::Key::Scan::Return)
		{
			if(callbackPTR_OnReturnDown)
			{
				callbackPTR_OnReturnDown((char*)ev->window->getName().c_str());
			}
		}
		if(ev->window->getName().compare("Edit Box")==0)
		{
			if(ev->scancode==CEGUI::Key::Scan::ArrowUp)
			{
				ev->window->setText(chatboxHistory_Up());
			}
			else if(ev->scancode==CEGUI::Key::Scan::ArrowDown)
			{
				ev->window->setText(chatboxHistory_Down());
			}
		}
		return false;
	}

	bool onKeyUp(const CEGUI::EventArgs& e)
	{
		CEGUI::WindowEventArgs *ev=(CEGUI::WindowEventArgs*)&e;
		return false;
	}

	/*
	**********************************************************
	*/

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

		wnd->subscribeEvent(CEGUI::Window::EventKeyDown,onKeyDown);
		wnd->subscribeEvent(CEGUI::Window::EventKeyUp,onKeyUp);

		/*if(wname.compare("st")==0)
		{
			wnd->setProperty("FrameColours","tl:FF111111 tr:FF111111 bl:FF111111 br:FF111111");
			//wnd->setProperty("FrameEnabled","True");
			wnd->setProperty("BackgroundColours","tl:FF111111 tr:FF111111 bl:FF111111 br:FF111111");
			//wnd->setProperty("BackgroundEnabled","True");
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