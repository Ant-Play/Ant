#include "antpch.h"
#include "SoundGraphNodes.h"
#include "Ant/Editor/NodeGraphEditor/SoundGraph/SoundGraphEditorTypes.h"
#include "Ant/Editor/NodeGraphEditor/SoundGraph/SoundGraphGraphEditor.h"

#include "Ant/Audio/SoundGraph/Nodes/NodeTypes.h"
#include "Ant/Audio/SoundGraph/Nodes/NodeDescriptors.h"

#include <array>

// This exists just to not have to copy paste category when declaring Node factory lists
class FactoryUtility
{
public:
	using TMap = std::map<std::string, std::function<Ant::Node* ()>>;
	using TElement = TMap::value_type;

	static TMap AssignCategory(const std::string& category, std::initializer_list<TElement>&& factories)
	{
		TMap map;

		for (const auto& factory : factories)
		{
			auto factoryFunction = factory.second;

			map[factory.first] = [category, factoryFunction]
			{
				Ant::Node* node = factoryFunction();
				node->Category = category;
				return node;
			};
		}

		return map;
	}
};

#define NODE_CATEGORY(category, nodeFactories, ...) { std::string { #category }, FactoryUtility::AssignCategory(std::string{ #category }, { nodeFactories, __VA_ARGS__ }) }
//#define NODE_CATEGORY(category, nodeFactories, ...) { std::string { #category }, std::map<std::string, std::function<Node*()>>{ nodeFactories, __VA_ARGS__ }}

#define DECLARE_NODE(category, name) {choc::text::replace(#name, "_", " "), category::name}
#define DECLARE_ARGS(name, args) {#name, args}

namespace Alias = Ant::SoundGraph::NameAliases;
namespace SG = Ant::SoundGraph;

namespace Ant::Nodes {

	//==========================================================================
	/// Base Nodes
	class Base
	{
	public:
		static Node* Input_Action()
		{
			const std::string nodeName = choc::text::replace(__func__, "_", " ");

			auto* node = new SGTypes::SGNode(0, nodeName.c_str(), ImColor(255, 128, 128));
			node->Category = "Base";

			node->Outputs.push_back(SGTypes::CreatePinForType(SGTypes::ESGPinType::Flow, "Play"));

			return node;
		}

		static Node* Output_Audio()
		{
			const std::string nodeName = choc::text::replace(__func__, "_", " ");

			auto* node = new SGTypes::SGNode(0, nodeName.c_str(), ImColor(255, 128, 128));
			node->Category = "Base";

			node->Inputs.push_back(SGTypes::CreatePinForType(SGTypes::ESGPinType::Audio, "Left"));
			node->Inputs.push_back(SGTypes::CreatePinForType(SGTypes::ESGPinType::Audio, "Right"));

			return node;
		}

		static Node* On_Finished()
		{
			const std::string nodeName = choc::text::replace(__func__, "_", " ");

			auto* node = new SGTypes::SGNode(0, nodeName.c_str(), ImColor(255, 128, 128));
			node->Category = "Base";

			node->Inputs.push_back(SGTypes::CreatePinForType(SGTypes::ESGPinType::Flow, "On Finished"));

			return node;
		}
	};


	class Utility
	{
	public:
		static Node* Comment()
		{
			const std::string nodeName = choc::text::replace(__func__, "_", " ");

			auto* node = new SGTypes::SGNode(0, nodeName.c_str());
			node->Category = "Utility";

			node->Type = NodeType::Comment;
			node->Color = ImColor(255, 255, 255, 20);
			node->Size = ImVec2(300, 200);

			return node;
		}
	};


	template<class TNodeType>
	static Node* CreateNode(std::string_view name, const ImColor& color, NodeType type)
	{
		static_assert(Nodes::DescribedNode<TNodeType>::value, "NodeProcessor type must be described.");
		Node* node = SoundGraphNodeFactory::CreateNodeForProcessor<TNodeType>();
		node->Name = name;
		node->Color = color;
		node->Type = type;

		auto changePinType = [](Pin*& pin, SGTypes::ESGPinType newType)
		{
			if (pin->GetType() == newType)
				return;

			// create new pin of the new type
			Pin* newPin = SGTypes::CreatePinForType(newType);

			// copy data from the old pin to the new pin
			*newPin = *pin;

			// delete old pin
			delete (pin);

			// emplace new pin instead of the old pin
			pin = newPin;
		};

		// set up pin type overrides for the alias,
		// might want to move this somewhere else at some point
		if (name == Alias::AddAudio || name == Alias::MultAudio || name == Alias::SubtractAudio
			|| name == Alias::MinAudio || name == Alias::MaxAudio || name == Alias::ClampAudio)
		{
			for (auto& in : node->Inputs)
				changePinType(in, SGTypes::ESGPinType::Audio);
			for (auto& out : node->Outputs)
				changePinType(out, SGTypes::ESGPinType::Audio);
		}
		else if (name == Alias::AddFloatAudio || name == Alias::MultAudioFloat || name == Alias::MapRangeAudio)
		{
			changePinType(node->Inputs[0], SGTypes::ESGPinType::Audio);
			changePinType(node->Outputs[0], SGTypes::ESGPinType::Audio);
		}

		//? this shouldn't be here
		if (name == Alias::MapRangeAudio)
		{
			node->Inputs[1]->Value = choc::value::Value(-1.0f);
			node->Inputs[2]->Value = choc::value::Value(1.0f);
			node->Inputs[3]->Value = choc::value::Value(-1.0f);
			node->Inputs[4]->Value = choc::value::Value(1.0f);
		}

		return node;
	}

	// TODO: need to assign category to the created nodes!
	template<class TNodeType>
	static std::pair<std::string, std::function<Node* ()>> CreateRegistryEntry(std::string_view typeName, const ImColor& colour, NodeType type, bool isAlias = false)
	{
		const std::string name = isAlias ? std::string(typeName) : Utils::CreateUserFriendlyTypeName(typeName);
		const auto constructor = [=] { return CreateNode<TNodeType>(name, colour, type); };
		return { name, constructor };
	}

#define DECLARE_NODE_CUSTOM(category, name) { #name, category::name }
#define DECLARE_NODE_N(TNodeType, Colour, Type) CreateRegistryEntry<TNodeType>(#TNodeType, Colour, Type)
#define DECLARE_NODE_ALIAS(AliasName, TNodeType, Colour, Type) CreateRegistryEntry<TNodeType>(AliasName, Colour, Type, true)

	//! To register a new node:
	//! 1. Create new NodeProcessor type.
	//! 2. Describe it in NodeDescriptors.h.
	//! 3. Add new entry here to the Registry under appropriate category, specify Colour and Type.
	//! 4. Optionally add pin type overrides to PinTypeOverrides if you need to represent in/out of type, for example, 'float' as a SGTypes::ESGPinType::Audio.
	//! 5. Optionally add default pin value to DefaultPinValues registry, otherwise the value is initialized to T(1).
	//! 6. Add the same new NodeProcessor type entry to the NodeProcessors registry in SoundGraphFactory.cpp.
	//
	// The static factory classes can still be used to describe utility nodes which don't have corresponding NodeProcessor,
	// like graph input nodes, or a comment node etc. They are a special case when parsing editing graph into a Graph Prototype.

	namespace Colours {
		static const ImColor Float = SGTypes::GetPinColour(SGTypes::ESGPinType::Float);
		static const ImColor Int = ImColor(128, 195, 248);// GetIconColor(SGTypes::ESGPinType::Int);
		static const ImColor Audio = SGTypes::GetPinColour(SGTypes::ESGPinType::Audio);
		static const ImColor Music = ImColor(255, 89, 183);
		static const ImColor Trigger = ImColor(232, 239, 255);
	}

	const std::map<std::string, std::map<std::string, std::function<Node* ()>>> SoundGraphNodeFactory::Registry =
	{
		NODE_CATEGORY(Base,
			DECLARE_NODE(Base, Input_Action),
			DECLARE_NODE(Base, Output_Audio),
			DECLARE_NODE(Base, On_Finished),
			DECLARE_NODE_N(SG::WavePlayer, ImColor(54, 207, 145), NodeType::Blueprint),
		),
		NODE_CATEGORY(Math,
				DECLARE_NODE_ALIAS(Alias::AddAudio, SG::Add<float>,				Colours::Audio, NodeType::Simple),
				DECLARE_NODE_ALIAS(Alias::AddFloatAudio, SG::Add<float>,		Colours::Audio, NodeType::Simple),
				DECLARE_NODE_N(SG::Add<float>,									Colours::Float, NodeType::Simple),
				DECLARE_NODE_N(SG::Add<int>,									Colours::Int, NodeType::Simple),
				DECLARE_NODE_N(SG::Divide<float>,								Colours::Float, NodeType::Simple),
				DECLARE_NODE_N(SG::Divide<int>,									Colours::Int, NodeType::Simple),
				DECLARE_NODE_N(SG::Log,											Colours::Float, NodeType::Simple),
				DECLARE_NODE_N(SG::LinearToLogFrequency,						Colours::Float, NodeType::Blueprint),
				DECLARE_NODE_N(SG::FrequencyLogToLinear,						Colours::Float, NodeType::Blueprint),
				DECLARE_NODE_N(SG::Modulo,										Colours::Int, NodeType::Simple),
				DECLARE_NODE_ALIAS(Alias::MultAudioFloat, SG::Multiply<float>,	Colours::Audio, NodeType::Simple),
				DECLARE_NODE_ALIAS(Alias::MultAudio, SG::Multiply<float>,		Colours::Audio, NodeType::Simple),
				DECLARE_NODE_N(SG::Multiply<float>,								Colours::Float, NodeType::Simple),
				DECLARE_NODE_N(SG::Multiply<int>,								Colours::Int, NodeType::Simple),
				DECLARE_NODE_N(SG::Power,										Colours::Float, NodeType::Simple),
				DECLARE_NODE_ALIAS(Alias::SubtractAudio, SG::Subtract<float>,	Colours::Audio, NodeType::Simple),
				DECLARE_NODE_N(SG::Subtract<float>,								Colours::Float, NodeType::Simple),
				DECLARE_NODE_N(SG::Subtract<int>,								Colours::Int, NodeType::Simple),

				DECLARE_NODE_N(SG::MapRange<float>,								Colours::Float, NodeType::Blueprint),
				DECLARE_NODE_N(SG::MapRange<int>,								Colours::Int, NodeType::Blueprint),
				DECLARE_NODE_ALIAS(Alias::MapRangeAudio, SG::MapRange<float>,	Colours::Audio, NodeType::Blueprint),
				DECLARE_NODE_N(SG::Min<float>,									Colours::Float, NodeType::Blueprint),
				DECLARE_NODE_N(SG::Min<int>,									Colours::Int, NodeType::Blueprint),
				DECLARE_NODE_ALIAS(Alias::MinAudio, SG::Min<float>,				Colours::Audio, NodeType::Blueprint),
				DECLARE_NODE_N(SG::Max<float>,									Colours::Float, NodeType::Blueprint),
				DECLARE_NODE_N(SG::Max<int>,									Colours::Int, NodeType::Blueprint),
				DECLARE_NODE_ALIAS(Alias::MaxAudio, SG::Max<float>,				Colours::Audio, NodeType::Blueprint),
				DECLARE_NODE_N(SG::Clamp<float>,								Colours::Float, NodeType::Blueprint),
				DECLARE_NODE_N(SG::Clamp<int>,									Colours::Int, NodeType::Blueprint),
				DECLARE_NODE_ALIAS(Alias::ClampAudio, SG::Clamp<float>,			Colours::Audio, NodeType::Blueprint),
			),
		NODE_CATEGORY(Array,
				DECLARE_NODE_N(SG::Get<float>,									Colours::Float, NodeType::Blueprint),
				DECLARE_NODE_N(SG::Get<int>,									Colours::Int, NodeType::Blueprint),
				DECLARE_NODE_ALIAS(Alias::GetWave, SG::Get<int64_t>,			SGTypes::GetPinColour(SGTypes::ESGPinType::Object), NodeType::Blueprint),
				DECLARE_NODE_N(SG::GetRandom<float>,							Colours::Float, NodeType::Blueprint),
				DECLARE_NODE_N(SG::GetRandom<int>,								Colours::Int, NodeType::Blueprint),
				DECLARE_NODE_ALIAS(Alias::GetRandomWave, SG::GetRandom<int64_t>, SGTypes::GetPinColour(SGTypes::ESGPinType::Object), NodeType::Blueprint),
		),
		NODE_CATEGORY(Generators,
				DECLARE_NODE_N(SG::Noise, ImColor(240, 235, 113), NodeType::Blueprint),
				DECLARE_NODE_N(SG::Sine, ImColor(240, 235, 113), NodeType::Blueprint),
		),
		NODE_CATEGORY(Trigger,
				DECLARE_NODE_N(SG::RepeatTrigger, Colours::Trigger, NodeType::Blueprint),
				DECLARE_NODE_N(SG::TriggerCounter, Colours::Trigger, NodeType::Blueprint),
				DECLARE_NODE_N(SG::DelayedTrigger, Colours::Trigger, NodeType::Blueprint),
		),
		NODE_CATEGORY(Envelope,
				DECLARE_NODE_ALIAS(Alias::ADEnvelope, SG::ADEnvelope, ImColor(217, 196, 255), NodeType::Blueprint),
		),
		NODE_CATEGORY(Music,
				DECLARE_NODE_ALIAS(Alias::BPMToSeconds, SG::BPMToSeconds, Colours::Music, NodeType::Blueprint),
				DECLARE_NODE_N(SG::NoteToFrequency<float>, Colours::Music, NodeType::Blueprint),
				DECLARE_NODE_N(SG::NoteToFrequency<int>, Colours::Music, NodeType::Blueprint),
				DECLARE_NODE_N(SG::FrequencyToNote, Colours::Music, NodeType::Blueprint),
		),
		NODE_CATEGORY(Utility,
				DECLARE_NODE_CUSTOM(Utility, Comment),
		),
	};

	//==============================================================================
	namespace EnumTokens {
		static const std::vector<Token> NoiseType
		{
			{ "WhiteNoise", SG::Noise::ENoiseType::WhiteNoise },
			{ "PinkNoise", SG::Noise::ENoiseType::PinkNoise },
			{ "BrownianNoise", SG::Noise::ENoiseType::BrownianNoise }
		};

	} // namespace EnumTokens

	//==============================================================================
#if 0 
	constexpr std::string_view GetClassNameFromMember(std::string_view memberFullName)
	{
		return std::string_view(memberFullName.data(),
			std::string_view(memberFullName).size() - Utils::RemoveNamespace(memberFullName).size() - 2);
	}
#endif

	std::string SanitizeMemberName(std::string_view name, bool removePrefixAndSuffix = false)
	{
		std::vector<std::string> tokens = Utils::SplitString(name, "::");
		ANT_CORE_ASSERT(tokens.size() >= 2);

		std::array<std::string, 2> validTokens = { *(tokens.rbegin() + 1),
													removePrefixAndSuffix ? std::string(Impl::RemovePrefixAndSuffix(*tokens.rbegin()))
																			: *tokens.rbegin() };
		return choc::text::joinStrings(validTokens, "::");
	}

#define DECLARE_PROCESSOR(ClassName) Utils::RemoveNamespace(#ClassName)
#define DECLARE_DEF_VALUE(MemberPointer, ValueOverride) { SanitizeMemberName(#MemberPointer, true), choc::value::Value(ValueOverride) } 
#define DECLARE_PIN_TYPE(MemberPointer, PinTypeOverride) { SanitizeMemberName(#MemberPointer, true),  PinTypeOverride }

	// TODO: add static_assert "Described"
	static const std::unordered_map<std::string, SGTypes::ESGPinType> PinTypeOverrides =
	{
		DECLARE_PIN_TYPE(SG::WavePlayer::out_OutLeft,	SGTypes::ESGPinType::Audio),
		DECLARE_PIN_TYPE(SG::WavePlayer::out_OutRight,	SGTypes::ESGPinType::Audio),
		DECLARE_PIN_TYPE(SG::Noise::in_Type,			SGTypes::ESGPinType::Enum),
		DECLARE_PIN_TYPE(SG::Noise::out_Value,			SGTypes::ESGPinType::Audio),
		DECLARE_PIN_TYPE(SG::Sine::out_Sine,			SGTypes::ESGPinType::Audio),
	};

	// TODO: should this info be just stored inside of the NodeProcessor?
	static const std::unordered_map<std::string, choc::value::Value> DefaultPinValues =
	{
		DECLARE_DEF_VALUE(SG::WavePlayer::in_StartTime, 0.0f),
		DECLARE_DEF_VALUE(SG::WavePlayer::in_Loop, false),
		DECLARE_DEF_VALUE(SG::WavePlayer::in_NumberOfLoops, int32_t(-1)),
		DECLARE_DEF_VALUE(SG::GetRandom<float>::in_Seed, int32_t(-1)),
		DECLARE_DEF_VALUE(SG::GetRandom<int>::in_Seed, int32_t(-1)),
		DECLARE_DEF_VALUE(SG::GetRandom<int64_t>::in_Seed, int32_t(-1)),
		DECLARE_DEF_VALUE(SG::Random<float>::in_Seed, int32_t(-1)),
		DECLARE_DEF_VALUE(SG::Random<int>::in_Seed, int32_t(-1)),
		DECLARE_DEF_VALUE(SG::Noise::in_Seed, int32_t(-1)),
		DECLARE_DEF_VALUE(SG::Sine::in_Frequency, float(440.0f)),
		DECLARE_DEF_VALUE(SG::Sine::in_PhaseOffset, float(0.0f)),

		DECLARE_DEF_VALUE(SG::RepeatTrigger::in_Period, 0.2f),
		DECLARE_DEF_VALUE(SG::TriggerCounter::in_StartValue, 0.0f),
		DECLARE_DEF_VALUE(SG::TriggerCounter::in_StepSize, 1.0f),
		DECLARE_DEF_VALUE(SG::TriggerCounter::in_ResetCount, int(0)),

		DECLARE_DEF_VALUE(SG::ADEnvelope::in_AttackTime, 1.0f),
		DECLARE_DEF_VALUE(SG::ADEnvelope::in_DecayTime, 1.0f),
		DECLARE_DEF_VALUE(SG::ADEnvelope::in_AttackCurve, 1.0f),
		DECLARE_DEF_VALUE(SG::ADEnvelope::in_DecayCurve, 1.0f),

		DECLARE_DEF_VALUE(SG::BPMToSeconds::in_BPM, 90.0f),
		DECLARE_DEF_VALUE(SG::NoteToFrequency<float>::in_MIDINote, 60.0f),
		DECLARE_DEF_VALUE(SG::NoteToFrequency<int>::in_MIDINote, int(60)),
		DECLARE_DEF_VALUE(SG::FrequencyToNote::in_Frequency, 440.0f),

		DECLARE_DEF_VALUE(SG::LinearToLogFrequency::in_Value, 0.5f),
		DECLARE_DEF_VALUE(SG::LinearToLogFrequency::in_Min, 0.0f),
		DECLARE_DEF_VALUE(SG::LinearToLogFrequency::in_Max, 1.0f),
		DECLARE_DEF_VALUE(SG::LinearToLogFrequency::in_MinFrequency, 20.0f),
		DECLARE_DEF_VALUE(SG::LinearToLogFrequency::in_MaxFrequency, 20000.0f),

		DECLARE_DEF_VALUE(SG::FrequencyLogToLinear::in_Frequency, 1000.0f),
		DECLARE_DEF_VALUE(SG::FrequencyLogToLinear::in_MinFrequency, 20.0f),
		DECLARE_DEF_VALUE(SG::FrequencyLogToLinear::in_MaxFrequency, 20000.0f),
		DECLARE_DEF_VALUE(SG::FrequencyLogToLinear::in_Min, 0.0f),
		DECLARE_DEF_VALUE(SG::FrequencyLogToLinear::in_Max, 1.0f),

		DECLARE_DEF_VALUE(SG::MapRange<int>::in_In, 0),
		DECLARE_DEF_VALUE(SG::MapRange<int>::in_InRangeA, 0),
		DECLARE_DEF_VALUE(SG::MapRange<int>::in_InRangeB, 100),
		DECLARE_DEF_VALUE(SG::MapRange<int>::in_OutRangeA, 0),
		DECLARE_DEF_VALUE(SG::MapRange<int>::in_OutRangeB, 100),

		DECLARE_DEF_VALUE(SG::MapRange<float>::in_In, 0.0f),
		DECLARE_DEF_VALUE(SG::MapRange<float>::in_InRangeA, 0.0f),
		DECLARE_DEF_VALUE(SG::MapRange<float>::in_OutRangeA, 0.0f),
	};

#define DECLARE_PIN_ENUM(MemberPointer, Tokens) { std::string(SanitizeMemberName(#MemberPointer, true)),  Tokens }

	// TODO: add static_assert "Described"
	const std::unordered_map<std::string, const std::vector<Token>*> SoundGraphNodeFactory::EnumTokensRegistry =
	{
		DECLARE_PIN_ENUM(SG::Noise::in_Type, &EnumTokens::NoiseType)
	};

	std::optional<choc::value::Value> GetPinDefaultValueOverride(std::string_view nodeName, std::string_view memberNameSanitized)
	{
		// Passed in names must not contain namespace
		ANT_CORE_ASSERT(nodeName.find("::") == std::string_view::npos);
		ANT_CORE_ASSERT(memberNameSanitized.find("::") == std::string_view::npos);

		const std::string fullName = choc::text::joinStrings<std::array<std::string_view, 2>>({ nodeName, memberNameSanitized }, "::");
		if (!DefaultPinValues.count(fullName))
		{
			//return choc::value::Value(1);
			return {};
		}

		return DefaultPinValues.at(fullName);
	}

	std::optional<SGTypes::ESGPinType> GetPinTypeForMemberOverride(std::string_view nodeName, std::string_view memberNameSanitized)
	{
		// Passed in names must not contain namespace
		ANT_CORE_ASSERT(nodeName.find("::") == std::string_view::npos);
		ANT_CORE_ASSERT(memberNameSanitized.find("::") == std::string_view::npos);

		const std::string fullName = choc::text::joinStrings<std::array<std::string_view, 2>>({ nodeName, memberNameSanitized }, "::");
		if (!PinTypeOverrides.count(fullName))
			return {};

		return PinTypeOverrides.at(fullName);
	}

	Node* SoundGraphNodeFactory::SpawnGraphPropertyNode(const Ref<SoundGraphAsset>& graph, std::string_view propertyName, ESoundGraphPropertyType type, std::optional<bool> isLocalVarGetter)
	{
		ANT_CORE_ASSERT(graph);
		ANT_CORE_ASSERT(((type != ESoundGraphPropertyType::LocalVariable) ^ isLocalVarGetter.has_value()), "'isLocalVarGetter' property must be set if 'type' is 'LocalVariable'");

		const bool isLocalVariable = isLocalVarGetter.has_value() && ESoundGraphPropertyType::LocalVariable;

		const Utils::PropertySet* properties = (type == ESoundGraphPropertyType::Input) ? &graph->GraphInputs
			: (type == ESoundGraphPropertyType::Output) ? &graph->GraphOutputs
			: (type == ESoundGraphPropertyType::LocalVariable) ? &graph->LocalVariables
			: nullptr;

		if (!properties)
			return nullptr;

		if (!properties->HasValue(propertyName))
		{
			ANT_CORE_ERROR("SpawnGraphPropertyNode() - property with the name \"{}\" doesn't exist!", propertyName);
			return nullptr;
		}

		choc::value::Value value = properties->GetValue(propertyName);

		const ImColor typeColour = SoundGraphNodeGraphEditor::GetTypeColour(value);
		const auto [pinType, storageKind] = SoundGraphNodeGraphEditor::GetPinTypeAndStorageKindForValue(value);

		if (!isLocalVariable && value.isVoid())
		{
			ANT_CORE_ERROR("SpawnGraphPropertyNode() - invalid Graph Property value to spawn Graph Property node. Property name {}", propertyName);
			return nullptr;
		}

		const std::string propertyTypeString = Utils::SplitAtUpperCase(magic_enum::enum_name<ESoundGraphPropertyType>(type));

		const std::string nodeName(isLocalVariable ? propertyName : propertyTypeString);

		Node* newNode = new Nodes::SGTypes::SGNode(0, nodeName, typeColour);

		newNode->Category = isLocalVariable ? propertyTypeString : nodeName;

		if (isLocalVarGetter.has_value())
			newNode->Type = NodeType::Simple;

		{
			Pin* newPin = Nodes::SGTypes::CreatePinForType(pinType, propertyName);
			newPin->Storage = storageKind;
			newPin->Value = value;

			using TList = decltype(newNode->Inputs);
			TList& list = (type == ESoundGraphPropertyType::Input) ? newNode->Outputs
				: (type == ESoundGraphPropertyType::Output) ? newNode->Inputs
				: *isLocalVarGetter ? newNode->Outputs
				: newNode->Inputs;

			newPin->Kind = &list == &newNode->Inputs ? PinKind::Input : PinKind::Output;

			list.push_back(newPin);
		}

		return newNode;
	}

} // Ant::Nodes