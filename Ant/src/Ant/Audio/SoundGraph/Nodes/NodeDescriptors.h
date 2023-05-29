#pragma once

#include "NodeTypes.h"
#include "Ant/Reflection/TypeDescriptor.h"

// TODO: TEMP
#include "Ant/Editor/NodeGraphEditor/SoundGraph/SoundGraphEditorTypes.h"

// Sometimes we need to reuse NodeProcessor types for different nodes,
// so that they have different names and other editor UI properties
// but the same NodeProcessor type in the backend.
// In such case we declare alias ID here and define it in the SoundGraphNodes.cpp,
// adding and extra entry to the registry of the same NodeProcessor type,
// but with a differen name.
// Actual NodeProcessor type for the alias is assigned in SoundgGraphFactory.cpp
//! Name aliases must already be "User Friendly Type Name" in format: "Get Random (Float)" instead of "GetRandom<float>"
namespace Ant::SoundGraph::NameAliases
{
	static constexpr auto AddAudio = "Add (Audio)"
		, AddFloatAudio = "Add (Float to Audio)"
		, MultAudioFloat = "Multiply (Audio by Float)"
		, MultAudio = "Multiply (Audio)"
		, SubtractAudio = "Subtract (Audio)"
		, MinAudio = "Min (Audio)"
		, MaxAudio = "Max (Audio)"
		, ClampAudio = "Clamp (Audio)"
		, MapRangeAudio = "Map Range (Audio)"
		, GetWave = "Get (Wave)"
		, GetRandomWave = "Get Random (Wave)"
		, ADEnvelope = "AD Envelope"
		, BPMToSeconds = "BPM to Seconds";

#undef NAME_ALIAS
}

// TODO: somehow add value overrides here?
//		Alternativelly I could create a function that takes NodeProcessor type and returns only named overrides and verifies member names

namespace Ant::Nodes {
	struct TagInputs {};
	struct TagOutputs {};
	template<typename T> struct NodeDescription;

	template<typename T>
	using DescribedNode = Type::is_specialized<NodeDescription<std::remove_cvref_t<T>>>;
}

//! Example
#if 0
DESCRIBED_TAGGED(Ant::SoundGraph::Add<float>, Ant::Nodes::TagInputs,
	&Ant::SoundGraph::Add<float>::in_Value1,
	&Ant::SoundGraph::Add<float>::in_Value2);

DESCRIBED_TAGGED(Ant::SoundGraph::Add<float>, Ant::Nodes::TagOutputs,
	&Ant::SoundGraph::Add<float>::out_Out);

template<> struct Ant::Nodes::NodeDescription<Ant::SoundGraph::Add<float>>
{
	using Inputs = Ant::Type::Description<Ant::SoundGraph::Add<float>, Ant::Nodes::TagInputs>;
	using Outputs = Ant::Type::Description<Ant::SoundGraph::Add<float>, Ant::Nodes::TagOutputs>;
};
#endif

#ifndef NODE_INPUTS
#define NODE_INPUTS(...) __VA_ARGS__
#endif // !NODE_INPUTS

#ifndef NODE_OUTPUTS
#define NODE_OUTPUTS(...) __VA_ARGS__
#endif // !NODE_OUTPUTS

// TODO: type and name overrides
// TODO: node with no inputs / outputs
#ifndef DESCRIBE_NODE
#define DESCRIBE_NODE(NodeType, InputList, OutputList)								\
	DESCRIBED_TAGGED(NodeType, Ant::Nodes::TagInputs, InputList)						\
	DESCRIBED_TAGGED(NodeType, Ant::Nodes::TagOutputs, OutputList)					\
																						\
	template<> struct Ant::Nodes::NodeDescription<NodeType>							\
	{																					\
		using Inputs = Ant::Type::Description<NodeType, Ant::Nodes::TagInputs>;		\
		using Outputs = Ant::Type::Description<NodeType, Ant::Nodes::TagOutputs>;	\
	};		
#endif // !DESCRIBE_NODE

DESCRIBE_NODE(Ant::SoundGraph::Add<float>,
	NODE_INPUTS(
		&Ant::SoundGraph::Add<float>::in_Value1,
		&Ant::SoundGraph::Add<float>::in_Value2),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Add<float>::out_Out)
);

DESCRIBE_NODE(Ant::SoundGraph::Add<int>,
	NODE_INPUTS(
		&Ant::SoundGraph::Add<int>::in_Value1,
		&Ant::SoundGraph::Add<int>::in_Value2),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Add<int>::out_Out)
);

DESCRIBE_NODE(Ant::SoundGraph::Subtract<float>,
	NODE_INPUTS(
		&Ant::SoundGraph::Subtract<float>::in_Value1,
		&Ant::SoundGraph::Subtract<float>::in_Value2),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Subtract<float>::out_Out)
);

DESCRIBE_NODE(Ant::SoundGraph::Subtract<int>,
	NODE_INPUTS(
		&Ant::SoundGraph::Subtract<int>::in_Value1,
		&Ant::SoundGraph::Subtract<int>::in_Value2),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Subtract<int>::out_Out)
);

DESCRIBE_NODE(Ant::SoundGraph::Multiply<float>,
	NODE_INPUTS(
		&Ant::SoundGraph::Multiply<float>::in_Value,
		&Ant::SoundGraph::Multiply<float>::in_Multiplier),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Multiply<float>::out_Out)
);

DESCRIBE_NODE(Ant::SoundGraph::Multiply<int>,
	NODE_INPUTS(
		&Ant::SoundGraph::Multiply<int>::in_Value,
		&Ant::SoundGraph::Multiply<int>::in_Multiplier),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Multiply<int>::out_Out)
);

DESCRIBE_NODE(Ant::SoundGraph::Divide<float>,
	NODE_INPUTS(
		&Ant::SoundGraph::Divide<float>::in_Value,
		&Ant::SoundGraph::Divide<float>::in_Denominator),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Divide<float>::out_Out)
);

DESCRIBE_NODE(Ant::SoundGraph::Divide<int>,
	NODE_INPUTS(
		&Ant::SoundGraph::Divide<int>::in_Value,
		&Ant::SoundGraph::Divide<int>::in_Denominator),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Divide<int>::out_Out)
);

DESCRIBE_NODE(Ant::SoundGraph::Power,
	NODE_INPUTS(
		&Ant::SoundGraph::Power::in_Base,
		&Ant::SoundGraph::Power::in_Exponent),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Power::out_Out)
);

DESCRIBE_NODE(Ant::SoundGraph::Log,
	NODE_INPUTS(
		&Ant::SoundGraph::Log::in_Base,
		&Ant::SoundGraph::Log::in_Value),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Log::out_Out)
);

DESCRIBE_NODE(Ant::SoundGraph::LinearToLogFrequency,
	NODE_INPUTS(
		&Ant::SoundGraph::LinearToLogFrequency::in_Value,
		&Ant::SoundGraph::LinearToLogFrequency::in_Min,
		&Ant::SoundGraph::LinearToLogFrequency::in_Max,
		&Ant::SoundGraph::LinearToLogFrequency::in_MinFrequency,
		&Ant::SoundGraph::LinearToLogFrequency::in_MaxFrequency),
	NODE_OUTPUTS(
		&Ant::SoundGraph::LinearToLogFrequency::out_Frequency)
);

DESCRIBE_NODE(Ant::SoundGraph::FrequencyLogToLinear,
	NODE_INPUTS(
		&Ant::SoundGraph::FrequencyLogToLinear::in_Frequency,
		&Ant::SoundGraph::FrequencyLogToLinear::in_MinFrequency,
		&Ant::SoundGraph::FrequencyLogToLinear::in_MaxFrequency,
		&Ant::SoundGraph::FrequencyLogToLinear::in_Min,
		&Ant::SoundGraph::FrequencyLogToLinear::in_Max),
	NODE_OUTPUTS(
		&Ant::SoundGraph::FrequencyLogToLinear::out_Value)
);

DESCRIBE_NODE(Ant::SoundGraph::Modulo,
	NODE_INPUTS(
		&Ant::SoundGraph::Modulo::in_Value,
		&Ant::SoundGraph::Modulo::in_Modulo),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Modulo::out_Out)
);

DESCRIBE_NODE(Ant::SoundGraph::Min<float>,
	NODE_INPUTS(
		&Ant::SoundGraph::Min<float>::in_A,
		&Ant::SoundGraph::Min<float>::in_B),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Min<float>::out_Value)
);

DESCRIBE_NODE(Ant::SoundGraph::Min<int>,
	NODE_INPUTS(
		&Ant::SoundGraph::Min<int>::in_A,
		&Ant::SoundGraph::Min<int>::in_B),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Min<int>::out_Value)
);

DESCRIBE_NODE(Ant::SoundGraph::Max<float>,
	NODE_INPUTS(
		&Ant::SoundGraph::Max<float>::in_A,
		&Ant::SoundGraph::Max<float>::in_B),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Max<float>::out_Value)
);

DESCRIBE_NODE(Ant::SoundGraph::Max<int>,
	NODE_INPUTS(
		&Ant::SoundGraph::Max<int>::in_A,
		&Ant::SoundGraph::Max<int>::in_B),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Max<int>::out_Value)
);

DESCRIBE_NODE(Ant::SoundGraph::Clamp<float>,
	NODE_INPUTS(
		&Ant::SoundGraph::Clamp<float>::in_In,
		&Ant::SoundGraph::Clamp<float>::in_Min,
		&Ant::SoundGraph::Clamp<float>::in_Max),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Clamp<float>::out_Value)
);

DESCRIBE_NODE(Ant::SoundGraph::Clamp<int>,
	NODE_INPUTS(
		&Ant::SoundGraph::Clamp<int>::in_In,
		&Ant::SoundGraph::Clamp<int>::in_Min,
		&Ant::SoundGraph::Clamp<int>::in_Max),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Clamp<int>::out_Value)
);

DESCRIBE_NODE(Ant::SoundGraph::MapRange<float>,
	NODE_INPUTS(
		&Ant::SoundGraph::MapRange<float>::in_In,
		&Ant::SoundGraph::MapRange<float>::in_InRangeA,
		&Ant::SoundGraph::MapRange<float>::in_InRangeB,
		&Ant::SoundGraph::MapRange<float>::in_OutRangeA,
		&Ant::SoundGraph::MapRange<float>::in_OutRangeB,
		&Ant::SoundGraph::MapRange<float>::in_Clamped),
	NODE_OUTPUTS(
		&Ant::SoundGraph::MapRange<float>::out_Value)
);

DESCRIBE_NODE(Ant::SoundGraph::MapRange<int>,
	NODE_INPUTS(
		&Ant::SoundGraph::MapRange<int>::in_In,
		&Ant::SoundGraph::MapRange<int>::in_InRangeA,
		&Ant::SoundGraph::MapRange<int>::in_InRangeB,
		&Ant::SoundGraph::MapRange<int>::in_OutRangeA,
		&Ant::SoundGraph::MapRange<int>::in_OutRangeB,
		&Ant::SoundGraph::MapRange<int>::in_Clamped),
	NODE_OUTPUTS(
		&Ant::SoundGraph::MapRange<int>::out_Value)
);

DESCRIBE_NODE(Ant::SoundGraph::WavePlayer,
	NODE_INPUTS(
		&Ant::SoundGraph::WavePlayer::Play,
		&Ant::SoundGraph::WavePlayer::Stop,
		&Ant::SoundGraph::WavePlayer::in_WaveAsset,
		&Ant::SoundGraph::WavePlayer::in_StartTime,
		&Ant::SoundGraph::WavePlayer::in_Loop,
		&Ant::SoundGraph::WavePlayer::in_NumberOfLoops),
	NODE_OUTPUTS(
		&Ant::SoundGraph::WavePlayer::out_OnPlay,
		&Ant::SoundGraph::WavePlayer::out_OnFinish,
		&Ant::SoundGraph::WavePlayer::out_OnLooped,
		&Ant::SoundGraph::WavePlayer::out_PlaybackPosition,
		&Ant::SoundGraph::WavePlayer::out_OutLeft,
		&Ant::SoundGraph::WavePlayer::out_OutRight)
);

DESCRIBE_NODE(Ant::SoundGraph::Get<float>,
	NODE_INPUTS(
		&Ant::SoundGraph::Get<float>::Trigger,
		&Ant::SoundGraph::Get<float>::in_Array,
		&Ant::SoundGraph::Get<float>::in_Index),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Get<float>::out_OnTrigger,
		&Ant::SoundGraph::Get<float>::out_Element)
);

DESCRIBE_NODE(Ant::SoundGraph::Get<int>,
	NODE_INPUTS(
		&Ant::SoundGraph::Get<int>::Trigger,
		&Ant::SoundGraph::Get<int>::in_Array,
		&Ant::SoundGraph::Get<int>::in_Index),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Get<int>::out_OnTrigger,
		&Ant::SoundGraph::Get<int>::out_Element)
);

DESCRIBE_NODE(Ant::SoundGraph::Get<int64_t>,
	NODE_INPUTS(
		&Ant::SoundGraph::Get<int64_t>::Trigger,
		&Ant::SoundGraph::Get<int64_t>::in_Array,
		&Ant::SoundGraph::Get<int64_t>::in_Index),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Get<int64_t>::out_OnTrigger,
		&Ant::SoundGraph::Get<int64_t>::out_Element)
);

DESCRIBE_NODE(Ant::SoundGraph::GetRandom<float>,
	NODE_INPUTS(
		&Ant::SoundGraph::GetRandom<float>::Next,
		&Ant::SoundGraph::GetRandom<float>::Reset,
		&Ant::SoundGraph::GetRandom<float>::in_Array,
		&Ant::SoundGraph::GetRandom<float>::in_Seed),
	NODE_OUTPUTS(
		&Ant::SoundGraph::GetRandom<float>::out_OnNext,
		&Ant::SoundGraph::GetRandom<float>::out_OnReset,
		&Ant::SoundGraph::GetRandom<float>::out_Element)
);

DESCRIBE_NODE(Ant::SoundGraph::GetRandom<int>,
	NODE_INPUTS(
		&Ant::SoundGraph::GetRandom<int>::Next,
		&Ant::SoundGraph::GetRandom<int>::Reset,
		&Ant::SoundGraph::GetRandom<int>::in_Array,
		&Ant::SoundGraph::GetRandom<int>::in_Seed),
	NODE_OUTPUTS(
		&Ant::SoundGraph::GetRandom<int>::out_OnNext,
		&Ant::SoundGraph::GetRandom<int>::out_OnReset,
		&Ant::SoundGraph::GetRandom<int>::out_Element)
);

DESCRIBE_NODE(Ant::SoundGraph::GetRandom<int64_t>,
	NODE_INPUTS(
		&Ant::SoundGraph::GetRandom<int64_t>::Next,
		&Ant::SoundGraph::GetRandom<int64_t>::Reset,
		&Ant::SoundGraph::GetRandom<int64_t>::in_Array,
		&Ant::SoundGraph::GetRandom<int64_t>::in_Seed),
	NODE_OUTPUTS(
		&Ant::SoundGraph::GetRandom<int64_t>::out_OnNext,
		&Ant::SoundGraph::GetRandom<int64_t>::out_OnReset,
		&Ant::SoundGraph::GetRandom<int64_t>::out_Element)
);

DESCRIBE_NODE(Ant::SoundGraph::Random<int>,
	NODE_INPUTS(
		&Ant::SoundGraph::Random<int>::Next,
		&Ant::SoundGraph::Random<int>::Reset,
		&Ant::SoundGraph::Random<int>::in_Min,
		&Ant::SoundGraph::Random<int>::in_Max,
		&Ant::SoundGraph::Random<int>::in_Seed),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Random<int>::out_OnNext,
		&Ant::SoundGraph::Random<int>::out_OnReset,
		&Ant::SoundGraph::Random<int>::out_Value)
);

DESCRIBE_NODE(Ant::SoundGraph::Random<float>,
	NODE_INPUTS(
		&Ant::SoundGraph::Random<float>::Next,
		&Ant::SoundGraph::Random<float>::Reset,
		&Ant::SoundGraph::Random<float>::in_Min,
		&Ant::SoundGraph::Random<float>::in_Max,
		&Ant::SoundGraph::Random<float>::in_Seed),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Random<float>::out_OnNext,
		&Ant::SoundGraph::Random<float>::out_OnReset,
		&Ant::SoundGraph::Random<float>::out_Value)
);

DESCRIBE_NODE(Ant::SoundGraph::Noise,
	NODE_INPUTS(
		&Ant::SoundGraph::Noise::in_Seed,
		&Ant::SoundGraph::Noise::in_Type),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Noise::out_Value)
);

DESCRIBE_NODE(Ant::SoundGraph::Sine,
	NODE_INPUTS(
		&Ant::SoundGraph::Sine::ResetPhase,
		&Ant::SoundGraph::Sine::in_Frequency,
		&Ant::SoundGraph::Sine::in_PhaseOffset),
	NODE_OUTPUTS(
		&Ant::SoundGraph::Sine::out_Sine)
);

DESCRIBE_NODE(Ant::SoundGraph::ADEnvelope,
	NODE_INPUTS(
		&Ant::SoundGraph::ADEnvelope::Trigger,
		&Ant::SoundGraph::ADEnvelope::in_AttackTime,
		&Ant::SoundGraph::ADEnvelope::in_DecayTime,
		&Ant::SoundGraph::ADEnvelope::in_AttackCurve,
		&Ant::SoundGraph::ADEnvelope::in_DecayCurve,
		&Ant::SoundGraph::ADEnvelope::in_Looping),
	NODE_OUTPUTS(
		&Ant::SoundGraph::ADEnvelope::out_OnTrigger,
		&Ant::SoundGraph::ADEnvelope::out_OnComplete,
		&Ant::SoundGraph::ADEnvelope::out_OutEnvelope)
);


DESCRIBE_NODE(Ant::SoundGraph::RepeatTrigger,
	NODE_INPUTS(
		&Ant::SoundGraph::RepeatTrigger::Start,
		&Ant::SoundGraph::RepeatTrigger::Stop,
		&Ant::SoundGraph::RepeatTrigger::in_Period),
	NODE_OUTPUTS(
		&Ant::SoundGraph::RepeatTrigger::out_Trigger)
);

DESCRIBE_NODE(Ant::SoundGraph::TriggerCounter,
	NODE_INPUTS(
		&Ant::SoundGraph::TriggerCounter::Trigger,
		&Ant::SoundGraph::TriggerCounter::Reset,
		&Ant::SoundGraph::TriggerCounter::in_StartValue,
		&Ant::SoundGraph::TriggerCounter::in_StepSize,
		&Ant::SoundGraph::TriggerCounter::in_ResetCount),
	NODE_OUTPUTS(
		&Ant::SoundGraph::TriggerCounter::out_OnTrigger,
		&Ant::SoundGraph::TriggerCounter::out_OnReset,
		&Ant::SoundGraph::TriggerCounter::out_Count,
		&Ant::SoundGraph::TriggerCounter::out_Value)
);

DESCRIBE_NODE(Ant::SoundGraph::DelayedTrigger,
	NODE_INPUTS(
		&Ant::SoundGraph::DelayedTrigger::Trigger,
		&Ant::SoundGraph::DelayedTrigger::Reset,
		&Ant::SoundGraph::DelayedTrigger::in_DelayTime),
	NODE_OUTPUTS(
		&Ant::SoundGraph::DelayedTrigger::out_DelayedTrigger,
		&Ant::SoundGraph::DelayedTrigger::out_OnReset)
);


DESCRIBE_NODE(Ant::SoundGraph::BPMToSeconds,
	NODE_INPUTS(
		&Ant::SoundGraph::BPMToSeconds::in_BPM),
	NODE_OUTPUTS(
		&Ant::SoundGraph::BPMToSeconds::out_Seconds)
);

DESCRIBE_NODE(Ant::SoundGraph::NoteToFrequency<float>,
	NODE_INPUTS(
		&Ant::SoundGraph::NoteToFrequency<float>::in_MIDINote),
	NODE_OUTPUTS(
		&Ant::SoundGraph::NoteToFrequency<float>::out_Frequency)
);

DESCRIBE_NODE(Ant::SoundGraph::NoteToFrequency<int>,
	NODE_INPUTS(
		&Ant::SoundGraph::NoteToFrequency<int>::in_MIDINote),
	NODE_OUTPUTS(
		&Ant::SoundGraph::NoteToFrequency<int>::out_Frequency)
);

DESCRIBE_NODE(Ant::SoundGraph::FrequencyToNote,
	NODE_INPUTS(
		&Ant::SoundGraph::FrequencyToNote::in_Frequency),
	NODE_OUTPUTS(
		&Ant::SoundGraph::FrequencyToNote::out_MIDINote)
);

#include "choc/text/choc_StringUtilities.h"
#include <optional>

//=============================================================================
/**
	Utilities to procedurally registerand initialize node processor endpoints.
*/
namespace Ant::SoundGraph::EndpointUtilities
{
	namespace Impl
	{
		// TODO: remove  prefix from the members, maybe keep in_ / out_
		constexpr std::string_view RemovePrefixAndSuffix(std::string_view name)
		{
			if (Ant::Utils::StartsWith(name, "in_"))
				name.remove_prefix(sizeof("in_") - 1);
			else if (Ant::Utils::StartsWith(name, "out_"))
				name.remove_prefix(sizeof("out_") - 1);

			return name;
		}

		//=============================================================================
		/// Implementation of the RegisterEndpoints function. Parsing type data into
		/// node processor enpoints.
		template<typename T>
		static bool RegisterEndpointInputsImpl(NodeProcessor* node, T& v, std::string_view memberName)
		{
			using TMember = T;
			constexpr bool isInputEvent = std::is_member_function_pointer_v<T>;
			//constexpr bool isOutputEvent = std::is_same_v<TMember, Ant::SoundGraph::NodeProcessor::OutputEvent>;

			if constexpr (isInputEvent)
			{
			}
			/*else if constexpr (isOutputEvent)
			{
				node->AddOutEvent(Identifier(RemovePrefixAndSuffix(memberName)), v);
			}*/
			else
			{
				//const bool isArray = std::is_array_v<TMember>;
				//using TMemberDecay = std::conditional_t<isArray, std::remove_pointer_t<std::decay_t<TMember>>, std::decay_t<TMember>>;

				//const bool isArray = Type::is_array_v<TMember> || std::is_array_v<TMember> || Ant::Utils::StartsWith(pinName, "in_Array") || Ant::Utils::StartsWith(pinName, "out_Array");
				//constexpr bool isInput = /*isArray ? Ant::Utils::StartsWith(pinName, "in_") : */std::is_pointer_v<TMember>;

				//if constexpr (isInput)
				{
					node->AddInStream(Identifier(RemovePrefixAndSuffix(memberName)));
				}
			}

			return true;
		}

		template<typename T>
		static bool RegisterEndpointOutputsImpl(NodeProcessor* node, T& v, std::string_view memberName)
		{
			using TMember = T;
			constexpr bool isOutputEvent = std::is_same_v<TMember, Ant::SoundGraph::NodeProcessor::OutputEvent>;

			if constexpr (isOutputEvent)
			{
				node->AddOutEvent(Identifier(RemovePrefixAndSuffix(memberName)), v);
			}
			else
			{
				node->AddOutStream<TMember>(Identifier(RemovePrefixAndSuffix(memberName)), v);
			}

			return true;
		}

		template<typename T>
		static bool InitializeInputsImpl(NodeProcessor* node, T& v, std::string_view memberName)
		{
			using TMember = T;
			constexpr bool isInputEvent = std::is_member_function_pointer_v<T>;
			constexpr bool isOutputEvent = std::is_same_v<TMember, Ant::SoundGraph::NodeProcessor::OutputEvent>;

			//? DBG
			//std::string_view str = typeid(TMember).name();

			if constexpr (isInputEvent || isOutputEvent)
			{
			}
			else
			{
				//const bool isArray = std::is_array_v<TMember>;
				//using TMemberDecay = std::conditional_t<isArray, std::remove_pointer_t<std::decay_t<TMember>>, std::decay_t<TMember>>;

				//const bool isArray = Type::is_array_v<TMember> || std::is_array_v<TMember> || Ant::Utils::StartsWith(pinName, "in_Array") || Ant::Utils::StartsWith(pinName, "out_Array");
				constexpr bool isInput = /*isArray ? Ant::Utils::StartsWith(pinName, "in_") : */std::is_pointer_v<TMember>;

				if constexpr (isInput)
					v = (TMember)node->InValue(Identifier(RemovePrefixAndSuffix(memberName))).getRawData();
			}

			return true;
		}

		//=============================================================================
		template<typename TNodeType>
		static bool RegisterEndpoints(TNodeType* node)
		{
			static_assert(Nodes::DescribedNode<TNodeType>::value);
			using InputsDescription = Nodes::NodeDescription<TNodeType>::Inputs;
			using OutputsDescription = Nodes::NodeDescription<TNodeType>::Outputs;

			const bool insResult = InputsDescription::ApplyToStaticType(
				[&node](const auto&... members)
				{
					auto unpack = [&node, memberIndex = 0](auto memberPtr) mutable
					{
						using TMember = std::remove_reference_t<decltype(memberPtr)>;
						constexpr bool isInputEvent = std::is_member_function_pointer_v<TMember>;
						const std::string_view name = InputsDescription::MemberNames[memberIndex++];

						if constexpr (isInputEvent)
						{
							// TODO: hook up fFlags (?)
							return true;
						}
						else // output events also go here because they are wrapped into a callable object
						{
							return RegisterEndpointInputsImpl(node, node->*memberPtr, name);
						}

						return true;
					};

					return (unpack(members) && ...);
				});

			const bool outsResult = OutputsDescription::ApplyToStaticType(
				[&node](const auto&... members)
				{
					auto unpack = [&node, memberIndex = 0](auto memberPtr) mutable
					{
						using TMember = std::remove_reference_t<decltype(memberPtr)>;
						constexpr bool isInputEvent = std::is_member_function_pointer_v<TMember>;
						const std::string_view name = OutputsDescription::MemberNames[memberIndex++];

						if constexpr (isInputEvent)
						{
							return true;
						}
						else // output events also go here because they are wrapped into a callable object
						{
							return RegisterEndpointOutputsImpl(node, node->*memberPtr, name);
						}

						return true;
					};

					return (unpack(members) && ...);
				});

			return insResult && outsResult;
		}

		//=============================================================================
		template<typename TNodeType>
		static bool InitializeInputs(TNodeType* node)
		{
			static_assert(Nodes::DescribedNode<TNodeType>::value);
			using InputsDescription = Nodes::NodeDescription<TNodeType>::Inputs;

			return InputsDescription::ApplyToStaticType(
				[&node](const auto&... members)
				{
					auto unpack = [&node, memberIndex = 0](auto memberPtr) mutable
					{
						using TMember = decltype(memberPtr);
						constexpr bool isInputEvent = std::is_member_function_pointer_v<TMember>;
						const std::string_view name = InputsDescription::MemberNames[memberIndex++];

						//? DBG
						//std::string_view str = typeid(TMember).name();

						if constexpr (isInputEvent)
							return true;
						else
							return InitializeInputsImpl(node, node->*memberPtr, name);
					};
					return (unpack(members) && ...);
				});
		}

	} // namespace Impl

	template<typename TNodeType>
	static bool RegisterEndpoints(TNodeType* node)
	{
		return Impl::RegisterEndpoints(node);
	}

	template<typename TNodeType>
	static bool InitializeInputs(TNodeType* node)
	{
		return Impl::InitializeInputs(node);
	}
}