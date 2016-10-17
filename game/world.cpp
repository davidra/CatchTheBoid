#include "stdafx.h"

#include "world.h"
#include "game\modelrepository.h"

std::unique_ptr<cWorld> cWorld::sWorldInstance;

namespace
{
	bool sGenerateRandomCity = false;

	static const float SPACE_BETWEEN_BUILDINGS = 3.0f;
	static const float BUILDING_SIDE_SIZE = 4.0f;
	static const float GROUND_HEIGHT = 0.1f;
	static const float BLOCK_SIZE = BUILDING_SIDE_SIZE + SPACE_BETWEEN_BUILDINGS;
}

//----------------------------------------------------------------------------
void cWorld::InitInstance(const char* init_file)
{
	sWorldInstance = std::unique_ptr<cWorld>(new cWorld);
	sWorldInstance->Init(init_file);
}

//----------------------------------------------------------------------------
void cWorld::tCityMatrix::Reset()
{
	mMatrix.clear();

	// Some reasonable initial reservation to avoid re-allocation
	static const unsigned MAX_EXPECTED_NUM_ROWS = 10;
	mMatrix.reserve(MAX_EXPECTED_NUM_ROWS);

	mColumns = 0;
	mRows = 0;
}

//----------------------------------------------------------------------------
void cWorld::Init(const char* init_file)
{
	CPR_assert(mStaticGeo.capacity() == 0, "cWorld has been already initialized!");

	Mesh* const building_model = ModelRepo::GetModel(MID_BOX);
	CPR_assert(building_model != nullptr, "Could not find mesh for building model!");
	if (!building_model)
		return;

	// Test implementation first
	if (sGenerateRandomCity)
	{ 
		std::mt19937 mersenne_twister_generator(static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));

		auto rand_generator_from_2_to_10(std::bind(std::uniform_int_distribution<int>(2, 10), mersenne_twister_generator));

		const int num_rows = rand_generator_from_2_to_10();
		const int num_columns = rand_generator_from_2_to_10();
		const int num_blocks = num_rows * num_columns;

		mCityMatrix.mMatrix.reserve(num_rows);
		for (int i = 0; i < num_rows; ++i)
		{
			mCityMatrix.mMatrix.emplace_back(num_blocks, cAABB(cVector3::ZERO()));
		}
		mStaticGeo.reserve(num_blocks + 1);

		// Create the ground surface
		const float width = (num_columns * BUILDING_SIDE_SIZE) + ((num_columns - 1) * SPACE_BETWEEN_BUILDINGS);
		const float length = (num_rows * BUILDING_SIDE_SIZE) + ((num_rows - 1) * SPACE_BETWEEN_BUILDINGS);
		mStaticGeo.emplace_back(cVector3(width * HALF, -GROUND_HEIGHT * 0.5f, -length * HALF), cVector3(width, GROUND_HEIGHT, length), TCOLOR_GREY, building_model);

		auto rand_generator_from_0_to_15(std::bind(std::uniform_real_distribution<float>(0.0f, 15.0f), mersenne_twister_generator));

		// Create the buildings
		for (int i = 0; i < num_blocks; ++i)
		{
			const float height = rand_generator_from_0_to_15();
			if (height > 0.0f)
			{
				const int column = i % num_columns;
				const int row = i / num_columns;
				const auto building_aabb = ComputeAABBForRowColumn(row, column, height);
				mCityMatrix[row][column] = building_aabb;

				const float x = building_aabb.mMin.x + (BUILDING_SIDE_SIZE * HALF);
				const float z = building_aabb.mMin.z - (BUILDING_SIDE_SIZE * HALF);

				mStaticGeo.emplace_back(cVector3(x, height * HALF, z), cVector3(BUILDING_SIDE_SIZE, height, BUILDING_SIDE_SIZE), TCOLOR_BLUE, building_model);
			}
		}
	}
	else
	{
		// Parse buildings from init_file
		const bool parse_ok = ParseCityMatrix(init_file, mCityMatrix);
		CPR_assert(parse_ok, "There was an error during parsing!");

		const unsigned num_rows = mCityMatrix.mRows;
		const unsigned num_columns = mCityMatrix.mColumns;

		if (parse_ok)
		{
			// Create the ground surface
			const float width = (num_columns * BUILDING_SIDE_SIZE) + ((num_columns - 1) * SPACE_BETWEEN_BUILDINGS);
			const float length = (num_rows * BUILDING_SIDE_SIZE) + ((num_rows - 1) * SPACE_BETWEEN_BUILDINGS);
			mStaticGeo.emplace_back(cVector3(width * HALF, -GROUND_HEIGHT * 0.5f, -length * HALF), cVector3(width, GROUND_HEIGHT, length), TCOLOR_GREY, building_model);

			for (unsigned row = 0; row < num_rows; ++row)
			{
				for (unsigned column = 0; column < num_columns; ++column)
				{
					const cAABB& building_aabb = mCityMatrix[row][column];
					if (building_aabb.mMax.y > 0.0f)
					{
						const float height = building_aabb.mMax.y;
						const float x = building_aabb.mMin.x + (BUILDING_SIDE_SIZE * HALF);
						const float z = building_aabb.mMax.z - (BUILDING_SIDE_SIZE * HALF);

						mStaticGeo.emplace_back(cVector3(x, height * HALF, z), cVector3(BUILDING_SIDE_SIZE, height, BUILDING_SIDE_SIZE), TCOLOR_BLUE, building_model);
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
void cWorld::Render()
{
	for (tWorldStaticGeo& geo : mStaticGeo)
	{
		CPR_assert(geo.mMesh != nullptr, "Geo (%d/%d) has no valid mesh!", &geo - mStaticGeo.data(), mStaticGeo.size());

		geo.mMesh->Render(geo.mWorldPos, cVector3::ZERO(), geo.mScale, geo.mColor);
	};
}

//----------------------------------------------------------------------------
// We can make a lot of assumptions here to simplify the collision algorithm due the requirements of the test (AABB blocks equally spaced)
//
// The idea is roughly cast a shape along the cur_pos + linear_velocity vector to find collision point, then solve based on collider type (player: project rest of vel across colliding plane, 
// bullet: reflect). This can recurse for bullets that are reflected. The player will not collide after projecting a collision
cVector3 cWorld::StepPlayerCollision(const cVector3& cur_pos, const cVector3& linear_velocity, float radius, float elapsed) const
{
	if (linear_velocity.IsZero() || (elapsed == 0.0f))
		return cur_pos;

	cVector3 desired_pos = cur_pos + (linear_velocity * elapsed);

	// find all blocks potentially colliding with the vector by finding its bounding AABB and
	// intersecting it with the city matrix
	cVector2 cur_pos_2D(cur_pos.GetXZ());
	cVector2 desired_pos_2D(desired_pos.GetXZ());
	cVector2 movement_dir_2D(Normalize(desired_pos_2D - cur_pos_2D));

	// Clamp to boundaries of world
	const bool start_is_inside = Is2DPointInsideWorld(cur_pos_2D);
	const bool end_is_inside = Is2DPointInsideWorld(desired_pos_2D);
	if (!start_is_inside && !end_is_inside)
	{
		return desired_pos;
	}
	else if (!end_is_inside)
	{
		// Find collision with boundaries
		bool already_in_y_aligned_boundary = false;
		float y_aligned_boundary = 0.0f;
		if (movement_dir_2D.x > 0.0f)
		{
			// rightmost boundary
			y_aligned_boundary = mCityMatrix.mWorldAABB.mMax.x;
			already_in_y_aligned_boundary = (mCityMatrix.mWorldAABB.mMax.x - cur_pos_2D.x) < radius;
		}
		else
		{
			// leftmost boundary
			y_aligned_boundary = mCityMatrix.mWorldAABB.mMin.x;
			already_in_y_aligned_boundary = (cur_pos_2D.x - mCityMatrix.mWorldAABB.mMin.x) < radius;
		}

		bool already_in_x_aligned_boundary = false;
		float x_aligned_boundary = 0.0f;
		if (movement_dir_2D.y > 0.0f)
		{
			// top boundary
			x_aligned_boundary = mCityMatrix.mWorldAABB.mMax.z;
			already_in_x_aligned_boundary = (mCityMatrix.mWorldAABB.mMax.z - cur_pos_2D.y) < radius;
		}
		else
		{
			// bottom boundary
			x_aligned_boundary = mCityMatrix.mWorldAABB.mMin.z;
			already_in_x_aligned_boundary = (cur_pos_2D.y - mCityMatrix.mWorldAABB.mMin.z) < radius;
		}

		if (already_in_y_aligned_boundary)
		{
			movement_dir_2D.x = 0.0f;
		}
		if (already_in_x_aligned_boundary)
		{
			movement_dir_2D.y = 0.0f;
		}

		if (movement_dir_2D.IsZero())
		{
			return cur_pos;
		}
		else
		{
			movement_dir_2D.SetNormalized();

			const float intersect_x = already_in_x_aligned_boundary ? INVALID_INTERSECT_RESULT : IntersectRayWithXAxisAlignedLine2D(cur_pos_2D, movement_dir_2D, x_aligned_boundary);
			const float intersect_y = already_in_y_aligned_boundary ? INVALID_INTERSECT_RESULT : IntersectRayWithYAxisAlignedLine2D(cur_pos_2D, movement_dir_2D, y_aligned_boundary);
			const float closest_intersection = (std::min)(intersect_x, intersect_y);
			CPR_assert(closest_intersection != INVALID_INTERSECT_RESULT, "We should have intersected the 2D boundaries here...what happened?");
			if (closest_intersection != INVALID_INTERSECT_RESULT)
			{
				desired_pos_2D = cur_pos_2D + (movement_dir_2D * closest_intersection);
				desired_pos = cVector3(desired_pos_2D.x, desired_pos.y, desired_pos_2D.y);
			}
		}
	}


	cVector2 colliding_pos;
	cAABB colliding_building;
	while (FindCollidingBuilding2D(cur_pos_2D, desired_pos_2D, radius, colliding_building, colliding_pos))
	{
		// TODO: Review this clusterfuck, all the efficiency of the 2D check gets lost of we redundantly do a 3D check now...and the retry is shaky...sigh

		// Now refine this with a proper 3D check
		cVector3 colliding_surface_normal;
		const cVector3 desired_movement = desired_pos - cur_pos;
		const float test_distance = IntersectAABBWithSphereCast(colliding_building, cur_pos, desired_movement, radius, colliding_surface_normal);
		if (test_distance != INVALID_INTERSECT_RESULT)
		{
			const cVector3 movement_dir = Normalize(desired_movement);
			const cVector3 colliding_pos_3D(cur_pos + (movement_dir * test_distance));

			// project the remaining vector into the collision plane
			return colliding_pos_3D + ProjectVectorOntoPlane(desired_pos - colliding_pos_3D, colliding_surface_normal);
		}
		else
		{
			// Progress a bit and keep trying 
			cur_pos_2D += movement_dir_2D * 0.01f;
		}
	}

	return desired_pos;
}

//----------------------------------------------------------------------------
bool cWorld::ParseCityMatrix(const char* city_file, tCityMatrix& city_matrix) const
{
	FILE* file_handle = fopen(city_file, "rb");
	CPR_assert(file_handle != nullptr, "Could not open file %s (%X)!", city_file, GetLastError());
	if (!file_handle)
	{
		return false;
	}

	fseek(file_handle, 0, SEEK_END);
	const size_t file_size = ftell(file_handle);
	fseek(file_handle, 0, SEEK_SET);

	std::unique_ptr<char[]> const buffer(new char[file_size + 1]);
	const size_t read = fread(buffer.get(), 1, file_size, file_handle);
	CPR_assert(read == file_size, "Paranoid assert: We read less characters than expected (?!)");
	fclose(file_handle);

	char* const end_of_file = buffer.get() + file_size;
	*end_of_file = '\0';
	char* str = buffer.get();

	unsigned	num_rows = 0;
	unsigned	max_num_columns = 0;
	float		max_height = 0.0f;

	city_matrix.Reset();

	do
	{
		char* line_end = strchr(str, '\n');
		if (line_end == nullptr)
		{
			line_end = end_of_file;
		}

		*line_end = '\0';

		if (line_end > str)
		{
			// Skip leading spaces
			for (; isspace(*str); ++str);

			// skip comment lines
			static const char COMMENT_TOKEN = '/';
			const bool comment_line = (str[0] == COMMENT_TOKEN) && (str[1] == COMMENT_TOKEN);
			if (!comment_line)
			{
				// pre-process the line to replace separator tokens with spaces
				const auto replace_pred = [](char chr)
				{
					using namespace std;
					static const char SEPARATOR_TOKENS[] = "\f\r\t\v,;"; // blank spaces and comma and semicolon

					return any_of(begin(SEPARATOR_TOKENS), end(SEPARATOR_TOKENS), [chr](char token) { return token == chr; });
				};
				std::replace_if(str, line_end, replace_pred, ' ');

				tCityMatrix::tRow new_row;
				if (max_num_columns > 0)
				{
					new_row.reserve(max_num_columns);
				}

				char* new_str = nullptr;
				// TODO: use strtof if I ever get VS2015 libraries
				for (float value = static_cast<float>(strtod(str, &new_str)); str != new_str; value = static_cast<float>(strtod(str, &new_str)))
				{
					CPR_assert(value >= 0.0f, "Error: Negative number (%f) found in matrix on text %s", str);

					max_height = (std::max)(max_height, value);

					new_row.push_back((value > 0.0f) ? ComputeAABBForRowColumn(num_rows, new_row.size(), value) : cAABB(cVector3::ZERO()));
					str = new_str;
				}

				const unsigned num_columns = new_row.size();
				if (num_columns > 0)
				{
					++num_rows;
					max_num_columns = (std::max)(num_columns, max_num_columns);
					const bool parsing_error = std::any_of(str, line_end, [](char chr) { return chr != ' '; });
					if (parsing_error)
					{
						CPR_assert(false, "Could not parse as a float: %s", str);
						return false;
					}

					city_matrix.mMatrix.push_back(std::move(new_row));
				}
			}
		}

		str = line_end + 1;
	} while (str < end_of_file);

	// Make sure we have a square matrix
	for (auto& row : city_matrix)
	{
		const auto missing_columns = max_num_columns - row.size();
		if (missing_columns > 0)
		{
			std::generate_n(std::back_inserter(row), missing_columns, [] { return cAABB(cVector3::ZERO()); });
		}
	}

	city_matrix.mColumns = max_num_columns;
	city_matrix.mRows = num_rows;

	const float width = (max_num_columns * BUILDING_SIDE_SIZE) + ((max_num_columns - 1) * SPACE_BETWEEN_BUILDINGS);
	const float length = (num_rows * BUILDING_SIDE_SIZE) + ((num_rows - 1) * SPACE_BETWEEN_BUILDINGS);
	city_matrix.mWorldAABB = cAABB(cVector3(0.0f, 0.0f, -length), cVector3(width, max_height, 0.0f));

	return true;
}

//----------------------------------------------------------------------------
cAABB cWorld::ComputeAABBForRowColumn(unsigned row, unsigned column, float height) const
{
	// Negating z so the representation on the file obeys intuitive positive Z axis (i.e., the first row would be the one with the greatest z value)
	const cVector3 aabb_min(column * BLOCK_SIZE, 0.0f, (row * -BLOCK_SIZE) - BUILDING_SIDE_SIZE);
	const cVector3 aabb_max(aabb_min.x + BUILDING_SIDE_SIZE, height, aabb_min.z + BUILDING_SIDE_SIZE);

	return cAABB(aabb_min, aabb_max);
}

//----------------------------------------------------------------------------
bool cWorld::Is2DPointInsideWorld(const cVector2& pos) const
{
	return mCityMatrix.mWorldAABB.IsInside(cVector3(pos.x, 0.1f, pos.y));
}

//----------------------------------------------------------------------------
bool cWorld::FindCollidingBuilding2D(const cVector2& start_pos, const cVector2& desired_pos, float radius, cAABB& out_colliding_building, cVector2& out_colliding_pos) const
{
	bool collision_found = false;

	const cVector2 movement = desired_pos - start_pos;
	const float movement_length = movement.Length();
	const cVector2 movement_dir(movement / movement_length);

	// Find the starting and ending blocks 
	const int start_row = Clamp(0, static_cast<int>(start_pos.y / -BLOCK_SIZE), static_cast<int>(mCityMatrix.mRows) - 1);
	const int start_column = Clamp(0, static_cast<int>(start_pos.x / BLOCK_SIZE), static_cast<int>(mCityMatrix.mColumns) - 1);

	const int end_row = Clamp(0, static_cast<int>(desired_pos.y / -BLOCK_SIZE), static_cast<int>(mCityMatrix.mRows) - 1);
	const int end_column = Clamp(0, static_cast<int>(desired_pos.x / BLOCK_SIZE), static_cast<int>(mCityMatrix.mColumns) - 1);

	// Traverse the blocks based on grid distance to the initial block
	const int row_delta = end_row - start_row;
	const int column_delta = end_column - start_column;
	const int inc_row = (row_delta >= 0) ? 1 : -1;
	const int inc_column = (column_delta >= 0) ? 1 : -1;
	const int max_blocks_considered = abs((row_delta + inc_row) * (column_delta + inc_column));
	for (int current_block_distance = 0, blocks_considered = 0; blocks_considered < max_blocks_considered; ++current_block_distance)
	{
		for (int i = current_block_distance; i >= 0; --i)
		{
			const int row = start_row + (inc_row * i);
			const int column = start_column + (inc_column * (current_block_distance - i));

			const bool row_in_range = ((row - start_row) * inc_row) <= (row_delta * inc_row);
			const bool column_in_range = ((column - start_column) * inc_column) <= (column_delta * inc_column);
			if (row_in_range && column_in_range)
			{
				const cAABB& building_block = mCityMatrix[row][column];
				collision_found = TestCollisionWithBlock2D(building_block, start_pos, desired_pos, movement_dir, movement_length, radius, out_colliding_building, out_colliding_pos);

				// TODO: We may want not to early out as soon as we find a 2D collision for when we want all the candidates for a refining 3D test. For now, start earlying out on first collision
				if (collision_found)
				{
					Debug::WriteLine("collision: (%f, %f, %f) [%d][%d]", out_colliding_pos.x, out_colliding_pos.y, out_colliding_pos.y, row, column);

					return true;
				}

				++blocks_considered;
			}
		}
	}

	return collision_found;
}

//----------------------------------------------------------------------------
bool cWorld::TestCollisionWithBlock2D(const cAABB& block_building_3D, const cVector2& start_pos, const cVector2& desired_pos, const cVector2& movement_dir, float movement_length, float radius, cAABB& out_colliding_building, cVector2& out_colliding_pos) const
{
	const cAABB2D block_building = block_building_3D.GetXZ();

	// early out for buildings with height 0 or when the movement_dir is zero
	if (IsSimilar(block_building_3D.mMax.y, 0.0f) || movement_dir.IsZero())
	{
		return false;
	}

	bool collision_found = false;

	// Get the axis-aligned lines we can collide with from the AABB of the building based on the direction of the test
	static const float INVALID_LINE = FLT_MAX;
	float y_aligned_line = INVALID_LINE;
	if (movement_dir.x > 0.0f)
	{
		// left to right
		y_aligned_line = block_building.mMin.x;
	}
	else if (movement_dir.x < 0.0f)
	{
		// right to left
		y_aligned_line = block_building.mMax.x;
	}

	float x_aligned_line = INVALID_LINE;
	if (movement_dir.y > 0.0f)
	{
		// down to up
		x_aligned_line = block_building.mMin.y;
	}
	else if (movement_dir.y < 0.0f)
	{
		// up to down
		x_aligned_line = block_building.mMax.y;
	}

	CPR_assert((x_aligned_line != INVALID_LINE) || (y_aligned_line != INVALID_LINE), "Could not select any axis!");

	float intersect_x = INVALID_INTERSECT_RESULT;
	if (x_aligned_line != INVALID_LINE)
	{
		intersect_x = IntersectRayWithXAxisAlignedLine2D(start_pos, movement_dir, x_aligned_line);
	}

	float intersect_y = INVALID_INTERSECT_RESULT;
	if (y_aligned_line != INVALID_LINE)
	{
		intersect_y = IntersectRayWithYAxisAlignedLine2D(start_pos, movement_dir, y_aligned_line);
	}

	for (int axis = 0; (axis < 2) && !collision_found; ++axis)
	{
		if (intersect_x < intersect_y)
		{
			const cVector2 intersect_point(start_pos + (movement_dir * intersect_x));

			if (IsWithinRange(block_building.mMin.x - radius, intersect_point.x, block_building.mMax.x + radius))
			{
				if (intersect_x > movement_length)
				{
					// The collision is beyond our reach... or is it? Let's consider the radius
					if (DistanceToXAxisAlignedLine2D(desired_pos, x_aligned_line) <= radius)
					{
						// That's a hit
						out_colliding_building = block_building_3D;
						out_colliding_pos = cVector2(desired_pos.x, x_aligned_line);
						collision_found = true;
					}
				}
				else
				{
					out_colliding_building = block_building_3D;
					out_colliding_pos = cVector2(Clamp(block_building.mMin.x, intersect_point.x, block_building.mMax.x), intersect_point.y);
					collision_found = true;
				}
			}

			// Invalidate this axis for the next loop
			intersect_x = INVALID_INTERSECT_RESULT;
		}
		// Try with the other intersection then
		else if (intersect_y != INVALID_INTERSECT_RESULT)
		{
			const cVector2 intersect_point(start_pos + (movement_dir * intersect_y));

			if (IsWithinRange(block_building.mMin.y - radius, intersect_point.y, block_building.mMax.y + radius))
			{
				if (intersect_y > movement_length)
				{
					// The collision is beyond our reach... or is it? Let's consider the radius
					if (DistanceToYAxisAlignedLine2D(desired_pos, y_aligned_line) <= radius)
					{
						// That's a hit
						out_colliding_building = block_building_3D;
						out_colliding_pos = cVector2(y_aligned_line, desired_pos.y);
						collision_found = true;
					}
				}
				else
				{
					out_colliding_building = block_building_3D;
					out_colliding_pos = cVector2(intersect_point.x, Clamp(block_building.mMin.y, intersect_point.y, block_building.mMax.y));
					collision_found = true;
				}
			}

			intersect_y = INVALID_INTERSECT_RESULT;
		}
	}

	return collision_found;
}

//----------------------------------------------------------------------------
void cWorld::GetAllBuildingsIntersecting2DLine(const cVector2& start_pos, const cVector2& end_pos, tBuildingsContainer& out_intersected_buildings) const
{

}

//----------------------------------------------------------------------------
bool cWorld::CastSphereAgainstWorld(const cVector3& start_pos, const cVector3& desired_pos, float radius, cVector3& out_colliding_pos, cVector3& out_colliding_normal) const
{
	cVector3 end_pos = desired_pos;
	cVector3 distance = end_pos - start_pos;

	// Determine orientation of displacement and how our search will progress
	enum eORIENTATION : unsigned
	{
		OR_NONE 			= 0x00,

		OR_RIGHT_TO_LEFT	= 0x01,
		OR_LEFT_TO_RIGHT	= 0x02,

		OR_UP_TO_DOWN		= 0x04,
		OR_DOWN_TO_UP		= 0x08,

		OR_TOP_TO_BOTTOM	= 0x10,
		OR_BOTTOM_TO_TOP	= 0x20,
	};

	unsigned dis_orientation = OR_NONE;
	int column_grow = 0;
	float (*yzplane_x) (int) = nullptr;
	float yz_boundary_x = 0.0f;
	if (distance.x < 0.0f)
	{
		dis_orientation |= OR_RIGHT_TO_LEFT;
		column_grow = -1;

		yzplane_x = [](int column) { return (column * BLOCK_SIZE) + BUILDING_SIDE_SIZE; };
		yz_boundary_x = mCityMatrix.mWorldAABB.mMin.x;
	}
	else if (distance.x > 0.0f)
	{
		dis_orientation |= OR_LEFT_TO_RIGHT;
		column_grow = 1;

		yzplane_x = [](int column) { return (column + 1) * BLOCK_SIZE; };
		yz_boundary_x = mCityMatrix.mWorldAABB.mMax.x;
	}

	if (distance.y < 0.0f)
	{
		dis_orientation |= OR_UP_TO_DOWN;
	}
	else if (distance.y > 0.0f)
	{
		dis_orientation |= OR_DOWN_TO_UP;
	}

	int row_grow = 0;
	float (*yxplane_z)(int) = nullptr;
	float yx_boundary_z = 0.0f;
	if (distance.z < 0.0f)
	{
		dis_orientation |= OR_TOP_TO_BOTTOM;
		row_grow = 1;
		yxplane_z = [] (int row) { return -(BLOCK_SIZE * (row + 1)); };
		yx_boundary_z = mCityMatrix.mWorldAABB.mMin.z;
	}
	else if (distance.z > 0.0f)
	{
		dis_orientation |= OR_BOTTOM_TO_TOP;
		row_grow = -1;
		yxplane_z = [] (int row) { return -((BLOCK_SIZE * row) + BUILDING_SIDE_SIZE); };
		yx_boundary_z = mCityMatrix.mWorldAABB.mMax.z;
	}

	if (dis_orientation == OR_NONE)
	{
		// no displacement
		return false;
	}

	if ((dis_orientation & OR_DOWN_TO_UP) && start_pos.y > (mCityMatrix.mWorldAABB.mMax.y))
	{
		// We are higher than our highest building and moving up
		return false;
	}

	// Clamp within world boundaries
	// Clamp ground (there is no ceiling boundary)
	const float ground_y = mCityMatrix.mWorldAABB.mMin.y + radius;
	// Clamp with ground if we are going below it
	if (end_pos.y < ground_y)
	{
		// We elevate the ground a bit to account for radius
		const float dist_to_plane = IntersectRayWithXZPlane(start_pos, distance, ground_y, out_colliding_normal);
		CPR_assert(dist_to_plane != INVALID_INTERSECT_RESULT, "Collision with ground was expected...why?");
		end_pos = start_pos + (distance * dist_to_plane);
		distance = end_pos - start_pos;
	}

	// Clamp to horizontal boundaries
	if (!IsWithinRange(mCityMatrix.mWorldAABB.mMin.x, end_pos.x, mCityMatrix.mWorldAABB.mMax.x))
	{
		const float dist_to_plane = IntersectRayWithYZPlane(start_pos, distance, yz_boundary_x, out_colliding_normal);
		CPR_assert(dist_to_plane != INVALID_INTERSECT_RESULT, "Collision with yz boundary was expected...why?");
		end_pos = start_pos + (distance * dist_to_plane);
		distance = end_pos - start_pos;
	}

	// Clamp to vertical boundaries
	if (!IsWithinRange(mCityMatrix.mWorldAABB.mMin.z, end_pos.z, mCityMatrix.mWorldAABB.mMax.z))
	{
		const float dist_to_plane = IntersectRayWithYXPlane(start_pos, distance, yx_boundary_z, out_colliding_normal);
		CPR_assert(dist_to_plane != INVALID_INTERSECT_RESULT, "Collision with yx boundary was expected...why?");
		end_pos = start_pos + (distance * dist_to_plane);
		distance = end_pos - start_pos;
	}

	// Find starting and end cells
	int row = Clamp(0, static_cast<int>(start_pos.z / -BLOCK_SIZE), static_cast<int>(mCityMatrix.mRows) - 1);
	int column = Clamp(0, static_cast<int>(start_pos.x / BLOCK_SIZE), static_cast<int>(mCityMatrix.mColumns) - 1);

	const int end_row = Clamp(0, static_cast<int>(end_pos.z / -BLOCK_SIZE), static_cast<int>(mCityMatrix.mRows) - 1);
	const int end_column = Clamp(0, static_cast<int>(end_pos.x / BLOCK_SIZE), static_cast<int>(mCityMatrix.mColumns) - 1);

	bool keep_searching = true;
	while (keep_searching)
	{
		// Check distance against closest YX and YZ planes
		float YZdist = INVALID_INTERSECT_RESULT;
		cVector3 YZnormal;
		if (dis_orientation & (OR_LEFT_TO_RIGHT | OR_RIGHT_TO_LEFT))
		{
			YZdist = IntersectRayWithYZPlane(start_pos, distance, yzplane_x(column) - (column_grow * radius), YZnormal);
		}

		float YXdist = INVALID_INTERSECT_RESULT;
		cVector3 YXnormal;
		if (dis_orientation & (OR_TOP_TO_BOTTOM | OR_BOTTOM_TO_TOP))
		{
			YXdist = IntersectRayWithYXPlane(start_pos, distance, yxplane_z(row) + (row_grow * radius), YXnormal);
		}

		// Try once per axis
		for (int axis = 0; axis < 2; ++axis)
		{
			if (YZdist < YXdist)
			{
				// Closest collision with YZ plane, validate is in z-range of a building
				const float coll_z = start_pos.z + (distance.z * YZdist);
				const int coll_row = static_cast<int>(-coll_z / BLOCK_SIZE); // negated since rows grow when z goes down
				if (!IsWithinRange<int>(0, coll_row, mCityMatrix.mRows - 1))
				{
					break;
				}

				const cAABB& coll_building = mCityMatrix[coll_row][column];
				if (coll_building.mMax.y == 0.0f)
				{
					// Skip 0-height buildings
					break;
				}

				const float z_in_block = -coll_z - (coll_row * BLOCK_SIZE); // this will now give us fabsf(coll_z % BLOCKSIZE) 
				if (z_in_block <= BUILDING_SIDE_SIZE)
				{
					cVector3 coll_pos = start_pos + (distance * YZdist);

					bool discard = false;
					const float coll_y = coll_pos.y;
					if (coll_y > coll_building.mMax.y)
					{
						discard = true;

						// If the collision happens above the building, we can still collide with its "roof"

						float XZdist = INVALID_INTERSECT_RESULT;
						if (dis_orientation & OR_UP_TO_DOWN)
						{
							cVector3 XZnormal;
							XZdist = IntersectRayWithXZPlane(start_pos, distance, coll_building.mMax.y + radius, XZnormal);
							if (XZdist != INVALID_INTERSECT_RESULT)
							{
								coll_pos = start_pos + (distance * XZdist);
								discard = false;
							}
						}
					}

					// We now need to do the real check with the full AABB with this candidate collision pos, this will also get us a proper normal
					if (!discard && IntersectAABBWithSphere(coll_building, coll_pos, radius, coll_pos, out_colliding_normal))
					{
						// closest plane collision validated, this is our hit
						out_colliding_pos = coll_pos;
						return true;
					}

					YZdist = INVALID_INTERSECT_RESULT;
				}
			}
			else if (YXdist != INVALID_INTERSECT_RESULT)
			{
				// Closest collision with YX plane, validate is in x-range of a building
				const float coll_x = start_pos.x + distance.x * YXdist;
				const int coll_column = static_cast<int>(coll_x / BLOCK_SIZE);
				if (!IsWithinRange<int>(0, coll_column, mCityMatrix.mColumns - 1))
				{
					break;
				}

				const cAABB& coll_building = mCityMatrix[row][coll_column];
				if (coll_building.mMax.y == 0.0f)
				{
					// Skip 0-height buildings
					break;
				}

				const float x_in_block = coll_x - (coll_column * BLOCK_SIZE); // this will now give us fabsf(coll_x % BLOCKSIZE) 
				if (x_in_block <= BUILDING_SIDE_SIZE)
				{
					cVector3 coll_pos = start_pos + (distance * YXdist);

					bool discard = false;
					const float coll_y = coll_pos.y;
					if (coll_y > coll_building.mMax.y)
					{
						discard = true;

						// If the collision happens above the building, we can still collide with its "roof"
						float XZdist = INVALID_INTERSECT_RESULT;
						if (dis_orientation & OR_UP_TO_DOWN)
						{
							cVector3 XZnormal;
							XZdist = IntersectRayWithXZPlane(start_pos, distance, coll_building.mMax.y + radius, XZnormal);
							if (XZdist != INVALID_INTERSECT_RESULT)
							{
								coll_pos = start_pos + (distance * XZdist);
								discard = false;
							}
						}
					}

					// We now need to do the real check with the full AABB with this candidate collision pos, this will also get us a proper normal
					if (!discard && IntersectAABBWithSphere(coll_building, coll_pos, radius, coll_pos, out_colliding_normal))
					{
						// closest plane collision validated, this is our hit
						out_colliding_pos = coll_pos;
						return true;
					}

					YXdist = INVALID_INTERSECT_RESULT;
				}
			}
		}

		// try with next planes if not done
		keep_searching = (row != end_row) || (column != end_column);
		if (keep_searching)
		{
			row = Clamp<int>(0, row + row_grow, end_row);
			column = Clamp<int>(0, column + column_grow, end_column);
		}
	}

	return false;
}