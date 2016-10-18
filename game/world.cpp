#include "stdafx.h"

#include "world.h"
#include "game\modelrepository.h"
#include "debugutils\debugrenderer.h"

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

	cVector3 coll_pos;
	cVector3 coll_normal;
	if (CastSphereAgainstWorld(cur_pos, desired_pos, radius, false, coll_pos, coll_normal))
	{
		coll_pos += coll_normal * (radius + 0.01f);

		// We don't need to check for collisions again in the projected direction because of how our building grid is structured, it should be impossible to collide with anything else. Also, the projected vector
		// is highly unlikely to be big enough to collide with anything else again
		const cVector3 coll_plane_projected_remaining_movement = ProjectVectorOntoPlane(desired_pos - coll_pos, coll_normal);
		return coll_pos + coll_plane_projected_remaining_movement;
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
bool cWorld::FindBuildingOverlappingCircle(const cVector3& pos, float radius, cAABB& out_building) const
{
	CPR_assert((radius * 2.0f) < SPACE_BETWEEN_BUILDINGS, "A radius this big could overlap several buildings, but this function only handles one");

	int row = static_cast<int>(-(pos.z / BLOCK_SIZE));
	int column = static_cast<int>(pos.x / BLOCK_SIZE);

	if (!IsWithinRange<int>(0, row, mCityMatrix.mRows - 1) || !IsWithinRange<int>(0, column, mCityMatrix.mColumns - 1))
		return false;

	// Let's abuse the notion that buildings are always on the top-left corner of each grid
	const float building_square_base = -((row * BLOCK_SIZE) + BUILDING_SIDE_SIZE);
	if (pos.z < (building_square_base - radius))
	{
		// Below this cell's building, might be colliding with the building one row below
		if (pos.z <= (building_square_base - SPACE_BETWEEN_BUILDINGS + radius))
		{
			++row;
		}
		else
		{
			return false;
		}
	}

	const float building_right_side = (column * BLOCK_SIZE) + BUILDING_SIDE_SIZE;
	if (pos.x > (building_right_side + radius))
	{
		// To the right of this cell's building, might be colliding with the building one column to the right
		if (pos.x >= (building_right_side + SPACE_BETWEEN_BUILDINGS - radius))
		{
			++column;
		}
		else
		{
			// We are not
			return false;
		}
	}

	const cAABB building_collided = mCityMatrix[row][column];
	if (building_collided.mMax.y > 0.0f) // 0-height buildings don't exist
	{
		out_building = building_collided;
		return true;
	}
	else
	{
		return false;
	}
}

//----------------------------------------------------------------------------
bool cWorld::CastSphereAgainstWorld(const cVector3& org_pos, const cVector3& desired_pos, float radius, bool ignore_non_ground_boundaries, cVector3& out_colliding_pos, cVector3& out_colliding_normal) const
{
	cVector3 start_pos = org_pos;
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
		yz_boundary_x = mCityMatrix.mWorldAABB.mMin.x + radius;
	}
	else if (distance.x > 0.0f)
	{
		dis_orientation |= OR_LEFT_TO_RIGHT;
		column_grow = 1;

		yzplane_x = [](int column) { return (column + 1) * BLOCK_SIZE; };
		yz_boundary_x = mCityMatrix.mWorldAABB.mMax.x - radius;
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
		yx_boundary_z = mCityMatrix.mWorldAABB.mMin.z + radius;
	}
	else if (distance.z > 0.0f)
	{
		dis_orientation |= OR_BOTTOM_TO_TOP;
		row_grow = -1;
		yxplane_z = [] (int row) { return -((BLOCK_SIZE * row) + BUILDING_SIDE_SIZE); };
		yx_boundary_z = mCityMatrix.mWorldAABB.mMax.z - radius;
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

	cVector3 coll_normal(cVector3::YAXIS());

	// First, some special cases...*sigh*
	// Find if the start pos is on top of a building and we are aiming down. We need to do this since otherwise the algorithm would not consider it 
	cAABB start_building;
	if (FindBuildingOverlappingCircle(start_pos, radius, start_building))
	{
		if ((start_pos.y > start_building.mMax.y) && (dis_orientation & OR_UP_TO_DOWN))
		{
			cVector3 XZnormal;
			const float XZdist = IntersectRayWithXZPlane(start_pos, distance, start_building.mMax.y + radius, XZnormal);
			if (XZdist != INVALID_INTERSECT_RESULT)
			{
				cVector3 coll_pos = start_pos + (distance * XZdist);
				// Ok, this is a "bit" of a hack, but let's move the coll_pos towards the distance a bit to handle corners (where the distance to the corner
				// will be sqrt(2*radius*radius) due to how we extend the planes). Half a radius should be enough...Sorry
				coll_pos += Normalize(distance) * radius * HALF;
				if (IntersectAABBWithSphere(start_building, coll_pos, radius, coll_pos, coll_normal))
				{
					// We are a aiming down from the top of a building and this is our collision
					out_colliding_pos = coll_pos;
					out_colliding_normal = coll_normal;
					return true;
				}
			}
		}
		else
		{
			// The algorithm ignores the block we are already inside of, where inside of means the point is inside a square extended by the radius
			// Problem is, in the corners we can actually be closer, so try to find if we are in a corner and move the start_pos back in a way that it is outside that square
			cVector3 corner(
				Clamp(start_building.mMin.x, start_pos.x, start_building.mMax.x)
				, Clamp(start_building.mMin.y, start_pos.y, start_building.mMax.y)
				, Clamp(start_building.mMin.z, start_pos.z, start_building.mMax.z));

			const bool in_corner = !!(corner != start_pos);
			if (in_corner)
			{
				const cVector3 corner_to_start(start_pos - corner);
				const float dist_to_corner = corner_to_start.Length();
				if (dist_to_corner > radius)
				{
					const cVector3 corner_to_start_dir = corner_to_start / dist_to_corner;
					const cVector3 start_pos_delta = corner_to_start_dir * sqrt(2 * (radius*radius));
					start_pos += start_pos_delta;
					distance -= start_pos_delta;
				}
			}
		}
	}

	bool collided_with_boundaries = false;

	// Clamp within world boundaries
	// Clamp ground (there is no ceiling boundary)
	const float ground_y = mCityMatrix.mWorldAABB.mMin.y + radius;
	// Clamp with ground if we are going below it
	if (end_pos.y < ground_y)
	{
		// We elevate the ground a bit to account for radius
		const float dist_to_plane = IntersectRayWithXZPlane(start_pos, distance, ground_y, coll_normal);
		if (dist_to_plane != INVALID_INTERSECT_RESULT)
		{
			end_pos = start_pos + (distance * dist_to_plane);
			distance = end_pos - start_pos;
			collided_with_boundaries = true;
		}
		else
		{
			Debug::WriteLine("Collision with ground was expected...why?");
		}
	}

	// Clamp to horizontal boundaries
	if (!ignore_non_ground_boundaries && !IsWithinRange(mCityMatrix.mWorldAABB.mMin.x + radius, end_pos.x, mCityMatrix.mWorldAABB.mMax.x - radius))
	{
		const float dist_to_plane = IntersectRayWithYZPlane(start_pos, distance, yz_boundary_x, coll_normal);
		if (dist_to_plane != INVALID_INTERSECT_RESULT)
		{
			end_pos = start_pos + (distance * dist_to_plane);
			distance = end_pos - start_pos;
			collided_with_boundaries = true;
		}
		else
		{
			Debug::WriteLine("Collision with yz boundary was expected...why?");
		}
	}

	// Clamp to vertical boundaries
	if (!ignore_non_ground_boundaries && !IsWithinRange(mCityMatrix.mWorldAABB.mMin.z + radius, end_pos.z, mCityMatrix.mWorldAABB.mMax.z - radius))
	{
		const float dist_to_plane = IntersectRayWithYXPlane(start_pos, distance, yx_boundary_z, coll_normal);
		if (dist_to_plane != INVALID_INTERSECT_RESULT)
		{
			end_pos = start_pos + (distance * dist_to_plane);
			distance = end_pos - start_pos;
			collided_with_boundaries = true;
		}
		else
		{
			Debug::WriteLine("Collision with yx boundary was expected...why?");
		}
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
			// Vertical plane
			if (YZdist < YXdist)
			{
				cVector3 coll_pos = start_pos + (distance * YZdist);

				// Find the building we are potentially coliding with. This check is 2D, though, so we will need to check the XZ plane later
				cAABB coll_building;
				if (!FindBuildingOverlappingCircle(coll_pos, radius, coll_building))
				{
					YZdist = INVALID_INTERSECT_RESULT;
					continue;
				}

				bool discard = false;
				const float coll_y = coll_pos.y;
				if (coll_y > (coll_building.mMax.y + radius))
				{
					discard = true;

					// If the collision happens above the building, we can still collide with its "roof"
					float XZdist = INVALID_INTERSECT_RESULT;
					if (dis_orientation & OR_UP_TO_DOWN)
					{
						cVector3 XZnormal;
						XZdist = IntersectRayWithXZPlane(start_pos, distance, coll_building.mMax.y + radius, XZnormal);
						if ((XZdist != INVALID_INTERSECT_RESULT))
						{
							coll_pos = start_pos + (distance * XZdist);
							discard = false;
						}
					}
				}

				if (!discard)
				{
					// Now let's handle the tricky case of corners
					cVector3 corner;
					bool handle_corner = false;
					if (coll_pos.z > coll_building.mMax.z)
					{
						corner.z = coll_building.mMax.z;
						handle_corner = true;
					}
					else if (coll_pos.z < coll_building.mMin.z)
					{
						corner.z = coll_building.mMin.z;
						handle_corner = true;
					}

					if (handle_corner)
					{
						if (dis_orientation & OR_LEFT_TO_RIGHT)
						{
							corner.x = coll_building.mMin.x;
						}
						else
						{
							CPR_assert(dis_orientation & OR_RIGHT_TO_LEFT, "Huh?");
							corner.x = coll_building.mMax.x;
						}

						corner.y = Clamp(coll_building.mMin.y, coll_pos.y, coll_building.mMax.y);

						const cVector3 coll_to_corner = corner - coll_pos;
						const bool valid_coll = Dot(distance, coll_to_corner) >= 0.0f;
						if (valid_coll)
						{
							out_colliding_normal = Normalize(-coll_to_corner);
							out_colliding_pos = corner;

							return true;
						}
					}
					else
					{
						// Simple collision with a face
						out_colliding_normal = (dis_orientation & OR_LEFT_TO_RIGHT) ? -cVector3::XAXIS() : cVector3::XAXIS();
						out_colliding_pos = coll_pos - (out_colliding_normal * radius);

						return true;
					}
				}

				YZdist = INVALID_INTERSECT_RESULT;
			}
			// Horizontal plane
			else if (YXdist != INVALID_INTERSECT_RESULT)
			{
				cVector3 coll_pos = start_pos + (distance * YXdist);

				// Find the building we are potentially coliding with. This check is 2D, though, so we will need to check the XZ plane later
				cAABB coll_building;
				if (!FindBuildingOverlappingCircle(coll_pos, radius, coll_building))
				{
					YXdist = INVALID_INTERSECT_RESULT;
					continue;
				}

				bool discard = false;
				const float coll_y = coll_pos.y;
				if (coll_y > (coll_building.mMax.y + radius))
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

				if (!discard)
				{
					// Now let's handle the tricky case of corners
					cVector3 corner;
					bool handle_corner = false;
					if (coll_pos.x > coll_building.mMax.x)
					{
						corner.x = coll_building.mMax.x;
						handle_corner = true;
					}
					else if (coll_pos.x < coll_building.mMin.x)
					{
						corner.x = coll_building.mMin.x;
						handle_corner = true;
					}

					if (handle_corner)
					{
						if (dis_orientation & OR_BOTTOM_TO_TOP)
						{
							corner.z = coll_building.mMin.z;
						}
						else
						{
							CPR_assert(dis_orientation & OR_TOP_TO_BOTTOM, "Huh?");
							corner.z = coll_building.mMax.z;
						}

						corner.y = Clamp(coll_building.mMin.y, coll_pos.y, coll_building.mMax.y);

						const cVector3 coll_to_corner = corner - coll_pos;
						const bool valid_coll = Dot(distance, coll_to_corner) >= 0.0f;
						if (valid_coll)
						{
							out_colliding_normal = Normalize(-coll_to_corner);
							out_colliding_pos = corner;

							return true;
						}
					}
					else
					{
						// Simple collision with a face
						out_colliding_normal = (dis_orientation & OR_BOTTOM_TO_TOP) ? -cVector3::ZAXIS() : cVector3::ZAXIS();
						out_colliding_pos = coll_pos - (out_colliding_normal * radius);

						return true;
					}
				}

				// We now need to do the real check with the full AABB with this candidate collision pos, this will also get us a proper normal
				// Ok, this is a "bit" of a hack, but let's move the coll_pos towards the distance a bit to handle corners (where the distance to the corner
				// will be sqrt(2*radius*radius) due to how we extend the planes). Half a radius should be enough...Sorry
				coll_pos += Normalize(distance) * radius * HALF;
				if (!discard && IntersectAABBWithSphere(coll_building, coll_pos, radius, coll_pos, coll_normal))
				{
					// closest plane collision validated, this is our hit
					out_colliding_pos = coll_pos;
					out_colliding_normal = coll_normal;
					return true;
				}

				YXdist = INVALID_INTERSECT_RESULT;
			}
		}

		// try with next planes if not done
		keep_searching = (row != end_row) || (column != end_column);
		if (keep_searching)
		{
			row = Limit(row, row + row_grow, end_row);
			column = Limit(column, column + column_grow, end_column);
		}
	}

	if (collided_with_boundaries)
	{
		out_colliding_pos = end_pos - (coll_normal * radius);
		out_colliding_normal = coll_normal;
	}

	return collided_with_boundaries;
}