#include "antpch.h"
#include "GraphGeneration.h"

#include "Ant/Audio/SoundGraph/Nodes/NodeTypes.h"
#include "Ant/Audio/SoundGraph/NodeProcessor.h"
#include "Ant/Audio/SoundGraph/SoundGraphFactory.h"
#include "Ant/Audio/SoundGraph/SoundGraphPrototype.h"

#include "Ant/Utilities/StringUtils.h"

#include "Ant/Serialization/MemoryStream.h"

#include "text/choc_StringUtilities.h"

namespace Ant::SoundGraph {

	namespace Utils
	{
		bool isDigit(char c) { return c >= '0' && c <= '9'; };
		bool isSafeIdentifierChar(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || isDigit(c); };
		bool containsChar(const std::string& s, char c) noexcept
		{
			return s.find(c) != std::string::npos;
		};
		auto containsChar(const char* s, char c) noexcept
		{
			if (s == nullptr)
				return false;

			for (; *s != 0; ++s)
				if (*s == c)
					return true;

			return false;
		};

		static std::string MakeSafeIdentifier(std::string name)
		{
			// TODO: this is taken straight from soul stuff, move this to some sort of utility header
			auto makeSafeIdentifierName = [](std::string s) -> std::string
			{
				for (auto& c : s)
					if (containsChar(" ,./;", c))
						c = '_';

				s.erase(std::remove_if(s.begin(), s.end(), [&](char c) { return !isSafeIdentifierChar(c); }), s.end());

				// Identifiers can't start with a digit
				if (isDigit(s[0]))
					s = "_" + s;

				return s;
			};
			name = makeSafeIdentifierName(name);

			constexpr const char* reservedWords[] =
			{
				"alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel", "atomic_commit", "atomic_noexcept", "auto",
				"bitand", "bitor", "bool", "break", "case", "catch", "char", "char8_t", "char16_t", "char32_t", "class",
				"compl", "concept", "const", "consteval", "constexpr", "constinit", "const_cast", "continue", "co_await",
				"co_return", "co_yield", "decltype", "default", "delete", "do", "double", "dynamic_cast", "else", "enum",
				"explicit", "export", "extern", "false", "float", "for", "friend", "goto", "if", "inline", "int", "long",
				"mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private",
				"protected", "public", "reflexpr", "register", "reinterpret_cast", "requires", "return", "short", "signed",
				"sizeof", "static", "static_assert", "static_cast", "struct", "switch", "synchronized", "template", "this",
				"thread_local", "throw", "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using",
				"virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"
			};

			for (auto r : reservedWords)
				if (name == r)
					return name + "_";

			return name;
		}
		static std::string MangleStructOrFunctionName(const std::string& namespacedName)
		{
			return MakeSafeIdentifier(choc::text::replace(namespacedName, ":", "_"));
		}

		template <typename Vector, typename Type>
		inline bool Contains(const Vector& v, Type&& i)
		{
			return std::find(std::begin(v), std::end(v), i) != v.end();
		}

		template<typename Vector, typename Type>
		inline bool AppendIfNotPresent(Vector& v, Type&& i)
		{
			if (Contains(v, i))
				return false;

			v.push_back(i);
			return true;
		}
	}

	//==============================================================================
	struct Parser
	{
		Parser(GraphGeneratorOptions& options, Ref<Prototype>& outSoundGraph)
			: Options(options), Graph(options.Graph), OutSoundGraph(outSoundGraph)
		{
			ANT_CORE_ASSERT(Options.Model);
			ANT_CORE_ASSERT(Options.Graph);
			ANT_CORE_ASSERT(OutSoundGraph);
		}

		//==============================================================================
		GraphGeneratorOptions& Options;
		Ref<SoundGraphAsset> Graph;
		Ref<Prototype> OutSoundGraph;
		std::vector<UUID> OutWaveAssets;

		std::unordered_map<std::string, uint32_t> NodeIDMap;

		//==============================================================================
		static bool IsWaveAsset(const choc::value::ValueView& v) { return /*v.isInt64() || */ v.isObjectWithClassName(type::type_name<UUID>()); }
		static bool IsWaveAsset(const choc::value::Type& t) { return /*t.isInt64() || */t.isObjectWithClassName(type::type_name<UUID>()); }
		static uint64_t GetAssetHandleValue(const choc::value::ValueView& v)
		{
			ANT_CORE_ASSERT(IsWaveAsset(v));

			// Assuming asset handle value stored as int64 [Data]["Value"] member in choc Value object
			return v["Value"].getInt64();
		}

		/*	We want to pass WaveAsset handle as primitive integer instead of choc Value class
			because the later would impose heavy member lookup to get the value at runtime.
		*/
		static choc::value::Value TranslateIfWaveAssetValue(const choc::value::ValueView& value)
		{
			const bool isArray = value.isArray();
			const choc::value::Type& type = isArray ? value.getType().getElementType() : value.getType();

			if (!IsWaveAsset(type))
				return choc::value::Value(value);

			if (isArray)	return choc::value::createArray(value.size(), [copy = value](uint32_t i) { return (int64_t)GetAssetHandleValue(copy[i]); });
			else			return choc::value::createInt64((int64_t)GetAssetHandleValue(value));
		}

		bool IsOrphan(const Node* node) const
		{
			if (node->SortIndex == Node::UndefinedSortIndex)
				return true;

			for (const auto& in : node->Inputs)
			{
				if (Options.Model->IsPinLinked(in->ID))
					return false;
			}

			for (const auto& out : node->Outputs)
			{
				if (Options.Model->IsPinLinked(out->ID))
					return false;
			}

			return true;
		}

		//==============================================================================
		bool Run()
		{
			ConstructIO();

			ParseInputParameters();

			// TODO: Output parameters

			ParseNodes();

			// TODO: error messages for the user
			if (OutSoundGraph->Nodes.empty())
				return false;

			ParseConnections();

			// Collect all of the wave assets used by this patch for streaming
			ParseWaveReferences();

			// TODO: collect all the errors, potentially stop parsing at the first error encountered
			return true;
		}

		//==============================================================================
		void ConstructIO()
		{
			OutSoundGraph->Inputs.reserve(Options.NumInChannels);
			for (uint32_t i = 0; i < Options.NumInChannels; ++i)
			{
				const std::string idStr = std::string("In_") + std::to_string(i);
				const Identifier id(idStr.c_str());
				OutSoundGraph->Inputs.emplace_back(id, choc::value::Value(0.0f));
			}

			OutSoundGraph->Outputs.reserve(Options.NumOutChannels);
			for (uint32_t i = 0; i < Options.NumOutChannels; ++i)
			{
				const std::string idStr = std::string("Out_") + std::to_string(i);
				const Identifier id(idStr.c_str());
				OutSoundGraph->Outputs.emplace_back(id, choc::value::Value(0.0f));

				// TODO: make them reference raw member variables
				OutSoundGraph->OutputChannelIDs.push_back(id); // Store channel IDs to access in the audio callback
			}
		}

		//==============================================================================
		void ParseInputParameters()
		{
			// TODO:    parse initial values into some sort of 'Init' struct,
			//          or just use the PropertySet directly to set values after the patch has been compiled

			const Ant::Utils::PropertySet& inputs = Graph->GraphInputs;

			for (const auto& inputName : inputs.GetNames())
			{
				choc::value::Value value = inputs.GetValue(inputName);

				const bool isArray = value.isArray();
				const choc::value::Type& type = isArray ? value.getType().getElementType() : value.getType();

				if (IsWaveAsset(type))
				{
					value = TranslateIfWaveAssetValue(value);
				}
				else
				{
					ANT_CORE_ASSERT(!type.isObject());
				}

				OutSoundGraph->Inputs.emplace_back(Identifier(choc::text::replace(inputName, " ", "")), value);
			}
		}

		//==============================================================================
		void ParseNodes()
		{
			const std::vector<Node*>& nodes = Graph->Nodes;
			const Ant::Utils::PropertySet& graphInputs = Graph->GraphInputs;

			// TODO: the order matters, as it determintes initialization order

			for (const auto& node : nodes)
			{
				if (node->Type == NodeType::Comment)
					continue;

				std::string nodeName = node->Name;
				std::string safeName = Utils::MakeSafeIdentifier(nodeName);

				// TODO: these names might have changed
				if (safeName == "Input_Action" || safeName == "Output_Audio" || safeName == "Input" || safeName == "Output" || safeName == "On_Finished"
					|| nodeName == "Input Action" || nodeName == "Output Audio" || nodeName == "On Finished") // TODO: clean this up
					continue;

				// Local variable setter node is allowed to be an orphan
				const bool isLocalVariableNode = Options.Model->IsLocalVariableNode(node);

				// We don't want to parse nodes that don't have any connections (unless they send value to remote (in the future))
				if (IsOrphan(node) && !isLocalVariableNode)
					continue;

				if (isLocalVariableNode)
				{
					const bool isSetter = node->Inputs.size() == 1;
					if (isSetter)
					{
						const auto& input = node->Inputs[0];
						choc::value::ValueView value = input->Value;

						// if Local Variable doesn't have connection to other source,
						// we need to create a "default value plug" for it
						if (!value.isVoid() && !value.isString() && !input->IsType(Nodes::SGTypes::Flow))
						{
							const std::string inputValueName = choc::text::replace(input->Name, " ", "");
							const Identifier localVariableId(inputValueName);
							OutSoundGraph->LocalVariablePlugs.emplace_back(localVariableId, TranslateIfWaveAssetValue(value));
						}
					}
					continue;
				}

				//? safe identifier is unnecessary unless we're printing code
				//! in fact, here we must have the same node name we've generated from Node Descriptor
				//OutSoundGraph->Nodes.emplace_back(Identifier(choc::text::replace(safeName, "_", "")), node->ID);
				OutSoundGraph->Nodes.emplace_back(Identifier(nodeName), node->ID);

				Prototype::Node& graphNode = OutSoundGraph->Nodes.back();

				auto requiresPlug = [](const Pin* pin)
				{
					return !pin->Value.isVoid() && !pin->Value.isString() && !pin->IsType(Nodes::SGTypes::Flow);
				};

				const uint32_t defaultPlugsToAdd = (uint32_t)std::count_if(node->Inputs.begin(), node->Inputs.end(),
					[&requiresPlug](const Pin* in) { return requiresPlug(in); });

				graphNode.DefaultValuePlugs.reserve(defaultPlugsToAdd);

				// Default input values
				for (const auto& input : node->Inputs)
				{
					choc::value::ValueView value = input->Value;

					// connected pins are not Void
					if (!requiresPlug(input))
						continue;

					//? safe identifier is unnecessary unless we're printing code
					//const std::string inputValueName = choc::text::replace(Utils::MakeSafeIdentifier(input.Name), "_", "", " ", "");
					const std::string inputValueName = choc::text::replace(input->Name, " ", "");
					const Identifier destinationID(inputValueName);
					graphNode.DefaultValuePlugs.emplace_back(destinationID, TranslateIfWaveAssetValue(value));
				}
			}
		}

		//? NOT USED
		NodeProcessor* CreateNode(std::string_view nodeName, UUID nodeID)
		{
			NodeProcessor* node = Factory::Create(Identifier(nodeName), nodeID);

			if (!node)
			{
				ANT_CORE_ERROR("[SoundGraph] Parser. Node type '{}' is not implemented!", nodeName);
				ANT_CORE_ASSERT(false);
			}

			return node;
		}

		//==============================================================================
		void ParseConnections()
		{
			const std::vector<Link>& links = Graph->Links;

			// TODO: connection order matters, need to start from the graph Inputs

			for (const auto& link : links)
			{
				Pin* sourcePin = Options.Model->FindPin(link.StartPinID);
				Pin* destPin = Options.Model->FindPin(link.EndPinID);

				if (!sourcePin || !destPin)
					continue;

				Node* sourceNode = Options.Model->FindNode(sourcePin->NodeID);
				Node* destNode = Options.Model->FindNode(destPin->NodeID);

				if (!sourceNode || !destNode)
					continue;

				if (sourceNode->SortIndex == Node::UndefinedSortIndex || destNode->SortIndex == Node::UndefinedSortIndex)
					continue;

				// We parse destination local variables by rerouting source local variables
				if (Options.Model->IsLocalVariableNode(destNode))
					continue;

				std::string sourceNodeName = Utils::MakeSafeIdentifier(sourceNode->Name);
				std::string destNodeName = Utils::MakeSafeIdentifier(destNode->Name);
				std::string sourcePinName = Utils::MakeSafeIdentifier(sourcePin->Name);
				std::string destPinName = Utils::MakeSafeIdentifier(destPin->Name);

				sourcePinName = choc::text::replace(sourcePinName, "_", "");
				destPinName = choc::text::replace(destPinName, "_", "");

				if (Options.Model->IsLocalVariableNode(sourceNode))
				{
					/* Reroute from source node Local Variable to the source of the Local Variable
						Source -> LV(setter) | LV(getter) -> destination
						..becomes
						Source -> destination
					*/

					if (auto* localVarSourcePin = Options.Model->FindLocalVariableSource(sourceNode->Name))
					{
						sourcePin = localVarSourcePin;
						sourceNode = Options.Model->FindNode(sourcePin->NodeID);

						if (!sourceNode)
						{
							ANT_CORE_ERROR("Failed to find source node while parsing Local Variable.");
							continue;
						}
						sourcePinName = Utils::MakeSafeIdentifier(sourcePin->Name);
						sourcePinName = choc::text::replace(sourcePinName, "_", "");

						sourceNodeName = sourceNode->Name;
						sourceNodeName = Utils::MakeSafeIdentifier(sourceNode->Name);
					}
					else // Local variable plug connection
					{
						if (!sourcePin->IsType(Nodes::SGTypes::ESGPinType::Flow))

						{
							OutSoundGraph->Connections.emplace_back(Prototype::Connection::Endpoint{ sourceNode->ID, Identifier(sourcePinName) },
								Prototype::Connection::Endpoint{ destNode->ID, Identifier(destPinName) },
								Prototype::Connection::LocalVariable_NodeValue);
						}
						else
						{
							// TODO: in this case the graph should not crash, just the getters of this LV wouldn't be ever triggered
							ANT_CORE_ERROR("We can't have default plugs for Local Variables of function type!");
						}

						continue;
					}
				}


				// First handle special cases of graph inputs
				// then other node connections
				if (sourceNodeName == "Input_Action")
				{
					Prototype::Node* destination = FindNodeByID(destNode->ID);
					ANT_CORE_ASSERT(destination);

					const Identifier inputActionID = sourcePinName == "Play" ? SoundGraph::IDs::Play : Identifier();
					// Invalid Input Action ID
					{
						ANT_CORE_ASSERT(inputActionID != Identifier());
						// TODO: collect error and terminate compilation
					}

					// In Event -> In Event
					OutSoundGraph->Connections.emplace_back(Prototype::Connection::Endpoint{ OutSoundGraph->ID, inputActionID },
						Prototype::Connection::Endpoint{ destNode->ID, Identifier(destPinName) },
						Prototype::Connection::GraphEvent_NodeEvent);
				}
				else if (destNodeName == "Output_Audio")
				{
					//? For now only supporting Stereo channel layout

					Prototype::Node* source = FindNodeByID(sourceNode->ID); //? don't think this is needed anymore, should validate nodes before this point
					ANT_CORE_ASSERT(source);

					// TODO: ensure we can only connect one NodeProcessor to Output Audio, or implement some sort of automatic stream mixing plug

					const std::string outChannel = choc::text::replace(sourcePinName, "_", "");
					const std::string inChannel = destPinName == "Left" ? "audioOut_0" : "audioOut_1";

					const uint32_t channelIndex = destPinName == "Left" ? 0 : 1;

					OutSoundGraph->Connections.emplace_back(Prototype::Connection::Endpoint{ sourceNode->ID, Identifier(outChannel) },
						Prototype::Connection::Endpoint{ 0 /*EndpointOutputStreams.ID*/, OutSoundGraph->OutputChannelIDs[channelIndex] },
						Prototype::Connection::NodeValue_GraphValue);
				}
				else if (destNodeName == "On_Finished")
				{
					OutSoundGraph->Connections.emplace_back(Prototype::Connection::Endpoint{ sourceNode->ID, Identifier(sourcePinName) },
						Prototype::Connection::Endpoint{ OutSoundGraph->ID, Identifier(destPinName) },
						Prototype::Connection::NodeEvent_GraphEvent);
				}
				else if (sourceNodeName == "Input")
				{
					Prototype::Node* destination = FindNodeByID(destNode->ID);
					ANT_CORE_ASSERT(destination);

					// Route Graph Inputs to events of node processors
					// Differentiate between Events and Values

					if (sourcePin->IsType(Nodes::SGTypes::ESGPinType::Flow))
					{
						OutSoundGraph->Connections.emplace_back(Prototype::Connection::Endpoint{ OutSoundGraph->ID, Identifier(sourcePinName) },
							Prototype::Connection::Endpoint{ destNode->ID, Identifier(destPinName) },
							Prototype::Connection::GraphEvent_NodeEvent);
					}
					else
					{

						OutSoundGraph->Connections.emplace_back(Prototype::Connection::Endpoint{ OutSoundGraph->ID, Identifier(sourcePinName) },
							Prototype::Connection::Endpoint{ destNode->ID, Identifier(destPinName) },
							Prototype::Connection::GraphValue_NodeValue);
					}
				}
				else
				{
					// Inner routing between Nodes

					Prototype::Node* source = FindNodeByID(sourceNode->ID);
					Prototype::Node* destination = FindNodeByID(destNode->ID);

					ANT_CORE_ASSERT(source);
					ANT_CORE_ASSERT(destination);

					if (sourcePin->IsType(Nodes::SGTypes::ESGPinType::Flow))
					{
						OutSoundGraph->Connections.emplace_back(Prototype::Connection::Endpoint{ sourceNode->ID, Identifier(sourcePinName) },
							Prototype::Connection::Endpoint{ destNode->ID, Identifier(destPinName) },
							Prototype::Connection::NodeEvent_NodeEvent);
					}
					else
					{
						OutSoundGraph->Connections.emplace_back(Prototype::Connection::Endpoint{ sourceNode->ID, Identifier(sourcePinName) },
							Prototype::Connection::Endpoint{ destNode->ID, Identifier(destPinName) },
							Prototype::Connection::NodeValue_NodeValue);
					}
				}
			}
		}

		Prototype::Node* FindNodeByID(UUID id)
		{
			auto it = std::find_if(OutSoundGraph->Nodes.begin(), OutSoundGraph->Nodes.end(),
				[id](const Prototype::Node& nodePtr)
				{
					return nodePtr.ID == id;
				});

			if (it != OutSoundGraph->Nodes.end())
				return &(*it);
			else
				return nullptr;
		}

		//==============================================================================
		void ParseWaveReferences()
		{
			// Find wave asset handles directly assigned to node input pins
			for (const auto& node : Graph->Nodes)
			{
				for (const auto& input : node->Inputs)
				{
					choc::value::ValueView value = input->Value;

					if (value.isVoid() || input->IsType(Nodes::SGTypes::ESGPinType::Enum))
						continue;

					if (IsWaveAsset(value))
					{
						if (const auto assetId = GetAssetHandleValue(value))
						{
							//ANT_CORE_WARN("ParseWaveReferences() - Found Wave Asset reference pin!");

							// TODO: gather all used wave refs somewhere else, perhaps directly by parsing WavePlayer nodes and GraphInputs
							Utils::AppendIfNotPresent(OutWaveAssets, assetId);
						}
					}
				}
			}

			// Find wave asset handles in Graph Inputs
			for (const auto& graphInputName : Graph->GraphInputs.GetNames())
			{
				choc::value::Value value = Graph->GraphInputs.GetValue(graphInputName);
				const bool isArray = value.isArray();

				if (isArray && IsWaveAsset(value[0]))
				{
					for (const auto& wave : value)
					{
						if (const UUID handle = GetAssetHandleValue(wave))
							Utils::AppendIfNotPresent(OutWaveAssets, handle);
					}
				}
				else if (IsWaveAsset(value))
				{
					if (const UUID handle = GetAssetHandleValue(value))
						Utils::AppendIfNotPresent(OutWaveAssets, handle);
				}
			}
		}
	};

	//==============================================================================
	bool ParseGraph(GraphGeneratorOptions& options, Ref<Prototype>& outSoundGraph, std::vector<UUID>& waveAssets)
	{
		Parser parser{ options, outSoundGraph };

		const bool success = parser.Run();
		if (success)
			waveAssets = parser.OutWaveAssets;

		return success;
	}

	// @returns vector of error strings
	std::vector<std::string> PreValidateGraph(const Ref<SoundGraphAsset>& graph, const SoundGraphNodeEditorModel& model)
	{
		if (graph->Nodes.empty())
			return { "Graph contains no nodes." };

		/*enum EResult
		{
			Valid = 1,
			InvalidPinLinkage
		} error{ Valid };*/

		std::vector<std::string> errors;

		const auto validateNode = [&model, &errors](const Node* node)
		{
			if (node->Type == NodeType::Comment)
				return;

			if (node->Name != "Output Audio" && node->Name != "Input Action" && node->Name != "On Finished" && !model.IsPropertyNode(node)
				&& !Factory::Contains(Identifier(node->Name)))
			{
				errors.push_back("Sound Graph Factory doesn't contain node with type ID '" + node->Name + "'");
			}

			for (const auto& in : node->Inputs)
			{
				if (in->Value.isVoid() && !model.IsPinLinked(in->ID) && !in->IsType(Nodes::SGTypes::ESGPinType::Flow))
					errors.push_back("Invalid linkage: node '" + node->Name + "', pin '" + in->Name + "'");
			}

			//return Valid;
		};

		for (const auto& node : graph->Nodes)
			validateNode(node);

		return errors;
	}

	Ref<Prototype> ConstructPrototype(GraphGeneratorOptions options, std::vector<UUID>& waveAssetsToLoad)
	{
		if (options.Name.empty() || !options.Model)
			return nullptr;

		std::vector<std::string> errors = PreValidateGraph(options.Graph, *options.Model);

		if (!errors.empty())
		{
			errors.insert(errors.begin(), "Graph is invalid.");
			ANT_CONSOLE_LOG_ERROR("Failed to construct Sound Graph Prototype! " + choc::text::joinStrings(errors, "\n"));
			return nullptr;
		}

		Ref<Prototype> prototype = Ref<Prototype>::Create();
		prototype->DebugName = options.Name;
		prototype->ID = UUID(); // TODO: should this be passed in with options?

		if (ParseGraph(options, prototype, waveAssetsToLoad))
		{
			// At this state the Graph is constructed but not initialized

			// TODO: cache only if everything is valid
			if (options.Cache)
			{
				options.Cache->StorePrototype(options.Name.c_str(), *prototype);
				options.Graph->CachedPrototype = options.Cache->GetFileForKey(options.Name.c_str());
			}

			return prototype;
		}

		return nullptr;
	}

	//==============================================================================
	bool ValidateGraph(const Ref<SoundGraph>& graph)
	{
		if (!graph)
			return false;

		// TODO: add propper compilation/validation error messages
		//std::vector<std::string> errors;

		if (graph->Nodes.empty())
			return false;


		auto validateElements = [](const auto& elements, const auto& test)
		{
			for (const auto& el : elements)
			{
				if (!test(el))
				{
					ANT_CORE_ASSERT(false);
					return false;
				}
			}

			return true;
		};

		// 1. Graph ProcessorNode
		{
			bool pass = true;
			if (!validateElements(graph->Ins, [](const std::pair<const Identifier, const choc::value::ValueView>& in) { return (bool)in.second.getRawData(); })) pass = false;
			if (!validateElements(graph->Outs, [](const std::pair<const Identifier, const choc::value::ValueView>& out) { return (bool)out.second.getRawData(); })) pass = false;
			if (!validateElements(graph->InEvs, [](const std::pair<const Identifier, const NodeProcessor::InputEvent>& inEv) { return (bool)inEv.second.Event; })) pass = false;

			// We probably don't need to test output events, they use vectors of destinations internally, which may be empty
			//if (!validateElements(graph->OutEvs, [](const std::pair<const Identifier, const NodeProcessor::OutputEvent>& outEv) { return !outEv.second.DestinationEvs.empty(); })) pass = false;

			ANT_CORE_ASSERT(pass);
			if (!pass)
				return false;
		}

		using EndpointStream = std::unique_ptr<StreamWriter>;

		const auto validateEndpoints = [](const EndpointStream& endpointStream)
		{
			if (endpointStream->DestinationID)
			{
				if (!endpointStream->outV.getRawData())
					return false;
				else
					return true;
			}
			else
			{
				return false;
			}
		};

		//! 2. Graph endpoints (?)
		{
			bool pass = validateElements(graph->EndpointInputStreams, validateEndpoints);

			ANT_CORE_ASSERT(pass);
			if (!pass)
				return false;
		}

		//! 2.2. Local variables (?)
		{
			bool pass = validateElements(graph->LocalVariables, validateEndpoints);

			ANT_CORE_ASSERT(pass);
			if (!pass)
				return false;
		}

		if (!validateElements(graph->EndpointOutputStreams.Ins, [](const std::pair<const Identifier, const choc::value::ValueView>& in) { return (bool)in.second.getRawData(); }))
		{
			ANT_CORE_ASSERT(false);
			return false;
		}

		// 3. Nodes
		for (auto& node : graph->Nodes)
		{
			bool pass = true;
			if (!validateElements(node->Ins, [](const std::pair<const Identifier, const choc::value::ValueView>& in) { return (bool)in.second.getRawData(); })) pass = false;
			if (!validateElements(node->Outs, [](const std::pair<const Identifier, const choc::value::ValueView>& out) { return (bool)out.second.getRawData(); })) pass = false;
			if (!validateElements(node->InEvs, [](const std::pair<const Identifier, const NodeProcessor::InputEvent>& inEv) { return (bool)inEv.second.Event; })) pass = false;

			// We probably don't need to test output events, they use vectors of destinations internally, which may be empty
			//if (!validateElements(node->OutEvs, [](const std::pair<const Identifier, const NodeProcessor::OutputEvent>& outEv) { return (bool)!outEv.second.DestinationEvs.empty(); })) pass = false;

			ANT_CORE_ASSERT(pass);
			if (!pass)
				return false;
		}

		return true;
	}

	//==============================================================================
	Ref<SoundGraph> CreateInstance(const Ref<Prototype>& prototype)
	{
		if (!prototype || !prototype->Nodes.size())
		{
			ANT_CORE_ASSERT(false);
			return nullptr;
		}

		Ref<SoundGraph> instance = Ref<SoundGraph>::Create(prototype->DebugName, prototype->ID);

		instance->Nodes.reserve(prototype->Nodes.size());

		// Construct IO
		for (const Prototype::Endpoint& inStream : prototype->Inputs)
		{
			instance->AddGraphInputStream(inStream.EndpointID, choc::value::Value(inStream.DefaultValue));
		}
		for (const Prototype::Endpoint& outStream : prototype->Outputs)
		{
			// TODO: add support for other types of output streams, not just float
			instance->AddGraphOutputStream(outStream.EndpointID);
			instance->OutputChannelIDs.push_back(outStream.EndpointID);
		}

		// Local Variables
		for (const auto& localVar : prototype->LocalVariablePlugs)
		{
			instance->AddLocalVariableStream(localVar.EndpointID, localVar.DefaultValue);
		}

		// Construct Nodes
		for (const Prototype::Node& node : prototype->Nodes)
		{
			instance->AddNode(Factory::Create(node.NodeTypeID, node.ID));

			auto& newNode = instance->Nodes.back();

			for (const Prototype::Endpoint& inputValuePlug : node.DefaultValuePlugs)
				newNode->DefaultValuePlugs.emplace_back(new StreamWriter(newNode->InValue(inputValuePlug.EndpointID),
					choc::value::Value(inputValuePlug.DefaultValue),
					inputValuePlug.EndpointID));
		}

		// Construct Connections
		for (const auto& connection : prototype->Connections)
		{
			const auto& source = connection.Source;
			const auto& dest = connection.Destination;

			// TODO: add proper error message depending on which one of these fail
			switch (connection.Type)
			{
				case Prototype::Connection::NodeValue_NodeValue:
					if (!instance->AddValueConnection(source.NodeID, source.EndpointID, dest.NodeID, dest.EndpointID)) return nullptr;
					break;
				case Prototype::Connection::NodeEvent_NodeEvent:
					if (!instance->AddEventConnection(source.NodeID, source.EndpointID, dest.NodeID, dest.EndpointID)) return nullptr;
					break;
				case Prototype::Connection::GraphValue_NodeValue:
					if (!instance->AddInputValueRoute(source.EndpointID, dest.NodeID, dest.EndpointID)) return nullptr;
					break;
				case Prototype::Connection::GraphEvent_NodeEvent:
					if (!instance->AddInputEventsRoute(source.EndpointID, dest.NodeID, dest.EndpointID)) return nullptr;
					break;
				case Prototype::Connection::NodeValue_GraphValue:
					if (!instance->AddToGraphOutputConnection(source.NodeID, source.EndpointID, dest.EndpointID)) return nullptr;
					break;
				case Prototype::Connection::NodeEvent_GraphEvent:
					if (!instance->AddToGraphOutEventConnection(source.NodeID, source.EndpointID, dest.EndpointID)) return nullptr;
					break;
				case Prototype::Connection::LocalVariable_NodeValue:
					if (!instance->AddLocalVariableRoute(source.EndpointID, dest.NodeID, dest.EndpointID)) return nullptr;
					break;
				default: ANT_CORE_ASSERT(false);
					return nullptr;
					break;
			}
		}

		if (!ValidateGraph(instance))
			return nullptr;

		return instance;

		/*
			To clone a graph we need:

				IO
					1) Input Identifier and default value
					2) Output Identifier and (in the future) value type

				Nodes
					1) MakeEmptyCopy function
					2) DefaultValuePlugs
						- InValue Identifier
						- default value

				Connections
					1) Vector of SoundGraph::Connections

				Call to Init() graph should probably be done after construction
				Validate() graph
		*/
	}
}