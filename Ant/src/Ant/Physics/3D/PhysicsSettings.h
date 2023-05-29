#pragma once

#include "PhysicsTypes.h"

#include <glm/glm.hpp>

namespace Ant {

	enum class PhysicsDebugType
	{
		DebugToFile = 0,
		LiveDebug
	};

	struct PhysicsSettings
	{
		float FixedTimestep = 1.0f / 100.0f;
		glm::vec3 Gravity = { 0.0f, -9.81f, 0.0f };
		BroadphaseType BroadphaseAlgorithm = BroadphaseType::AutomaticBoxPrune;
		glm::vec3 WorldBoundsMin = glm::vec3(-100.0f);
		glm::vec3 WorldBoundsMax = glm::vec3(100.0f);
		uint32_t WorldBoundsSubdivisions = 2;
		FrictionType FrictionModel = FrictionType::Patch;
		uint32_t SolverIterations = 8;
		uint32_t SolverVelocityIterations = 2;

#ifdef ANT_DEBUG
		bool DebugOnPlay = true;
		PhysicsDebugType DebugType = PhysicsDebugType::LiveDebug;
#endif
	};

}