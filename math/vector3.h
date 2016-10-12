/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// Simple Vector3 class mainly destined to hide the ugliness of D3DXVECTOR3 while still being interchangeable
/// with it
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#pragma once

#include <D3dx9math.h>

class cVector3 : public D3DXVECTOR3
{
public:
	// Inherit constructors (no working in VS2012)
	// using D3DXVECTOR3::D3DXVECTOR3;

	cVector3(float x, float y, float z) : D3DXVECTOR3(x, y, z) {}

	// In newer compiler versions these could be constexpr, here we rely on RVO
	static cVector3 XAXIS() { return cVector3(1.0f, 0.0f, 0.0f); }
	static cVector3 YAXIS() { return cVector3(0.0f, 1.0f, 0.0f); }
	static cVector3 ZAXIS() { return cVector3(0.0f, 0.0f, 1.0f); }
	static cVector3 ZERO()	{ return cVector3(0.0f, 0.0f, 0.0f); }
	static cVector3 ONE()	{ return cVector3(1.0f, 1.0f, 1.0f); }
};