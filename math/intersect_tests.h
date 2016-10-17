/***************************************************************************************************
intersect_tests.h
 
by David Ramos
***************************************************************************************************/
#pragma once

//----------------------------------------------------------------------------
// Axis-aligned-2D-lines related tests
inline float IntersectRayWithXAxisAlignedLine2D(const cVector2& org, const cVector2& dir, float line_y)
{
	CPR_assert(dir.IsNormalized(), "dir is not normalized!");

	if (IsSimilar(dir.y, 0.0f))
	{
		// Parallel
		return INVALID_INTERSECT_RESULT;
	}

	const float result = (line_y - org.y) / dir.y;
	if (result < 0.0f)
	{
		// not coincidental
		return INVALID_INTERSECT_RESULT;
	}

	return result;

}

inline float IntersectRayWithYAxisAlignedLine2D(const cVector2& org, const cVector2& dir, float line_x)
{
	CPR_assert(dir.IsNormalized(), "dir is not normalized!");

	if (IsSimilar(dir.x, 0.0f))
	{
		// Parallel
		return INVALID_INTERSECT_RESULT;
	}

	const float result = (line_x - org.x) / dir.x;
	if (result < 0.0f)
	{
		// not coincidental
		return INVALID_INTERSECT_RESULT;
	}

	return result;
}

inline float IntersectRayWithXAxisAlignedSegment2D(const cVector2& org, const cVector2& dir, float min_x, float max_x, float line_y)
{
	CPR_assert(min_x < max_x, "Not a valid segment was provided");
	const float dist_to_line = IntersectRayWithYAxisAlignedLine2D(org, dir, line_y);

	if (!IsWithinRange(min_x, cVector2(org + (dir * dist_to_line)).x, max_x))
	{
		return INVALID_INTERSECT_RESULT;
	}

	return dist_to_line;
}

inline float IntersectRayWithYAxisAlignedSegment2D(const cVector2& org, const cVector2& dir, float min_y, float max_y, float line_x)
{
	CPR_assert(min_y < max_y, "Not a valid segment was provided");
	const float dist_to_line = IntersectRayWithYAxisAlignedLine2D(org, dir, line_x);

	if (!IsWithinRange(min_y, cVector2(org + (dir * dist_to_line)).y, max_y))
	{
		return INVALID_INTERSECT_RESULT;
	}

	return dist_to_line;
}

inline float DistanceToXAxisAlignedLine2D(const cVector2& point, float line_y)
{
	return fabsf(line_y - point.y);
}

inline float DistanceToYAxisAlignedLine2D(const cVector2& point, float line_x)
{
	return fabsf(line_x - point.x);
}

//----------------------------------------------------------------------------
// Intersection ray-AABB
// From "Fast Ray-Box Intersection," by Woo in Graphics Gems I,
// page 395.
//----------------------------------------------------------------------------
inline float IntersectAABBWithRay(const cAABB& aabb, const cVector3& org, const cVector3& distance, cVector3& out_normal)
{
	// Check for point inside box, trivial reject, and determine parametric
	// distance to each front face
	bool inside = true;

	float xt, xn;
	if (org.x < aabb.mMin.x)
	{
		xt = aabb.mMin.x - org.x;
		if (xt > distance.x) return INVALID_INTERSECT_RESULT;
		xt /= distance.x;
		inside = false;
		xn = -1.0f;
	}
	else if (org.x > aabb.mMax.x)
	{
		xt = aabb.mMax.x - org.x;
		if (xt < distance.x) return INVALID_INTERSECT_RESULT;
		xt /= distance.x;
		inside = false;
		xn = 1.0f;
	}
	else
	{
		xt = -1.0f;
	}

	float yt, yn;
	if (org.y < aabb.mMin.y)
	{
		yt = aabb.mMin.y - org.y;
		if (yt > distance.y) return INVALID_INTERSECT_RESULT;
		yt /= distance.y;
		inside = false;
		yn = -1.0f;
	}
	else if (org.y > aabb.mMax.y)
	{
		yt = aabb.mMax.y - org.y;
		if (yt < distance.y) return INVALID_INTERSECT_RESULT;
		yt /= distance.y;
		inside = false;
		yn = 1.0f;
	}
	else
	{
		yt = -1.0f;
	}

	float zt, zn;
	if (org.z < aabb.mMin.z)
	{
		zt = aabb.mMin.z - org.z;
		if (zt > distance.z) return INVALID_INTERSECT_RESULT;
		zt /= distance.z;
		inside = false;
		zn = -1.0f;
	}
	else if (org.z > aabb.mMax.z)
	{
		zt = aabb.mMax.z - org.z;
		if (zt < distance.z) return INVALID_INTERSECT_RESULT;
		zt /= distance.z;
		inside = false;
		zn = 1.0f;
	}
	else
	{
		zt = -1.0f;
	}

	// Inside box?
	if (inside)
	{
		out_normal = -distance;
		out_normal.SetNormalized();
		return 0.0f;
	}

	// Select farthest plane - this is
	// the plane of intersection.
	int which = 0;
	float t = xt;
	if (yt > t)
	{
		which = 1;
		t = yt;
	}
	if (zt > t)
	{
		which = 2;
		t = zt;
	}

	switch (which)
	{
	case 0: // intersect with yz plane
	{
		float y = org.y + distance.y*t;
		if (y < aabb.mMin.y || y > aabb.mMax.y) return INVALID_INTERSECT_RESULT;
		float z = org.z + distance.z*t;
		if (z < aabb.mMin.z || z > aabb.mMax.z) return INVALID_INTERSECT_RESULT;

		out_normal.x = xn;
		out_normal.y = 0.0f;
		out_normal.z = 0.0f;
	} break;
	case 1: // intersect with xz plane
	{
		float x = org.x + distance.x*t;
		if (x < aabb.mMin.x || x > aabb.mMax.x) return INVALID_INTERSECT_RESULT;
		float z = org.z + distance.z*t;
		if (z < aabb.mMin.z || z > aabb.mMax.z) return INVALID_INTERSECT_RESULT;

		out_normal.x = 0.0f;
		out_normal.y = yn;
		out_normal.z = 0.0f;
	} break;
	case 2: // intersect with xy plane
	{
		float x = org.x + distance.x*t;
		if (x < aabb.mMin.x || x > aabb.mMax.x) return INVALID_INTERSECT_RESULT;
		float y = org.y + distance.y*t;
		if (y < aabb.mMin.y || y > aabb.mMax.y) return INVALID_INTERSECT_RESULT;

		out_normal.x = 0.0f;
		out_normal.y = 0.0f;
		out_normal.z = zn;
	} break;
	}

	// Return parametric point of intersection
	return t;
}

//----------------------------------------------------------------------------
// Intersection sphere cast-AABB
//----------------------------------------------------------------------------
inline float IntersectAABBWithSphereCast(const cAABB& aabb, const cVector3& org, const cVector3& distance, float radius, cVector3& out_normal)
{
	// Extend the original AABB to account for the radius
	cAABB extended_aabb(aabb);
	extended_aabb.Extend(radius);

	return IntersectAABBWithRay(extended_aabb, org, distance, out_normal);
}
//----------------------------------------------------------------------------
inline bool IntersectAABBWithSphere(const cAABB& aabb, const cVector3& sphere_center, float sphere_radius, cVector3& out_coll_pos, cVector3& out_normal)
{
	cVector3 closest_point;
	cVector3 normal(cVector3::ZERO());

	if (sphere_center.x < aabb.mMin.x)
	{
		closest_point.x = aabb.mMin.x;
		normal.x = -1.0f;
	}
	else if (sphere_center.x > aabb.mMax.x)
	{
		closest_point.x = aabb.mMax.x;
		normal.x = 1.0f;
	}
	else
	{
		closest_point.x = sphere_center.x;
	}

	if (sphere_center.y < aabb.mMin.y)
	{
		closest_point.y = aabb.mMin.y;
		normal.y = -1.0f;
	}
	else if (sphere_center.y > aabb.mMax.y)
	{
		closest_point.y = aabb.mMax.y;
		normal.y = 1.0f;
	}
	else
	{
		closest_point.y = sphere_center.y;
	}

	if (sphere_center.z < aabb.mMin.z)
	{
		closest_point.z = aabb.mMin.z;
		normal.z = -1.0f;
	}
	else if (sphere_center.z > aabb.mMax.z)
	{
		closest_point.z = aabb.mMax.z;
		normal.z = 1.0f;
	}
	else
	{
		closest_point.z = sphere_center.z;
	}

	// If the point is completely inside the AABB
	if (!normal.IsZero())
	{
		if (cVector3(sphere_center - closest_point).LengthSqr() <= (sphere_radius * sphere_radius))
		{
			// There is a collision
			out_coll_pos = closest_point;
			out_normal = Normalize(normal);

			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------
// TODO: by making Vector3 have a non-type templated getter for coordinates (receiving an index to the coordinate in the non-type template arg) we could
// have these three functions share the code and avoid the blatant (and dangerous) code duplication
inline float IntersectRayWithXZPlane(const cVector3& org, const cVector3& distance, float plane_y, cVector3& out_normal)
{
	const float y_dist_to_plane = plane_y - org.y;

	float normal_y = 0.0f;
	if (org.y < plane_y)
	{
		if (y_dist_to_plane > distance.y) 
		{
			return INVALID_INTERSECT_RESULT;
		}

		normal_y = -1.0f;
	}
	else if (org.y > plane_y)
	{
		if (y_dist_to_plane < distance.y) 
		{
			return INVALID_INTERSECT_RESULT;
		}

		normal_y = 1.0f;
	}
	else
	{
		// origin is part of the plane
		return INVALID_INTERSECT_RESULT;
	}

	out_normal = cVector3(0.0f, normal_y, 0.0f);
	return y_dist_to_plane / distance.y;
}

//----------------------------------------------------------------------------
inline float IntersectRayWithYZPlane(const cVector3& org, const cVector3& distance, float plane_x, cVector3& out_normal)
{
	const float x_dist_to_plane = plane_x - org.x;

	float normal_x = 0.0f;
	if (org.x < plane_x)
	{
		if (x_dist_to_plane > distance.x) 
		{
			return INVALID_INTERSECT_RESULT;
		}

		normal_x = -1.0f;
	}
	else if (org.x > plane_x)
	{
		if (x_dist_to_plane < distance.x) 
		{
			return INVALID_INTERSECT_RESULT;
		}

		normal_x = 1.0f;
	}
	else
	{
		// origin is part of the plane
		return INVALID_INTERSECT_RESULT;
	}

	out_normal = cVector3(normal_x, 0.0f, 0.0f);
	return x_dist_to_plane / distance.x;
}

//----------------------------------------------------------------------------
inline float IntersectRayWithYXPlane(const cVector3& org, const cVector3& distance, float plane_z, cVector3& out_normal)
{
	const float z_dist_to_plane = plane_z - org.z;

	float normal_z = 0.0f;
	if (org.z < plane_z)
	{
		if (z_dist_to_plane > distance.z) 
		{
			return INVALID_INTERSECT_RESULT;
		}

		normal_z = -1.0f;
	}
	else if (org.z > plane_z)
	{
		if (z_dist_to_plane < distance.z) 
		{
			return INVALID_INTERSECT_RESULT;
		}

		normal_z = 1.0f;
	}
	else
	{
		// origin is part of the plane
		return INVALID_INTERSECT_RESULT;
	}

	out_normal = cVector3(0.0f, 0.0f, normal_z);
	return z_dist_to_plane / distance.z;
}
