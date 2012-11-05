#include "GUI.h"

GUI::GUI()
{
	visible=false;
	locked=false;
	sizeMult=1.0;
	scrollOffset=0;
	for(int i=0;i<sizeof(lines)/sizeof(GUIText*);i++)
		lines[i]=NULL;
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
	D3DXCreateFont(d,(size*sizeMult),0,700,1,false,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH|FF_DONTCARE,font.c_str(),&g_font);
	if (!(SUCCEEDED(D3DXCreateSprite(d,&sprite))))
	{
		 
	}
	if (!(SUCCEEDED(D3DXCreateTextureFromFile(d,"chatbox.png",&this->gTexture))))
	{
		 
	}
	
	b_device=d;
	b_fontFamily=font;
	b_fontSize=size;
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
	if(lines[49])
	{
		delete lines[49];
		lines[49]=0;
	}
	for(int i=48;i>=0;i--)
	{
		lines[i+1]=lines[i];
	}
	
	if(s.length()>86)
	{
		string line1="",line2="";
		for(int i=80;i>40;i++)
		{
			if(s[i]==' ')
			{
				line1=s.substr(0,i);
				line2=s.substr(i);
				break;
			}
		}
		if(line1=="")
		{
			line1=s.substr(0,85);
			line2=s.substr(85);
		}
		lines[0]=new GUIText((char*)line1.c_str(),g_font);
		AddLine(line2);
	}
	else
		lines[0]=new GUIText((char*)s.c_str(),g_font);

}

void GUI::AddToQueue(string a)
{
	q.push(a);
}

void GUI::SetWriting(string s)
{
	writingText=s;
}

void GUI::ScrollDown()
{
	scrollOffset--;
	if(scrollOffset<0)
		scrollOffset=0;
}

void GUI::ScrollUp()
{
	scrollOffset++;
	if(scrollOffset>4)
		scrollOffset=4;
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
	D3DXVECTOR2 scaling(sizeMult,sizeMult);
	D3DXVECTOR2 traslation(15,15);
	D3DXMatrixTransformation2D(&mat,NULL,NULL,&scaling,NULL,NULL,&traslation);

	d->SetTransform(D3DTS_WORLD,&mat);

	RECT src_rect;
	SetRect(&src_rect,0,0,625,276);
	sprite->Draw(gTexture,&src_rect,NULL,NULL,0xBBFFFFFF);

	sprite->End();

	//Ok, now i draw the arrows:

	sprite->Begin(D3DXSPRITE_ALPHABLEND);

	D3DXVECTOR2 arrowTraslation(traslation.x+(609*sizeMult),traslation.y+(220*sizeMult));
	D3DXMatrixTransformation2D(&mat,NULL,NULL,&scaling,NULL,NULL,&arrowTraslation);
	d->SetTransform(D3DTS_WORLD,&mat);

	SetRect(&src_rect,640,212,21+640,16+212);
	sprite->Draw(gTexture,&src_rect,NULL,NULL,0xBBFFFFFF);

	sprite->End();

	//Second arrow

	sprite->Begin(D3DXSPRITE_ALPHABLEND);

	arrowTraslation=D3DXVECTOR2(traslation.x+(609*sizeMult),traslation.y+(10*sizeMult));
	D3DXMatrixTransformation2D(&mat,NULL,NULL,&scaling,NULL,NULL,&arrowTraslation);
	d->SetTransform(D3DTS_WORLD,&mat);

	SetRect(&src_rect,640,13,21+640,16+13);
	sprite->Draw(gTexture,&src_rect,NULL,NULL,0xBBFFFFFF);

	sprite->End();

	//Scrollbar

	sprite->Begin(D3DXSPRITE_ALPHABLEND);

	//Top: 28
	//Bottom: 158

	arrowTraslation=D3DXVECTOR2(traslation.x+(614*sizeMult),traslation.y+((158-(32*scrollOffset))*sizeMult));
	D3DXMatrixTransformation2D(&mat,NULL,NULL,&scaling,NULL,NULL,&arrowTraslation);
	d->SetTransform(D3DTS_WORLD,&mat);

	SetRect(&src_rect,661,13,11+661,61+13);
	sprite->Draw(gTexture,&src_rect,NULL,NULL,0xBBFFFFFF);

	sprite->End();

	

	for(int i=0;i<10;i++)
	{
		GUIText *t=lines[i+(scrollOffset*10)];

		/*RECT font_rect;
		SetRect(&font_rect,15+(15*sizeMult),15+(18*sizeMult)+(((9-i)*20)*sizeMult),200,32);
		g_font->DrawText(NULL,t.c_str(),-1,&font_rect,DT_LEFT|DT_NOCLIP,0xFF000000);*/
		if(t)
			t->Draw(15+(15*sizeMult),15+(18*sizeMult)+(((9-i)*20)*sizeMult),sizeMult);
	}

	string t=writingText;

	//Calculate if size > 580
	{

		RECT font_rect;
		SetRect(&font_rect,0,0,0,0);
		g_font->DrawText(NULL,t.c_str(),-1,&font_rect,DT_LEFT|DT_CALCRECT,0xFF000000);
		if(font_rect.right>580)
		{
			RECT font_rect;
			SetRect(&font_rect,15+(15*sizeMult),15+(246*sizeMult),15+(15*sizeMult)+580,15+(246*sizeMult)+32);
			g_font->DrawText(NULL,t.c_str(),-1,&font_rect,DT_RIGHT,0xFF000000);
		}
		else
		{
			RECT font_rect;
			SetRect(&font_rect,15+(15*sizeMult),15+(246*sizeMult),15+(15*sizeMult)+580,15+(246*sizeMult)+32);
			g_font->DrawText(NULL,t.c_str(),-1,&font_rect,DT_LEFT,0xFF000000);
		}
	}
	
	
}

float GUI::GetSize()
{
	return this->sizeMult;
}

void GUI::SetSize(float f)
{
	if(f>0.1&&f<3)
	{
		sizeMult=f;
		g_font->Release();
		D3DXCreateFont(b_device,(b_fontSize*sizeMult),0,700,1,false,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH|FF_DONTCARE,b_fontFamily.c_str(),&g_font);
	}
}