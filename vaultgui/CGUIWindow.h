#pragma once

#include "CD3D9Render.h"
#include "CGUIObject.h"

enum states_e
{
	cWS_VISIBLE,
	cWS_MINI
};

class CWindow
{
public:
	CWindow();
	CWindow(float x, float y, float w, float h, CD3DRender* r, char* wtitle);
	~CWindow();

	void Add(float x, float y, float w, float h);
	void Render();

	void AddCheckbox(float x, float y, char* text, int* var, int mtab);
	void AddTab(char* text);
	void AddSlider(float x, float y, float w, char* text, float omax, float omin, float* ocur, int mtab);
	void AddTextbox(float x, float y, float w, float h, char* text, int mtab);

	CD3DRender* dr;
	CObject* objects[64];

	int state;
	int maxTabs;
	int nextTabX;
	int curTab;
	int dragging;
	char* title;
	float wx;
	float wy;
	float ww;
	float wh;
};