#include "antpch.h"
#include "AnimationController.h"

#include "Ant/Debug/Profiler.h"

#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <algorithm>

namespace Ant {

	namespace Utils {

		float AngleAroundYAxis(const glm::quat& quat)
		{
			static glm::vec3 xAxis = { 1.0f, 0.0f, 0.0f };
			static glm::vec3 yAxis = { 0.0f, 1.0f, 0.0f };
			auto rotatedOrthogonal = quat * xAxis;
			auto projected = glm::normalize(rotatedOrthogonal - (yAxis * glm::dot(rotatedOrthogonal, yAxis)));
			return acos(glm::dot(xAxis, projected));
		}

	}

	void AnimationController::SetSkeletonAsset(Ref<SkeletonAsset> skeletonAsset)
	{
		m_SkeletonAsset = skeletonAsset;
	}

	void AnimationController::OnUpdate(Timestep ts, bool isRootMotionEnabled, AnimationData& data) const
	{
		ANT_PROFILE_FUNC();
		data.m_RootMotion = {};

		if (m_AnimationStates.empty())
			return;

		const auto& state = m_AnimationStates[data.m_StateIndex];

		if (!state->IsValid())
			return;

		// getting assets is relatively expensive.  Do it here, once only.
		const auto skeletonAsset = GetSkeletonAsset();
		const auto animationAsset = state->GetAnimationAsset();

		if (!skeletonAsset || !animationAsset)
			return;

		const auto& skeleton = skeletonAsset->GetSkeleton();
		const auto& animation = animationAsset->GetAnimation(state->GetAnimationIndex(), skeleton);

		if (data.m_IsAnimationPlaying)
		{
			data.m_AnimationTime += ts * data.m_PlaybackSpeed / animation.GetDuration();
			if (state->IsLooping())
			{
				// Wraps the unit interval [0:1], even for negative values (the reason for using floorf).
				data.m_AnimationTime -= floorf(data.m_AnimationTime);
			}
			else
			{
				data.m_AnimationTime = std::clamp(0.0f, data.m_AnimationTime, 1.0f);
			}
		}

		if (data.m_AnimationTime != data.m_PreviousAnimationTime)
		{
			SampleAnimation(data, *state, skeleton, animation);

			if (isRootMotionEnabled)
			{
				// Need to remove root motion from the sampled bone transforms.
				// Otherwise it potentially gets applied twice (once when caller of this function "applies" the returned
				// root motion, and once again by the bone transforms).

				// Extract root transform so we can fiddle with it...
				glm::vec3 translation = data.m_LocalTranslations[0];
				glm::quat rotation = data.m_LocalRotations[0];
				glm::mat4 rootTransform = glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(rotation);

				// remove components of root transform, based on root motion extract mask
				translation = state->GetRootTranslationMask() * translation;
				if (state->GetRootRotationMask() > 0.0f)
				{
					auto angleY = Utils::AngleAroundYAxis(rotation);
					rotation = glm::quat(cos(angleY * 0.5f), glm::vec3{ 0.0f, 1.0f, 0.0f } *sin(angleY * 0.5f));
				}
				else
				{
					rotation = glm::identity<glm::quat>();
				}
				glm::mat4 extractTransform = glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(rotation);
				rootTransform = glm::inverse(extractTransform) * rootTransform;
				translation = rootTransform[3];
				rotation = glm::quat_cast(rootTransform);

				// put the modified root transform back into the bone transforms
				data.m_LocalTranslations[0] = translation;
				data.m_LocalRotations[0] = rotation;
			}

			data.m_PreviousAnimationTime = data.m_AnimationTime;
		}
	}

	void AnimationController::SetStateIndex(const uint32_t stateIndex, AnimationData& data) const
	{
		if (stateIndex != data.m_StateIndex)
		{
			data.m_PreviousAnimationTime = 0.0f;
			data.m_StateIndex = stateIndex;

			const auto& state = m_AnimationStates[data.m_StateIndex];
			if (!state->IsValid())
				return;

			const auto& skeleton = GetSkeletonAsset()->GetSkeleton();
			const auto& animation = state->GetAnimationAsset()->GetAnimation(state->GetAnimationIndex(), skeleton);

			data.m_AnimationTime = 1.0f;
			SampleAnimation(data, *state, skeleton, animation);
			data.m_RootPoseEnd = data.m_RootPose;

			data.m_AnimationTime = 0.0f;
			SampleAnimation(data, *state, skeleton, animation);
			data.m_RootPoseStart = data.m_RootPose;
		}
	}

	void AnimationController::SetAnimationState(const std::string_view stateName, Ref<AnimationState> state)
	{
		size_t existingIndex = ~0;
		for (size_t i = 0; i < GetNumStates(); ++i)
		{
			if (m_StateNames[i] == stateName)
			{
				break;
			}
		}

		if (existingIndex != ~0)
		{
			m_AnimationStates[existingIndex] = state;
		}
		else
		{
			m_StateNames.emplace_back(stateName);
			m_AnimationStates.emplace_back(state);
		}
	}


	size_t AnimationController::AddState(std::string_view name, Ref<AnimationState> state)
	{
		m_StateNames.emplace_back(name);
		m_AnimationStates.emplace_back(state);
		return m_StateNames.size() - 1;
	}


	void AnimationController::RemoveState(size_t stateIndex)
	{
		ANT_CORE_ASSERT(stateIndex < m_StateNames.size());
		ANT_CORE_ASSERT(stateIndex < m_AnimationStates.size());
		m_StateNames.erase(m_StateNames.begin() + stateIndex);
		m_AnimationStates.erase(m_AnimationStates.begin() + stateIndex);
	}


	void AnimationController::SampleAnimation(AnimationData& data, const AnimationState& state, const Skeleton& skeleton, const Animation& animation) const
	{
		ANT_PROFILE_FUNC();

		if (data.m_TranslationCache.GetAnimation() != &animation)
		{
			data.m_TranslationCache.Reset(&animation, data.m_LocalTranslations);
			data.m_RotationCache.Reset(&animation, data.m_LocalRotations);
			data.m_ScaleCache.Reset(&animation, data.m_LocalScales);
		}

		data.m_TranslationCache.Step(data.m_AnimationTime, animation.GetTranslationKeys());
		data.m_RotationCache.Step(data.m_AnimationTime, animation.GetRotationKeys());
		data.m_ScaleCache.Step(data.m_AnimationTime, animation.GetScaleKeys());

		data.m_TranslationCache.Interpolate(data.m_AnimationTime, data.m_LocalTranslations, [](const glm::vec3& a, const glm::vec3& b, const float t) {return glm::mix(a, b, t); });
		data.m_RotationCache.Interpolate(data.m_AnimationTime, data.m_LocalRotations, [](const glm::quat& a, const glm::quat& b, const float t) {return glm::slerp(a, b, t); });
		data.m_ScaleCache.Interpolate(data.m_AnimationTime, data.m_LocalScales, [](const glm::vec3& a, const glm::vec3& b, const float t) {return glm::mix(a, b, t); });

		// Get pose of root bone right now...
		RootPose rootPoseNew = { data.m_LocalTranslations[0], data.m_LocalRotations[0] };

		// ... and work out how much it has changed since the previous call to SampleAnimation().
		// This change is the "root motion".
		// Bear in mind some tricky cases:
		// 1) The animation might have looped back around to the beginning.
		// 2) We might be playing the animation backwards.
		// 3) We might have paused the animation, and be "debugging" it, stepping backwards (or forwards) deliberately.
		if (data.m_IsAnimationPlaying)
		{
			const auto& skeleton = GetSkeletonAsset();
			if (data.m_PlaybackSpeed >= 0.0f)
			{
				// Animation is playing forwards
				if (data.m_AnimationTime >= data.m_PreviousAnimationTime)
				{
					data.m_RootMotion.Translation = state.GetRootTranslationExtractMask() * (rootPoseNew.Translation - data.m_RootPose.Translation);
					data.m_RootMotion.Rotation = (state.GetRootRotationExtractMask() > 0.0)
						? glm::conjugate(data.m_RootPose.Rotation) * rootPoseNew.Rotation
						: glm::identity<glm::quat>()
						;
				}
				else
				{
					data.m_RootMotion.Translation = state.GetRootTranslationExtractMask() * (data.m_RootPoseEnd.Translation - data.m_RootPose.Translation + rootPoseNew.Translation - data.m_RootPoseStart.Translation);
					data.m_RootMotion.Rotation = (state.GetRootRotationExtractMask() > 0.0)
						? (glm::conjugate(data.m_RootPose.Rotation) * data.m_RootPoseEnd.Rotation) * (glm::conjugate(data.m_RootPoseStart.Rotation) * rootPoseNew.Rotation)
						: glm::identity<glm::quat>()
						;
				}
			}
			else
			{
				// Animation is playing backwards
				if (data.m_AnimationTime <= data.m_PreviousAnimationTime)
				{
					data.m_RootMotion.Translation = state.GetRootTranslationExtractMask() * (rootPoseNew.Translation - data.m_RootPose.Translation);
					data.m_RootMotion.Rotation = (state.GetRootRotationExtractMask() > 0.0)
						? glm::conjugate(data.m_RootPose.Rotation) * rootPoseNew.Rotation
						: glm::identity<glm::quat>()
						;
				}
				else
				{
					data.m_RootMotion.Translation = state.GetRootTranslationExtractMask() * (data.m_RootPoseStart.Translation - data.m_RootPose.Translation + rootPoseNew.Translation - data.m_RootPoseEnd.Translation);
					data.m_RootMotion.Rotation = (state.GetRootRotationExtractMask() > 0.0)
						? (glm::conjugate(data.m_RootPose.Rotation) * data.m_RootPoseStart.Rotation) * (glm::conjugate(data.m_RootPoseEnd.Rotation) * rootPoseNew.Rotation)
						: glm::identity<glm::quat>()
						;
				}

			}
		}
		else
		{
			// Animation is paused (but may have been stepped forwards or backwards via ImGui animation "debugging" tools)
			data.m_RootMotion.Translation = state.GetRootTranslationExtractMask() * (rootPoseNew.Translation - data.m_RootPose.Translation);
			data.m_RootMotion.Rotation = (state.GetRootRotationExtractMask() > 0.0)
				? glm::conjugate(data.m_RootPose.Rotation) * rootPoseNew.Rotation
				: glm::identity<glm::quat>()
				;
		}

		data.m_RootPose = rootPoseNew;
	}
}