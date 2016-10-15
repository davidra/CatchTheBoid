#include "stdafx.h"

#include "world.h"
#include "game\modelrepository.h"

namespace
{
	bool sGenerateRandomCity = false;

	static const float SPACE_BETWEEN_BUILDINGS = 3.0f;
	static const float BUILDING_SIDE_SIZE = 4.0f;
	static const float GROUND_HEIGHT = 0.1f;
	static const float BLOCK_SIZE = BUILDING_SIDE_SIZE + SPACE_BETWEEN_BUILDINGS;
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
						const float z = building_aabb.mMin.z - (BUILDING_SIDE_SIZE * HALF);

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

/*
//----------------------------------------------------------------------------
// We can make a lot of assumptions here to simplify the collision algorithm due the requirements of the test (AABB blocks equally spaced)
//
// The idea is roughly cast a shape along the cur_pos + linear_velocity vector to find collision point, then solve based on collider type (player: project rest of vel across colliding plane, 
// bullet: reflect). This can recurse for bullets that are reflected. The player will not collide after projecting a collision
cVector3 cWorld::StepPlayerCollision(const cVector3& cur_pos, const cVector3& linear_velocity, float radius, float elapsed) const
{
	if (linear_velocity.IsZero())
		return;

	const cVector3 desired_pos = cur_pos + (linear_velocity * elapsed);

	// find all blocks potentially colliding with the vector by finding its bounding AABB and
	// intersecting it with the city matrix
	cVector2 cur_pos_2D(cur_pos.GetXZ());
	cVector2 desired_pos_2D(desired_pos.GetXZ());

	// project the desired-current vector in the X axis and find out how many columns it encompasses
	ComputeBlocksIn2DAABB();


	// Collide with world boundaries

	// Collide with ground surfaces
}
*/

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

	unsigned num_rows = 0;
	unsigned max_num_columns = 0;

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

	return true;
}

//----------------------------------------------------------------------------
cAABB cWorld::ComputeAABBForRowColumn(unsigned row, unsigned column, float height) const
{
	// Negating z so the representation on the file obeys intuitive positive Z axis (i.e., the first row would be the one with the greatest z value)
	const cVector3 aabb_min(column * BLOCK_SIZE, 0.0f, row * -BLOCK_SIZE);
	const cVector3 aabb_max(aabb_min.x + BUILDING_SIDE_SIZE, height, aabb_min.z - BUILDING_SIDE_SIZE);

	return cAABB(aabb_min, aabb_max);
}

/*
//----------------------------------------------------------------------------
bool cWorld::ComputeBlocksInSegment(float start, float end, float enlarge_by, int& start_idx, int& end_idx)
{
	if (start > end)
	{
		std::swap(start, end);
	}

	start -= enlarge_by;
	end += enlarge_by;

	static const int BUCKET_SIZE = SPACE_BETWEEN_BUILDINGS + BUILDING_SIDE_SIZE;
	int first_bucket_idx = start / BUCKET_SIZE;
	const int last_bucket_idx = end / BUCKET_SIZE;

	if ((fmodf(start, BUCKET_SIZE)) > BUILDING_SIDE_SIZE)
	{
		// This means min_x lies in the space between buildings, so we can ignore this column, we won't collide with it
		++first_bucket_idx;
	}

	for (int column_idx = leftmost_column; column_idx <= rightmost_column; ++column_idx)
	{

	}
}*/

//----------------------------------------------------------------------------
bool cWorld::FindCollidingBuilding2D(const cVector2& start_pos, const cVector2& desired_pos, float radius, cAABB& out_colliding_building, cVector2& out_colliding_pos) const
{
	const cVector2 movement = desired_pos - start_pos;
	const float movement_length = movement.Length();
	const cVector2 movement_dir(movement / movement_length);

	bool collision_found = false;

	while (!collision_found)
	{
		// Find the block where the start pos lies
		const cAABB& block_building_3D = GetBuildingOfBlockAt2DPos(start_pos);
		const cAABB2D block_building = block_building_3D.GetXZ();

		// Get the axis-aligned lines defined by the building with what we can collide
		static const float INVALID_LINE = FLT_MAX;
		float y_aligned_line = INVALID_LINE;
		if (start_pos.x <= block_building.mMin.x)
		{
			y_aligned_line = block_building.mMin.x;
		}
		else if (start_pos.x >= block_building.mMax.x)
		{
			y_aligned_line = block_building.mMax.x;
		}
		else
		{
			// We are between the vertical lines defined by this building. This means we are either inside of 
			// the building or that won't collide with the vertical faces. Don't test
		}

		float x_aligned_line = INVALID_LINE;
		if (start_pos.y <= block_building.mMin.y)
		{
			x_aligned_line = block_building.mMin.y;
		}
		else if (start_pos.y >= block_building.mMax.y)
		{
			x_aligned_line = block_building.mMax.y;
		}
		else
		{
			// We are between the horizontal lines defined by this building. This means we are either inside of 
			// the building or that won't collide with the horizontal faces. Don't test against this block's building
		}

		if ((x_aligned_line == INVALID_LINE) && (y_aligned_line == INVALID_LINE))
		{
			// We are completely inside the building, skip this block
		}
		else
		{
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

			if (intersect_x < intersect_y)
			{
				const cVector2 intersect_point(start_pos + (movement_dir * intersect_x));

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
					else
					{
						// we are done here
						break;
					}
				}
				else
				{
					if (IsWithinRange(block_building.mMin.x - radius, intersect_point.x, block_building.mMax.x + radius))
					{
						out_colliding_building = block_building_3D;
						out_colliding_pos = cVector2(Clamp(block_building.mMin.x, intersect_point.x, block_building.mMax.x), intersect_point.y);
						collision_found = true;
					}
					else
					{
						// continue from this point
						movement_length -= intersect_x;
						start_pos = intersect_point;
					}
				}
			}
			else if (intersect_y != INVALID_INTERSECT_RESULT)
			{
				const cVector2 intersect_point(start_pos + (movement_dir * intersect_y));

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
					else
					{
						// we are done here
						break;
					}
				}
				else
				{
					if (IsWithinRange(block_building.mMin.y - radius, intersect_point.y, block_building.mMax.y + radius))
					{
						out_colliding_building = block_building_3D;
						out_colliding_pos = cVector2(intersect_point.x, Clamp(block_building.mMin.y, intersect_point.y, block_building.mMax.y));
						collision_found = true;
					}
					else
					{
						// continue from this point
						movement_length -= intersect_y;
						start_pos = intersect_point;
					}
				}
			}
			else
			{
				// Could not collide with this block lines, continue from the boundaries of the next block

			}
		}
	}

	return collision_found;
}

//----------------------------------------------------------------------------
const cAABB& cWorld::GetBuildingOfBlockAt2DPos(const cVector2& pos) const
{
	const int start_row = Clamp<int>(0, pos.y / -BLOCK_SIZE, mCityMatrix.mRows - 1);
	const int start_column = Clamp<int>(0, pos.x / BLOCK_SIZE, mCityMatrix.mColumns - 1);
	return mCityMatrix.mMatrix[start_row][start_column];
}
