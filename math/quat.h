/***************************************************************************************************
quat.h
 
Quaternion class built to hide the C ugliness of the D3DXQuaternion

by David Ramos
***************************************************************************************************/
#pragma once

#include <D3dx9math.h>

//----------------------------------------------------------------------------
class cQuat : public D3DXQUATERNION
{
public:
	cQuat() : D3DXQUATERNION {}
	cQuat(const cVector3& axis, float angle);

	cVector3 RotateVector(const cVector3& vector) const;
};

//----------------------------------------------------------------------------
inline cQuat::cQuat(const cVector3& axis, float angle)
{
	D3DXQuaternionRotationAxis(this, &axis, angle);
}

//----------------------------------------------------------------------------
inline cVector3 cQuat::RotateVector(const cVector3& vector) const
{
	D3DX
}
