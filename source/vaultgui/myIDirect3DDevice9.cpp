#include "myIDirect3DDevice9.h"
#include <fstream>
#include <intrin.h>

#include "utils.h"

#include "Export.h"

#include "HookedFunctions.h"
#include "TextureHooking.h"
#include "GUIHelper.h"

bool logging=false;

//#define DB(a) 	/*{if(logging){std::ofstream d;d.open("C:\\Users\\PC\\Desktop\\debug.txt",std::ios::app);  d<<a<<" Address: "<<_ReturnAddress()<<std::endl;d.flush();d.close();}}*/

#define DB(a) 	{/*char tmp[100];sprintf(tmp,"%s 0x%x",a,_ReturnAddress());SendToLog(tmp);*/}

#define DB2(a) 	{char tmp[100];sprintf(tmp,"%s 0x%x",a,_ReturnAddress());SendToLog(tmp);}

#define DBB(a) 	/*std::ofstream d;d.open("C:\\Users\\PC\\Desktop\\debug.txt",std::ios::app);  d<<a<<" Address: "<<_ReturnAddress()<<std::endl;d.flush();d.close();*/

//#define DB(a)  if(debugFrame==1){char tmp[100]="";sprintf(tmp,"C:\\Users\\PC\\Desktop\\frame_debug\\frame%d",debugCount);std::ofstream d;d.open(tmp,std::ios::app|std::ios::out);chatbox.AddLine(tmp);  d<<a<<" Address: "<<_ReturnAddress()<<std::endl;d.flush();d.close();}

//#define DB3(a)  if(debugFrame==1){char tmp[100]="";sprintf(tmp,"C:\\Users\\PC\\Desktop\\frame_debug\\frame%d",debugCount);std::ofstream d;d.open(tmp,std::ios::app|std::ios::out);chatbox.AddLine(tmp);  d<<a<<" Address: "<<_ReturnAddress()<<std::endl;d.flush();d.close();}



myIDirect3DDevice9::myIDirect3DDevice9(IDirect3DDevice9* pOriginal)
{
	/*remotePlayers *players=new remotePlayers[5];
	players[0].health = 80.0;
	sprintf(players[0].name, "asdf");
	players[0].pos[0] = 995.0;
	players[0].pos[1] = 7157.0;
	players[0].pos[2] = 6802.0;
	players[0].pid = 1;
	players[0].player = true;


	players[1].health = 80.0;
	sprintf(players[1].name, "asdf123");
	players[1].pos[0] = 985.0;
	players[1].pos[1] = 7357.0;
	players[1].pos[2] = 6802.0;
	players[1].pid = 2;
	players[1].player = false;

	playersData=players;*/

	/*playersData=new remotePlayers[5];
	for(int i=0;i<5;i++)
	{
		playersData[i].health=1;
		strcpy(playersData[i].name,"test");
		playersData[i].player=false;
		playersData[i].rot[0]=10;
		playersData[i].rot[1]=10;
		playersData[i].rot[2]=10;
	}

	playersData[0].player=true;
	playersData[0].pos[0]=10;
	playersData[0].pos[1]=10;
	playersData[0].pos[2]=0;

	playersData[1].pos[0]=10;
	playersData[1].pos[1]=-10;
	playersData[1].pos[2]=0;

	playersData[2].pos[0]=-10;
	playersData[2].pos[1]=-10;
	playersData[2].pos[2]=0;

	playersData[3].pos[0]=-10;
	playersData[3].pos[1]=-10;
	playersData[3].pos[2]=10;

	playersData[4].pos[0]=10;
	playersData[4].pos[1]=-10;
	playersData[4].pos[2]=10;*/

	/*Test Data for names above characters*/
	/*for(int i=0;i<5;i++)
	{
		strcpy(playersData[i].name,"TEST");
		playersData[i].player=false;
		playersData[i].pos[0]=100;
		playersData[i].pos[1]=100;
		playersData[i].pos[2]=100;
	}
	playersData[0].player=true;
	playersData[0].pos[0]=0;
	playersData[0].pos[1]=0;
	playersData[0].pos[2]=0;*/

    m_pIDirect3DDevice9 = pOriginal; // store the pointer to original object

	extern myIDirect3DDevice9* gl_pmyIDirect3DDevice9;

	gl_pmyIDirect3DDevice9=this;

	hdc=CreateCompatibleDC(NULL);

   font=CreateFont(100,0,0,0,FW_NORMAL,false,false,false,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE, "Arial");

   SelectObject(hdc, font);

   D3DXCreateFont(m_pIDirect3DDevice9,12,0,700,1,false,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH|FF_DONTCARE,"arial",&g_font);



   #ifdef USE_CEGUI

   SendToLog("CEGUI BootStrap");

   try
   {

	   GUIHelper::chatboxHistory.historyIndex=0;
	   GUIHelper::chatboxHistory.history.clear();

		GUI=&CEGUI::Direct3D9Renderer::bootstrapSystem(pOriginal);

		CEGUI::DefaultResourceProvider* rp = static_cast<CEGUI::DefaultResourceProvider*>(CEGUI::System::getSingleton().getResourceProvider());
		rp->setResourceGroupDirectory("schemes", "vaultmp/datafiles/schemes/");
		rp->setResourceGroupDirectory("imagesets", "vaultmp/datafiles/imagesets/");
		rp->setResourceGroupDirectory("fonts", "vaultmp/datafiles/fonts/");
		rp->setResourceGroupDirectory("layouts", "vaultmp/datafiles/layouts/");
		rp->setResourceGroupDirectory("looknfeels", "vaultmp/datafiles/looknfeel/");
		rp->setResourceGroupDirectory("lua_scripts", "vaultmp/datafiles/lua_scripts/");

		CEGUI::Imageset::setDefaultResourceGroup("imagesets");
		CEGUI::Font::setDefaultResourceGroup("fonts");
		CEGUI::Scheme::setDefaultResourceGroup("schemes");
		CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
		CEGUI::WindowManager::setDefaultResourceGroup("layouts");
		CEGUI::ScriptModule::setDefaultResourceGroup("lua_scripts");
		CEGUI::AnimationManager::setDefaultResourceGroup("animations");

		using namespace CEGUI;
		SchemeManager::getSingleton().create("TaharezLook.scheme");
		//SchemeManager::getSingleton().create("VanillaSkin.scheme");


		CEGUI::System::getSingleton().setDefaultMouseCursor("TaharezLook", "MouseArrow");
		WindowManager& winMgr = WindowManager::getSingleton();
		DefaultWindow* root = (DefaultWindow*)winMgr.createWindow("DefaultWindow", "Root");
		CEGUI::System::getSingleton().setGUISheet(root);
		wnd = (FrameWindow*)winMgr.createWindow("TaharezLook/FrameWindow", "Main Window");
		root->addChildWindow(wnd);
		wnd->setPosition(UVector2(cegui_reldim(0.01f), cegui_reldim( 0.01f)));
		wnd->setSize(UVector2(cegui_reldim(0.35f), cegui_reldim( 0.30f)));
		wnd->setMaxSize(UVector2(cegui_reldim(1.0f), cegui_reldim( 1.0f)));
		wnd->setMinSize(UVector2(cegui_reldim(0.1f), cegui_reldim( 0.1f)));
		wnd->setText("Chat Box");
		wnd->setAlpha(0.9);
		//wnd->setDragMovingEnabled(false);
		
		CEGUI::Editbox* editb=(CEGUI::Editbox*)winMgr.createWindow("TaharezLook/Editbox", "Edit Box");
		editb->subscribeEvent(CEGUI::Window::EventKeyDown,GUIHelper::onKeyDown);
		editb->setMaxTextLength(120);
		wnd->addChildWindow(editb);
		editb->setText("");
		editb->setSize(UVector2(cegui_reldim(1.0f), cegui_reldim( 0.15f)));
		editb->setPosition(UVector2(cegui_reldim(0.0f), cegui_reldim( 0.85f)));
		

		CEGUI::Listbox* listb=(CEGUI::Listbox*)winMgr.createWindow("TaharezLook/Listbox", "List Box");
		wnd->addChildWindow(listb);
		listb->setText("");
		listb->setSize(UVector2(cegui_reldim(1.0f), cegui_reldim( 0.85f)));
		listb->setPosition(UVector2(cegui_reldim(0.0f), cegui_reldim( 0.0f)));

		/*Close BTN*/
		wnd = (FrameWindow*)winMgr.createWindow("DefaultWindow", "Close Button Window");
		wnd->setWidth(UDim(0,16.0f));
		wnd->setHeight(UDim(0,16.0f));
		wnd->setPosition(UVector2(UDim(1.0f,-18.0f), UDim(0.0f,2.0f)));
		root->addChildWindow(wnd);

		CEGUI::ImagesetManager::getSingleton().createFromImageFile("quitBTN","close.png");
		PushButton* closeBTN = (PushButton*)winMgr.createWindow("TaharezLook/Button", "closeBTN");
		closeBTN->setWidth(UDim(0,16.0f));
		closeBTN->setHeight(UDim(0,16.0f));
		/*closeBTN->setXPosition(UDim(1.0f, -16.0f));
		closeBTN->setYPosition(UDim(0.0f, 0.0f));*/
		closeBTN->setPosition(UVector2(cegui_reldim(0.0f), cegui_reldim( 0.0f)));
		closeBTN->setProperty( "NormalImage", "set:quitBTN image:full_image" );
		closeBTN->setProperty( "HoverImage", "set:quitBTN image:full_image" );
		closeBTN->setProperty( "PushedImage", "set:quitBTN image:full_image" );
		closeBTN->subscribeEvent(CEGUI::PushButton::EventClicked,GUI_MouseClickCallback);
		closeBTN->setAlpha(0);

		wnd->addChildWindow(closeBTN);

		/*GUI_CreateFrameWindow("w1");
		GUI_SetSize("w1",0.35,0.4,0,0);
		GUI_SetPosition("w1",0.6,0.1,0,0);*/

		/*GUI_AddRadioButton("w1","r",1);
		GUI_SetPosition("r",0.0,0.0,0,0);
		GUI_SetSize("r",1,0.1,0,0);
		GUI_SetChecked("r",true);*/

		/*GUI_AddRadioButton("w1","r2",1);
		GUI_SetPosition("r2",0.0,0.1,0,0);
		GUI_SetSize("r2",1,0.2,0,0);

		GUI_AddRadioButton("w1","r3",1);
		GUI_SetPosition("r3",0.0,0.2,0,0);
		GUI_SetSize("r3",1,0.2,0,0);*/

		/*GUI_AddStaticText("w1","st");
		GUI_StaticText_SetAlign("st","RightAligned");
		GUI_SetText("st","testtest\r\nblboblba\r\nasgatesttest\r\nblboblba\r\nasga");
		GUI_SetPosition("st",0.0,0.1,0,0);
		GUI_SetSize("st",1,0.7,0,0);*/

		//GUI_SetTextColour("st","FFFF0000");

		/*GUI_AddCheckbox("w1","c");
		GUI_SetPosition("c",0.1,0.1,0,0);
		GUI_Checkbox_SetChecked("c",true);*/


		/*GUI_AddTextbox("w1","txt");
		GUI_SetSize("txt",0.35,0.4,0,0);
		GUI_SetPosition("txt",0.6,0.1,0,0);
		GUI_Textbox_SetMaxLength("txt",12);
		GUI_Textbox_SetValidationString("txt","[0-9]+");*/

		/*GUI_AddListbox("w1","l");
		GUI_SetSize("l",1,0.9,0,0);
		GUI_SetPosition("l",0,0,0,0);

		GUI_Listbox_AddItem("l","1","1");
		GUI_Listbox_AddItem("l","2","2");
		GUI_Listbox_AddItem("l","3","3");
		GUI_Listbox_AddItem("l","4","4");
		GUI_Listbox_SetItemSelected("l","3",true);
		GUI_Listbox_SetItemText("l","3","12");*/
		/*

		GUI_Listbox_RemoveItem("l","7");

		GUI_Listbox_EnableMultiSelect("l",true);

		/*GUI_AddStaticText("w1","ctrl");
		GUI_SetSize("ctrl",0.5,0.1,0,0);
		GUI_SetPosition("ctrl",0,0.9,0,0);
		GUI_SetText("ctrl","ctrl");
		GUI_SetVisible("ctrl",false);*/

		/*GUI_CreateFrameWindow("debugmain");
		GUI_SetFrameWindowPosition("debugmain",0.75,0.05);
		GUI_SetFrameWindowSize("debugmain",0.20,0.3);
		GUI_AddStaticText("debugmain","debug");
		GUI_SetText("debug","This is a static text");
		GUI_SetPosition("debug",0,0);
		GUI_SetSize("debug",1,0.4);

		GUI_AddTextbox("debugmain","debug2");
		GUI_SetText("debug2","This is a textbox");
		GUI_SetPosition("debug2",0,0.4);
		GUI_SetSize("debug2",1,0.3);

		GUI_AddButton("debugmain","debug3");
		GUI_SetText("debug3","This is a button");
		GUI_SetPosition("debug3",0,0.7);
		GUI_SetSize("debug3",1,0.3);*/

		debug_TexturesCount=0;

		CEGUI::MouseCursor::getSingleton().hide();
   }
   catch(CEGUI::Exception e)
   {
	   MessageBoxA(0,e.getMessage().c_str(),"",0);
	   exit(0);
   }

	#endif

   minVertices=0;
   maxVertices=0;
   grabMatrix=0;

   m_pIDirect3DDevice9->CreateStateBlock(D3DSBT_ALL, &pStateBlockBackup[0]);
   m_pIDirect3DDevice9->CreateStateBlock(D3DSBT_ALL, &pStateBlockBackup[1]);
   pStateBlockBackup[0]->Capture();

   //memset(&gData,0,sizeof(gData));
}

myIDirect3DDevice9::~myIDirect3DDevice9(void)
{
	pStateBlockBackup[0]->Release();
	pStateBlockBackup[1]->Release();
}

HRESULT myIDirect3DDevice9::QueryInterface (REFIID riid, void** ppvObj)
{
	DB("myIDirect3DDevice9::QueryInterface");
    // check if original dll can provide interface. then send *our* address
	*ppvObj = NULL;

	HRESULT hRes = m_pIDirect3DDevice9->QueryInterface(riid, ppvObj); 

	if (hRes == NOERROR)
	{
		*ppvObj = this;
	}
	
	return hRes;
}

ULONG myIDirect3DDevice9::AddRef(void)
{
	DB("myIDirect3DDevice9::AddRef");
    return(m_pIDirect3DDevice9->AddRef());
}

ULONG myIDirect3DDevice9::Release(void)
{
	DB("myIDirect3DDevice9::Release");
	extern myIDirect3DDevice9* gl_pmyIDirect3DDevice9;

	ULONG count = m_pIDirect3DDevice9->Release();
	
	if(count==0)
	{
		gl_pmyIDirect3DDevice9 = NULL;
		delete(this);
	}

	return (count);
}

HRESULT myIDirect3DDevice9::TestCooperativeLevel(void)
{
	DB("myIDirect3DDevice9::TestCooperativeLevel");
    return(m_pIDirect3DDevice9->TestCooperativeLevel());
}

UINT myIDirect3DDevice9::GetAvailableTextureMem(void)
{
	DB("myIDirect3DDevice9::GetAvailableTextureMem");
    return(m_pIDirect3DDevice9->GetAvailableTextureMem());
}

HRESULT myIDirect3DDevice9::EvictManagedResources(void)
{
	DB("myIDirect3DDevice9::EvictManagedResources");
    return(m_pIDirect3DDevice9->EvictManagedResources());
}

HRESULT myIDirect3DDevice9::GetDirect3D(IDirect3D9** ppD3D9)
{
	DB("myIDirect3DDevice9::GetDirect3D");
    return(m_pIDirect3DDevice9->GetDirect3D(ppD3D9));
}

HRESULT myIDirect3DDevice9::GetDeviceCaps(D3DCAPS9* pCaps)
{ DB("myIDirect3DDevice9::GetDeviceCaps");
    return(m_pIDirect3DDevice9->GetDeviceCaps(pCaps));
}

HRESULT myIDirect3DDevice9::GetDisplayMode(UINT iSwapChain,D3DDISPLAYMODE* pMode)
{ DB("myIDirect3DDevice9::GetDisplayMode");
    return(m_pIDirect3DDevice9->GetDisplayMode(iSwapChain, pMode));
}

HRESULT myIDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{ DB("myIDirect3DDevice9::GetCreationParameters");
    return(m_pIDirect3DDevice9->GetCreationParameters(pParameters));
}

HRESULT myIDirect3DDevice9::SetCursorProperties(UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap)
{ DB("myIDirect3DDevice9::SetCursorProperties");
    return(m_pIDirect3DDevice9->SetCursorProperties(XHotSpot,YHotSpot,pCursorBitmap));
}

void    myIDirect3DDevice9::SetCursorPosition(int X,int Y,DWORD Flags)
{ DB("myIDirect3DDevice9::SetCursorPosition");
    return(m_pIDirect3DDevice9->SetCursorPosition(X,Y,Flags));
}

BOOL    myIDirect3DDevice9::ShowCursor(BOOL bShow)
{ 
	DB("myIDirect3DDevice9::ShowCursor");
	/*if(bShow)
		Reversing_AddDebug("myIDirect3DDevice9::ShowCursor(true)");
	else
		Reversing_AddDebug("myIDirect3DDevice9::ShowCursor(false)");*/

    return(m_pIDirect3DDevice9->ShowCursor(bShow));
}

HRESULT myIDirect3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain)  
{ DB("myIDirect3DDevice9::CreateAdditionalSwapChain");
    return(m_pIDirect3DDevice9->CreateAdditionalSwapChain(pPresentationParameters,pSwapChain));
}

HRESULT myIDirect3DDevice9::GetSwapChain(UINT iSwapChain,IDirect3DSwapChain9** pSwapChain)
{ DB("myIDirect3DDevice9::GetSwapChain");
    return(m_pIDirect3DDevice9->GetSwapChain(iSwapChain,pSwapChain));
}

UINT    myIDirect3DDevice9::GetNumberOfSwapChains(void)
{ DB("myIDirect3DDevice9::GetNumberOfSwapChains");
    return(m_pIDirect3DDevice9->GetNumberOfSwapChains());
}

HRESULT myIDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{ DB("myIDirect3DDevice9::Reset");
	
    HRESULT t=(m_pIDirect3DDevice9->Reset(pPresentationParameters));
	CEGUI::System::getSingleton().invalidateAllCachedRendering();
	return t;
}

HRESULT myIDirect3DDevice9::Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
{ DB("myIDirect3DDevice9::Present");
    
#ifdef USE_CEGUI
	if(gData.gameReady&&!gData.hideChatbox)
	{
		pStateBlockBackup[1]->Capture();
		pStateBlockBackup[0]->Apply();

		if(gData.chatting)
		{
			((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"))->setAlpha(0.9);
			
		}
		else
		{
			if(GetTickCount()-gData.lastChatTextTick<2000)
			{
				((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"))->setAlpha(1);
			}
			else
			if(GetTickCount()-gData.lastChatTextTick>=2000&&GetTickCount()-gData.lastChatTextTick<5000)
			{
				float rat=0.6/3000;
				float var=GetTickCount()-gData.lastChatTextTick;
				((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"))->setAlpha(1-((var-2000)*rat));
			}

			/*if(GetTickCount()-gData.lastChatTextTick<2000)
			{
				((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"))->setAlpha(0.85);
			}
			else if(GetTickCount()-gData.lastChatTextTick<3000)
			{
				((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"))->setAlpha(0.8);
			}
			else if(GetTickCount()-gData.lastChatTextTick<3500)
			{
				((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"))->setAlpha(0.75);
			}
			else if(GetTickCount()-gData.lastChatTextTick<4000)
			{
				((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"))->setAlpha(0.7);
			}
			else if(GetTickCount()-gData.lastChatTextTick<4500)
			{
				((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"))->setAlpha(0.65);
			}
			else  if(GetTickCount()-gData.lastChatTextTick<5000)
			{
				((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"))->setAlpha(0.60);
			}
			else  if(GetTickCount()-gData.lastChatTextTick<5500)
			{
				((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"))->setAlpha(0.55);
			}
			else  if(GetTickCount()-gData.lastChatTextTick<6000)
			{
				((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"))->setAlpha(0.50);
			}*/
			else 
				((CEGUI::FrameWindow*)CEGUI::WindowManager::getSingleton().getWindow("Main Window"))->setAlpha(0.40);
		}

		CEGUI::System::getSingleton().renderGUI();




		
		/*D3DXMATRIX world_matrix;
		D3DXMatrixIdentity(&world_matrix);
		D3DXMatrixTranslation(&world_matrix,20,20,10);
		this->SetPixelShader(0);
		this->SetVertexShader(0);
		this->SetRenderState(D3DRS_LIGHTING,FALSE);
		this->SetTransform(D3DTS_WORLD,&world_matrix);
		mesh->DrawSubset(0);
		*/





		pStateBlockBackup[1]->Apply();

		

		
	}
#endif

	{
		char buf[100]="";
		D3DXVECTOR2 screenPos;
		float distance;

		RECT font_rect;
		bool vis;

		if(playersData)
		{
			float x=0;
			float y=0;
			float z=0;
			bool go=false;
			for(unsigned int i=0;i<5;i++)
			{
				if(playersData[i].player)
				{
					x=playersData[i].pos[0];
					y=playersData[i].pos[1];
					z=playersData[i].pos[2];
					go=true;
					break;
				}
			}

			if(go)
			{
				for(unsigned int i=0;i<5;i++)
				{
					vis=GetScreenPosition(screenPos,distance,D3DXVECTOR3(playersData[i].pos[0]-x,playersData[i].pos[1]-y,playersData[i].pos[2]-z));
					if(vis)
					{
						SetRect(&font_rect,screenPos.x,screenPos.y,200,32);
						sprintf(buf,playersData[i].name,screenPos.x,screenPos.y);
						g_font->DrawTextA(NULL,buf,-1,&font_rect,DT_LEFT|DT_NOCLIP,0xFFFFFFFF);
						//exit(0);
					}
				}
			}
		}

		/*for(unsigned int i=0;i<GameData::playersScreenName.size();i++)
		{
			vis=GetScreenPosition(screenPos,distance,D3DXVECTOR3(*GameData::playersScreenName[i].posX,*GameData::playersScreenName[i].posY,*GameData::playersScreenName[i].posZ));

			if(vis)
			{
				SetRect(&font_rect,screenPos.x,screenPos.y,200,32);
				sprintf(buf,GameData::playersScreenName[i].name.c_str(),screenPos.x,screenPos.y);
				g_font->DrawTextA(NULL,buf,-1,&font_rect,DT_LEFT|DT_NOCLIP,0xFFFFFFFF);
			}
		}*/


		/*SetRect(&font_rect,20,400,800,32);
		sprintf(buf,"Player pointer: 0x%x",playerPointer);
		g_font->DrawTextA(NULL,buf,-1,&font_rect,DT_LEFT|DT_NOCLIP,0xFFFFFFFF);

		if(playerPointer)
		{
			SetRect(&font_rect,20,415,800,32);
			sprintf(buf,"Player angle: %.4f %.4f %.4f",*(float*)(playerPointer+0x24),*(float*)(playerPointer+0x28),*(float*)(playerPointer+0x2C));
			g_font->DrawTextA(NULL,buf,-1,&font_rect,DT_LEFT|DT_NOCLIP,0xFFFFFFFF);

			SetRect(&font_rect,20,430,800,32);
			sprintf(buf,"Player position: %.4f %.4f %.4f",*(float*)(playerPointer+0x30),*(float*)(playerPointer+0x34),*(float*)(playerPointer+0x38));
			g_font->DrawTextA(NULL,buf,-1,&font_rect,DT_LEFT|DT_NOCLIP,0xFFFFFFFF);
		}

		SetRect(&font_rect,20,445,800,32);
		sprintf(buf,"%d task in queue",TaskManager::taskQueue.size());
		g_font->DrawTextA(NULL,buf,-1,&font_rect,DT_LEFT|DT_NOCLIP,0xFFFFFFFF);*/

	}

	HRESULT hres = m_pIDirect3DDevice9->Present( pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);

	logging=false;

	return (hres);
}

HRESULT myIDirect3DDevice9::GetBackBuffer(UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer)
{ DB("myIDirect3DDevice9::GetBackBuffer");
    return(m_pIDirect3DDevice9->GetBackBuffer(iSwapChain,iBackBuffer,Type,ppBackBuffer));
}

HRESULT myIDirect3DDevice9::GetRasterStatus(UINT iSwapChain,D3DRASTER_STATUS* pRasterStatus)
{ DB("myIDirect3DDevice9::GetRasterStatus");
    return(m_pIDirect3DDevice9->GetRasterStatus(iSwapChain,pRasterStatus));
}

HRESULT myIDirect3DDevice9::SetDialogBoxMode(BOOL bEnableDialogs)
{ DB("myIDirect3DDevice9::SetDialogBoxMode");
    return(m_pIDirect3DDevice9->SetDialogBoxMode(bEnableDialogs));
}

void    myIDirect3DDevice9::SetGammaRamp(UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp)
{ DB("myIDirect3DDevice9::SetGammaRamp");
    return(m_pIDirect3DDevice9->SetGammaRamp(iSwapChain,Flags,pRamp));
}

void    myIDirect3DDevice9::GetGammaRamp(UINT iSwapChain,D3DGAMMARAMP* pRamp)
{ DB("myIDirect3DDevice9::GetGammaRamp");
    return(m_pIDirect3DDevice9->GetGammaRamp(iSwapChain,pRamp));
}

HRESULT myIDirect3DDevice9::CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle)
{
	DB("myIDirect3DDevice9::CreateTexture");
	char tmp[512];
	if(lastTextureLoaded)
	{
		sprintf(tmp,"myIDirect3DDevice9::CreateTexture %s",lastTextureLoaded);
		SendToLog(tmp);
		lastTextureLoaded=0;
	}

	debug_TexturesCount++;
	/*sprintf(tmp,"%d textures loaded",debug_TexturesCount);
	GUI_SetText("debug",tmp);*/

    return(m_pIDirect3DDevice9->CreateTexture(Width,Height,Levels,Usage,Format,Pool,ppTexture,pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateVolumeTexture(UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle)
{ DB("myIDirect3DDevice9::CreateVolumeTexture");
    return(m_pIDirect3DDevice9->CreateVolumeTexture(Width,Height,Depth,Levels,Usage,Format,Pool,ppVolumeTexture,pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateCubeTexture(UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle)
{ DB("myIDirect3DDevice9::CreateCubeTexture");
    return(m_pIDirect3DDevice9->CreateCubeTexture(EdgeLength,Levels,Usage,Format,Pool,ppCubeTexture,pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle)
{ DB("myIDirect3DDevice9::CreateVertexBuffer");
    return(m_pIDirect3DDevice9->CreateVertexBuffer(Length,Usage,FVF,Pool,ppVertexBuffer,pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateIndexBuffer(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle)
{ DB("myIDirect3DDevice9::CreateIndexBuffer");
    return(m_pIDirect3DDevice9->CreateIndexBuffer(Length,Usage,Format,Pool,ppIndexBuffer,pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateRenderTarget(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{ DB("myIDirect3DDevice9::CreateRenderTarget");
    return(m_pIDirect3DDevice9->CreateRenderTarget(Width,Height,Format,MultiSample,MultisampleQuality,Lockable,ppSurface,pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateDepthStencilSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{ DB("myIDirect3DDevice9::CreateDepthStencilSurface");
    return(m_pIDirect3DDevice9->CreateDepthStencilSurface(Width,Height,Format,MultiSample,MultisampleQuality,Discard,ppSurface,pSharedHandle));
}

HRESULT myIDirect3DDevice9::UpdateSurface(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint)
{ DB("myIDirect3DDevice9::UpdateSurface");
    return(m_pIDirect3DDevice9->UpdateSurface(pSourceSurface,pSourceRect,pDestinationSurface,pDestPoint));
}

HRESULT myIDirect3DDevice9::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture)
{ DB("myIDirect3DDevice9::UpdateTexture");
    return(m_pIDirect3DDevice9->UpdateTexture(pSourceTexture,pDestinationTexture));
}

HRESULT myIDirect3DDevice9::GetRenderTargetData(IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface)
{ DB("myIDirect3DDevice9::GetRenderTargetData");
    return(m_pIDirect3DDevice9->GetRenderTargetData(pRenderTarget,pDestSurface));
}

HRESULT myIDirect3DDevice9::GetFrontBufferData(UINT iSwapChain,IDirect3DSurface9* pDestSurface)
{ DB("myIDirect3DDevice9::GetFrontBufferData");
    return(m_pIDirect3DDevice9->GetFrontBufferData(iSwapChain,pDestSurface));
}

HRESULT myIDirect3DDevice9::StretchRect(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter)
{ DB("myIDirect3DDevice9::StretchRect");
    return(m_pIDirect3DDevice9->StretchRect(pSourceSurface,pSourceRect,pDestSurface,pDestRect,Filter));
}

HRESULT myIDirect3DDevice9::ColorFill(IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color)
{ DB("myIDirect3DDevice9::ColorFill");
	/*Reversing_AddDebug("myIDirect3DDevice9::ColorFill("+string(""+color)+")");*/
    return(m_pIDirect3DDevice9->ColorFill(pSurface,pRect,color));
}

HRESULT myIDirect3DDevice9::CreateOffscreenPlainSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{ DB("myIDirect3DDevice9::CreateOffscreenPlainSurface");
    return(m_pIDirect3DDevice9->CreateOffscreenPlainSurface(Width,Height,Format,Pool,ppSurface,pSharedHandle));
}

HRESULT myIDirect3DDevice9::SetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget)
{ DB("myIDirect3DDevice9::SetRenderTarget");
    return(m_pIDirect3DDevice9->SetRenderTarget(RenderTargetIndex,pRenderTarget));
}

HRESULT myIDirect3DDevice9::GetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget)
{ DB("myIDirect3DDevice9::GetRenderTarget");
    return(m_pIDirect3DDevice9->GetRenderTarget(RenderTargetIndex,ppRenderTarget));
}

HRESULT myIDirect3DDevice9::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{ DB("myIDirect3DDevice9::SetDepthStencilSurface");
    return(m_pIDirect3DDevice9->SetDepthStencilSurface(pNewZStencil));
}

HRESULT myIDirect3DDevice9::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{ DB("myIDirect3DDevice9::GetDepthStencilSurface");
    return(m_pIDirect3DDevice9->GetDepthStencilSurface(ppZStencilSurface));
}

HRESULT myIDirect3DDevice9::BeginScene(void)
{ DB("myIDirect3DDevice9::BeginScene");
	
    return(m_pIDirect3DDevice9->BeginScene());
}

HRESULT myIDirect3DDevice9::EndScene(void)
{
	DB("myIDirect3DDevice9::EndScene");

    return(m_pIDirect3DDevice9->EndScene());
}

HRESULT myIDirect3DDevice9::Clear(DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil)
{ 
	if(Flags==7)
		grabMatrix=1;
	DB("myIDirect3DDevice9::Clear");
	/*Reversing_AddDebug(string("myIDirect3DDevice9::Clear(")+string(""+Count)+","+string(""+Flags)+","+string(""+Color)+")");*/
	//TODO: Da finire
    return(m_pIDirect3DDevice9->Clear(Count,pRects,Flags,Color,Z,Stencil));
}

HRESULT myIDirect3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
{ 
	if(State==D3DTS_PROJECTION)
	{
		if(grabMatrix==1)
		{
			GameData::lastProjection=(*pMatrix);
			grabMatrix=0;
		}
	}
	if(State==D3DTS_VIEW)
	{
		if(grabMatrix==1)
		{
			GameData::lastView=(*pMatrix);
		}
		
	}

	DB("myIDirect3DDevice9::SetTransform");
    return(m_pIDirect3DDevice9->SetTransform(State,pMatrix));
}

HRESULT myIDirect3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix)
{ DB("myIDirect3DDevice9::GetTransform");
    return(m_pIDirect3DDevice9->GetTransform(State,pMatrix));
}

HRESULT myIDirect3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
{ DB("myIDirect3DDevice9::MultiplyTransform");
    return(m_pIDirect3DDevice9->MultiplyTransform(State,pMatrix));
}

HRESULT myIDirect3DDevice9::SetViewport(CONST D3DVIEWPORT9* pViewport)
{ DB("myIDirect3DDevice9::SetViewport");
    return(m_pIDirect3DDevice9->SetViewport(pViewport));
}

HRESULT myIDirect3DDevice9::GetViewport(D3DVIEWPORT9* pViewport)
{ DB("myIDirect3DDevice9::GetViewport");
    return(m_pIDirect3DDevice9->GetViewport(pViewport));
}

HRESULT myIDirect3DDevice9::SetMaterial(CONST D3DMATERIAL9* pMaterial)
{ DB("myIDirect3DDevice9::SetMaterial");
    return(m_pIDirect3DDevice9->SetMaterial(pMaterial));
}

HRESULT myIDirect3DDevice9::GetMaterial(D3DMATERIAL9* pMaterial)
{ DB("myIDirect3DDevice9::GetMaterial");
    return(m_pIDirect3DDevice9->GetMaterial(pMaterial));
}

HRESULT myIDirect3DDevice9::SetLight(DWORD Index,CONST D3DLIGHT9* pLight)
{ DB("myIDirect3DDevice9::SetLight");
    return(m_pIDirect3DDevice9->SetLight(Index,pLight));
}

HRESULT myIDirect3DDevice9::GetLight(DWORD Index,D3DLIGHT9* pLight)
{ DB("myIDirect3DDevice9::GetLight");
    return(m_pIDirect3DDevice9->GetLight(Index,pLight));
}

HRESULT myIDirect3DDevice9::LightEnable(DWORD Index,BOOL Enable)
{ DB("myIDirect3DDevice9::LightEnable");
    return(m_pIDirect3DDevice9->LightEnable(Index,Enable));
}

HRESULT myIDirect3DDevice9::GetLightEnable(DWORD Index,BOOL* pEnable)
{ DB("myIDirect3DDevice9::GetLightEnable");
    return(m_pIDirect3DDevice9->GetLightEnable(Index, pEnable));
}

HRESULT myIDirect3DDevice9::SetClipPlane(DWORD Index,CONST float* pPlane)
{ DB("myIDirect3DDevice9::SetClipPlane");
    return(m_pIDirect3DDevice9->SetClipPlane(Index, pPlane));
}

HRESULT myIDirect3DDevice9::GetClipPlane(DWORD Index,float* pPlane)
{ DB("myIDirect3DDevice9::GetClipPlane");
    return(m_pIDirect3DDevice9->GetClipPlane(Index,pPlane));
}

HRESULT myIDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE State,DWORD Value)
{ 
	DB("myIDirect3DDevice9::SetRenderState()");
    return(m_pIDirect3DDevice9->SetRenderState(State, Value));
}

HRESULT myIDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE State,DWORD* pValue)
{ DB("myIDirect3DDevice9::GetRenderState");
    return(m_pIDirect3DDevice9->GetRenderState(State, pValue));
}

HRESULT myIDirect3DDevice9::CreateStateBlock(D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB)
{ DB("myIDirect3DDevice9::CreateStateBlock");
    return(m_pIDirect3DDevice9->CreateStateBlock(Type,ppSB));
}

HRESULT myIDirect3DDevice9::BeginStateBlock(void)
{ DB("myIDirect3DDevice9::BeginStateBlock");
    return(m_pIDirect3DDevice9->BeginStateBlock());
}

HRESULT myIDirect3DDevice9::EndStateBlock(IDirect3DStateBlock9** ppSB)
{ DB("myIDirect3DDevice9::EndStateBlock");
    return(m_pIDirect3DDevice9->EndStateBlock(ppSB));
}

HRESULT myIDirect3DDevice9::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus)
{ DB("myIDirect3DDevice9::SetClipStatus");
    return(m_pIDirect3DDevice9->SetClipStatus(pClipStatus));
}

HRESULT myIDirect3DDevice9::GetClipStatus(D3DCLIPSTATUS9* pClipStatus)
{ DB("myIDirect3DDevice9::GetClipStatus");
    return(m_pIDirect3DDevice9->GetClipStatus( pClipStatus));
}

HRESULT myIDirect3DDevice9::GetTexture(DWORD Stage,IDirect3DBaseTexture9** ppTexture)
{ DB("myIDirect3DDevice9::GetTexture");
    return(m_pIDirect3DDevice9->GetTexture(Stage,ppTexture));
}

HRESULT myIDirect3DDevice9::SetTexture(DWORD Stage,IDirect3DBaseTexture9* pTexture)
{ DB("myIDirect3DDevice9::SetTexture");
	
    return(m_pIDirect3DDevice9->SetTexture(Stage,pTexture));
}

HRESULT myIDirect3DDevice9::GetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue)
{ DB("myIDirect3DDevice9::GetTextureStageState");
    return(m_pIDirect3DDevice9->GetTextureStageState(Stage,Type, pValue));
}

HRESULT myIDirect3DDevice9::SetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value)
{ DB("myIDirect3DDevice9::SetTextureStageState");
    return(m_pIDirect3DDevice9->SetTextureStageState(Stage,Type,Value));
}

HRESULT myIDirect3DDevice9::GetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue)
{ DB("myIDirect3DDevice9::GetSamplerState");
    return(m_pIDirect3DDevice9->GetSamplerState(Sampler,Type, pValue));
}

HRESULT myIDirect3DDevice9::SetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value)
{ DB("myIDirect3DDevice9::SetSamplerState");
    return(m_pIDirect3DDevice9->SetSamplerState(Sampler,Type,Value));
}

HRESULT myIDirect3DDevice9::ValidateDevice(DWORD* pNumPasses)
{ DB("myIDirect3DDevice9::ValidateDevice");
    return(m_pIDirect3DDevice9->ValidateDevice( pNumPasses));
}

HRESULT myIDirect3DDevice9::SetPaletteEntries(UINT PaletteNumber,CONST PALETTEENTRY* pEntries)
{ DB("myIDirect3DDevice9::SetPaletteEntries");
    return(m_pIDirect3DDevice9->SetPaletteEntries(PaletteNumber, pEntries));
}

HRESULT myIDirect3DDevice9::GetPaletteEntries(UINT PaletteNumber,PALETTEENTRY* pEntries)
{ DB("myIDirect3DDevice9::GetPaletteEntries");
    return(m_pIDirect3DDevice9->GetPaletteEntries(PaletteNumber, pEntries));
}

HRESULT myIDirect3DDevice9::SetCurrentTexturePalette(UINT PaletteNumber)
{ DB("myIDirect3DDevice9::SetCurrentTexturePalette");
    return(m_pIDirect3DDevice9->SetCurrentTexturePalette(PaletteNumber));
}

HRESULT myIDirect3DDevice9::GetCurrentTexturePalette(UINT *PaletteNumber)
{ DB("myIDirect3DDevice9::GetCurrentTexturePalette");
    return(m_pIDirect3DDevice9->GetCurrentTexturePalette(PaletteNumber));
}

HRESULT myIDirect3DDevice9::SetScissorRect(CONST RECT* pRect)
{ DB("myIDirect3DDevice9::SetScissorRect");
    return(m_pIDirect3DDevice9->SetScissorRect( pRect));
}

HRESULT myIDirect3DDevice9::GetScissorRect( RECT* pRect)
{ DB("myIDirect3DDevice9::GetScissorRect");
    return(m_pIDirect3DDevice9->GetScissorRect( pRect));
}

HRESULT myIDirect3DDevice9::SetSoftwareVertexProcessing(BOOL bSoftware)
{ DB("myIDirect3DDevice9::SetSoftwareVertexProcessing");
    return(m_pIDirect3DDevice9->SetSoftwareVertexProcessing(bSoftware));
}

BOOL    myIDirect3DDevice9::GetSoftwareVertexProcessing(void)
{ DB("myIDirect3DDevice9::GetSoftwareVertexProcessing");
    return(m_pIDirect3DDevice9->GetSoftwareVertexProcessing());
}

HRESULT myIDirect3DDevice9::SetNPatchMode(float nSegments)
{ DB("myIDirect3DDevice9::SetNPatchMode");
    return(m_pIDirect3DDevice9->SetNPatchMode(nSegments));
}

float   myIDirect3DDevice9::GetNPatchMode(void)
{ DB("myIDirect3DDevice9::GetNPatchMode");
    return(m_pIDirect3DDevice9->GetNPatchMode());
}

HRESULT myIDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount)
{ DB("myIDirect3DDevice9::DrawPrimitive");
    return(m_pIDirect3DDevice9->DrawPrimitive(PrimitiveType,StartVertex,PrimitiveCount));
}

HRESULT myIDirect3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount)
{ 
	//DB2("myIDirect3DDevice9::DrawIndexedPrimitive () ");

	/*if((unsigned int)_ReturnAddress()==0x879968)	//Disattiva la landscape
		return 0;*/

	/*if((unsigned int)_ReturnAddress()==0x879745&&GetAsyncKeyState('A'))	//Disattiva tutto il rendering
		return 0;*/

    return(m_pIDirect3DDevice9->DrawIndexedPrimitive(PrimitiveType,BaseVertexIndex,MinVertexIndex,NumVertices,startIndex,primCount));
}

HRESULT myIDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{ DB("myIDirect3DDevice9::DrawPrimitiveUP");
    return(m_pIDirect3DDevice9->DrawPrimitiveUP(PrimitiveType,PrimitiveCount,pVertexStreamZeroData,VertexStreamZeroStride));
}

HRESULT myIDirect3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{ DB("myIDirect3DDevice9::DrawIndexedPrimitiveUP");
    return(m_pIDirect3DDevice9->DrawIndexedPrimitiveUP(PrimitiveType,MinVertexIndex,NumVertices,PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData,VertexStreamZeroStride));
}

HRESULT myIDirect3DDevice9::ProcessVertices(UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags)
{ DB("myIDirect3DDevice9::ProcessVertices");
    return(m_pIDirect3DDevice9->ProcessVertices( SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags));
}

HRESULT myIDirect3DDevice9::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl)
{ DB("myIDirect3DDevice9::CreateVertexDeclaration");
    return(m_pIDirect3DDevice9->CreateVertexDeclaration( pVertexElements,ppDecl));
}

HRESULT myIDirect3DDevice9::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{ DB("myIDirect3DDevice9::SetVertexDeclaration");
    return(m_pIDirect3DDevice9->SetVertexDeclaration(pDecl));
}

HRESULT myIDirect3DDevice9::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{ DB("myIDirect3DDevice9::GetVertexDeclaration");
    return(m_pIDirect3DDevice9->GetVertexDeclaration(ppDecl));
}

HRESULT myIDirect3DDevice9::SetFVF(DWORD FVF)
{ DB("myIDirect3DDevice9::SetFVF");
    return(m_pIDirect3DDevice9->SetFVF(FVF));
}

HRESULT myIDirect3DDevice9::GetFVF(DWORD* pFVF)
{ DB("myIDirect3DDevice9::GetFVF");
    return(m_pIDirect3DDevice9->GetFVF(pFVF));
}

HRESULT myIDirect3DDevice9::CreateVertexShader(CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader)
{ DB("myIDirect3DDevice9::CreateVertexShader");
    return(m_pIDirect3DDevice9->CreateVertexShader(pFunction,ppShader));
}

HRESULT myIDirect3DDevice9::SetVertexShader(IDirect3DVertexShader9* pShader)
{ DB("myIDirect3DDevice9::SetVertexShader");
    return(m_pIDirect3DDevice9->SetVertexShader(pShader));
}

HRESULT myIDirect3DDevice9::GetVertexShader(IDirect3DVertexShader9** ppShader)
{ DB("myIDirect3DDevice9::GetVertexShader");
    return(m_pIDirect3DDevice9->GetVertexShader(ppShader));
}

HRESULT myIDirect3DDevice9::SetVertexShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{ DB("myIDirect3DDevice9::SetVertexShaderConstantF");
    return(m_pIDirect3DDevice9->SetVertexShaderConstantF(StartRegister,pConstantData, Vector4fCount));
}

HRESULT myIDirect3DDevice9::GetVertexShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{ DB("myIDirect3DDevice9::GetVertexShaderConstantF");
    return(m_pIDirect3DDevice9->GetVertexShaderConstantF(StartRegister,pConstantData,Vector4fCount));
}

HRESULT myIDirect3DDevice9::SetVertexShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{ DB("myIDirect3DDevice9::SetVertexShaderConstantI");
    return(m_pIDirect3DDevice9->SetVertexShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT myIDirect3DDevice9::GetVertexShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{ DB("myIDirect3DDevice9::GetVertexShaderConstantI");
    return(m_pIDirect3DDevice9->GetVertexShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT myIDirect3DDevice9::SetVertexShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{ DB("myIDirect3DDevice9::SetVertexShaderConstantB");
    return(m_pIDirect3DDevice9->SetVertexShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT myIDirect3DDevice9::GetVertexShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{ DB("myIDirect3DDevice9::GetVertexShaderConstantB");
    return(m_pIDirect3DDevice9->GetVertexShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT myIDirect3DDevice9::SetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride)
{ DB("myIDirect3DDevice9::SetStreamSource");
    return(m_pIDirect3DDevice9->SetStreamSource(StreamNumber,pStreamData,OffsetInBytes,Stride));
}

HRESULT myIDirect3DDevice9::GetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* OffsetInBytes,UINT* pStride)
{ DB("myIDirect3DDevice9::GetStreamSource");
    return(m_pIDirect3DDevice9->GetStreamSource(StreamNumber,ppStreamData,OffsetInBytes,pStride));
}

HRESULT myIDirect3DDevice9::SetStreamSourceFreq(UINT StreamNumber,UINT Divider)
{ DB("myIDirect3DDevice9::SetStreamSourceFreq");
    return(m_pIDirect3DDevice9->SetStreamSourceFreq(StreamNumber,Divider));
}

HRESULT myIDirect3DDevice9::GetStreamSourceFreq(UINT StreamNumber,UINT* Divider)
{ DB("myIDirect3DDevice9::GetStreamSourceFreq");
    return(m_pIDirect3DDevice9->GetStreamSourceFreq(StreamNumber,Divider));
}

HRESULT myIDirect3DDevice9::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{ DB("myIDirect3DDevice9::SetIndices");
    return(m_pIDirect3DDevice9->SetIndices(pIndexData));
}

HRESULT myIDirect3DDevice9::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{ DB("myIDirect3DDevice9::GetIndices");
    return(m_pIDirect3DDevice9->GetIndices(ppIndexData));
}

HRESULT myIDirect3DDevice9::CreatePixelShader(CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader)
{ DB("myIDirect3DDevice9::CreatePixelShader");
    return(m_pIDirect3DDevice9->CreatePixelShader(pFunction,ppShader));
}

HRESULT myIDirect3DDevice9::SetPixelShader(IDirect3DPixelShader9* pShader)
{ DB("myIDirect3DDevice9::SetPixelShader");
    return(m_pIDirect3DDevice9->SetPixelShader(pShader));
}

HRESULT myIDirect3DDevice9::GetPixelShader(IDirect3DPixelShader9** ppShader)
{ DB("myIDirect3DDevice9::GetPixelShader");
    return(m_pIDirect3DDevice9->GetPixelShader(ppShader));
}

HRESULT myIDirect3DDevice9::SetPixelShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{ DB("myIDirect3DDevice9::SetPixelShaderConstantF");
    return(m_pIDirect3DDevice9->SetPixelShaderConstantF(StartRegister,pConstantData,Vector4fCount));
}

HRESULT myIDirect3DDevice9::GetPixelShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{ DB("myIDirect3DDevice9::GetPixelShaderConstantF");
    return(m_pIDirect3DDevice9->GetPixelShaderConstantF(StartRegister,pConstantData,Vector4fCount));
}

HRESULT myIDirect3DDevice9::SetPixelShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{ DB("myIDirect3DDevice9::SetPixelShaderConstantI");
    return(m_pIDirect3DDevice9->SetPixelShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT myIDirect3DDevice9::GetPixelShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{ DB("myIDirect3DDevice9::GetPixelShaderConstantI");
    return(m_pIDirect3DDevice9->GetPixelShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT myIDirect3DDevice9::SetPixelShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{ DB("myIDirect3DDevice9::SetPixelShaderConstantB");
    return(m_pIDirect3DDevice9->SetPixelShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT myIDirect3DDevice9::GetPixelShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{ DB("myIDirect3DDevice9::GetPixelShaderConstantB");
    return(m_pIDirect3DDevice9->GetPixelShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT myIDirect3DDevice9::DrawRectPatch(UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{ DB("myIDirect3DDevice9::DrawRectPatch");
    return(m_pIDirect3DDevice9->DrawRectPatch(Handle,pNumSegs, pRectPatchInfo));
}

HRESULT myIDirect3DDevice9::DrawTriPatch(UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{ DB("myIDirect3DDevice9::DrawTriPatch");
    return(m_pIDirect3DDevice9->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo));
}

HRESULT myIDirect3DDevice9::DeletePatch(UINT Handle)
{ DB("myIDirect3DDevice9::DeletePatch");
    return(m_pIDirect3DDevice9->DeletePatch(Handle));
}

HRESULT myIDirect3DDevice9::CreateQuery(D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery)
{ DB("myIDirect3DDevice9::CreateQuery");
    return(m_pIDirect3DDevice9->CreateQuery(Type,ppQuery));
}




