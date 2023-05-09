#pragma once


#include <glm/glm.hpp>

#include "Ant/Core/Math/AABB.h"

namespace Ant{
	struct Ray
	{
		glm::vec3 Origin, Direction;

		bool IntersectsAABB(const AABB& aabb, float& t) const
		{
			glm::vec3 dirFrac;
			// r.dir is unit direction vector of ray
			dirFrac.x = 1.0f / Direction.x;
			dirFrac.y = 1.0f / Direction.y;
			dirFrac.z = 1.0f / Direction.z;
			// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
		    // r.org is origin of ray
			const glm::vec3& lb = aabb.Min;
			const glm::vec3& rt = aabb.Max;
			float t1 = (lb.x - Origin.x) * dirFrac.x;
			float t2 = (rt.x - Origin.x) * dirFrac.x;
			float t3 = (lb.y - Origin.y) * dirFrac.y;
			float t4 = (rt.y - Origin.y) * dirFrac.y;
			float t5 = (lb.z - Origin.z) * dirFrac.z;
			float t6 = (rt.z - Origin.z) * dirFrac.z;

			float tmin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
			float tmax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

			// if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
			if(tmax < 0)
			{
				t = tmax;
				return false;
			}

			// if tmin > tmax, ray doesn't intersect AABB
			if(tmin > tmax)
			{
				t = tmax;
				return false;
			}

			t = tmin;
			return true;
		}

		bool IntersectsTriangle(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C, float& t)
		{
			glm::vec3 E1 = B - A;
			glm::vec3 E2 = C - A;
			glm::vec3 N = cross(E1, E2);
			float det = -glm::dot(Direction, N);
			float invdet = 1.0f / det;
			glm::vec3 AO = Origin - A;
			glm::vec3 DAO = cross(AO, Direction);
			float u = glm::dot(E2, DAO) * invdet;
			float v = -glm::dot(E1, DAO) * invdet;
			t = glm::dot(AO, N) * invdet;
			return (det >= 1e-6 && t >= 0.0 && u >= 0.0 && v >= 0.0 && (u + v) <= 1.0);

		}
	};
}