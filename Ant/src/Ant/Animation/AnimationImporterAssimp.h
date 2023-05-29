#pragma once

#include "Animation.h"
#include "Skeleton.h"

#include "Ant/Core/Base.h"

#include <assimp/scene.h>

#include <string_view>
#include <vector>

namespace Ant::AnimationImporterAssimp {

#ifdef ANT_DIST
	Scope<Skeleton> ImportSkeleton(const std::string_view filename) { return nullptr; }
	Scope<Skeleton> ImportSkeleton(const aiScene* scene) { return nullptr; }

	Scope<Animation> ImportAnimation(const std::string_view filename, const Skeleton& skeleton) { return nullptr; }
	std::vector<std::string> GetAnimationNames(const aiScene* scene) { return std::vector<std::string>(); }
	Scope<Animation> ImportAnimation(const aiScene* scene, const std::string_view animationName, const Skeleton& skeleton) { return nullptr; }
#else
	Scope<Skeleton> ImportSkeleton(const std::string_view filename);
	Scope<Skeleton> ImportSkeleton(const aiScene* scene);

	Scope<Animation> ImportAnimation(const std::string_view filename, const Skeleton& skeleton);
	std::vector<std::string> GetAnimationNames(const aiScene* scene);
	Scope<Animation> ImportAnimation(const aiScene* scene, const std::string_view animationName, const Skeleton& skeleton);

#endif // ANT_DIST
}
