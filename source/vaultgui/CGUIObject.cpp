#include "CGUIObject.h"
#include "CGUISettings.h"

CObject::CObject( float ox, float oy, float ow, float oh, char* otext ) // generic constructor
{
	this->x = ox;
	this->y = oy;
	this->w = ow;
	this->h = oh;
	this->tabid = 0;
	this->isDragging = 0;
	this->text = otext;
	this->type = 0;

	return;
}

float GetRealValue( DWORD min, DWORD cur, DWORD max, float fmin, float fmax )
{
	float fR = fmax - fmin;
	float dR = ( float )min - ( float )max;
	float dD = ( float )( cur - min );
	float dP = dD / dR;
	float fV = fR * dP;

	return fmin - fV;
}

DWORD GetSliderValue( DWORD min, DWORD max, float cur, float fmin, float fmax )
{
	float dR = ( float )max - ( float )min;
	float fR = fmax - fmin;
	float fP = ( cur - fmin ) / fR;
	float dV = dR * fP;

	return min + ( DWORD )dV;
}

CObject::~CObject()
{
}

#define CHECKBOX_SIZE 15
#define OOUTLINE_COL D3DCOLOR_ARGB(100,10,130,10)
#define OFILL_COL D3DCOLOR_ARGB(100,20,20,20)
#define ODATA_COL D3DCOLOR_ARGB(255,50,210,50)
#define OARROW_COL D3DCOLOR_ARGB(100,50,210,50)
#define ODATASEL_COL D3DCOLOR_ARGB(255,150,255,150)
#define ODATA_HIGHLIGHT_COL D3DCOLOR_ARGB(40, 30, 30, 30)

//initializers
void CObject::Checkbox( int* var ) // var will be changed depending on the checkbox's state
{
	this->w = CHECKBOX_SIZE;
	this->h = CHECKBOX_SIZE;
	this->state = var;
	this->type = OT_CHECKBOX;
}

void CObject::Tab()
{
	this->h = 20;
	this->type = OT_TAB;
}

void CObject::Slider( float omax, float omin, float* ocur )
{
	this->isDragging = 0;
	this->h = 10;
	this->sVal = ocur;
	this->sMax = omax;
	this->sMin = omin;
	this->type = OT_SLIDER;
	this->sCur = GetSliderValue( 0, ( DWORD )this->w, *ocur, omin, omax );
}

void CObject::Textbox()
{
	this->type = OT_TEXTBOX;
}

void CObject::RenderShape( float wx, float wy )
{
	switch( this->type )
	{
		case OT_CHECKBOX:
			if( *this->tab != this->tabid )
				return;

			this->r->DrawSquareEdges( wx + this->x, wy + this->y, this->w, this->h, OOUTLINE_COL ); //outline
			this->r->DrawSquare( wx + this->x + 1, wy + this->y + 1, this->w - 1, h - 1, OFILL_COL ); //fill

			if( *this->state == 1 ) //checked
			{
				float cx = wx + this->x + ( this->w / 2 );
				float cy = wy + this->y + ( this->h / 2 );
				this->r->DrawXCross( cx, cy, 4, 4, 4, ODATASEL_COL );
			}

			break;

		case OT_TAB:
			//this->r->DrawSquare(this->x+1,this->y,1,20,OOUTLINE_COL);
			this->r->DrawSquare( this->x + this->w - 2 + wx, this->y + wy, 1, 20, OOUTLINE_COL );
			break;

		case OT_SLIDER:
			if( *this->tab != this->tabid )
				return;

			if( this->r->cState == CS_UP )
				this->isDragging = 0;

			if( this->isDragging == 1 )
			{
				float nx = this->r->cX;

				if( nx < this->x + wx ) // mouse is left of the slider
					this->sCur = 0;

				else if( nx > this->x + wx + this->w ) // mouse is right of the slider
					this->sCur = ( DWORD )this->w;

				else // mouse is within the slider
					this->sCur = ( DWORD )( nx - this->x - wx );

				*this->sVal = GetRealValue( 0, this->sCur, ( DWORD )this->w, this->sMin, this->sMax ); // update float
			}

			this->r->DrawSquare( wx + this->x, wy + this->y, this->w, 2, OOUTLINE_COL ); // slider line

			this->r->DrawSquareEdges(
				wx + this->x - 1 + this->sCur,
				wy + this->y - ( this->h / 2 ) - 1,
				3,
				this->h + 1, OOUTLINE_COL );

			this->r->DrawSquare( wx + this->x + this->sCur, wy + this->y - ( this->h / 2 ), 2, this->h, ODATA_COL );
			//draw marker, draw marker highlighted if it's being dragged
			break;

		case OT_COMBOBOX:
			if( *this->tab != this->tabid )
				return;

			//if open and something is selected, draw it's background inverted
			break;

		case OT_TEXT:
			if( *this->tab != this->tabid )
				return;

			this->r->DrawSquare( wx + this->x, wy + this->y - ( this->h / 2 ), 2, this->h, ODATA_COL );
			break;

		case OT_TEXTBOX:
			if( *this->tab != this->tabid )
				return;

			// up arrow
			this->r->Draw2DTriangle( wx + this->x + this->w + 7, wy + this->y, // top vertex
									 wx + this->x + this->w + 1, wy + this->y + 14, // bottom left vertex
									 wx + this->x + this->w + 14, wy + this->y + 14, OARROW_COL ); // bottom right vertex & color
			// down arrow
			this->r->Draw2DTriangle( wx + this->x + this->w + 7, wy + this->y + this->h, // bottom
									 wx + this->x + this->w + 1, wy + this->y + this->h - 14, // top left vertex
									 wx + this->x + this->w + 14, wy + this->y + this->h - 14, OARROW_COL ); // top right vertex & color
			break;
	}
}

char slidertext[128];
RECT rr;

#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD_PTR)(ptr) + (DWORD_PTR)(addValue)) // by Matt Pietrek
#define MakeDelta(cast, x, y) (cast) ( (DWORD_PTR)(x) - (DWORD_PTR)(y))

char* findstrend( char* str )
{
	for( int i = 0; i < 0xFFFFFFFF; i++ )
	{
		char* ptr = MakePtr( char*, str, i );

		if( *ptr == 0x00 )
			return MakeDelta( char*, ptr, 1 );
	}

	return 0;
}

char* findlastmsg( char* end )
{
	for( int i = 0; i < 0xFFFFFFFF; i++ )
	{
		char* ptr = MakeDelta( char*, end, i );

		if( *ptr == '\n' || *ptr == 0x00 )
			return MakePtr( char*, ptr, 1 );

		//return MakePtr(char*, ptr, 0);
		//return MakeDelta(char*, ptr, 1);
		//return ptr;
	}

	return 0;
}

int getmsgsize( char* msg )
{
	for( int i = 0; i < 0xFFFFFFFF; i++ )
	{
		char* ptr = MakePtr( char*, msg, i );

		if( *ptr == '\n' || *ptr == 0x00 )
			return i;
	}

	return strlen( msg );
}

char tmpmsg[255];

int debug = 0;
char* debugc = 0;

char* getchatboxpos( char* text, CD3DRender* r )
{
	char* end = findstrend( text );
	int cH = 0;
	char* lastmsg = 0;
	char* msgbeforethat = 0;

	while( 1 ) // loop through all messages until we have enough to fill the chatbox
	{
		//if(lastmsg && strlen(lastmsg) > 0)
		msgbeforethat = lastmsg;
		lastmsg = findlastmsg( end );

		if( lastmsg )
		{
			RECT trect = {0, 0, CHATBOX_W, 0};
			memset( tmpmsg, 0, sizeof( tmpmsg ) );
			debug = getmsgsize( lastmsg );
			debugc = lastmsg;
			memcpy( tmpmsg, lastmsg, getmsgsize( lastmsg ) );
			r->GetTextRect( &trect, r->Normal, tmpmsg );

			if( cH + trect.bottom < CHATBOX_H )
				cH += trect.bottom;

			else
				return msgbeforethat;

			end = MakeDelta( char*, lastmsg, 1 );

			if( *end == 0x00 )
				return lastmsg;

			end = MakeDelta( char*, lastmsg, 2 );
		}

		else
		{
			return msgbeforethat;
		}
	}
}

void getmsgrect( char* msg, RECT* outr, CD3DRender* r )
{
	outr->left = 0;
	outr->top = 0;
	outr->right = CHATBOX_W;
	outr->bottom = CHATBOX_H;

	//r->
}

void CObject::RenderText( float wx, float wy )
{
	switch( this->type )
	{
		case OT_CHECKBOX:
			if( *this->tab != this->tabid )
				return;

			this->r->DrawTextMain( wx + this->x + this->w + 3, wy + this->y, this->r->Normal, this->text, ODATA_COL );
			break;

		case OT_TAB:
			if( *this->tab == this->tabid )
				this->r->DrawTextMain( wx + this->x, wy + this->y, this->r->Normal, this->text, ODATASEL_COL );

			else
				this->r->DrawTextMain( wx + this->x, wy + this->y, this->r->Normal, this->text, ODATA_COL );

			break;

		case OT_SLIDER:
			if( *this->tab != this->tabid )
				return;

			sprintf( slidertext, "%s: %.1f", this->text, *this->sVal );
			rr.left = ( LONG )( this->x + wx );
			rr.top = ( LONG )( this->y + wy + 5 );
			rr.right = ( LONG )( this->x + wx + this->w );
			rr.bottom = ( LONG )( this->y + wy + 22 );

			this->r->Normal->DrawTextA( this->r->s, slidertext, -1, &rr, DT_CENTER, ODATA_COL );
			break;

		case OT_COMBOBOX:
			// todo: ...
			break;

		case OT_TEXT:
			if( *this->tab == this->tabid )
			{
				this->r->DrawTextMain( this->x, this->y, this->r->Normal, this->text, ODATA_COL );
			}

			break;

		case OT_TEXTBOX:
			if( *this->tab == this->tabid )
			{
				RECT tbRect = {( LONG )( wx + this->x ), ( LONG )( wy + this->y ), ( LONG )( wx + this->x + this->w ), ( LONG )( wy + this->y + this->h )};
				char* pos = getchatboxpos( this->text, this->r );
				this->r->DrawTextRect( &tbRect, this->r->Normal, pos, ODATA_COL );
			}

			break;
	}
}

int CObject::MouseEvent( float wx, float wy ) // returns 1 if the mouse is interacting with this object
{
	if( this->type == OT_CHECKBOX )
	{
		if( *this->tab != this->tabid )
			return 0;

		if( this->r->IsCursorIn( wx + this->x, wy + this->y, this->w, this->h ) )
		{
			if( *this->state == 0 )
				*this->state = 1;

			else
				*this->state = 0;

			return 1;//handled event
		}

		return 0;
	}

	else if( this->type == OT_TAB )
	{
		if( this->r->IsCursorIn( wx + this->x, wy + this->y, this->w, this->h ) )
		{
			if( *this->tab != this->tabid )
			{
				*this->tab = this->tabid;
			}

			return 1;
		}
	}

	else if( this->type == OT_SLIDER )
	{
		if( *this->tab != this->tabid )
			return 0;

		if( this->r->IsCursorIn( wx + this->x, wy + this->y - ( this->h / 2 ), this->w, this->h ) )
		{
			this->isDragging = 1;
		}
	}

	return 0;
}
