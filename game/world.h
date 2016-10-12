/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// Defining the world (more like city) class
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#pragma once

#include "CPR_Framework.h"

//----------------------------------------------------------------------------
class cWorld
{
public:
	void Init(const char* init_file);
	void Render();

private:
	struct tWorldStaticGeo
	{
		tWorldStaticGeo(const cVector3& world_pos, const cVector3& scale, Mesh* mesh) 
			: mWorldPos(world_pos)
			, mScale(scale)
			, mMesh(mesh)
		{}

		cVector3	mWorldPos;
		cVector3	mScale;
		Mesh*		mMesh;
	};

	typedef std::vector<tWorldStaticGeo> tStaticGeoContainer;

	tStaticGeoContainer mStaticGeo;
};
