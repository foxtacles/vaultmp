#pragma once
#include <windows.h>
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>

enum curstates_e
{
	CS_UP,
	CS_CLICKED,
	CS_HELD
};

class CD3DRender
{
	public:
		//colors by chaser343
		static const DWORD			Black				= D3DCOLOR_ARGB( 255, 0, 0, 0 );
		static const DWORD			White				= D3DCOLOR_ARGB( 255, 255, 255, 255 );
		static const DWORD			Red					= D3DCOLOR_ARGB( 255, 255, 0, 0 );
		static const DWORD			Green				= D3DCOLOR_ARGB( 255, 0, 255, 0 );
		static const DWORD			Blue				= D3DCOLOR_ARGB( 255, 0, 0, 255 );
		static const DWORD			Pink				= D3DCOLOR_ARGB( 255, 255, 0, 255 );
		static const DWORD			Orange				= D3DCOLOR_ARGB( 255, 255, 106, 0 );
		static const DWORD			Yellow				= D3DCOLOR_ARGB( 255, 255, 255, 0 );
		static const DWORD			Cyan				= D3DCOLOR_ARGB( 255, 0, 255, 255 );
		static const DWORD			Lime				= D3DCOLOR_ARGB( 255, 182, 255, 0 );
		static const DWORD			LightGreen			= D3DCOLOR_ARGB( 255, 76, 255, 0 );
		static const DWORD			DarkGreen			= D3DCOLOR_ARGB( 255, 0, 127, 14 );
		static const DWORD			LightBlue			= D3DCOLOR_ARGB( 255, 0, 148, 255 );
		static const DWORD			DarkBlue			= D3DCOLOR_ARGB( 255, 0, 19, 127 );
		static const DWORD			LightPink			= D3DCOLOR_ARGB( 255, 178, 0, 255 );
		static const DWORD			DarkPink			= D3DCOLOR_ARGB( 255, 255, 0, 110 );
		static const DWORD			DarkGrey			= D3DCOLOR_ARGB( 255, 51, 51, 51 );
		static const DWORD			Grey				= D3DCOLOR_ARGB( 255, 64, 64, 64 );

		static const DWORD			Easy_Red			= D3DCOLOR_ARGB( 255, 255, 127, 127 );
		static const DWORD			Easy_Green			= D3DCOLOR_ARGB( 255, 127, 255, 127 );
		static const DWORD			Easy_Blue			= D3DCOLOR_ARGB( 255, 127, 127, 255 );
		static const DWORD			Easy_Pink			= D3DCOLOR_ARGB( 255, 255, 127, 255 );
		static const DWORD			Easy_Orange			= D3DCOLOR_ARGB( 255, 255, 141, 66 );
		static const DWORD			Easy_Yellow			= D3DCOLOR_ARGB( 255, 255, 255, 127 );
		static const DWORD			Easy_Cyan			= D3DCOLOR_ARGB( 255, 127, 255, 255 );
		static const DWORD			Easy_Lime			= D3DCOLOR_ARGB( 255, 182, 255, 127 );
		static const DWORD			Easy_Grey			= D3DCOLOR_ARGB( 255, 84, 84, 84 );

		CD3DRender();
		CD3DRender( LPDIRECT3DDEVICE9 dev );
		~CD3DRender();

		void Initialize();
		void Lost();
		void Reset();
		void Release();

		void BeginScene();

		//2D
		//text
		HRESULT LoadFont( ID3DXFont *&Font, int Height, UINT Weight, BOOL Italic, LPCSTR FontName );
		void Text_Begin();
		int GetTextLen( LPCSTR tx, ID3DXFont* font );
		int GetTextHeight( LPCSTR tx, ID3DXFont* font ); // doesn't support word wrapping in most cases
		void CalcFontRect( LPCSTR tx, ID3DXFont* font, RECT* rect ); // you need to fill rect before this
		void DrawTextMain( float X, float Y, ID3DXFont *Font, LPCSTR TextString, DWORD TextColor );
		void DrawTextRect( RECT* rect, ID3DXFont *Font, LPCSTR TextString, DWORD TextColor );
		int GetTextRect( RECT* rect, ID3DXFont *Font, LPCSTR TextString );
		void DrawTextOutline( float X, float Y, ID3DXFont *Font, LPCSTR TextString, DWORD TextColor, DWORD strokeColor );
		void DrawTextShadow( float X, float Y, ID3DXFont *Font, LPCSTR TextString, DWORD TextColor, DWORD shadowColor );
		void Text_End();

		void LineEx_Begin();
		void Draw2DLineEx( float x1, float y1, float x2, float y2, DWORD color ); // antialiased
		void Draw3DLineEx( float x1, float y1, float z1, float x2, float y2, float z2, DWORD color ); // antialiased
		void LineEx_End();

		void Shape_Begin();
		void Draw2DLine( float x1, float y1, float x2, float y2, DWORD color );
		void Draw2DTriangle( float vert1x, float vert1y, float vert2x, float vert2y, float vert3x, float vert3y, DWORD color );
		void Draw2DGradientLine( float x1, float y1, float x2, float y2, DWORD color, DWORD color2 );
		void Draw3DLine( float x1, float y1, float z1, float x2, float y2, float z2, DWORD color );
		void Draw3DGradientLine( float x1, float y1, float z1, float x2, float y2, float z2, DWORD color, DWORD color2 );
		void DrawSquare( float x, float y, float w, float h, DWORD color );
		void DrawSquareEdges( float x, float y, float w, float h, DWORD color );
		void DrawGradientSquare( float x, float y, float w, float h, DWORD color, DWORD color2 );
		void DrawCross( float x, float y, float w, float h, float s, DWORD color ); //xpos,ypos,width,height,spread
		void DrawXCross( float x, float y, float w, float h, float s, DWORD color ); //xpos,ypos,width,height,spread
		void DrawC2Cross( float x, float y, float w, float h, float s, DWORD color ); //xpos,ypos,width,height,spread
		void DrawCursor();
		int IsCursorIn( float x, float y, float w, float h );
		int IsVisible( float x, float y );
		void Shape_End();
		//void DrawCircle
		//void DrawFillCircle

		//3D
		void DrawSphere( float x, float y, float z );
		//void DrawSphereWireframe
		//void Draw3DBoxLines(float x, float y, float z, float w, float h, float d, DWORD color);

		//internal
		HRESULT GenerateTexture( IDirect3DTexture9 **ppD3dTex, DWORD Colour32 );


		IDirect3DDevice9* d;
		D3DVIEWPORT9 viewport;
		IDirect3DTexture9* p;
		ID3DXLine* l;
		ID3DXSprite* s;
		ID3DXMesh* m;
		ID3DXFont* Normal;
		ID3DXFont* Console;
		ID3DXFont* Bold;
		ID3DXFont* Big;
		ID3DXFont* Objective;
		IDirect3DBaseTexture9* oTx;
		DWORD oFVF;
		int cState;// cursor state
		float cX;
		float cY;
};
