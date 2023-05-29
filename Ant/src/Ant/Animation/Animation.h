#pragma once

#include "Skeleton.h"

#include "Ant/Asset/Asset.h"
#include "Ant/Core/Base.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace Ant{

	class MeshSource;

	// A single keyed (by frame time and track number) value for an animation
	template<typename T>
	struct AnimationKey
	{
		T Value;
		float FrameTime;		// 0.0f = beginning of animation clip, 1.0f = end of animation clip
		uint32_t Track;

		AnimationKey() = default;
		AnimationKey(const float frameTime, const uint32_t track, const T& value)
					: FrameTime(frameTime), Track(track), Value(value) {}

		static void Serialize(StreamWriter* serializer, const AnimationKey<T>& instance)
		{
			serializer->WriteRaw(instance.Value);
			serializer->WriteRaw(instance.FrameTime);
			serializer->WriteRaw(instance.Track);
		}

		static void Deserialize(StreamReader* deserializer, AnimationKey<T>& instance)
		{
			deserializer->ReadRaw(instance.Value);
			deserializer->ReadRaw(instance.FrameTime);
			deserializer->ReadRaw(instance.Track);
		}	
	};
	using TranslationKey = AnimationKey<glm::vec3>;
	using RotationKey = AnimationKey<glm::quat>;
	using ScaleKey = AnimationKey<glm::vec3>;

	// Animation is a collection of keyed values for translation, rotation, and scale of a number of "tracks"
	// Typically one "track" = one bone of a skeleton.
	// (but later we may also want to animate other things, so a "track" isn't necessarily a bone)
	class Animation
	{
	public:
		Animation() = default;
		Animation(const std::string_view name, const float duration);

		const std::string& GetName() const { return m_Name; }
		float GetDuration() const { return m_Duration; }

		void SetKeyFrames(std::vector<TranslationKey> translations, std::vector<RotationKey> rotations, std::vector<ScaleKey> scales);

		const auto& GetTranslationKeys() const { return m_TranslationKeys; }
		const auto& GetRotationKeys() const { return m_RotationKeys; }
		const auto& GetScaleKeys() const { return m_ScaleKeys; }

		static void Serialize(StreamWriter* serializer, const Animation& instance)
		{
			serializer->WriteArray(instance.m_TranslationKeys);
			serializer->WriteArray(instance.m_RotationKeys);
			serializer->WriteArray(instance.m_ScaleKeys);
			serializer->WriteString(instance.m_Name);
			serializer->WriteRaw(instance.m_Duration);
		}

		static void Deserialize(StreamReader* deserializer, Animation& instance)
		{
			deserializer->ReadArray(instance.m_TranslationKeys);
			deserializer->ReadArray(instance.m_RotationKeys);
			deserializer->ReadArray(instance.m_ScaleKeys);
			deserializer->ReadString(instance.m_Name);
			deserializer->ReadRaw(instance.m_Duration);
		}
	private:
		std::vector<TranslationKey> m_TranslationKeys;
		std::vector<RotationKey> m_RotationKeys;
		std::vector<ScaleKey> m_ScaleKeys;
		std::string m_Name;
		float m_Duration;
	};

	// Animation (or Animations) asset.
	// Animations ultimately come from a mesh source (so named because it just so happened that meshes were
	// the first thing that "sources" were used for).   A source is an externally authored digital content file)
	class AnimationAsset : public Asset
	{
	public:
		AnimationAsset(const UUID meshSource);
		AnimationAsset(const Ref<MeshSource> meshSource);

		static AssetType GetStaticType() { return AssetType::Animation; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }

		Ref<MeshSource> GetMeshSource() const;

		uint32_t GetAnimationCount() const;
		const Animation& GetAnimation(const uint32_t animationIndex, const Skeleton& skeleton) const;
	private:
		Ref<MeshSource> m_MeshSource;
	};
}

