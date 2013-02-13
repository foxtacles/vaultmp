#include <iostream>

#include "common.h"

#include "HookedTextures\Text_Loading.h"
#include "TextureHooking.h"

namespace TextureHooking
{
	vector<registeredTexture> textures;

	char* loadingScreen="";

	void hookTextureIfNecessary(char* path,char** data,int* size)
	{
		if(!path)
			return;
		//textures\interface\loading\loading_screen
		if(strnicmp(path,"textures\\interface\\loading\\loading_screen",strlen("textures\\interface\\loading\\loading_screen"))==0)
		{
			(*data)=(char*)loading_dds;
			(*size)=loading_dds_size;
			SendToLog("Hooked!");
		}

		if(strnicmp(path,"textures\\interface\\loading\\loading_background",strlen("textures\\interface\\loading\\loading_background"))==0)
		{
			(*data)=(char*)loading_dds;
			(*size)=loading_dds_size;
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