// all-in-one class for tabs, checkboxes, and sliders
#pragma once
#include "CD3D9Render.h"

enum types_e
{
	OT_CHECKBOX,
	OT_TAB,
	OT_SLIDER,
	OT_COMBOBOX,
	OT_TEXT,
	OT_TEXTBOX
};

typedef struct
{
	int index;
	char* value;
	void* next;
} cbE_t;

class CObject
{
	public:
		// positions are RELATIVE to the parent window
		CObject( float ox, float oy, float ow, float oh, char* otext ); // generic constructor
		~CObject();

		//initializers
		void Checkbox( int* var ); // var will be changed depending on the checkbox's state
		void Tab();
		void Slider( float omax, float omin, float* ocur );
		void Textbox();
		void Combobox( cbE_t* centries, int num, int* var ); // var will be changed depending on the combobox's state

		void RenderShape( float wx, float wy );
		void RenderText( float wx, float wy );

		int MouseEvent( float wx, float wy ); // returns 1 if the mouse is interacting with this object

		CObject* children;
		CD3DRender* r;
		int children_num;

		int type;//what type of object this is

		int* tab;// current tab
		int tabid;// this tab's id OR this object's tab
		float x;
		float y;
		float w;
		float h;

		//slider specific vars
		int isDragging;
		DWORD sCur;// current position in the GUI slider
		float sMax;// maximum value for *sVal
		float sMin;// minimum value for *sVal
		float* sVal;// is proportional to sCur...

		//combo box vars
		int selectedId;// what is selected in the combo box?
		int isOpen;// is the combo box open?
		cbE_t* entries;// linearly linked list of entries in the combobox


		char* text;
		int* state;
		RECT textrect;
};
