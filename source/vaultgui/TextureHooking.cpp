#include <iostream>

#include "common.h"

#include "HookedTextures\Text_Loading.h"
#include "HookedTextures\Text_Loading_2.h"
#include "TextureHooking.h"

namespace TextureHooking
{
	vector<registeredTexture> textures;

	char* loadingScreen="";

	void hookTextureIfNecessary(char* path,char** data,int* size)
	{
		static int textureHookCount=0;
		if(!path)
			return;
		//textures\interface\loading\loading_screen
		if(strnicmp(path,"textures\\interface\\loading\\loading_screen",strlen("textures\\interface\\loading\\loading_screen"))==0)
		{
			if(textureHookCount%2==1)
			{
				(*data)=(char*)loading_dds;
				(*size)=loading_dds_size;
			}
			else
			{
				(*data)=(char*)loading2_dds;
				(*size)=loading2_dds_size;
			}
			SendToLog("Hooked!");
			textureHookCount++;
		}

		if(strnicmp(path,"textures\\interface\\loading\\loading_background",strlen("textures\\interface\\loading\\loading_background"))==0)
		{
			if(textureHookCount%2==1)
			{
				(*data)=(char*)loading_dds;
				(*size)=loading_dds_size;
			}
			else
			{
				(*data)=(char*)loading2_dds;
				(*size)=loading2_dds_size;
			}
			textureHookCount++;
			SendToLog("Hooked!");
		}
		
		
	}

	void registerTexture(char* path,void* ptr)
	{
		registeredTexture tmp;
		tmp.path=path;
		tmp.ptr=ptr;
		textures.push_back(tmp);
		
		if(strnicmp(path,"Data\\Textures\\interface\\icons\\typeicons\\hot_keys_icon_ammoswap.dds",strlen("Data\\Textures\\interface\\icons\\typeicons\\hot_keys_icon_ammoswap.dds"))==0)
		{
			//exit(0);
			/*unsigned char * pt=(unsigned char*)0x61E77;
			pt[0]=0;*/
			gData.gameReady=true;
			//Tell Vaultmp to start
		}
	}
};