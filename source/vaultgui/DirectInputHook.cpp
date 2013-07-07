#include "DirectInputHook.h"
#include "myDirectDevice.h"


bool lastMouseState[2]={false,false};

int DIPointers[2]={0,0};

void RealityKeyDown(LPVOID ths, DWORD size, LPVOID data)
{
	char buf[150];
	MyDirectDevice* dev=(MyDirectDevice*)ths;

	if(ths!=(LPVOID)DIPointers[0])
	{
		if(DIPointers[0]==0)
		{
			DIPointers[0]=(int)ths;
		}
		else
		{
			DIPointers[1]=(int)ths;
		}
	}

	if(memcmp(dev->dataFormat,&c_dfDIMouse,sizeof(DIDATAFORMAT)))
	{
		
		DIMOUSESTATE *state=(DIMOUSESTATE*)data;
		#ifdef USE_CEGUI
		if(gData.chatting&&DIPointers[1]==(int)ths)
		{
			CEGUI::System::getSingleton().injectMouseMove(state->lX,state->lY);
			if(state->rgbButtons[0] & 0x80)
			{
				if(!lastMouseState[0])
				{
					CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::MouseButton::LeftButton);
				}
				lastMouseState[0]=true;
				
			}
			else if(lastMouseState[0])
			{
				lastMouseState[0]=false;
				CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::MouseButton::LeftButton);
			}

			if(state->rgbButtons[1] & 0x80)
			{
				CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::MouseButton::RightButton);
			}
		}
#endif

		if(gData.disableMouseInput)
		{
			state->lX=0;
			state->lY=0;
		}
	}

	if(memcmp(dev->dataFormat,&c_dfDIMouse2,sizeof(DIDATAFORMAT)))
	{

		DIMOUSESTATE2 *state=(DIMOUSESTATE2*)data;
		#ifdef USE_CEGUI
		if(gData.chatting)
		{
			CEGUI::System::getSingleton().injectMouseMove(state->lX,state->lY);
			/*if(state->rgbButtons[0] & 0x80)
			{
				if(!lastMouseState[0])
				{
					CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::MouseButton::LeftButton);
				}
				lastMouseState[0]=true;
			}
			else if(lastMouseState[0])
			{
				lastMouseState[0]=false;
				CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::MouseButton::LeftButton);
			}
			if(state->rgbButtons[1] & 0x80)
			{
				CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::MouseButton::RightButton);
			}*/
		}
#endif

		if(gData.disableMouseInput)
		{
			state->lX=0;
			state->lY=0;
		}
	}
	
	if(memcmp(dev->dataFormat,&c_dfDIKeyboard,sizeof(DIDATAFORMAT)))
	{
		//if (diKeys[DIK_ESCAPE] & 0x80) DoSomething();
		if(gData.disableKeyboardInput)
		{
			memset(data,0,size);
		}
	}
}

void SimulateKeyDown(LPVOID ths, DWORD size, LPVOID data)
{
	MyDirectDevice* dev=(MyDirectDevice*)ths;

	if(memcmp(dev->dataFormat,&c_dfDIMouse,sizeof(DIDATAFORMAT)))
	{
		/*DIMOUSESTATE *state=(DIMOUSESTATE*)data;
		state->lX=0;
		state->lY=0;*/
	}

	if(memcmp(dev->dataFormat,&c_dfDIMouse2,sizeof(DIDATAFORMAT)))
	{
		/*DIMOUSESTATE2 *state=(DIMOUSESTATE2*)data;
		state->lX=0;
		state->lY=0;*/
	}
	
	if(memcmp(dev->dataFormat,&c_dfDIKeyboard,sizeof(DIDATAFORMAT)))
	{
		//if (diKeys[DIK_ESCAPE] & 0x80) DoSomething();
		//memset(data,0,size);
		/*if(TaskManager::taskQueue.size())
		{
			Task* tmp=&TaskManager::taskQueue[0];
			if(tmp->type==TASK_KEYPRESS)
			{
				((char*)data)[(*((int*)tmp->data))]=0x80;
				tmp->done=true;
			}
			else if(tmp->type==TASK_SETANGLE)
			{
				float* f=(float*)tmp->data;
				float* f2=(float*)(playerPointer+0x24);
				f2[0]=f[0];
				f2[1]=f[1];
				f2[2]=f[2];
				tmp->done=true;
			}
			else
			{
				tmp->done=true;
			}
		}*/

		/*wnd->setPosition(UVector2(cegui_reldim(0.01f), cegui_reldim( 0.01f)));
		wnd->setSize(UVector2(cegui_reldim(0.35f), cegui_reldim( 0.30f)));*/

		if(!gData.lockChatbox)
		{
			char* d=(char*)data;
			/*if (d[DIK_NUMPAD9] & 0x80 && d[DIK_LSHIFT] & 0x80)
			{
				CEGUI::FrameWindow *w = ((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"));
				w->setPosition(CEGUI::UVector2(cegui_reldim(1.0f-0.35f-0.01f), cegui_reldim( 0.01f)));
			}
			if (d[DIK_NUMPAD8] & 0x80 && d[DIK_LSHIFT] & 0x80)
			{
				CEGUI::FrameWindow *w = ((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"));
				w->setPosition(CEGUI::UVector2(cegui_reldim((1.0f-0.35f)/2-0.01f), cegui_reldim( 0.01f)));
			}
			if (d[DIK_NUMPAD7] & 0x80 && d[DIK_LSHIFT] & 0x80)
			{
				CEGUI::FrameWindow *w = ((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"));
				w->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim( 0.01f)));
			}

			if (d[DIK_NUMPAD6] & 0x80 && d[DIK_LSHIFT] & 0x80)
			{
				CEGUI::FrameWindow *w = ((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"));
				w->setPosition(CEGUI::UVector2(cegui_reldim(1.0f-0.35f-0.01f), cegui_reldim( (1.0f-0.30f)/2-0.01f)));
			}
			if (d[DIK_NUMPAD5] & 0x80 && d[DIK_LSHIFT] & 0x80)
			{
				CEGUI::FrameWindow *w = ((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"));
				w->setPosition(CEGUI::UVector2(cegui_reldim((1.0f-0.35f)/2-0.01f), cegui_reldim( (1.0f-0.30f)/2-0.01f)));
			}
			if (d[DIK_NUMPAD4] & 0x80 && d[DIK_LSHIFT] & 0x80)
			{
				CEGUI::FrameWindow *w = ((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"));
				w->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim( (1.0f-0.30f)/2-0.01f)));
			}

			if (d[DIK_NUMPAD3] & 0x80 && d[DIK_LSHIFT] & 0x80)
			{
				CEGUI::FrameWindow *w = ((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"));
				w->setPosition(CEGUI::UVector2(cegui_reldim(1.0f-0.35f-0.01f), cegui_reldim( 1.0f-0.30f-0.01f)));
			}
			if (d[DIK_NUMPAD2] & 0x80 && d[DIK_LSHIFT] & 0x80)
			{
				CEGUI::FrameWindow *w = ((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"));
				w->setPosition(CEGUI::UVector2(cegui_reldim((1.0f-0.35f)/2-0.01f), cegui_reldim( 1.0f-0.30f-0.01f)));
			}
			if (d[DIK_NUMPAD1] & 0x80 && d[DIK_LSHIFT] & 0x80)
			{
				CEGUI::FrameWindow *w = ((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"));
				w->setPosition(CEGUI::UVector2(cegui_reldim(0.01f), cegui_reldim( 1.0f-0.30f-0.01f)));
			}*/
		}
	}

	//TaskManager::Update();
}