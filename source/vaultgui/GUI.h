#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <queue>
#include "GUIText.h"

using namespace std;

class GUI
{
public:
	ID3DXFont* g_font;
private:
	GUIText* lines[50];
	string writingText;
	
	LPD3DXSPRITE sprite;
	LPDIRECT3DTEXTURE9 gTexture;
	bool visible;
	bool locked;
	float sizeMult;
	int scrollOffset;

	IDirect3DDevice9 *b_device;
	int b_fontSize;
	string b_fontFamily;
	
public:
	queue <string> q;
	GUI();
	~GUI();
	void Init(IDirect3DDevice9*,int size=12,string font="Arial");
	void Release();
	void OnReset();
	void Show(bool);
	void AddLine(string);
	void AddToQueue(string);
	void SetWriting(string);
	void ScrollDown();
	void ScrollUp();
	float GetSize();
	void SetSize(float);

	void Think();

	void Lock();
	void Unlock();

	void Draw(IDirect3DDevice9*);
};