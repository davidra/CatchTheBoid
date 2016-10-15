/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// Color definitions
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#pragma once

#include <D3dx9math.h>

class cColor : public D3DXVECTOR4
{
public:
	// using D3DXVECTOR4::D3DXVECTOR4;
	cColor(float x, float y, float z, float w) : D3DXVECTOR4(x, y, z, w) {}
};

#define TCOLOR_RED		cColor(1.f, 0.f, 0.f, 1.0f)
#define TCOLOR_GREEN	cColor(0.f, 1.f, 0.f, 1.0f)
#define TCOLOR_BLUE		cColor(0.f, 0.f, 1.f, 1.0f)
#define TCOLOR_YELLOW	cColor(1.f, 1.f, 0.f, 1.0f)
#define TCOLOR_PURPLE	cColor(1.f, 0.f, 1.f, 1.0f)
#define TCOLOR_WHITE	cColor(1.f, 1.f, 1.f, 1.0f)
#define TCOLOR_BLACK	cColor(0.f, 0.f, 0.f, 1.0f)
#define TCOLOR_GREY		cColor(0.5f, 0.5f, 0.5f, 1.0f)
#define TCOLOR_CYAN		cColor(0.f, 1.f, 1.f, 1.0f)
