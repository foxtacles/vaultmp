#pragma once

namespace TextureHooking
{
	struct registeredTexture
	{
		string path;
		void* ptr;
	};

	extern vector<registeredTexture> textures;

	void hookTextureIfNecessary(char* path,char** data,int* size);
	void registerTexture(char* path,void* ptr);
};