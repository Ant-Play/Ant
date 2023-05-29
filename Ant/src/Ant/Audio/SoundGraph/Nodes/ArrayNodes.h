#pragma once

#include "Ant/Audio/SoundGraph/NodeProcessor.h"

#include "Ant/Core/FastRandom.h"

#include <vector>
#include <chrono>

#define DECLARE_ID(name) static constexpr Identifier name{ #name }

namespace Ant::SoundGraph {

	//==============================================================================
	/** Get random item from Array.
	*/
	template<typename T>
	struct GetRandom : public NodeProcessor
	{
		struct IDs // TODO: could replace this with simple enum
		{
			DECLARE_ID(Next);
			DECLARE_ID(Reset);

			DECLARE_ID(Array);
			DECLARE_ID(NoRepeats);			// TODO
			DECLARE_ID(EnabledSharedState);	// TODO

		private:
			IDs() = delete;
		};

		explicit GetRandom(const char* dbgName, UUID id) : NodeProcessor(dbgName, id)
		{
			AddInEvent(IDs::Next, [this](float v) { fNext.SetDirty(); });
			AddInEvent(IDs::Reset, [this](float v) { fReset.SetDirty(); });

			//AddInStream(IDs::NoRepeats); // TODO
			//AddInStream(IDs::EnabledSharedState); // TODO

			EndpointUtilities::RegisterEndpoints(this);
		}

		T* in_Array;
		int* in_Seed = nullptr;

		OutputEvent out_OnNext{ *this };
		OutputEvent out_OnReset{ *this };

		T out_Element{ 0 };

	private:
		Flag fNext;
		Flag fReset;

	public:

		void Init() final
		{
			EndpointUtilities::InitializeInputs(this);

			arraySize = InValue(IDs::Array).size();

			// If seed is at default value -1, take current time as a seed

			seed = *in_Seed;
			if (seed == -1)
			{
				seed = Utils::GetSeedFromCurrentTime();
			}

			random.SetSeed(seed);
			random.GetInt32(); //? need to get rid of the initial 0 value the generator produces

			const int index = random.GetInt32InRange(0, (int)(arraySize - 1));
			out_Element = (in_Array)[index];
		}

		inline void Next()
		{
			// If input Array has changed, we need to get random index for the new range
			arraySize = InValue(IDs::Array).size();

			const int index = random.GetInt32InRange(0, (int)(arraySize - 1));
			out_Element = (in_Array)[index];
			out_OnNext(1.0f);
		}

		inline void Reset()
		{
			seed = Utils::GetSeedFromCurrentTime();
			random.SetSeed(seed);
			out_OnReset(1.0f);
		}

		void Process() final
		{
			if (fNext.CheckAndResetIfDirty())
				Next();

			if (fReset.CheckAndResetIfDirty())
				Reset();
		}

	private:
		size_t arraySize = 0;
		int seed = -1;

		FastRandom random;
	};

	//==============================================================================
	/** Get item from Array.
	*/
	template<typename T>
	struct Get : public NodeProcessor
	{
		struct IDs
		{
			DECLARE_ID(Trigger);
			DECLARE_ID(Array);
		private:
			IDs() = delete;
		};

		explicit Get(const char* dbgName, UUID id) : NodeProcessor(dbgName, id)
		{
			AddInEvent(IDs::Trigger, [this](float v) { fTrigger.SetDirty(); });

			EndpointUtilities::RegisterEndpoints(this);
		}


		T* in_Array;
		int32_t* in_Index = nullptr;

		OutputEvent out_OnTrigger{ *this };

		T out_Element{ 0 };

	private:
		Flag fTrigger;

	public:

		void Init() final
		{
			EndpointUtilities::InitializeInputs(this); EndpointUtilities::InitializeInputs(this);

			const uint32_t arraySize = InValue(IDs::Array).size();
			const auto index = (uint32_t)(*in_Index);
			//ANT_CORE_ASSERT(index < arraySize);

			const auto& element = in_Array[(index >= arraySize) ? (index % arraySize) : index];
			out_Element = element;
		}

		inline void Trigger(float v)
		{
			const uint32_t arraySize = InValue(IDs::Array).size();
			const auto index = (uint32_t)(*in_Index);
			//ANT_CORE_ASSERT(index < arraySize);

			const auto& element = in_Array[(index >= arraySize) ? (index % arraySize) : index];
			out_Element = element;
			out_OnTrigger((float)element); // TODO: value typed events? This seems to be redundand to send to Out Value and trigger Out Event
		}

		void Process() final
		{
			if (fTrigger.CheckAndResetIfDirty())
				Trigger(1.0f);
		}
	};
}
#undef DECLARE_ID