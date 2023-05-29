#pragma once

#include "Ant/Scene/Components.h"
#include "PhysicsTypes.h"
#include "PhysicsLayer.h"

#include <PhysX/PxPhysicsAPI.h>
#include <glm/gtc/type_ptr.hpp>

namespace Ant {

	namespace PhysXUtils {

		physx::PxTransform ToPhysXTransform(const TransformComponent& transform);
		physx::PxTransform ToPhysXTransform(const glm::mat4& transform);
		physx::PxTransform ToPhysXTransform(const glm::vec3& translation, const glm::quat& rotation);
		physx::PxMat44 ToPhysXMatrix(const glm::mat4& matrix);
		const physx::PxVec3& ToPhysXVector(const glm::vec3& vector);
		const physx::PxVec4& ToPhysXVector(const glm::vec4& vector);
		physx::PxExtendedVec3 ToPhysXExtendedVector(const glm::vec3& vector);
		physx::PxQuat ToPhysXQuat(const glm::quat& quat);

		glm::mat4 FromPhysXTransform(const physx::PxTransform& transform);
		glm::mat4 FromPhysXMatrix(const physx::PxMat44& matrix);
		glm::vec3 FromPhysXVector(const physx::PxVec3& vector);
		glm::vec4 FromPhysXVector(const physx::PxVec4& vector);
		glm::quat FromPhysXQuat(const physx::PxQuat& quat);

		CookingResult FromPhysXCookingResult(physx::PxConvexMeshCookingResult::Enum cookingResult);
		CookingResult FromPhysXCookingResult(physx::PxTriangleMeshCookingResult::Enum cookingResult);
		const char* CookingResultToString(CookingResult cookingResult);

		physx::PxBroadPhaseType::Enum AntToPhysXBroadphaseType(BroadphaseType type);
		physx::PxFrictionType::Enum AntToPhysXFrictionType(FrictionType type);

		physx::PxFilterData BuildFilterData(const PhysicsLayer& layerInfo, CollisionDetectionType collisionDetection);

		const char* PhysXGeometryTypeToString(physx::PxGeometryType::Enum geometryType);

	}
}
