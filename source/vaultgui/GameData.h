#pragma once

#include "common.h"

#include <d3dx9.h>

namespace GameData
{
	extern D3DMATRIX lastView;
	extern D3DMATRIX lastProjection;
	extern int windowHeight;
	extern int windowWidth;

	extern vector <PlayerScreenName> playersScreenName;
}
