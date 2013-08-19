#include "CEGUI\cegui\include\CEGUI.h"

#include "common.h"

namespace GUIHelper
{
	CEGUI::Window* newFramedWindow(string wname);
	CEGUI::Window* newWindow(string parent,string wname,string type);

	CEGUI::Window* getWindow(string name);

	void destroyWindow(string name);

	void setTextColour(CEGUI::Window* w,string t);
};