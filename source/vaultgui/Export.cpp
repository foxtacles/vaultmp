#include <windows.h>
#include <WinBase.h>
#include "myIDirect3DDevice9.h"
#include <queue>

#include "global.h"

#include "GameData.h"

#include "ChatUtils.h"

extern myIDirect3DDevice9* gl_pmyIDirect3DDevice9;

CRITICAL_SECTION cs_GetQueue;


extern "C"
{
	__declspec(dllexport) void Chatbox_AddToChat(char* c)
	{
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
	}

	__declspec(dllexport) char* Chatbox_GetQueue()
	{
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

		return buffer;
	}

	__declspec(dllexport) void Chatbox_AddPlayerName(string name,int* x,int* y,int* z)
	{
		PlayerScreenName tmp;
		tmp.name=name;
		tmp.posX=x;
		tmp.posY=y;
		tmp.posZ=z;
		GameData::playersScreenName.push_back(tmp);
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
		playersData=(remotePlayers*)p;
	}

	__declspec(dllexport) void HideChatbox(bool hide)
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
	}
}