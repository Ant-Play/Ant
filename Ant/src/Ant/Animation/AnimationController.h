#pragma once

#include "Animation.h"
#include "Skeleton.h"

#include "Ant/Asset/Asset.h"
#include "Ant/Core/Base.h"
#include "Ant/Core/TimeStep.h"

#include <glm/glm.hpp>

#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace Ant{

	// Represents the position and orientation (in model-space) of the root bone of a skeleton.
	// We are not interested in scale.
	struct RootPose
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };        // aka. Position
		glm::quat Rotation = glm::identity<glm::quat>();     // aka. Orientation
	};

	// Represents the motion (i.e. difference between current pose and a previous one) of the root bone of a skeleton.
	// Only translation and rotation for now (ignore scale).
	using RootMotion = RootPose;


	// AnimationState references an animation, together with some extra meta data about that animation
	// such as how we want to extract root motion, looping, etc.
	class AnimationState : public RefCounted
	{
	public:
		AnimationState() = default;
		AnimationState(Ref<AnimationAsset> animation, uint32_t index) : m_AnimationAsset(animation), m_AnimationIndex(index) {}
		virtual ~AnimationState() = default;

		// Asset from which this animation comes
		Ref<AnimationAsset> GetAnimationAsset() const { return m_AnimationAsset; }
		void SetAnimationAsset(Ref<AnimationAsset> animationAsset) {
			m_AnimationAsset = animationAsset;
			EnsureValidAnimationIndex();
		}

		// Animation Asset can contain multiple animations.
		// This AnimationState refers to the [index]th one.
		uint32_t GetAnimationIndex() const { return m_AnimationIndex; }
		void SetAnimationIndex(const uint32_t animationIndex)
		{
			m_AnimationIndex = animationIndex;
			EnsureValidAnimationIndex();
		}

		// Root Mask can be used to "zero out" some components of root bone transforms so that animations play "in place"
		// So Translation Mask = {1, 1, 0} means that root bone motion in Z axis will be zeroed out.  The animation will not move the character in Z-axis
		// For rotation, only rotation about up-axis (Y) can be zeroed.
		// RootRotationMask = 0 means animation will not rotate the character about Y-axis.  1 means that it will.
		//
		// Usually, you will want the mask here to be the opposite of the Extract Mask (see below).
		// However, it is sometimes useful to zero out components of motion that you are not extracting. (i.e. have them both set to zeros)
		const glm::vec3& GetRootTranslationMask() const { return m_RootTranslationMask; }
		void SetRootTranslationMask(const glm::vec3& mask) { m_RootTranslationMask = mask; }
		const float GetRootRotationMask() const { return m_RootRotationMask; }
		void SetRootRotationMask(const float mask) { m_RootRotationMask = mask; }

		// Root Extract Mask can be used to control which components of the root bone transforms are extracted as "root motion" so
		// that they can be applied to a root motion target.
		// So Translation Extract Mask = {0, 0, 1} means that if the animation moves the character in the Z-axis, that movement will be extracted as "root motion".
		// For rotation, only rotation about up-axis (Y) can be extracted.  RootRotationExtractMask = 1 means extract rotation about Y-axis, 0 means do not.
		const glm::vec3& GetRootTranslationExtractMask() const { return m_RootTranslationExtractMask; }
		void SetRootTranslationExtractMask(const glm::vec3& mask) { m_RootTranslationExtractMask = mask; }
		const float GetRootRotationExtractMask() const { return m_RootRotationExtractMask; }
		void SetRootRotationExtractMask(const float mask) { m_RootRotationExtractMask = mask; }

		bool IsLooping() const { return m_IsLooping; }
		void SetIsLooping(const bool b) { m_IsLooping = b; }

		bool IsValid() const { return m_IsValid; }
		void SetValid(const bool valid) { m_IsValid = valid; }

	private:
		void EnsureValidAnimationIndex() { if (m_AnimationAsset) m_AnimationIndex = glm::clamp<uint32_t>(m_AnimationIndex, 0, m_AnimationAsset->GetAnimationCount() - 1); }

	private:
		uint32_t m_AnimationIndex = 0;
		Ref<AnimationAsset> m_AnimationAsset;
		glm::vec3 m_RootTranslationMask = { 1.0f, 1.0f, 0.0f };         // default is to apply root bone transforms in X and Y only. (Z is extracted as root motion)
		glm::vec3 m_RootTranslationExtractMask = { 0.0f, 0.0f, 1.0f };  // default is to extract root bone transform in forwards (Z) axis
		float m_RootRotationMask = 1.0f;                                // default is to apply root bone rotation (Y-axis)
		float m_RootRotationExtractMask = 0.0f;                         // default is to not extract root bone rotation

		bool m_IsLooping = true;
		bool m_IsValid = false; // false indicates something wrong (e.g. AnimationAsset skeleton does not match the AnimationController)
	};


	// Caches results of sampling animations to take advantage of fact that usually animations play forwards
	// Everything will still work if you play an animation backwards, or skip around in it,
	// its just that in the usual situation (playing forwards) we can optimize some things
	template<typename T>
	class SamplingCache
	{
	public:

		const Animation* GetAnimation() const { return m_Animation; }

		uint32_t GetSize() const { return static_cast<uint32_t>(m_Values.size()) / 2; }

		void Resize(const uint32_t numTracks)
		{
			m_Values.resize(numTracks * 2, T());       // Values vector stores the current and next key for each track, interleaved.  These are the values that we interpolate to sample animation at a given time
			m_FrameTimes.resize(numTracks * 2, 0.0f);  // FrameTimes vector stores the frame time for current and next key.  This is used to figure out the interpolation between values.
		}

		void Reset(const Animation* animation, const std::vector<T>& values)
		{
			for (uint32_t i = 0, N = static_cast<uint32_t>(values.size()); i < N; ++i)
			{
				m_Values[NextIndex(i)] = values[i];
				m_FrameTimes[NextIndex(i)] = 0.0f;
			}
			m_Animation = animation;
			m_Cursor = 0;
		}

		// step cache forward to given time, using given key frames
		void Step(const float sampleTime, const std::vector<AnimationKey<T>>& keys)
		{
			if ((m_Cursor == static_cast<uint32_t>(keys.size())) || (sampleTime < m_PrevSampleTime))
			{
				Loop();
			}
			auto track = keys[m_Cursor].Track;
			while (m_FrameTimes[NextIndex(track)] <= sampleTime)
			{
				m_Values[CurrentIndex(track)] = m_Values[NextIndex(track)];
				m_Values[NextIndex(track)] = keys[m_Cursor].Value;
				m_FrameTimes[CurrentIndex(track)] = m_FrameTimes[NextIndex(track)];
				m_FrameTimes[NextIndex(track)] = keys[m_Cursor].FrameTime;

				if (++m_Cursor == static_cast<uint32_t>(keys.size()))
				{
					break;
				}
				track = keys[m_Cursor].Track;
			}
			m_PrevSampleTime = sampleTime;
		}

		// loop back to the beginning of animation
		void Loop()
		{
			m_Cursor = 0;
			for (uint32_t track = 0, N = static_cast<uint32_t>(m_Values.size() / 2); track < N; ++track)
			{
				m_FrameTimes[NextIndex(track)] = 0.0;
			}
			m_PrevSampleTime = 0.0f;
		}

		void Interpolate(const float sampleTime, std::vector<T>& result, const std::function<T(const T&, const T&, const float)>& interpolater)
		{
			for (uint32_t i = 0, N = static_cast<uint32_t>(m_Values.size()); i < N; i += 2)
			{
				const float t = (sampleTime - m_FrameTimes[i]) / (m_FrameTimes[i + 1] - m_FrameTimes[i]);
				//ANT_CORE_ASSERT(t > -0.0000001f && t < 1.0000001f);
				result[i / 2] = interpolater(m_Values[i], m_Values[i + 1], t);
			}
		}

	public:
		static uint32_t CurrentIndex(const uint32_t i) { return 2 * i; }
		static uint32_t NextIndex(const uint32_t i) { return 2 * i + 1; }

	private:
		std::vector<T> m_Values;
		std::vector<float> m_FrameTimes;

		const Animation* m_Animation = nullptr;
		float m_PrevSampleTime = 0.0f;
		uint32_t m_Cursor = 0;
	};
	using TranslationCache = SamplingCache<glm::vec3>;
	using RotationCache = SamplingCache<glm::quat>;
	using ScaleCache = SamplingCache<glm::vec3>;


	// AnimationData stores a bunch of information regarding the current state of an animated character.
	// It includes such things as the bone transforms and root motion (from most recent animation update)
	// and other cached data used to efficiently sample animation clips.
	struct AnimationData : public RefCounted
	{
		TranslationCache m_TranslationCache;
		RotationCache m_RotationCache;
		ScaleCache m_ScaleCache;

		std::vector<glm::vec3> m_LocalTranslations;
		std::vector<glm::quat> m_LocalRotations;
		std::vector<glm::vec3> m_LocalScales;

		RootPose m_RootPoseStart; // root pose at start of animation (for current state)
		RootPose m_RootPoseEnd;   // root pose and end of animation (for current state)
		RootPose m_RootPose;      // last sampled root pose
		RootMotion m_RootMotion;  // "motion" of root (i.e the difference between current sampled root pose and previous sampled root pose)

		float m_PreviousAnimationTime = 0.0f;
		float m_AnimationTime = 0.0f;
		float m_PlaybackSpeed = 1.0f;
		size_t m_StateIndex = 0;
		bool m_IsAnimationPlaying = true;

		AnimationData() = default;

		AnimationData(Ref<AnimationData> other)
			: m_TranslationCache(other->m_TranslationCache)
			, m_RotationCache(other->m_RotationCache)
			, m_ScaleCache(other->m_ScaleCache)
			, m_LocalTranslations(other->m_LocalTranslations)
			, m_LocalRotations(other->m_LocalRotations)
			, m_LocalScales(other->m_LocalScales)
			, m_RootPoseStart(other->m_RootPoseStart)
			, m_RootPoseEnd(other->m_RootPoseEnd)
			, m_RootPose(other->m_RootPose)
			, m_RootMotion(other->m_RootMotion)
			, m_PreviousAnimationTime(other->m_PreviousAnimationTime)
			, m_AnimationTime(other->m_AnimationTime)
			, m_StateIndex(other->m_StateIndex)
			, m_IsAnimationPlaying(other->m_IsAnimationPlaying)
			, m_PlaybackSpeed(other->m_PlaybackSpeed)
		{
		}

		uint32_t GetSize() const
		{
			return m_TranslationCache.GetSize();
		}

		void Resize(uint32_t numBones)
		{
			m_TranslationCache.Resize(numBones);
			m_RotationCache.Resize(numBones);
			m_ScaleCache.Resize(numBones);
		}

		void SetLocalTransforms(const std::vector<glm::vec3>& localTranslations, const std::vector<glm::quat>& localRotations, const std::vector<glm::vec3>& localScales)
		{
			m_LocalTranslations = localTranslations;
			m_LocalRotations = localRotations;
			m_LocalScales = localScales;
		}
	};


	// Responsible for controlling animations.
	// AnimationController is responsible for updating a character's AnimationData each frame
	//
	// This will eventually evolve into some sort of state machine, but for now all it does
	// is defines the state-space,  and each frame it steps AnimationData forwards in time
	// depending on whatever state it is currently in.
	class AnimationController : public Asset
	{
	public:
		virtual ~AnimationController() = default;

		static AssetType GetStaticType() { return AssetType::AnimationController; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }

		// The skeleton that is being animated
		// This is not necessarily the same as the skeleton that the mesh will eventually be skinned with.
		Ref<SkeletonAsset> GetSkeletonAsset() { return m_SkeletonAsset; }
		Ref<SkeletonAsset> GetSkeletonAsset() const { return m_SkeletonAsset; }
		void SetSkeletonAsset(Ref<SkeletonAsset> skeletonAsset);

		// Updates passed in AnimationData by passed in timestep
		void OnUpdate(Timestep ts, bool isRootMotionEnabled, AnimationData& data) const;

		// set passed in AnimationData to the specified state
		void SetStateIndex(const uint32_t stateIndex, AnimationData& data) const;

		size_t GetNumStates() const { return m_StateNames.size(); }
		const std::string& GetStateName(const size_t stateIndex) const { return m_StateNames[stateIndex]; }
		void SetStateName(const size_t stateIndex, std::string_view name) { m_StateNames[stateIndex] = name; }

		Ref<AnimationState> GetAnimationState(const size_t stateIndex) { return m_AnimationStates[stateIndex]; }
		Ref<AnimationState> GetAnimationState(const size_t stateIndex) const { return m_AnimationStates[stateIndex]; }
		void SetAnimationState(const std::string_view stateName, Ref<AnimationState> state);
		const std::vector<Ref<AnimationState>>& GetAnimationStates() const { return m_AnimationStates; }

		size_t AddState(std::string_view name, Ref<AnimationState> state);
		void RemoveState(size_t stateIndex);

	private:
		void SampleAnimation(AnimationData& data, const AnimationState& statem, const Skeleton& skeleton, const Animation& animation) const;

	private:
		std::vector<std::string> m_StateNames;
		std::vector<Ref<AnimationState>> m_AnimationStates;

		Ref<SkeletonAsset> m_SkeletonAsset;
	};
}
