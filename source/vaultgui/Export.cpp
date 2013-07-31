#include <windows.h>
#include <WinBase.h>
#include "myIDirect3DDevice9.h"
#include <queue>

#include "global.h"

#include "GameData.h"

#include "ChatUtils.h"

#include "debug.h"

#include "Export.h"

#include "CEGUI/cegui/include/CEGUIUDim.h"

extern myIDirect3DDevice9* gl_pmyIDirect3DDevice9;

CRITICAL_SECTION cs_GetQueue;

void (*callbackPTR_OnClick)(char* name)=0;
void (*callbackPTR_OnTextChange)(char* name,char* text)=0;
void (*callbackPTR_OnListboxSelectionChange)(char* name,char** text)=0;

bool GUI_MouseClickCallback(const CEGUI::EventArgs& e)
{
	//char buf[120];
	const CEGUI::MouseEventArgs& we = static_cast<const CEGUI::MouseEventArgs&>(e);

	//sprintf(buf,"CLICK ON %s",we.window->getName().c_str());

	//Chatbox_AddToChat(buf);

	if(callbackPTR_OnClick)
	{
		callbackPTR_OnClick((char*)we.window->getName().c_str());
	}

	return true;
}

bool GUI_TextChanged(const CEGUI::EventArgs& e)
{
	//char buf[120];
	const CEGUI::WindowEventArgs& we = static_cast<const CEGUI::WindowEventArgs&>(e);
	//sprintf(buf,"TEXT CHANGED ON %s",we.window->getName().c_str());
	//Chatbox_AddToChat(buf);

	if(callbackPTR_OnTextChange)
	{
		if(gData.sendClickCallbacks)
		{
			callbackPTR_OnTextChange((char*)we.window->getName().c_str(),(char*)we.window->getText().c_str());
		}
	}

	return true;
}

bool GUI_ListboxSelectionChange(const CEGUI::EventArgs& e)
{
	const CEGUI::WindowEventArgs& we = static_cast<const CEGUI::WindowEventArgs&>(e);



	vector<string>* arr=GUI_Listbox_GetSelectedItems((char*)we.window->getName().c_str());

	char **tmp=new char*[arr->size()+1];
	tmp[arr->size()]=0;

	for(int i=0;i<arr->size();i++)
	{
		tmp[i]=(char*)((*arr)[i].c_str());
	}


	if(callbackPTR_OnListboxSelectionChange)
	{
		callbackPTR_OnListboxSelectionChange((char*)we.window->getName().c_str(),tmp);
	}

	delete[] tmp;

	return true;
}


extern "C"
{
	__declspec(dllexport) void Chatbox_AddToChat(char* c)
	{
		Debug::FunctionCall("Chatbox_AddToChat");
		string inp=c;
		ParseChatText(inp);
		CEGUI::FormattedListboxTextItem* itm=new CEGUI::FormattedListboxTextItem(inp.c_str(),CEGUI::HTF_WORDWRAP_LEFT_ALIGNED);
		itm->setTextColours(0xFFFFFFFF);
		CEGUI::Listbox* listb=((CEGUI::Listbox*)CEGUI::WindowManager::getSingleton().getWindow("List Box"));
		listb->addItem(itm);
		while(listb->getItemCount() > D_MAX_CHAT_ENTRIES) {
			listb->removeItem(listb->getListboxItemFromIndex(0));
		}
		listb->ensureItemIsVisible(itm);

		gData.lastChatTextTick=GetTickCount();
		Debug::FunctionReturn("Chatbox_AddToChat");
	}

	__declspec(dllexport) char* Chatbox_GetQueue()
	{
		Debug::FunctionCall("Chatbox_GetQueue");
		static char buffer[300]={0};
		string tmp="";

		buffer[0]=0;

		EnterCriticalSection(&cs_GetQueue);
		if(chatQueue.size())
		{
			tmp=chatQueue[0];
			strcpy(buffer,tmp.c_str());
			chatQueue.pop_front();
		}
		//TODO return string from queue

		LeaveCriticalSection(&cs_GetQueue);

		Debug::FunctionReturn("Chatbox_GetQueue");

		return buffer;
	}

	__declspec(dllexport) void Chatbox_AddPlayerName(string name,int* x,int* y,int* z)
	{
		Debug::FunctionCall("Chatbox_AddPlayerName");
		PlayerScreenName tmp;
		tmp.name=name;
		tmp.posX=x;
		tmp.posY=y;
		tmp.posZ=z;
		GameData::playersScreenName.push_back(tmp);
		Debug::FunctionReturn("Chatbox_AddPlayerName");
	}

	__declspec(dllexport) void Chatbox_DeletePlayerName(string name)
	{
		for(unsigned int i=0;i<GameData::playersScreenName.size();i++)
		{
			if(GameData::playersScreenName[i].name==name)
			{
				GameData::playersScreenName.erase(GameData::playersScreenName.begin()+i);
				i--;
			}
		}
	}

	__declspec(dllexport) void SetPlayersDataPointer(void* p)
	{
		Debug::FunctionCall("SetPlayersDataPointer");
		playersData=(remotePlayers*)p;
		Debug::FunctionReturn("SetPlayersDataPointer");
	}

	/*__declspec(dllexport) void HideChatbox(bool hide)
	{
		gData.hideChatbox=hide;
	}

	__declspec(dllexport) void LockChatbox(bool lock)
	{
		gData.lockChatbox=lock;
	}

	__declspec(dllexport) void SetChatboxPos(float x,float y)
	{
		CEGUI::FrameWindow *w = ((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"));
		w->setPosition(CEGUI::UVector2(cegui_reldim(x), cegui_reldim(y)));
	}

	__declspec(dllexport) void SetChatboxSize(float x,float y)
	{
		CEGUI::FrameWindow *w = ((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"));
		w->setSize(CEGUI::UVector2(cegui_reldim(x), cegui_reldim(y)));
	}*/




	__declspec(dllexport) void GUI_CreateFrameWindow(char *name)
	{
		CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
		CEGUI::DefaultWindow* root = (CEGUI::DefaultWindow*)winMgr.getWindow("Root");
		CEGUI::FrameWindow * wnd = (CEGUI::FrameWindow*)winMgr.createWindow("TaharezLook/FrameWindow", name);
		root->addChildWindow(wnd);
	}

	__declspec(dllexport) void GUI_AddStaticText(char* parent,char* name)
	{
		CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();

		CEGUI::FrameWindow *w = ((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow(parent));
		CEGUI::DefaultWindow* wnd=(CEGUI::DefaultWindow*)winMgr.createWindow("TaharezLook/StaticText", name);

		w->addChildWindow(wnd);

		wnd->subscribeEvent(CEGUI::Window::EventMouseClick,GUI_MouseClickCallback);
	}

	__declspec(dllexport) void GUI_AddTextbox(char* parent,char* name)
	{
		CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();

		CEGUI::FrameWindow *w = ((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow(parent));

		CEGUI::Editbox* wnd=(CEGUI::Editbox*)winMgr.createWindow("TaharezLook/Editbox", name);

		w->addChildWindow(wnd);

		wnd->subscribeEvent(CEGUI::Editbox::EventTextChanged,GUI_TextChanged);
	}

	__declspec(dllexport) void GUI_Textbox_SetMaxLength(char* name,int maxL)
	{
		if(CEGUI::WindowManager::getSingleton().getWindow(name)->getType().compare("TaharezLook/Editbox")==0)
		{
			CEGUI::Editbox *w = ((CEGUI::Editbox*)CEGUI::WindowManager::getSingleton().getWindow(name));
			w->setMaxTextLength(maxL);
		}
	}

	__declspec(dllexport) void GUI_AddButton(char* parent,char* name)
	{
		CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();

		CEGUI::FrameWindow *w = ((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow(parent));

		CEGUI::Editbox* wnd=(CEGUI::Editbox*)winMgr.createWindow("TaharezLook/Button",name);

		w->addChildWindow(wnd);

		wnd->subscribeEvent(CEGUI::PushButton::EventClicked,GUI_MouseClickCallback);
	}

	__declspec(dllexport) void GUI_SetPosition(char* name,float x,float y,float xOffset,float yOffset)
	{
		CEGUI::DefaultWindow *w = ((CEGUI::DefaultWindow*)CEGUI::WindowManager::getSingleton().getWindow(name));
		w->setPosition(CEGUI::UVector2(CEGUI::UDim(x,xOffset), CEGUI::UDim(y,yOffset)));
	}

	__declspec(dllexport) void GUI_SetSize(char* name,float x,float y,float xOffset,float yOffset)
	{
		CEGUI::DefaultWindow *w = ((CEGUI::DefaultWindow*)CEGUI::WindowManager::getSingleton().getWindow(name));
		w->setSize(CEGUI::UVector2(CEGUI::UDim(x,xOffset), CEGUI::UDim(y,yOffset)));
	}

	__declspec(dllexport) void GUI_SetText(char* name,char* txt)
	{
		CEGUI::DefaultWindow *w = ((CEGUI::DefaultWindow*)CEGUI::WindowManager::getSingleton().getWindow(name));

		gData.sendClickCallbacks=false;
		w->setText(txt);
		gData.sendClickCallbacks=true;
	}

	__declspec(dllexport) void GUI_RemoveWindow(char* name)
	{
		CEGUI::WindowManager::getSingleton().destroyWindow(name);
	}

	__declspec(dllexport) void GUI_SetClickCallback(void (*pt)(char* name))
	{
		callbackPTR_OnClick=pt;
	}

	__declspec(dllexport) void GUI_SetTextChangedCallback(void (*pt)(char* name,char* t))
	{
		callbackPTR_OnTextChange=pt;
	}
	
	__declspec(dllexport) void GUI_SetListboxSelectionChangedCallback(void (*pt)(char* name,char** text))
	{
		callbackPTR_OnListboxSelectionChange=pt;
	}

	__declspec(dllexport) void GUI_ForceGUI(bool inGui)
	{
		gData.chatting=inGui;

		if(inGui)
			CEGUI::MouseCursor::getSingleton().show();
		else
			CEGUI::MouseCursor::getSingleton().hide();
	}

	__declspec(dllexport) void GUI_SetVisible(char* name,bool visible)
	{
		CEGUI::DefaultWindow *w = ((CEGUI::DefaultWindow*)CEGUI::WindowManager::getSingleton().getWindow(name));
		w->setVisible(visible);
	}

	__declspec(dllexport) void GUI_AllowDrag(char* name,bool allow)
	{
		if(CEGUI::WindowManager::getSingleton().getWindow(name)->getType().compare("TaharezLook/FrameWindow")==0)
		{
			CEGUI::FrameWindow *w = ((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow(name));
			w->setDragMovingEnabled(allow);
		}
	}

	__declspec(dllexport) void GUI_AddListbox(char* parent,char* name)
	{
		CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();

		CEGUI::FrameWindow *w = ((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow(parent));
		CEGUI::Listbox* wnd=(CEGUI::Listbox*)winMgr.createWindow("TaharezLook/Listbox", name);

		wnd->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,GUI_ListboxSelectionChange);

		w->addChildWindow(wnd);
	}

	__declspec(dllexport) void GUI_Listbox_AddItem(char* name,char* t)
	{
		CEGUI::Listbox *w = ((CEGUI::Listbox*)CEGUI::WindowManager::getSingleton().getWindow(name));

		CEGUI::FormattedListboxTextItem* itm=new CEGUI::FormattedListboxTextItem(t,CEGUI::HTF_WORDWRAP_LEFT_ALIGNED);
		itm->setTextColours(0xFFFFFFFF);
		itm->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush");

		w->addItem(itm);

		/*
		Debug::FunctionCall("Chatbox_AddToChat");
		string inp=c;
		ParseChatText(inp);
		CEGUI::FormattedListboxTextItem* itm=new CEGUI::FormattedListboxTextItem(inp.c_str(),CEGUI::HTF_WORDWRAP_LEFT_ALIGNED);
		itm->setTextColours(0xFFFFFFFF);
		CEGUI::Listbox* listb=((CEGUI::Listbox*)CEGUI::WindowManager::getSingleton().getWindow("List Box"));
		listb->addItem(itm);
		while(listb->getItemCount() > D_MAX_CHAT_ENTRIES) {
			listb->removeItem(listb->getListboxItemFromIndex(0));
		}
		listb->ensureItemIsVisible(itm);

		gData.lastChatTextTick=GetTickCount();
		Debug::FunctionReturn("Chatbox_AddToChat");
		*/
	}

	__declspec(dllexport) void GUI_Listbox_RemoveItem(char* name,char* t)
	{
		CEGUI::Listbox *w = ((CEGUI::Listbox*)CEGUI::WindowManager::getSingleton().getWindow(name));

		w->removeItem(w->findItemWithText(t,NULL));
	}

	__declspec(dllexport) void GUI_Listbox_EnableMultiSelect(char* name,bool e)
	{
		CEGUI::Listbox *w = ((CEGUI::Listbox*)CEGUI::WindowManager::getSingleton().getWindow(name));

		w->setMultiselectEnabled(e);
	}

	__declspec(dllexport) vector<string>* GUI_Listbox_GetSelectedItems(char* name)
	{
		static vector<string> str;
		CEGUI::ListboxItem *tmp;

		tmp=0;

		str.clear();

		CEGUI::Listbox *w = ((CEGUI::Listbox*)CEGUI::WindowManager::getSingleton().getWindow(name));

		tmp=w->getFirstSelectedItem();

		while(tmp)
		{
			str.push_back(tmp->getText().c_str());

			tmp=w->getNextSelected(tmp);
		}

		return &str;
	}


}