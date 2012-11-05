#include "GUIText.h"

GUIText::GUIText( char* s , ID3DXFont* f )
{
	static char* colorsName[]={"red","blue","green","pink","black","white"};
	static int colorsHex[]={0xFFFF0000,0xFF0000FF,0xFF00FF00,0xFFFF6EC7,0xFFFFFFFF,0xFF000000};

	static int predefinedColorsHex[]={0xFFFF0000,0xFF0000FF,0xFF00FF00,0xFFFF6EC7,0xFFFFFFFF,0xFF000000};

	int lastIndex=0;

	font=f;
	if(strlen(s)>=sizeof(str))
	{
		//Error!!
		strncpy(str,s,sizeof(str)-5);
		strcat(str,"[..]");
	}
	else
		strcpy(str,s);

	for(int i=0;i<strlen(str);i++)
	{
		if(str[i]=='$')
		{
			bool done=false;
			for(int k=0;k<(sizeof(colorsName)/sizeof(char*));k++)
			{
				if(strncmp((str+i+1),colorsName[k],strlen(colorsName[k]))==0)
				{
					//Color found
					GUIColorChunk tmp;
					tmp.color=colorsHex[k];
					tmp.start=lastIndex;
					tmp.end=i;
					lastIndex=strlen(colorsName[k])+1+i;
					done=true;
					textChunks.push_back((tmp));
					break;
				}
			}
			if(done)
			{

			}
			else
				if(strlen(str+i)>2&&IsHex(str[i+1])&&!IsHex(str[i+2]))
				{
					//Predefined colors (1,2,3 etc.)
					GUIColorChunk tmp;
					tmp.color=predefinedColorsHex[str[i+1]-'0'];
					tmp.start=lastIndex;
					tmp.end=i;
					lastIndex=i+1+1;
					textChunks.push_back((tmp));
				}
			else
				if(strlen(str+i)>3&&IsHex(str[i+1])&&IsHex(str[i+2])&&IsHex(str[i+3]))
				{
					//3 byte hex colors
					int color=0xFF000000;
					for(int k=0;k<3;k++)
					{
						char c=str[i+k+1];
						int val=0;
						if(c>='0'&&c<='9')
							val=c-'0';
						else if(c>='A'&&c<='F')
						{
							val=c-'A'+10;
						}
						else if(c>='a'&&c<='f')
						{
							val=c-'a'+10;
						}
						val=val+(val<<4);

						color|=(val<<((2-k)*8));
					}
					
					GUIColorChunk tmp;
					tmp.color=color;
					tmp.start=lastIndex;
					tmp.end=i;
					lastIndex=i+4;
					textChunks.push_back((tmp));
				}
		}
	}
	{
		GUIColorChunk tmp;
		tmp.color=0xFFFFFFFF;
		tmp.start=lastIndex;
		tmp.end=strlen(str);
		textChunks.push_back((tmp));
	}
	/*
	1 - Shift colors right by 1
	*/
	for(int i=textChunks.size()-2;i>=0;i--)
	{
		textChunks[i+1].color=textChunks[i].color;
	}
	/*
	2 - Calculate offsets
	*/
	for(int i=0;i<textChunks.size();i++)
	{
		char tmp[512];
		memset(tmp,0,sizeof(tmp));
		RECT font_rect;
		font_rect.left=0;
		font_rect.right=500;
		font_rect.top=0;
		font_rect.bottom=200;


		
		if(i>0)
		{
			strncpy(tmp,str+textChunks[i-1].start,textChunks[i-1].end-textChunks[i-1].start);
			if(tmp[textChunks[i-1].end-textChunks[i-1].start-1]==' ')
			{
				tmp[textChunks[i-1].end-textChunks[i-1].start-1]='_';
				font->DrawTextA(NULL,tmp,-1,&font_rect,DT_CALCRECT|DT_LEFT|DT_TOP,textChunks[i].color);
				tmp[textChunks[i-1].end-textChunks[i-1].start-1]=' ';
			}
			else
				font->DrawTextA(NULL,tmp,-1,&font_rect,DT_CALCRECT|DT_LEFT|DT_TOP,textChunks[i].color);

			textChunks[i].offsetX=textChunks[i-1].offsetX+font_rect.right;
		}
		else
		{
			
			textChunks[i].color=0xFF000000;
			textChunks[i].offsetX=0;
		}

		//TODO:Calculate top offset for line breaks
		textChunks[i].offsetY=0;
	}

	DEBUG("TextChunks:"<<textChunks.size()<<"("<<textChunks[0].start<<" - "<<textChunks[0].end<<")")
}

GUIText::~GUIText()
{

}

void GUIText::Draw( int xOff , int yOff , float mul )
{
	char tmp[512];
	for(int i=0;i<textChunks.size();i++)
	{
		memset(tmp,0,sizeof(tmp));
		strncpy(tmp,str+textChunks[i].start,textChunks[i].end-textChunks[i].start);
		DEBUG(xOff+textChunks[i].offsetX)
		RECT font_rect;
		SetRect(&font_rect,xOff+(textChunks[i].offsetX*mul),yOff+(textChunks[i].offsetY*mul),200,32);
		font->DrawTextA(NULL,tmp,-1,&font_rect,DT_LEFT|DT_NOCLIP,textChunks[i].color);
	}
}

