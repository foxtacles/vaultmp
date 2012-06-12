#include "GUI.h"

GUI::GUI()
{
	visible=false;
	locked=false;
}

GUI::~GUI()
{

}

void GUI::Lock()
{
	while(locked)
	{
		
	}
	locked=true;
}

void GUI::Unlock()
{
	locked=false;
}

void GUI::Init(IDirect3DDevice9* d,int size,string font)
{
	D3DXCreateFont(d,size,0,700,1,false,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH|FF_DONTCARE,font.c_str(),&g_font);
	if (!(SUCCEEDED(D3DXCreateSprite(d,&sprite))))
	{
		 
	}
	if (!(SUCCEEDED(D3DXCreateTextureFromFile(d,"chatbox.png",&this->gTexture))))
	{
		 
	}
	
}

void GUI::Release()
{
	if(g_font){
      g_font->Release();
      g_font=NULL;
   }
	if(sprite){
      sprite->Release();
      sprite=NULL;
   }
	if(gTexture){
      gTexture->Release();
      gTexture=NULL;
   }
	
}

void GUI::OnReset()
{
	g_font->OnLostDevice();
	g_font->OnResetDevice();
	sprite->OnLostDevice();
	sprite->OnResetDevice();
}

void GUI::Show(bool v)
{
	this->visible=v;
}

void GUI::AddLine(string s)
{
	for(int i=8;i>=0;i--)
	{
		lines[i+1]=lines[i];
	}
	lines[0]=s;
}

void GUI::AddToQueue(string a)
{
	q.push(a);
}

void GUI::SetWriting(string s)
{
	writingText=s;
}

void GUI::Think()
{

}

void GUI::Draw(IDirect3DDevice9* d)
{
	if(!visible)
		return;
	
	

	sprite->Begin(D3DXSPRITE_ALPHABLEND);

	D3DXMATRIX mat;
	D3DXVECTOR2 scaling(0.5f,0.5f);
	D3DXVECTOR2 traslation(15,15);
	D3DXMatrixTransformation2D(&mat,NULL,NULL,&scaling,NULL,NULL,&traslation);

	d->SetTransform(D3DTS_WORLD,&mat);

	

	sprite->Draw(gTexture,NULL,NULL,NULL,0xBBFFFFFF);

	sprite->End();

	for(int i=0;i<10;i++)
	{
		string t=lines[i];

		RECT font_rect;
		SetRect(&font_rect,30,30+((9-i)*20),200,32);
		/*font_height=*/g_font->DrawText(NULL,t.c_str(),-1,&font_rect,DT_LEFT|DT_NOCLIP,0xFF000000); //Color
	}

	string t=writingText;

	RECT font_rect;
	SetRect(&font_rect,30,30+((9+2)*20)-8,200,32);
	/*font_height=*/g_font->DrawText(NULL,t.c_str(),-1,&font_rect,DT_LEFT|DT_NOCLIP,0xFF000000); //Color
}