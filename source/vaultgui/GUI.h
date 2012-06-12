#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <queue>

using namespace std;

class GUI
{
private:
	string lines[10];
	string writingText;
	ID3DXFont* g_font;
	LPD3DXSPRITE sprite;
	LPDIRECT3DTEXTURE9 gTexture;
	bool visible;
	bool locked;
	
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

	void Think();

	void Lock();
	void Unlock();

	void Draw(IDirect3DDevice9*);
};