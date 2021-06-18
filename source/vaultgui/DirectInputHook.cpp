#include "DirectInputHook.h"
#include "myDirectDevice.h"


bool lastMouseState[2]={false,false};

int DIPointers[2]={0,0};

void RealityKeyDown(LPVOID ths, DWORD size, LPVOID data)
{
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
		if(gData.disableKeyboardInput)
		{
			memset(data,0,size);
		}
	}
}

void SimulateKeyDown(LPVOID ths, DWORD size, LPVOID data)
{

}
