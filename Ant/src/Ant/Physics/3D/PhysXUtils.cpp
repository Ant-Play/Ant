#include "antpch.h"
#include "PhysXUtils.h"

#include "Ant/Math/Math.h"

namespace Ant::PhysXUtils {

	physx::PxTransform ToPhysXTransform(const TransformComponent& transform)
	{
		physx::PxQuat r = ToPhysXQuat(transform.GetRotation());
		physx::PxVec3 p = ToPhysXVector(transform.Translation);
		return physx::PxTransform(p, r);
	}

	physx::PxTransform ToPhysXTransform(const glm::mat4& transform)
	{
		glm::vec3 translation;
		glm::quat rotation;
		glm::vec3 scale;
		Math::DecomposeTransform(transform, translation, rotation, scale);

		physx::PxQuat r = ToPhysXQuat(rotation);
		physx::PxVec3 p = ToPhysXVector(translation);
		return physx::PxTransform(p, r);
	}

	physx::PxTransform ToPhysXTransform(const glm::vec3& translation, const glm::quat& rotation)
	{
		return physx::PxTransform(ToPhysXVector(translation), ToPhysXQuat(rotation));
	}

	physx::PxMat44 ToPhysXMatrix(const glm::mat4& matrix) { return *(physx::PxMat44*)&matrix; }
	const physx::PxVec3& ToPhysXVector(const glm::vec3& vector) { return *(physx::PxVec3*)&vector; }
	const physx::PxVec4& ToPhysXVector(const glm::vec4& vector) { return *(physx::PxVec4*)&vector; }
	physx::PxExtendedVec3 ToPhysXExtendedVector(const glm::vec3& vector) { return physx::PxExtendedVec3(vector.x, vector.y, vector.z); }
	physx::PxQuat ToPhysXQuat(const glm::quat& quat) { return physx::PxQuat(quat.x, quat.y, quat.z, quat.w); }

	glm::mat4 FromPhysXTransform(const physx::PxTransform& transform)
	{
		glm::quat rotation = FromPhysXQuat(transform.q);
		glm::vec3 position = FromPhysXVector(transform.p);
		return glm::translate(glm::mat4(1.0F), position) * glm::toMat4(rotation);
	}

	glm::mat4 FromPhysXMatrix(const physx::PxMat44& matrix) { return *(glm::mat4*)&matrix; }
	glm::vec3 FromPhysXVector(const physx::PxVec3& vector) { return *(glm::vec3*)&vector; }
	glm::vec4 FromPhysXVector(const physx::PxVec4& vector) { return *(glm::vec4*)&vector; }
	glm::quat FromPhysXQuat(const physx::PxQuat& quat) { return *(glm::quat*)&quat; }

	CookingResult FromPhysXCookingResult(physx::PxConvexMeshCookingResult::Enum cookingResult)
	{
		switch (cookingResult)
		{
			case physx::PxConvexMeshCookingResult::eSUCCESS: return CookingResult::Success;
			case physx::PxConvexMeshCookingResult::eZERO_AREA_TEST_FAILED: return CookingResult::ZeroAreaTestFailed;
			case physx::PxConvexMeshCookingResult::ePOLYGONS_LIMIT_REACHED: return CookingResult::PolygonLimitReached;
			case physx::PxConvexMeshCookingResult::eFAILURE: return CookingResult::Failure;
		}

		return CookingResult::Failure;
	}

	CookingResult FromPhysXCookingResult(physx::PxTriangleMeshCookingResult::Enum cookingResult)
	{
		switch (cookingResult)
		{
			case physx::PxTriangleMeshCookingResult::eSUCCESS: return CookingResult::Success;
			case physx::PxTriangleMeshCookingResult::eLARGE_TRIANGLE: return CookingResult::LargeTriangle;
			case physx::PxTriangleMeshCookingResult::eFAILURE: return CookingResult::Failure;
		}

		return CookingResult::Failure;
	}

	const char* CookingResultToString(CookingResult cookingResult)
	{
		switch (cookingResult)
		{
			case Ant::CookingResult::Success: return "Success";
			case Ant::CookingResult::ZeroAreaTestFailed: return "ZeroAreTestFailed";
			case Ant::CookingResult::PolygonLimitReached: return "PolygonLimitReached";
			case Ant::CookingResult::LargeTriangle: return "LargeTriangle";
			case Ant::CookingResult::InvalidMesh: return "InvalidMesh";
			case Ant::CookingResult::Failure: return "Failure";
		}

		ANT_CORE_ASSERT(false);
		return "";
	}

	physx::PxBroadPhaseType::Enum AntToPhysXBroadphaseType(BroadphaseType type)
	{
		switch (type)
		{
			case Ant::BroadphaseType::SweepAndPrune: return physx::PxBroadPhaseType::eSAP;
			case Ant::BroadphaseType::MultiBoxPrune: return physx::PxBroadPhaseType::eMBP;
			case Ant::BroadphaseType::AutomaticBoxPrune: return physx::PxBroadPhaseType::eABP;
		}

		return physx::PxBroadPhaseType::eABP;
	}

	physx::PxFrictionType::Enum AntToPhysXFrictionType(FrictionType type)
	{
		switch (type)
		{
			case Ant::FrictionType::Patch:			return physx::PxFrictionType::ePATCH;
			case Ant::FrictionType::OneDirectional:	return physx::PxFrictionType::eONE_DIRECTIONAL;
			case Ant::FrictionType::TwoDirectional:	return physx::PxFrictionType::eTWO_DIRECTIONAL;
		}

		return physx::PxFrictionType::ePATCH;
	}

	physx::PxFilterData BuildFilterData(const PhysicsLayer& layerInfo, CollisionDetectionType collisionDetection)
	{
		physx::PxFilterData filterData;
		filterData.word0 = layerInfo.BitValue;
		filterData.word1 = layerInfo.CollidesWith;
		filterData.word2 = (uint32_t)collisionDetection;
		filterData.word3 = layerInfo.LayerID;

		return filterData;
	}

	const char* PhysXGeometryTypeToString(physx::PxGeometryType::Enum geometryType)
	{
		switch (geometryType)
		{
			case physx::PxGeometryType::eSPHERE: return "Sphere";
			case physx::PxGeometryType::ePLANE: return "Plane";
			case physx::PxGeometryType::eCAPSULE: return "Capsule";
			case physx::PxGeometryType::eBOX: return "Box";
			case physx::PxGeometryType::eCONVEXMESH: return "Convex Mesh";
			case physx::PxGeometryType::eTRIANGLEMESH: return "Triangle Mesh";
			case physx::PxGeometryType::eHEIGHTFIELD: return "Height Field";
		}

		ANT_CORE_VERIFY(false);
		return "Invalid";
	}
}