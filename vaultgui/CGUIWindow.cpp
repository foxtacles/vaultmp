#include "CGUIWindow.h"

CWindow::CWindow()
{
}

CWindow::CWindow( float x, float y, float w, float h, CD3DRender* r, char* wtitle )
{
	this->dr = r;
	this->dragging = 0;
	this->nextTabX = 2;
	this->maxTabs = 0;
	this->title = wtitle;
	this->curTab = 0;
	this->state = cWS_VISIBLE;
	ZeroMemory( this->objects, sizeof( this->objects ) );
	this->Add( x, y, w, h );
}

CWindow::~CWindow()
{
}

void CWindow::Add( float x, float y, float w, float h )
{
	this->wx = x;
	this->wy = y;
	this->ww = w;
	this->wh = h;
}

#define OUTLINE_C D3DCOLOR_ARGB(220,10,200,10)
#define TITLE_C D3DCOLOR_ARGB(255,30,255,30)
#define TBG_H 25

void CWindow::Render()
{
	if( this->dr->cState == CS_CLICKED )
	{
		if( this->dr->IsCursorIn( this->wx + this->ww - 19, this->wy + 5, 16, 16 ) )
		{
			if( this->state == cWS_MINI )
				this->state = cWS_VISIBLE;

			else
				this->state = cWS_MINI;
		}

		else
		{
			if( this->state != cWS_MINI )
			{
				//check if it's any of this window's objects
				for( int i = 0; i < 64; i++ )
				{
					if( this->objects[i] != NULL )
					{
						if( this->objects[i]->MouseEvent( this->wx, this->wy ) )
							break;
					}

					else
					{
						break;
					}
				}
			}

			//check if the event is to move this window
			if( this->dr->IsCursorIn( this->wx, this->wy, this->ww, 26 ) )
			{
				this->dragging = 1;
			}
		}
	}

	else if( this->dr->cState == CS_HELD )
	{
	}
	else if( this->dr->cState == CS_UP )
	{
		this->dragging = 0;
	}

	if( this->dragging )
	{
		this->wx = this->dr->cX;
		this->wy = this->dr->cY;

		if( this->wx < ( float )this->dr->viewport.X )
			this->wx = ( float )this->dr->viewport.X;

		if( this->wy < ( float )this->dr->viewport.Y )
			this->wy = ( float )this->dr->viewport.Y;

		float maxx = ( float )( this->dr->viewport.X + this->dr->viewport.Width ) - this->ww - 1, maxy;

		if( this->state != cWS_MINI )
			maxy = ( float )( this->dr->viewport.Y + this->dr->viewport.Height ) - this->wh - 1;

		else
			maxy = ( float )( this->dr->viewport.Y + this->dr->viewport.Height ) - TBG_H - 2;

		if( this->wx > maxx )
			this->wx = maxx;

		if( this->wy > maxy )
			this->wy = maxy;
	}

	if( this->state != cWS_MINI )
		this->dr->DrawSquareEdges( this->wx, this->wy, this->ww, this->wh, OUTLINE_C ); // outline

	else
		this->dr->DrawSquareEdges( this->wx, this->wy, this->ww, TBG_H + 1, OUTLINE_C ); // title outline

	this->dr->DrawGradientSquare( this->wx + 1, this->wy + 1, this->ww - 1, TBG_H, D3DCOLOR_ARGB( 150, 30, 30, 30 ), D3DCOLOR_ARGB( 150, 5, 5, 5 ) ); // title background

	if( this->state != cWS_MINI )
	{
		this->dr->DrawSquare( this->wx + 1, this->wy + TBG_H + 1, this->ww - 1, 1, OUTLINE_C ); // title seperator
		this->dr->DrawSquare( this->wx + 1, this->wy + TBG_H + 2, this->ww - 1, this->wh - TBG_H - 2, D3DCOLOR_ARGB( 120, 0, 0, 0 ) ); // main background
	}

	this->dr->DrawSquareEdges( this->wx + this->ww - 19, this->wy + 5, 16, 16, OUTLINE_C );
	this->dr->DrawSquare( this->wx + this->ww - 15, this->wy + 13, 9, 1, OUTLINE_C );

	if( this->state != cWS_MINI )
	{
		for( int i = 0; i < 64; i++ )
		{
			if( this->objects[i] != NULL )
			{
				this->objects[i]->RenderShape( this->wx, this->wy );
			}

			else
			{
				break;
			}
		}
	}

	else
	{
		this->dr->DrawSquare( this->wx + this->ww - 11, this->wy + 9, 1, 9, OUTLINE_C );
	}

	RECT tit = {( long )this->wx + 1, ( long )this->wy + 4, ( long )( this->ww + this->wx ) - 2, ( long )( this->wy + 30 )};
	this->dr->Normal->DrawTextA( this->dr->s, this->title, -1, &tit, DT_CENTER, TITLE_C );

	if( this->state != cWS_MINI )
	{
		for( int i = 0; i < 64; i++ )
			if( this->objects[i] != NULL )
				this->objects[i]->RenderText( this->wx, this->wy );

			else
				break;
	}
}

void CWindow::AddCheckbox( float x, float y, char* text, int* var, int mtab )
{
	int oo = -1;

	for( int i = 0; i < 64; i++ )
	{
		if( this->objects[i] == NULL )
		{
			oo = i;
			break;
		}
	}

	if( oo == -1 )
		return;

	this->objects[oo] = new CObject( x, y, 0, 0, text );
	this->objects[oo]->r = this->dr;
	this->objects[oo]->tabid = mtab;
	this->objects[oo]->tab = &this->curTab;
	this->objects[oo]->Checkbox( var );
}

void CWindow::AddTab( char* text )
{
	int oo = -1;

	for( int i = 0; i < 64; i++ )
	{
		if( this->objects[i] == NULL )
		{
			oo = i;
			break;
		}
	}

	if( oo == -1 )
		return;

	float tx;

	if( this->maxTabs == 0 )
		tx = 1;

	else
		tx = this->nextTabX;

	//int ttabid = this->maxTabs;
	float tw = this->dr->GetTextLen( text, this->dr->Normal );
	tw += 6;
	this->nextTabX = tx + tw;
	this->objects[oo] = new CObject( tx + ( 3 ), 28, tw, 20, text );
	this->objects[oo]->r = this->dr;
	this->objects[oo]->tabid = this->maxTabs;
	this->objects[oo]->tab = &this->curTab;
	this->objects[oo]->Tab();
	this->maxTabs++;
}

void CWindow::AddSlider( float x, float y, float w, char* text, float omax, float omin, float* ocur, int mtab )
{
	int oo = -1;

	for( int i = 0; i < 64; i++ )
	{
		if( this->objects[i] == NULL )
		{
			oo = i;
			break;
		}
	}

	if( oo == -1 )
		return;

	this->objects[oo] = new CObject( x, y, w, 0, text );
	this->objects[oo]->r = this->dr;
	this->objects[oo]->tabid = mtab;
	this->objects[oo]->tab = &this->curTab;
	this->objects[oo]->Slider( omax, omin, ocur );
}

void CWindow::AddTextbox( float x, float y, float w, float h, char* text, int mtab )
{
	int oo = -1;

	for( int i = 0; i < 64; i++ )
	{
		if( this->objects[i] == NULL )
		{
			oo = i;
			break;
		}
	}

	if( oo == -1 )
		return;

	this->objects[oo] = new CObject( x, y, w, h, text );
	this->objects[oo]->r = this->dr;
	this->objects[oo]->tabid = mtab;
	this->objects[oo]->tab = &this->curTab;
	this->objects[oo]->Textbox();
}
