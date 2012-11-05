#include <iostream>
#include <vector>
#include <d3dx9.h>
#include <fstream>

#include "common.h"

#define DEBUG(a) 	{std::ofstream d;d.open("C:\\Users\\PC\\Desktop\\debug.txt",std::ios::app);  d<<a<<std::endl;d.flush();d.close();}

using namespace std;

struct GUIColorChunk
{
	unsigned int start,end;
	unsigned int color;
	int offsetX,offsetY;
};

class GUIText
{
	char str[200];
	vector<GUIColorChunk> textChunks;
	ID3DXFont* font;

	string overflowingText;
public:
	GUIText(char*,ID3DXFont*);
	~GUIText();
	void Draw( int xOff , int yOff , float mul );
};