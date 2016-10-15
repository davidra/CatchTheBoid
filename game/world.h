/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// Defining the world (more like city) class
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
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

	bool			Is2DPointInsideWorld(const cVector2& pos) const;
	bool			FindCollidingBuilding2D(const cVector2& start_pos, const cVector2& desired_pos, float radius, cAABB& out_colliding_building, cVector2& out_colliding_pos) const;
	bool			TestCollisionWithBlock2D(const cAABB& block_building_3D, const cVector2& start_pos, const cVector2& desired_pos, const cVector2& movement_dir, float movement_length, float radius, cAABB& out_colliding_building, cVector2& out_colliding_pos) const;

	static std::unique_ptr<cWorld> sWorldInstance;

	tStaticGeoContainer mStaticGeo;
	tCityMatrix			mCityMatrix;
};
