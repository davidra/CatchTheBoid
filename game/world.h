/***************************************************************************************************
world.h

Defining the world (more like city) class, it will also handle collisions 
 
by David Ramos
***************************************************************************************************/
#pragma once

class Mesh;

//----------------------------------------------------------------------------
class cWorld
{
public:
	static void		InitInstance(const char* init_file);
	static cWorld*	GetInstance() { CPR_assert(sWorldInstance != nullptr, "cWorld::InitInstance not called yet!"); return sWorldInstance.get(); }

	void			Render();

	cVector3		StepPlayerCollision(const cVector3& cur_pos, const cVector3& linear_velocity, float radius, float elapsed) const;
	const cAABB&	GetWorldBoundaries() const { return mCityMatrix.mWorldAABB; }

	bool			CastSphereAgainstWorld(const cVector3& org_pos, const cVector3& desired_pos, float radius, bool ignore_non_ground_boundaries, cVector3& out_colliding_pos, cVector3& out_colliding_normal) const;

private:
	cWorld() {}
	void			Init(const char* init_file);


	struct tWorldStaticGeo
	{
		tWorldStaticGeo(const cVector3& world_pos, const cVector3& scale, const cColor& color, Mesh* mesh) 
			: mWorldPos(world_pos)
			, mScale(scale)
			, mColor(color)
			, mMesh(mesh)
		{}

		cVector3	mWorldPos;
		cVector3	mScale;
		cColor		mColor;
		Mesh*		mMesh;
	};

	typedef std::vector<tWorldStaticGeo> tStaticGeoContainer;

	struct tCityMatrix
	{
		typedef std::vector<cAABB>	tRow;
		typedef std::vector<tRow>	tMatrix;
		tMatrix mMatrix;

		void Reset();
		tRow& operator[](unsigned idx) { return mMatrix[idx];  }
		const tRow& operator[](unsigned idx) const { return mMatrix[idx]; }

		tMatrix::iterator begin() { return mMatrix.begin(); }
		tMatrix::iterator end() { return mMatrix.end(); }

		tMatrix::const_iterator cbegin() const { return mMatrix.cbegin(); }
		tMatrix::const_iterator cend() const { return mMatrix.cend(); }

		unsigned	mColumns;
		unsigned	mRows;
		cAABB		mWorldAABB;
	};

	bool			ParseCityMatrix(const char* city_file, tCityMatrix& city_matrix) const;
	cAABB			ComputeAABBForRowColumn(unsigned row, unsigned column, float height) const;

	bool			FindBuildingOverlappingCircle(const cVector3& pos, float radius, cAABB& out_building) const;

	static std::unique_ptr<cWorld> sWorldInstance;

	tStaticGeoContainer mStaticGeo;
	tCityMatrix			mCityMatrix;
};
