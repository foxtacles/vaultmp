#include "CEGUI\cegui\include\CEGUI.h"

#include "common.h"

namespace GUIHelper
{
	/*Events callback*/
	bool onKeyDown(const CEGUI::EventArgs& e);
	bool onKeyUp(const CEGUI::EventArgs& e);
	/**/
	struct ChatboxHistory
	{
		vector<string> history;
		int historyIndex;
	};
	extern ChatboxHistory chatboxHistory;
	void chatboxHistory_Add(string);
	string chatboxHistory_Up();
	string chatboxHistory_Down();

	extern void (*callbackPTR_OnClick)(char* name);
	extern void (*callbackPTR_OnTextChange)(char* name,char* text);
	extern void (*callbackPTR_OnListboxSelectionChange)(char* name,char** text);
	extern void (*callbackPTR_OnCheckboxChange)(char* name,bool checked);
	extern void (*callbackPTR_OnReturnDown)(char* name);

	CEGUI::Window* newFramedWindow(string wname);
	CEGUI::Window* newWindow(string parent,string wname,string type);

	CEGUI::Window* getWindow(string name);

	void destroyWindow(string name);

	void setTextColour(CEGUI::Window* w,string t);
};