#include "stdafx.h"

#include "world.h"
#include "math/color.h"

//----------------------------------------------------------------------------
void cWorld::Init(const char* init_file)
{
	CPR_assert(mStaticGeo.capacity() == 0, "cWorld has been already initialized!");

	// Parse buildings from init_file

	// Test implementation first
	std::uniform_real_distribution<float> rand_distribution(0.0f, 15.0f);
	std::mt19937 mersenne_twister_generator(static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
	auto rand_generator_from_0_to_15(std::bind(rand_distribution, mersenne_twister_generator));
	
	static const int ROWS = 6;
	static const int COLUMNS = 7;
	static const int BLOCKS = ROWS * COLUMNS;

	static const float SPACE_BETWEEN_BUILDINGS = 3.0f;
	static const float BUILDING_SIDE_SIZE = 4.0f;

	static Mesh* const BOX_MODEL = Mesh::LoadFromFile("resources/meshes/unitbox.x");

	mStaticGeo.reserve(BLOCKS + 1);

	// Create the ground surface
	static const float GROUND_HEIGHT = 0.1f;
	const float width = (COLUMNS * BUILDING_SIDE_SIZE) + ((COLUMNS - 1) * SPACE_BETWEEN_BUILDINGS);
	const float length = (ROWS * BUILDING_SIDE_SIZE) + ((ROWS - 1) * SPACE_BETWEEN_BUILDINGS);
	mStaticGeo.emplace_back(cVector3(0.0f, -GROUND_HEIGHT * 0.5f, 0.0f), cVector3(width, GROUND_HEIGHT, length), BOX_MODEL);

	// Create the buildings
	for (int i = 0; i < BLOCKS; ++i)
	{
		const int column = i % COLUMNS;
		const float x = ((column * BUILDING_SIDE_SIZE) + (column * SPACE_BETWEEN_BUILDINGS)) - (width * 0.5f) + (BUILDING_SIDE_SIZE * 0.5f);

		const int row = i / COLUMNS;
		const float z = ((row * BUILDING_SIDE_SIZE) + (row * SPACE_BETWEEN_BUILDINGS)) - (length * 0.5f) + (BUILDING_SIDE_SIZE * 0.5f);

		const float height = rand_generator_from_0_to_15();
		mStaticGeo.emplace_back(cVector3(x, height * 0.5f, z), cVector3(BUILDING_SIDE_SIZE, height, BUILDING_SIDE_SIZE), BOX_MODEL);
	}
}

//----------------------------------------------------------------------------
void cWorld::Render()
{
	for (tWorldStaticGeo& geo : mStaticGeo)
	{
		CPR_assert(geo.mMesh != nullptr, "Geo (%d/%d) has no valid mesh!", &geo - mStaticGeo.data(), mStaticGeo.size());

		geo.mMesh->Render(geo.mWorldPos, cVector3::ZERO(), geo.mScale, TCOLOR_GREY);
	};
}
