#include "myIDirect3DDevice9.h"
#include <fstream>
#include <intrin.h>

bool logging=false;

#define DB(a) 	/*{if(logging){std::ofstream d;d.open("C:\\Users\\PC\\Desktop\\debug.txt",std::ios::app);  d<<a<<" Address: "<<_ReturnAddress()<<std::endl;d.flush();d.close();}}*/
#define DBB(a) 	/*std::ofstream d;d.open("C:\\Users\\PC\\Desktop\\debug.txt",std::ios::app);  d<<a<<" Address: "<<_ReturnAddress()<<std::endl;d.flush();d.close();*/

//#define DB(a)  if(debugFrame==1){char tmp[100]="";sprintf(tmp,"C:\\Users\\PC\\Desktop\\frame_debug\\frame%d",debugCount);std::ofstream d;d.open(tmp,std::ios::app|std::ios::out);chatbox.AddLine(tmp);  d<<a<<" Address: "<<_ReturnAddress()<<std::endl;d.flush();d.close();}

//#define DB3(a)  if(debugFrame==1){char tmp[100]="";sprintf(tmp,"C:\\Users\\PC\\Desktop\\frame_debug\\frame%d",debugCount);std::ofstream d;d.open(tmp,std::ios::app|std::ios::out);chatbox.AddLine(tmp);  d<<a<<" Address: "<<_ReturnAddress()<<std::endl;d.flush();d.close();}

myIDirect3DDevice9::myIDirect3DDevice9(IDirect3DDevice9* pOriginal)
{
    m_pIDirect3DDevice9 = pOriginal; // store the pointer to original object

	extern myIDirect3DDevice9* gl_pmyIDirect3DDevice9;
	gl_pmyIDirect3DDevice9=this;

	hdc=CreateCompatibleDC(NULL);

   font=CreateFont(100,0,0,0,FW_NORMAL,false,false,false,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE, "Arial");

   SelectObject(hdc, font);

   chatbox.Init(m_pIDirect3DDevice9,18);


   minVertices=0;
   maxVertices=0;
   grabMatrix=0;
}

myIDirect3DDevice9::~myIDirect3DDevice9(void)
{
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
		chatbox.Release();
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
{ DB("myIDirect3DDevice9::ShowCursor");
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
	chatbox.OnReset();
	return t;
}

HRESULT myIDirect3DDevice9::Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
{ DB("myIDirect3DDevice9::Present");
    
	if(GetAsyncKeyState('K')!=0)
		chatbox.AddLine("PRESENT");

	chatbox.Show(true);
	chatbox.Lock();
	chatbox.Draw(m_pIDirect3DDevice9);
	chatbox.Unlock();

	{
		char buf[100]="";
		D3DXVECTOR2 screenPos;
		float distance;

		RECT font_rect;
		bool vis;

		for(int i=0;i<GameData::playersScreenName.size();i++)
		{
			vis=GetScreenPosition(screenPos,distance,D3DXVECTOR3(*GameData::playersScreenName[i].posX,*GameData::playersScreenName[i].posY,*GameData::playersScreenName[i].posZ));

			if(vis)
			{
				SetRect(&font_rect,screenPos.x,screenPos.y,200,32);
				sprintf(buf,GameData::playersScreenName[i].name.c_str(),screenPos.x,screenPos.y);
				chatbox.g_font->DrawTextA(NULL,buf,-1,&font_rect,DT_LEFT|DT_NOCLIP,0xFFFFFFFF);
			}
		}

		
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
{ DB("myIDirect3DDevice9::CreateTexture");
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
	DB("myIDirect3DDevice9::SetRenderState("<<State<<","<<Value<<")");
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
	DB("myIDirect3DDevice9::DrawIndexedPrimitive ("<<NumVertices<<" NumVertices)");
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




