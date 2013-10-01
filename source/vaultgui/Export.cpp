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
#include "GUIHelper.h"

extern myIDirect3DDevice9* gl_pmyIDirect3DDevice9;

CRITICAL_SECTION cs_GetQueue;



bool GUI_MouseClickCallback(const CEGUI::EventArgs& e)
{
	//char buf[120];
	const CEGUI::MouseEventArgs& we = static_cast<const CEGUI::MouseEventArgs&>(e);

	//sprintf(buf,"CLICK ON %s",we.window->getName().c_str());

	//Chatbox_AddToChat(buf);

	if(GUIHelper::callbackPTR_OnClick)
	{
		GUIHelper::callbackPTR_OnClick((char*)we.window->getName().c_str());
	}

	return true;
}

bool GUI_TextChanged(const CEGUI::EventArgs& e)
{
	//char buf[120];
	const CEGUI::WindowEventArgs& we = static_cast<const CEGUI::WindowEventArgs&>(e);
	//sprintf(buf,"TEXT CHANGED ON %s",we.window->getName().c_str());
	//Chatbox_AddToChat(buf);

	if(GUIHelper::callbackPTR_OnTextChange)
	{
		if(gData.sendClickCallbacks)
		{
			GUIHelper::callbackPTR_OnTextChange((char*)we.window->getName().c_str(),(char*)we.window->getText().c_str());
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


	if(GUIHelper::callbackPTR_OnListboxSelectionChange)
	{
		if(gData.sendListboxCallbacks)
			GUIHelper::callbackPTR_OnListboxSelectionChange((char*)we.window->getName().c_str(),tmp);
	}

	delete[] tmp;

	return true;
}

bool GUI_CheckboxChanged(const CEGUI::EventArgs& e)
{
	const CEGUI::WindowEventArgs& we = static_cast<const CEGUI::WindowEventArgs&>(e);

	if(we.window->getType().compare("TaharezLook/Checkbox")==0)
	{
		CEGUI::Checkbox* c=(CEGUI::Checkbox*)we.window;
		if(GUIHelper::callbackPTR_OnCheckboxChange)
		{
			if(gData.sendCheckboxCallbacks)
				GUIHelper::callbackPTR_OnCheckboxChange((char*)c->getName().c_str(),c->isSelected());
			
		}
	}

	if(we.window->getType().compare("TaharezLook/RadioButton")==0)
	{
		CEGUI::RadioButton* c=(CEGUI::RadioButton*)we.window;
		if(GUIHelper::callbackPTR_OnCheckboxChange)
		{
			if(gData.sendCheckboxCallbacks)
			{
				if(c->isSelected())	//Fire event only if selected radio button, not unseelected one
					GUIHelper::callbackPTR_OnCheckboxChange((char*)c->getName().c_str(),c->isSelected());
			}
		}
	}

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



	__declspec(dllexport) void GUI_CreateFrameWindow(char *name)
	{
		GUIHelper::newFramedWindow(name);
	}

	__declspec(dllexport) void GUI_AddStaticText(char* parent,char* name)
	{
		GUIHelper::newWindow(parent,name,"TaharezLook/StaticText")->subscribeEvent(CEGUI::Window::EventMouseClick,GUI_MouseClickCallback);
	}

	__declspec(dllexport) void GUI_StaticText_SetAlign(char* name,char* align)
	{
		if(GUIHelper::getWindow(name)->getType().compare("TaharezLook/StaticText")==0)
			GUIHelper::getWindow(name)->setProperty("HorzFormatting",align);
	}

	__declspec(dllexport) void GUI_AddTextbox(char* parent,char* name)
	{
		GUIHelper::newWindow(parent,name,"TaharezLook/Editbox")->subscribeEvent(CEGUI::Editbox::EventTextChanged,GUI_TextChanged);
	}

	__declspec(dllexport) void GUI_AddMultilineTextbox(char* parent,char* name)
	{
		GUIHelper::newWindow(parent,name,"TaharezLook/MultiLineEditbox")->subscribeEvent(CEGUI::Editbox::EventTextChanged,GUI_TextChanged);
	}

	__declspec(dllexport) void GUI_Textbox_SetMaxLength(char* name,int maxL)
	{
		if(GUIHelper::getWindow(name)->getType().compare("TaharezLook/Editbox")==0)
		{
			CEGUI::Editbox *w = ((CEGUI::Editbox*)GUIHelper::getWindow(name));
			w->setMaxTextLength(maxL);
		}
	}

	__declspec(dllexport) void GUI_Textbox_SetValidationString(char* name,char* val)
	{
		if(GUIHelper::getWindow(name)->getType().compare("TaharezLook/Editbox")==0)
		{
			CEGUI::Editbox *w = ((CEGUI::Editbox*)GUIHelper::getWindow(name));
			w->setValidationString(val);
		}
	}

	__declspec(dllexport) void GUI_AddButton(char* parent,char* name)
	{
		GUIHelper::newWindow(parent,name,"TaharezLook/Button")->subscribeEvent(CEGUI::PushButton::EventClicked,GUI_MouseClickCallback);
	}

	__declspec(dllexport) void GUI_SetPosition(char* name,float x,float y,float xOffset,float yOffset)
	{
		CEGUI::DefaultWindow *w = ((CEGUI::DefaultWindow*)GUIHelper::getWindow(name));
		w->setPosition(CEGUI::UVector2(CEGUI::UDim(x,xOffset), CEGUI::UDim(y,yOffset)));
	}

	__declspec(dllexport) void GUI_SetSize(char* name,float x,float y,float xOffset,float yOffset)
	{
		CEGUI::DefaultWindow *w = ((CEGUI::DefaultWindow*)GUIHelper::getWindow(name));
		w->setSize(CEGUI::UVector2(CEGUI::UDim(x,xOffset), CEGUI::UDim(y,yOffset)));
	}

	__declspec(dllexport) void GUI_SetText(char* name,char* txt)
	{
		CEGUI::DefaultWindow *w = ((CEGUI::DefaultWindow*)GUIHelper::getWindow(name));

		gData.sendClickCallbacks=false;
		w->setText(txt);
		gData.sendClickCallbacks=true;
	}

	__declspec(dllexport) void GUI_RemoveWindow(char* name)
	{
		GUIHelper::destroyWindow(name);
	}

	__declspec(dllexport) void GUI_SetClickCallback(void (*pt)(char* name))
	{
		GUIHelper::callbackPTR_OnClick=pt;
	}

	__declspec(dllexport) void GUI_SetTextChangedCallback(void (*pt)(char* name,char* t))
	{
		GUIHelper::callbackPTR_OnTextChange=pt;
	}
	
	__declspec(dllexport) void GUI_SetListboxSelectionChangedCallback(void (*pt)(char* name,char** text))
	{
		GUIHelper::callbackPTR_OnListboxSelectionChange=pt;
	}

	__declspec(dllexport) void GUI_SetCheckboxChangedCallback(void (*pt)(char* name,bool checked))
	{
		GUIHelper::callbackPTR_OnCheckboxChange=pt;
	}

	__declspec(dllexport) void GUI_SetReturnKeyDownCallback(void (*pt)(char* name))
	{
		GUIHelper::callbackPTR_OnReturnDown=pt;
	}

	__declspec(dllexport) void GUI_ForceGUI(bool inGui)
	{
		gData.chatting=inGui;

		if(inGui)
		{
			CEGUI::MouseCursor::getSingleton().show();
			gData.disableMouseInput=true;
			CEGUI::Editbox* edb = ((CEGUI::Editbox*)CEGUI::WindowManager::getSingleton().getWindow("Edit Box"));

			((CEGUI::Editbox*)CEGUI::WindowManager::getSingleton().getWindow("closeBTN"))->setAlpha(1);

			edb->activate();
		}
		else
		{
			CEGUI::MouseCursor::getSingleton().hide();
			((CEGUI::Editbox*)CEGUI::WindowManager::getSingleton().getWindow("closeBTN"))->setAlpha(0);
			gData.disableMouseInput=false;
		}
	}

	__declspec(dllexport) void GUI_SetVisible(char* name,bool visible)
	{
		CEGUI::DefaultWindow *w = ((CEGUI::DefaultWindow*)GUIHelper::getWindow(name));
		w->setVisible(visible);
	}

	__declspec(dllexport) void GUI_AllowDrag(char* name,bool allow)
	{
		if(GUIHelper::getWindow(name)->getType().compare("TaharezLook/FrameWindow")==0)
		{
			CEGUI::FrameWindow *w = ((CEGUI::FrameWindow*)GUIHelper::getWindow(name));
			w->setDragMovingEnabled(allow);
		}
	}

	__declspec(dllexport) void GUI_AddListbox(char* parent,char* name)
	{
		GUIHelper::newWindow(parent,name,"TaharezLook/Listbox")->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,GUI_ListboxSelectionChange);
	}

	__declspec(dllexport) void GUI_Listbox_AddItem(char* name,char* id,char* t)
	{
		CEGUI::Listbox *w = ((CEGUI::Listbox*)GUIHelper::getWindow(name));

		CEGUI::FormattedListboxTextItem* itm=new CEGUI::FormattedListboxTextItem(t,CEGUI::HTF_WORDWRAP_LEFT_ALIGNED);
		itm->setTextColours(0xFFFFFFFF);
		itm->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush");
		itm->setCustomID(id);

		w->addItem(itm);
	}

	__declspec(dllexport) void GUI_Listbox_RemoveItem(char* name,char* itemID)
	{
		CEGUI::Listbox *w = ((CEGUI::Listbox*)GUIHelper::getWindow(name));

		for(int i=0;i<w->getItemCount();i++)
		{
			CEGUI::FormattedListboxTextItem* itm=(CEGUI::FormattedListboxTextItem*)w->getListboxItemFromIndex(i);
			if(itm->getCustomID().compare(itemID)==0)
			{
				w->removeItem(itm);
				i--;
			}
		}
		
	}

	__declspec(dllexport) void GUI_Listbox_SetItemText(char* name,char* itemID,char* newText)
	{
		gData.sendListboxCallbacks=false;
		CEGUI::Listbox *w = ((CEGUI::Listbox*)GUIHelper::getWindow(name));

		for(int i=0;i<w->getItemCount();i++)
		{
			CEGUI::FormattedListboxTextItem* itm=(CEGUI::FormattedListboxTextItem*)w->getListboxItemFromIndex(i);
			if(itm->getCustomID().compare(itemID)==0)
			{
				CEGUI::String id=itm->getCustomID();

				CEGUI::FormattedListboxTextItem *n=new CEGUI::FormattedListboxTextItem(newText,CEGUI::HTF_WORDWRAP_LEFT_ALIGNED);
				n->setTextColours(0xFFFFFFFF);
				n->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush");
				n->setCustomID(id);

				w->insertItem(n,itm);

				if(itm->isSelected())
					w->setItemSelectState(n,true);

				
				w->removeItem(itm);
				break;
			}
		}
		gData.sendListboxCallbacks=true;
	}
	__declspec(dllexport) void GUI_Listbox_SetItemSelected(char* name,char* itemID,bool selected)
	{
		gData.sendListboxCallbacks=false;
		CEGUI::Listbox *w = ((CEGUI::Listbox*)GUIHelper::getWindow(name));

		for(int i=0;i<w->getItemCount();i++)
		{
			CEGUI::FormattedListboxTextItem* itm=(CEGUI::FormattedListboxTextItem*)w->getListboxItemFromIndex(i);
			if(itm->getCustomID().compare(itemID)==0)
			{
				w->setItemSelectState(itm,selected);
			}
		}
		gData.sendListboxCallbacks=true;
	}

	__declspec(dllexport) void GUI_Listbox_SetItemTextColor(char* name,char* itemID,unsigned int color)
	{
		CEGUI::Listbox *w = ((CEGUI::Listbox*)GUIHelper::getWindow(name));

		for(int i=0;i<w->getItemCount();i++)
		{
			CEGUI::FormattedListboxTextItem* itm=(CEGUI::FormattedListboxTextItem*)w->getListboxItemFromIndex(i);
			if(itm->getCustomID().compare(itemID)==0)
			{
				CEGUI::colour col;
				col.setARGB(color);
				itm->setTextColours(col);
			}
		}
	}

	__declspec(dllexport) void GUI_Listbox_EnableMultiSelect(char* name,bool e)
	{
		CEGUI::Listbox *w = ((CEGUI::Listbox*)GUIHelper::getWindow(name));

		w->setMultiselectEnabled(e);
	}

	__declspec(dllexport) vector<string>* GUI_Listbox_GetSelectedItems(char* name)
	{
		static vector<string> str;
		CEGUI::FormattedListboxTextItem *tmp;

		tmp=0;

		str.clear();

		CEGUI::Listbox *w = ((CEGUI::Listbox*)GUIHelper::getWindow(name));

		tmp=(CEGUI::FormattedListboxTextItem*)w->getFirstSelectedItem();

		while(tmp)
		{
			str.push_back(tmp->getCustomID().c_str());

			tmp=(CEGUI::FormattedListboxTextItem*)w->getNextSelected(tmp);
		}

		return &str;
	}

	__declspec(dllexport) void GUI_AddCheckbox(char* parent,char* name)
	{
		GUIHelper::newWindow(parent,name,"TaharezLook/Checkbox")->subscribeEvent(CEGUI::Checkbox::EventCheckStateChanged,GUI_CheckboxChanged);
	}

	__declspec(dllexport) void GUI_AddRadioButton(char* parent,char* name,unsigned long int groupID)
	{
		CEGUI::RadioButton* wnd=(CEGUI::RadioButton*)GUIHelper::newWindow(parent,name,"TaharezLook/RadioButton");
		wnd->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged,GUI_CheckboxChanged);
		wnd->setGroupID(groupID);
	}

	__declspec(dllexport) void GUI_SetChecked(char* name,bool checked)
	{
		if(GUIHelper::getWindow(name)->getType().compare("TaharezLook/Checkbox")==0)
		{
			CEGUI::Checkbox *w = ((CEGUI::Checkbox*)GUIHelper::getWindow(name));
			gData.sendCheckboxCallbacks=false;
			w->setSelected(checked);
			gData.sendCheckboxCallbacks=true;
		}
		
		if(GUIHelper::getWindow(name)->getType().compare("TaharezLook/RadioButton")==0)
		{
			CEGUI::RadioButton *w = ((CEGUI::RadioButton*)GUIHelper::getWindow(name));
			gData.sendCheckboxCallbacks=false;
			w->setSelected(checked);
			gData.sendCheckboxCallbacks=true;
		}
	}

	__declspec(dllexport) void GUI_Radio_SetGroupID(char* name,int gid)
	{
		if(GUIHelper::getWindow(name)->getType().compare("TaharezLook/RadioButton")==0)
		{
			CEGUI::RadioButton *w = ((CEGUI::RadioButton*)GUIHelper::getWindow(name));
			w->setGroupID(gid);
		}
	}

	__declspec(dllexport) void GUI_SetTextColour(char* name,char* colour)	//AARRGGBB
	{
		GUIHelper::setTextColour(GUIHelper::getWindow(name),colour);
	}
}