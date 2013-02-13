#include "common.h"

#include "utils.h"

vector<string> mainMenu;
vector<string> game;
vector<string> menu;

bool Reversing_FunctionAlreadyCalled(string f,int c)
{
	if(c==0)
	{
		for(int i=0;i<mainMenu.size();i++)
		{
			if(mainMenu[i]==f)
				return true;
		}
	}
	if(c==1)
	{
		for(int i=0;i<game.size();i++)
		{
			if(game[i]==f)
				return true;
		}
	}
	if(c==2)
	{
		for(int i=0;i<menu.size();i++)
		{
			if(menu[i]==f)
				return true;
		}
	}


	return false;
	
}

void Reversing_AddFunctionCall(string f,int c)
{
	if(Reversing_FunctionAlreadyCalled(f,c))
		return;
	if(c==0)
	{
		mainMenu.push_back(f);
	}
	if(c==1)
	{
		game.push_back(f);
	}
	if(c==2)
	{
		menu.push_back(f);
	}
}

void Reversing_AddDebug(string s)
{
	/*if(GetAsyncKeyState('1'))
	{
		Reversing_AddFunctionCall(s,0);
	}
	if(GetAsyncKeyState('2'))
	{
		Reversing_AddFunctionCall(s,1);
	}
	if(GetAsyncKeyState('3'))
	{
		Reversing_AddFunctionCall(s,2);
	}*/
}