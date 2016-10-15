/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// Simple file for storing and retrieving the model meshes
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#pragma once

#include "CPR_Framework.h"

#define MODEL_TUPLES \
	_MODEL_DATA(BOX, "resources/meshes/unitbox.x") \
	_MODEL_DATA(SPHERE, "resources/meshes/unitsphere.x") \
	_MODEL_DATA(TEAPOT, "resources/meshes/teapot.x")

#undef _MODEL_DATA
#define _MODEL_DATA(name,...) MID_##name,
enum eModelId
{
	MID_INVALID,
	MODEL_TUPLES
};
#undef _MODEL_DATA

#define _MODEL_DATA(name, model_file) \
	case MID_##name : { static Mesh* name##_model = Mesh::LoadFromFile(model_file); return name##_model; } break;

namespace ModelRepo
{ 
	inline Mesh* GetModel(eModelId mid)
	{

		switch (mid)
		{
			MODEL_TUPLES
			default:
			{
				CPR_assert(false, "Unknown model type %d", mid);
				return nullptr;
			}
		}
	}
}
#undef _MODEL_DATA
