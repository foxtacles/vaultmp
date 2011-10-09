#include "CD3D9Render.h"

extern int iWindowed;

#define CURSOR_COLOR D3DCOLOR_ARGB(255,0,0,0)

#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD_PTR)(ptr) + (DWORD_PTR)(addValue))
#define _SAFE_LOST(x) if(x) x->OnLostDevice()
#define _SAFE_RESE(x) if(x) x->OnResetDevice()
#define _SAFE_RELE(x) if(x) { x->Release(); x = NULL; }

struct Vertex
{
	float X, Y, Z;
	DWORD dColor;
};

const DWORD D3DFVF_TL = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;

CD3DRender::CD3DRender()
{
}

CD3DRender::CD3DRender( LPDIRECT3DDEVICE9 dev )
{
	this->d = dev;
}

CD3DRender::~CD3DRender()
{
}

void CD3DRender::Initialize()
{
	this->GenerateTexture( &this->p, D3DCOLOR_ARGB( 255, 255, 255, 255 ) );
	D3DXCreateSprite( this->d, &this->s );
	D3DXCreateLine( this->d, &this->l );
	this->l->SetWidth( 1 );
	this->l->SetAntialias( true );
	this->LoadFont( this->Normal,			15, FW_NORMAL,	0, "Verdana" );
	this->LoadFont( this->Console,		15, FW_NORMAL,	0, "Courier" );
	this->LoadFont( this->Bold,			15, FW_BOLD,	0, "Verdana" );
	this->LoadFont( this->Big,			23, FW_NORMAL,	0, "Verdana" );
	this->LoadFont( this->Objective,		35, FW_BOLD,	0, "Verdana" );
}

void CD3DRender::Lost()
{
	_SAFE_LOST( this->s );
	_SAFE_LOST( this->l );
	_SAFE_LOST( this->Normal );
	_SAFE_LOST( this->Console );
	_SAFE_LOST( this->Bold );
	_SAFE_LOST( this->Big );
	_SAFE_LOST( this->Objective );
}

void CD3DRender::Reset()
{
	_SAFE_RESE( this->s );
	_SAFE_RESE( this->l );
	_SAFE_RESE( this->Normal );
	_SAFE_RESE( this->Console );
	_SAFE_RESE( this->Bold );
	_SAFE_RESE( this->Big );
	_SAFE_RESE( this->Objective );
}

void CD3DRender::Release()
{
	_SAFE_RELE( this->m );
	_SAFE_RELE( this->p );
	_SAFE_RELE( this->s );
	_SAFE_RELE( this->l );
	_SAFE_RELE( this->Normal );
	_SAFE_RELE( this->Console );
	_SAFE_RELE( this->Bold );
	_SAFE_RELE( this->Big );
	_SAFE_RELE( this->Objective );
}

POINT pPoint;
HWND Win;
RECT ClientRect;
WINDOWPLACEMENT WinPlace;

void CD3DRender::BeginScene()
{
	// update viewpoint
	this->d->GetViewport( &this->viewport );

	// update cursor pos
	GetCursorPos( &pPoint );
	Win = GetForegroundWindow();
	GetClientRect( Win, &ClientRect );

	this->cX = ( float )( pPoint.x - ClientRect.left );
	this->cY = ( float )( pPoint.y - ClientRect.top );

	GetWindowPlacement( Win, &WinPlace );

	this->cX -= ( float )( WinPlace.rcNormalPosition.left );
	this->cY -= ( float )( WinPlace.rcNormalPosition.top );

	if( !iWindowed )
	{
		float fX = ( float )GetSystemMetrics( SM_CXSCREEN ) / ( float )this->viewport.Width;
		float fY = ( float )GetSystemMetrics( SM_CYSCREEN ) / ( float )this->viewport.Height;

		this->cX *= fX;
		this->cY *= fY;
	}

	// update cursor state
	if( GetAsyncKeyState( VK_LBUTTON ) )
	{
		if( this->cState == CS_CLICKED )
			this->cState = CS_HELD;

		else if( this->cState == CS_UP )
			this->cState = CS_CLICKED;
	}

	else
	{
		this->cState = CS_UP;
	}
}

//-----------------------2D
//text
HRESULT CD3DRender::LoadFont( ID3DXFont *&Font, int Height, UINT Weight, BOOL Italic, LPCSTR FontName )
{
	return D3DXCreateFontA( this->d, Height, 0, Weight, 1, Italic, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, FontName, &Font );
}

int CD3DRender::GetTextLen( LPCSTR tx, ID3DXFont* font )
{
	RECT oRect = { 0, 0, 500, 500 };
	font->DrawTextA( NULL, tx, -1, &oRect, DT_CALCRECT, NULL );
	return oRect.right;
}

int CD3DRender::GetTextHeight( LPCSTR tx, ID3DXFont* font ) // doesn't support word wrapping in most cases
{
	RECT oRect = { 0, 0, 500, 500 };
	font->DrawTextA( NULL, tx, -1, &oRect, DT_CALCRECT, NULL );
	return oRect.bottom;
}

void CD3DRender::CalcFontRect( LPCSTR tx, ID3DXFont* font, RECT* rect )
{
	font->DrawTextA( NULL, tx, -1, rect, DT_CALCRECT, NULL );
}

void CD3DRender::Text_Begin()
{
	this->s->Begin( D3DXSPRITE_ALPHABLEND );
}

void CD3DRender::Text_End()
{
	this->s->End();
}

void CD3DRender::DrawTextMain( float X, float Y, ID3DXFont *Font, LPCSTR TextString, DWORD TextColor )
{
	RECT FontPos = { ( long )X, ( long )Y, ( long )X + 300, ( long )Y + 36 };
	Font->DrawTextA( this->s, TextString, -1, &FontPos, DT_NOCLIP, TextColor );
}

void CD3DRender::DrawTextRect( RECT* rect, ID3DXFont *Font, LPCSTR TextString, DWORD TextColor )
{
	Font->DrawTextA( this->s, TextString, -1, rect, DT_WORDBREAK, TextColor );
}

int CD3DRender::GetTextRect( RECT* rect, ID3DXFont *Font, LPCSTR TextString )
{
	return Font->DrawTextA( NULL, TextString, -1, rect, DT_CALCRECT | DT_WORDBREAK, NULL );
}

void CD3DRender::DrawTextOutline( float X, float Y, ID3DXFont *Font, LPCSTR TextString, DWORD TextColor, DWORD strokeColor )
{
	// TODO: efficient outlined text rendering
	this->DrawTextShadow( X, Y, Font, TextString, TextColor, strokeColor );
}

void CD3DRender::DrawTextShadow( float X, float Y, ID3DXFont *Font, LPCSTR TextString, DWORD TextColor, DWORD shadowColor )
{
	RECT sPos = { ( long )X + 1, ( long )Y + 1, ( long )X + 300, ( long )Y + 36 };
	Font->DrawTextA( this->s, TextString, -1, &sPos, DT_NOCLIP, shadowColor );

	sPos.left--;
	sPos.top--;
	Font->DrawTextA( this->s, TextString, -1, &sPos, DT_NOCLIP, TextColor );
}

void CD3DRender::Shape_Begin()
{
	this->d->GetFVF( &this->oFVF );
	this->d->GetTexture( 0, &this->oTx );

	this->s->Flush();

	this->d->SetFVF( D3DFVF_TL );
	this->d->SetTexture( 0, this->p );
}

void CD3DRender::Shape_End()
{
	this->d->SetFVF( this->oFVF );
	this->d->SetTexture( 0, this->oTx );
}

void CD3DRender::LineEx_Begin()
{
	this->l->Begin();
}

void CD3DRender::LineEx_End()
{
	this->l->End();
}

void CD3DRender::Draw2DTriangle( float vert1x, float vert1y, float vert2x, float vert2y, float vert3x, float vert3y, DWORD color )
{
	Vertex VertexList[4] =
	{
		{ vert1x, vert1y, 0.0f, color },
		{ vert2x, vert2y, 0.0f, color },
		{ vert3x, vert3y, 0.0f, color },
	};

	this->d->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 1, VertexList, sizeof( Vertex ) );
}

void CD3DRender::Draw2DLine( float x1, float y1, float x2, float y2, DWORD color )
{
	Vertex VertexList[4] =
	{
		{ x1, y1, 0.0f, color },
		{ x1 + 1.0f, y1, 0.0f, color },
		{ x2, y2, 0.0f, color },
		{ x2 + 1.0f, y2, 0.0f, color },
	};

	this->d->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, VertexList, sizeof( Vertex ) );
}

void CD3DRender::Draw2DLineEx( float x1, float y1, float x2, float y2, DWORD color ) // antialiased
{
	D3DXVECTOR2 vLine[ 2 ];
	vLine[0][0] = x1;
	vLine[0][1] = y1;
	vLine[1][0] = x2;
	vLine[1][1] = y2;
	this->l->Draw( vLine, 2, color );
}

void CD3DRender::Draw2DGradientLine( float x1, float y1, float x2, float y2, DWORD color, DWORD color2 )
{
	Vertex VertexList[4] =
	{
		{ x1, y1, 0.0f, color },
		{ x1 + 1.0f, y1, 0.0f, color },
		{ x2, y2, 0.0f, color2 },
		{ x2 + 1.0f, y2 + 1.0f, 0.0f, color2 },
	};

	this->d->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, VertexList, sizeof( Vertex ) );
}

void CD3DRender::DrawSquare( float x, float y, float w, float h, DWORD color )
{
	Vertex VertexList[4] =
	{
		{ x, y + h, 0.0f, color },
		{ x, y, 0.0f, color },
		{ x + w, y + h, 0.0f, color },
		{ x + w , y, 0.0f, color },
	};

	this->d->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, VertexList, sizeof( Vertex ) );
}

void CD3DRender::DrawGradientSquare( float x, float y, float w, float h, DWORD color, DWORD color2 )
{
	Vertex VertexList[4] =
	{
		{ x, y + h, 0.0f, color2 },
		{ x, y, 0.0f, color },
		{ x + w, y + h, 0.0f, color2 },
		{ x + w , y, 0.0f, color },
	};

	this->d->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, VertexList, sizeof( Vertex ) );
}

void CD3DRender::DrawSquareEdges( float x, float y, float w, float h, DWORD color )
{
	this->DrawSquare( x, y, 1, h + 1, color ); //left
	this->DrawSquare( x + 1, y, w - 1, 1, color ); //top
	this->DrawSquare( x + w, y, 1, h, color ); //right
	this->DrawSquare( x, y + h, w + 1, 1, color ); //bottom
}

void CD3DRender::DrawCross( float x, float y, float w, float h, float s, DWORD color ) //xpos,ypos,width,height,spread
{
	this->DrawSquare( x - s, y - ( h / 2.0f ), w, h, color ); //left
	this->DrawSquare( x - ( h / 2.0f ), y - s, h, w, color ); //top
	this->DrawSquare( x + s - w - 1, y - ( h / 2.0f ), w, h, color ); //right
	this->DrawSquare( x - ( h / 2.0f ), y + s - w - 1, h, w, color ); //bottom
}

void CD3DRender::DrawXCross( float x, float y, float w, float h, float s, DWORD color ) //xpos,ypos,width,height,spread
{
	this->Draw2DLine( x - s, y - s, x - s + w, y - s + h, color ); //top-left
	this->Draw2DLine( x + s/*-1*/, y - s, x + s - w/*-1*/, y - s + h, color ); //top-right
	this->Draw2DLine( x - s - 1, y + s + 1, x - s + w, y + s - h, color ); //bottom-left
	this->Draw2DLine( x + s, y + s + 1, x + s - w, y + s - h, color ); //bottom-right
}

void CD3DRender::DrawC2Cross( float x, float y, float w, float h, float s, DWORD color ) //xpos,ypos,width,height,spread
{
	this->DrawSquare( x - 1, y - s, 1, h, color ); //top
	this->Draw2DLine( x - s, y + s, x - s + w, y + s - h, color ); //bottom-left
	this->Draw2DLine( x + s, y + s, x + s - w - 1, y + s - h, color ); //bottom-right
}

void CD3DRender::DrawCursor()
{
	// TODO: fix this?
	this->DrawSquare( this->cX, this->cY, 8, 1, CURSOR_COLOR );
	this->DrawSquare( this->cX, this->cY, 1, 8, CURSOR_COLOR );
	this->Draw2DLine( this->cX, this->cY, this->cX + 100, this->cY + 100, CURSOR_COLOR );
}

int CD3DRender::IsCursorIn( float x, float y, float w, float h )
{
	if( this->cX >= x &&
			this->cY >= y &&
			this->cX <= x + w &&
			this->cY <= y + h )
		return 1;

	else
		return 0;
}

int CD3DRender::IsVisible( float x, float y )
{
	if( x >= 0 && y >= 0 &&
			( DWORD )x <= this->viewport.Width &&
			( DWORD )y <= this->viewport.Height )
		return 1;

	else
		return 0;
}

//void DrawCircle
//void DrawFillCircle

//-----------------------3D
void CD3DRender::Draw3DLine( float x1, float y1, float z1, float x2, float y2, float z2, DWORD color ) // UNTESTED
{
	Vertex VertexList[4] =
	{
		{ x1, y1, z1, color },
		{ x1 + 1.0f, y1, z1, color },
		{ x2, y2, z2, color },
		{ x2 + 1.0f, y2, z2, color },
	};

	this->d->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, VertexList, sizeof( Vertex ) );
}

void CD3DRender::Draw3DGradientLine( float x1, float y1, float z1, float x2, float y2, float z2, DWORD color, DWORD color2 ) // UNTESTED
{
	Vertex VertexList[4] =
	{
		{ x1, y1, z1, color },
		{ x1 + 1.0f, y1, z1, color },
		{ x2, y2, z2, color2 },
		{ x2 + 1.0f, y2, z2, color2 },
	};

	this->d->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, VertexList, sizeof( Vertex ) );
}

void CD3DRender::Draw3DLineEx( float x1, float y1, float z1, float x2, float y2, float z2, DWORD color )
{
	D3DXVECTOR2 vLine[ 2 ];
	vLine[0][0] = x1;
	vLine[0][1] = y1;
	vLine[0][2] = z1;
	vLine[1][0] = x2;
	vLine[1][1] = y2;
	vLine[1][2] = z2;
	this->l->Draw( vLine, 2, color );
}

void CD3DRender::DrawSphere( float x, float y, float z )
{
	// TODO: learn how to modify the vertex buffer of the mesh realtime
	// OR, find the CreateSphere function on the internet and use that
}

//-----------------------internal
HRESULT CD3DRender::GenerateTexture( IDirect3DTexture9 **ppD3dTex, DWORD Colour32 ) // by azorbix
{
	D3DLOCKED_RECT d3dlr;
	WORD *pDst16 = NULL;

	if( FAILED( this->d->CreateTexture( 8, 8, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, ppD3dTex, NULL ) ) )
		return E_FAIL;

	WORD colour16 =	( ( WORD )( ( Colour32 >> 28 ) & 0xF ) << 12 )
					| ( WORD )( ( ( Colour32 >> 20 ) & 0xF ) << 8 )
					| ( WORD )( ( ( Colour32 >> 12 ) & 0xF ) << 4 )
					| ( WORD )( ( ( Colour32 >> 4 ) & 0xF ) << 0 );

	( *ppD3dTex )->LockRect( 0, &d3dlr, 0, 0 );
	pDst16 = ( WORD* )d3dlr.pBits;

	for( int xy = 0; xy < 8 * 8; xy++ )
		*pDst16++ = colour16;

	( *ppD3dTex )->UnlockRect( 0 );

	return S_OK;
}
