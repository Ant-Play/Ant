#include "antpch.h"
#include "SoundGraphNodeEditorModel.h"
#include "SoundGraphGraphEditor.h"

#include "Ant/Audio/SoundGraph/SoundGraph.h"
#include "Ant/Audio/SoundGraph/SoundGraphSource.h"
#include "Ant/Audio/SoundGraph/GraphGeneration.h"

#include "Ant/Asset/AssetManager.h"
#include "Ant/Project/Project.h"
#include "Ant/Utilities/FileSystem.h"
#include "Ant/Utilities/ContainerUtils.h"

#include "Ant/Audio/AudioEngine.h"

#include "imgui-node-editor/imgui_node_editor.h"

#include "choc/text/choc_UTF8.h"
#include "choc/text/choc_Files.h"
#include "choc/text/choc_JSON.h"

#include <limits>

static constexpr auto MAX_NUM_CACHED_GRAPHS = 256; // TODO: get this value from somewhere reasonable

namespace Ant
{
	//? DBG. Print values of input nodes
	void NodeDumb(const Ref<SoundGraph::SoundGraph>& soundGraph, UUID nodeID, std::string_view dumpIdentifier)
	{
		if (const auto* node = soundGraph->FindNodeByID(7220023940811122597))
		{
			ANT_CORE_WARN("======================");
			ANT_CORE_WARN("NODE DUMP");
			ANT_CORE_WARN(dumpIdentifier);
			ANT_CORE_TRACE("----------------------");
			ANT_CORE_TRACE("{}", node->dbgName);
			for (const auto& [id, value] : node->Ins)
			{
				ANT_CORE_TRACE("[{0}] {1}", id, choc::json::toString(value));
			}
			ANT_CORE_TRACE("----------------------");
		}
	}

	//static const char* s_PatchDirectory = "Resources/Cache/Shader/ShaderRegistry.cache";

	SoundGraphNodeEditorModel::SoundGraphNodeEditorModel(Ref<SoundGraphAsset> graphAsset)
		: m_GraphAsset(graphAsset)
	{
		// Initialize preview player
		m_SoundGraphSource = CreateScope<SoundGraphSource>();
		Audio::AudioCallback::BusConfig busConfig;
		busConfig.InputBuses.push_back(2);
		busConfig.OutputBuses.push_back(2);
		m_SoundGraphSource->Initialize(MiniAudioEngine::GetMiniaudioEngine(), busConfig);

		// Need to make sure the Context / Player is running before posting any events or parameter changes
		m_SoundGraphSource->StartNode();
		m_SoundGraphSource->SuspendProcessing(true);
		// Wait for audio Process Block to stop
		while (!m_SoundGraphSource->IsSuspended()) {}

		// Deserialize. Check if we have compiled graph prototype in cache
		std::filesystem::path cacheDirectory = Project::GetCacheDirectory() / "SoundGraph";
		m_Cache = CreateScope<SoundGraphCache>(cacheDirectory, MAX_NUM_CACHED_GRAPHS);

		Deserialize();

		onPinValueChanged = [&](UUID nodeID, UUID pinID)
		{
			if (const auto* node = FindNode(nodeID))
			{
				if (IsLocalVariableNode(node))
					SetPropertyValue(ESoundGraphPropertyType::LocalVariable, node->Name, node->Inputs[0]->Value);
			}

			InvalidatePlayer();
		};

		onCompile = [&](Ref<Asset>& graphAsset)
		{
			if (!Sort())
				return false;

			auto& graph = graphAsset.As<SoundGraphAsset>();
			const AssetMetadata& md = Project::GetEditorAssetManager()->GetMetadata(graph);
			const std::string name = md.FilePath.stem().string();

			SoundGraph::GraphGeneratorOptions options
			{
				name,
				0,
				2,
				graphAsset,
				this,
				m_Cache.get()
			};

			auto soundGraphPrototype = SoundGraph::ConstructPrototype(options, graph->WaveSources);

			if (!soundGraphPrototype)
				return false;

			auto soundGraph = SoundGraph::CreateInstance(soundGraphPrototype);

			// Return if failed to "compile" or validate graph
			if (!soundGraph)
				return false;

			graph->Prototype = soundGraphPrototype;

			// 4. Prepare streaming wave sources
			m_SoundGraphSource->SuspendProcessing(true);
			// Wait for audio Process Block to stop
			while (!m_SoundGraphSource->IsSuspended()) {}

			m_SoundGraphSource->UninitializeDataSources();
			m_SoundGraphSource->InitializeDataSources(graph->WaveSources);

			// 5. Update preview player with the new compiled patch player
			m_SoundGraphSource->ReplacePlayer(soundGraph);

			// 6. Initialize player's default parameter values (graph "preset")
			if (soundGraph)
			{
				bool parametersSet = m_SoundGraphSource->ApplyParameterPreset(graph->GraphInputs);

				// Preset must match parameters (input events) of the compiled patch player 
				ANT_CORE_ASSERT(parametersSet);
			}

			return soundGraph != nullptr;
		};

		onPlay = [&]
		{
			// in case graph is still processing, suspend
			m_SoundGraphSource->SuspendProcessing(true);
			// Wait for audio Process Block to stop
			while (!m_SoundGraphSource->IsSuspended()) {}

			// reset graph to initial state
			m_SoundGraphSource->GetGraph()->Reinit();
			m_SoundGraphSource->SuspendProcessing(false);

			if (m_SoundGraphSource->SendPlayEvent())
			{
				//? DBG
				//ANT_CORE_INFO("Posted Play");
			}
		};

		onStop = [&](bool onClose)
		{
			if (auto& graph = m_SoundGraphSource->GetGraph())
			{
				// TODO: fade out before stopping?
				// TODO: clear buffer

				m_SoundGraphSource->SuspendProcessing(true);
				// Wait for audio Process Block to stop
				while (!m_SoundGraphSource->IsSuspended()) {}

				graph->Reset();

				if (!onClose && !m_PlayerDirty)
				{
					// Need to reinitialize parameters after the player has been reset
					bool parametersSet = m_SoundGraphSource->ApplyParameterPreset(m_GraphAsset->GraphInputs);

					// Preset must match parameters (input events) of the compiled patch player 
					ANT_CORE_ASSERT(parametersSet);
				}
			}
		};
	}

	SoundGraphNodeEditorModel::~SoundGraphNodeEditorModel()
	{
		m_SoundGraphSource->Uninitialize();
	}

	bool SoundGraphNodeEditorModel::SaveGraphState(const char* data, size_t size)
	{
		m_GraphAsset->GraphState = data;
		return true;
	}

	const std::string& SoundGraphNodeEditorModel::LoadGraphState()
	{
		return m_GraphAsset->GraphState;
	}

	void SoundGraphNodeEditorModel::OnCompileGraph()
	{
		ANT_CORE_INFO("Compiling graph...");

		//SaveAll();

		if (onCompile)
		{
			if (onCompile(m_GraphAsset.As<Asset>()))
			{
				m_PlayerDirty = false;

				ANT_CORE_INFO("Graph has been compiled.");
				if (onCompiledSuccessfully)
					onCompiledSuccessfully();

				ANT_CONSOLE_LOG_TRACE("Sound Graph successfully compiled.");

				// Saving newly created prototype.
				// If failed to compile, the old prototype is going to be used
				// for playback
				SaveAll();
			}
			else
			{
				ANT_CONSOLE_LOG_ERROR("Failed to compile graph!");
			}
		}
		else
		{
			ANT_CONSOLE_LOG_ERROR("Failed to compile graph!");
		}
	}

	NodeEditorModel::LinkQueryResult SoundGraphNodeEditorModel::CanCreateLink(Pin* startPin, Pin* endPin)
	{
		LinkQueryResult result = NodeEditorModel::CanCreateLink(startPin, endPin);

		if (!result)
			return result;

		auto* startNode = FindNode(startPin->NodeID);
		auto* endNode = FindNode(endPin->NodeID);

		if (!startNode) return LinkQueryResult::InvalidStartPin;
		if (!endNode) return LinkQueryResult::InvalidEndPin;

		// TODO: in the future might need to change this to check if the nodes are from the same LV
		if (IsLocalVariableNode(startNode) && IsLocalVariableNode(endNode))
		{
			if (startPin->Name == endPin->Name)
				return LinkQueryResult::CausesLoop;
		}

		return LinkQueryResult::CanConnect;
	}

	void SoundGraphNodeEditorModel::OnNodeCreated()
	{
		InvalidatePlayer();

		if (onNodeCreated)
			onNodeCreated();
	}

	void SoundGraphNodeEditorModel::OnNodeDeleted()
	{
		InvalidatePlayer();

		if (onNodeDeleted)
			onNodeDeleted();
	}

	void SoundGraphNodeEditorModel::OnLinkCreated(UUID linkID)
	{
		if (auto* newLink = FindLink(linkID))
		{
			auto* startPin = FindPin(newLink->StartPinID);
			auto* endPin = FindPin(newLink->EndPinID);
			ANT_CORE_ASSERT(endPin);

			auto* startNode = FindNode(startPin->NodeID);
			ANT_CORE_ASSERT(startNode);

			auto* endNode = FindNode(endPin->NodeID);
			ANT_CORE_ASSERT(startPin && endNode);

			//? JP. no idea why we needed this :shrug:
#if 0
			// If pin is connected to Graph Input getter node,
			// set its name as the value for this pin
			const bool isGraphInput = startNode->Category == GetPropertyToken(ESoundGraphPropertyType::Input);
			if (isGraphInput && m_GraphAsset->GraphInputs.HasValue(startPin->Name))
			{
				endPin->Value = choc::value::Value(startPin->Name);
				if (onPinValueChanged)
					onPinValueChanged(endNode->ID, endPin->ID);
			}
#endif

			// TODO: handle connections to Local Variables?
			//else if (m_GraphAsset->LocalVariables.HasValue(startPin->Name))

			// Remove any other links connected to the same input pin
			// For Sound Graph nodes we only allow single input connection,
			// but multiple output connections.
			std::vector<UUID> linkIds = GetAllLinkIDsConnectedToPin(newLink->EndPinID);
			Utils::Remove(linkIds, newLink->ID);
			RemoveLinks(linkIds);
		}

		InvalidatePlayer();

		if (onLinkCreated)
			onLinkCreated(linkID);
	}

	void SoundGraphNodeEditorModel::OnLinkDeleted()
	{
		InvalidatePlayer();

		if (onLinkDeleted)
			onLinkDeleted();
	}

	void SoundGraphNodeEditorModel::AssignSomeDefaultValue(Pin* pin)
	{
		if (!pin || pin->Kind == PinKind::Output)
			return;

		// First we try to get default value override from the factory
		if (const Node* node = FindNode(pin->NodeID))
		{
			if (auto optValueOverride = Nodes::GetPinDefaultValueOverride(choc::text::replace(node->Name, " ", ""), choc::text::replace(pin->Name, " ", "")))
			{
				pin->Value = *optValueOverride;
				return;
			}
		}

		if (pin->Storage == StorageKind::Array)
		{
			// TODO: JP. in the future we might want to store actual array type since this is editor-only nodes
			pin->Value = choc::value::Value();
			return;
		}

		// If we couldn't find the default value override in the factory,
		// we construct a default node of the same type and get the default value from it.
		const Pin* defaultPin = Nodes::SGTypes::CreatePinForType((Nodes::SGTypes::ESGPinType)pin->GetType());

		pin->Value = defaultPin->Value;

		delete defaultPin;
	}

	void SoundGraphNodeEditorModel::InvalidatePlayer()
	{
		m_PlayerDirty = true;
	}

	// TODO: move this out to some sort of SoundGraph Utils header
	namespace Keyword
	{
		// TODO: change these for the new SoundGraph
#define SoundGraph_KEYWORDS(X) \
        X("if")         X("do")         X("for")        X("let") \
        X("var")        X("int")        X("try")        X("else") \
        X("bool")       X("true")       X("case")       X("enum") \
        X("loop")       X("void")       X("while")      X("break") \
        X("const")      X("int32")      X("int64")      X("float") \
        X("false")      X("using")      X("fixed")      X("graph") \
        X("input")      X("event")      X("class")      X("catch") \
        X("throw")      X("output")     X("return")     X("string") \
        X("struct")     X("import")     X("switch")     X("public") \
        X("double")     X("private")    X("float32")    X("float64") \
        X("default")    X("complex")    X("continue")   X("external") \
        X("operator")   X("processor")  X("namespace")  X("complex32") \
        X("complex64")  X("connection")

		struct Matcher
		{
			static bool Match(std::string_view name) noexcept
			{

#define SoundGraph_COMPARE_KEYWORD(str) if (name.length() == (int) sizeof (str) - 1 && choc::text::startsWith(name, (str))) return true;
				SoundGraph_KEYWORDS(SoundGraph_COMPARE_KEYWORD)
#undef SoundGraph_COMPARE_KEYWORD
					return false;
			}
		};
	}

	bool SoundGraphNodeEditorModel::IsGraphPropertyNameValid(ESoundGraphPropertyType type, std::string_view name) const noexcept
	{
		const auto isPropertyNameValid = [&](const Utils::PropertySet& propertySet)
		{
			return !propertySet.HasValue(name)
				&& !propertySet.HasValue(choc::text::replace(std::string(name), " ", ""))
				&& !Keyword::Matcher::Match(choc::text::replace(std::string(name), " ", ""));
		};

		switch (type)
		{
			case Invalid:
				return false;
			case Input:
				return isPropertyNameValid(m_GraphAsset->GraphInputs);
			case Output:
				return isPropertyNameValid(m_GraphAsset->GraphOutputs);
			case LocalVariable:
				return isPropertyNameValid(m_GraphAsset->LocalVariables);
			default:
				return false;
		}
	}

	bool SoundGraphNodeEditorModel::Sort()
	{
		for (auto& node : m_GraphAsset->Nodes)
			node->SortIndex = Node::UndefinedSortIndex;

		// TODO: get better candidate for the starting node?

		auto findEndpointNode = [&](std::string_view nodeName) -> Node*
		{
			for (auto& node : m_GraphAsset->Nodes)
			{
				if (node->Name == nodeName)
					return node;
			}
			return nullptr;
		};

		// For now we always start sorting form Output Audio, since our graph must play audio.
		// This might not be the case in the future when we implement nested graph processors/modules
		// that going to be able to do processiong on just value streams.
		Node* sortEndpointNode = findEndpointNode("Output Audio");
		if (!sortEndpointNode)
			sortEndpointNode = findEndpointNode("Input Action");

		if (!sortEndpointNode)
		{
			// TODO: JP. there may be cases when the user just saves graph without playable endpoints for the future,
			//		editor shouldn't crash in that case, need to handle it softer
			// TODO: JP. perhaps we could display a warning or error icon on top of the "Compile" button
			//		to show that compilation has failed and the graph is invalid
			ANT_CORE_ERROR("Failed to find endpoint node to sort the graph!");
			//ANT_CORE_ASSERT(false);
			return false;
		}

		uint32_t currentIndex = 0;
		sortEndpointNode->SortIndex = currentIndex++;

		auto correctSort = [&](Node* context, Node* previous, auto& correctSort) -> void
		{
			// Local variables are a special case,
			// we treat all nodes coresponding to the same LV
			// as the same node
			if (IsLocalVariableNode(context))
			{
				std::vector<Node*> localVarNodes;
				std::vector<const Pin*> localVarOutputs;

				const auto isSetter = [](const Node* n) { return n->Inputs.size() == 1; };

				volatile int32_t localVarSortIndex = context->SortIndex;

				// Collect all of the nodes corresponding to this Local Variable
				for (auto& n : GetNodes())
				{
					if (IsSameLocalVariableNodes(n, context))
						localVarNodes.push_back(n);
				}

				// Go over inputs, which should be just one
				// becase its value can reference only one output
				for (auto* n : localVarNodes)
				{
					if (isSetter(n))
					{
						// Find node connected to this input
						Node* node = GetNodeConnectedToPin(n->Inputs[0]->ID);

						// Local variable setter might not have conntected input,
						// we still need to assign it some valid sort index.
						if (!node && n->SortIndex == Node::UndefinedSortIndex)
						{
							n->SortIndex = currentIndex++;
							localVarSortIndex = n->SortIndex;
						}

						if (!node || node == previous || node->SortIndex != Node::UndefinedSortIndex) continue;

						correctSort(node, n, correctSort);

						if (n->SortIndex == Node::UndefinedSortIndex)
						{
							n->SortIndex = currentIndex++;
							localVarSortIndex = n->SortIndex;
						}

						// in some complex cases we can get into this situation, where we get to the same LV node from different branches
						if (context->SortIndex != Node::UndefinedSortIndex && localVarSortIndex == Node::UndefinedSortIndex)
							localVarSortIndex = context->SortIndex;

						break;
					}
				}
				ANT_CORE_ASSERT(localVarSortIndex != Node::UndefinedSortIndex);

				// Collect outputs from all instances of getter nodes
				for (auto* n : localVarNodes)
				{
					if (!isSetter(n))
					{
						localVarOutputs.push_back(n->Outputs[0]);
						if (n->SortIndex == Node::UndefinedSortIndex)
							n->SortIndex = localVarSortIndex;
					}
				}
				// Go over collected outputs
				for (const auto* out : localVarOutputs)
				{
					// Find node connected to this output
					Node* node = GetNodeConnectedToPin(out->ID);
					if (!node || node == previous || node->SortIndex != Node::UndefinedSortIndex) continue;

					correctSort(node, context, correctSort);
				}
			}
			else // Non Local Varialbe nodes are much simpler case
			{
				for (const auto& in : context->Inputs)
				{
					// Find node connected to this input
					Node* node = GetNodeConnectedToPin(in->ID);
					if (!node || node == previous || node->SortIndex != Node::UndefinedSortIndex) continue;

					correctSort(node, context, correctSort);
				}

				if (context->SortIndex == Node::UndefinedSortIndex)
					context->SortIndex = currentIndex++;

				for (auto& out : context->Outputs)
				{
					// Find node connected to this output
					Node* node = GetNodeConnectedToPin(out->ID);
					if (!node || node == previous || node->SortIndex != Node::UndefinedSortIndex) continue;

					correctSort(node, context, correctSort);
				}
			}
		};
		correctSort(sortEndpointNode, nullptr, correctSort);

		// Sort Nodes
		std::vector<Node*> sortedNodes = m_GraphAsset->Nodes;

		std::sort(sortedNodes.begin(), sortedNodes.end(), [&](const Node* first, const Node* second)
			{
				if (first->SortIndex == Node::UndefinedSortIndex)
					return false;
				if (second->SortIndex == Node::UndefinedSortIndex)
					return true;
				return first->SortIndex < second->SortIndex;
			});

		m_GraphAsset->Nodes = sortedNodes;

		for (uint32_t i = 0; i < m_GraphAsset->Nodes.size(); ++i)
		{
			auto& node = m_GraphAsset->Nodes[i];
			if (node->SortIndex == Node::UndefinedSortIndex)
				continue;

			node->SortIndex = i;
		}

		// Sort Links
		std::vector<Link> sortedLinks = m_GraphAsset->Links;
		std::sort(sortedLinks.begin(), sortedLinks.end(), [&](const Link& first, const Link& second)
			{
				bool isLess = false;

				auto getIndex = [](auto vec, auto valueFunc)
				{
					auto it = std::find_if(vec.begin(), vec.end(), valueFunc);
					ANT_CORE_ASSERT(it != vec.end());
					return it - vec.begin();
				};

				const Node* node1 = GetNodeForPin(first.StartPinID);
				const Node* node2 = GetNodeForPin(second.StartPinID);

				//if (node1 == node2)
				{
					// TODO: check index of pins in the nodes
				}

				const size_t index1 = getIndex(m_GraphAsset->Nodes, [node1](const Node* n) { return n->ID == node1->ID; });
				const size_t index2 = getIndex(m_GraphAsset->Nodes, [node2](const Node* n) { return n->ID == node2->ID; });

				if (index1 != index2)
				{
					isLess = index1 < index2;
				}
				else
				{
					const Node* nodeEnd1 = GetNodeForPin(first.EndPinID);
					const Node* nodeEnd2 = GetNodeForPin(second.EndPinID);

					//if (nodeEnd1 == nodeEnd2)
					{
						// TODO: check index of pins in the nodes
					}

					const size_t indexEnd1 = getIndex(m_GraphAsset->Nodes, [node1](const Node* n) { return n->ID == node1->ID; });
					const size_t indexEnd2 = getIndex(m_GraphAsset->Nodes, [node2](const Node* n) { return n->ID == node2->ID; });

					//if (indexEnd1 == indexEnd2)
					{
						// TODO: check index of pins in the nodes
					}

					isLess = indexEnd1 < indexEnd2;
				}

				return isLess;
			});

		m_GraphAsset->Links = sortedLinks;

		return true;
	}

	void SoundGraphNodeEditorModel::Deserialize()
	{
		BuildNodes();

		// If we have cached prototype, create graph playback instance
		if (!m_GraphAsset->CachedPrototype.empty())
		{
			// TODO: check if m_GraphAsset is newer than cached prototype, if yes, potentially invalidate cache

			const AssetMetadata& md = Project::GetEditorAssetManager()->GetMetadata(m_GraphAsset);
			const std::string name = md.FilePath.stem().string();

			if (Ref<SoundGraph::Prototype> prototype = m_Cache->ReadPtototype(name.c_str()))
			{
				auto soundGraph = SoundGraph::CreateInstance(prototype);
				// Return if failed to "compile" or validate graph
				if (!soundGraph)
				{
					m_PlayerDirty = true;
					return;
				}

				m_GraphAsset->Prototype = prototype;

				// TODO: validate Prototype graph

				// 4. Prepare streaming wave sources
				m_SoundGraphSource->SuspendProcessing(true);
				// Wait for audio Process Block to stop
				while (!m_SoundGraphSource->IsSuspended()) {}

				m_SoundGraphSource->UninitializeDataSources();
				m_SoundGraphSource->InitializeDataSources(m_GraphAsset->WaveSources);

				// 5. Update preview player with the new compiled patch player
				m_SoundGraphSource->ReplacePlayer(soundGraph);

				// 6. Initialize player's default parameter values (graph "preset")
				if (soundGraph)
				{
					bool parametersSet = m_SoundGraphSource->ApplyParameterPreset(m_GraphAsset->GraphInputs);

					// Preset must match parameters (input events) of the compiled patch player 
					m_PlayerDirty = !parametersSet;
				}
			}
		}
	}


	//==================================================================================
	std::string SoundGraphNodeEditorModel::GetPropertyToken(ESoundGraphPropertyType type)
	{
		return Utils::SplitAtUpperCase(magic_enum::enum_name<ESoundGraphPropertyType>(type));
	}

	ESoundGraphPropertyType SoundGraphNodeEditorModel::GetPropertyType(std::string_view propertyToken)
	{
		if (propertyToken == GetPropertyToken(Input))
			return Input;
		else if (propertyToken == GetPropertyToken(Output))
			return Output;
		else if (propertyToken == GetPropertyToken(LocalVariable))
			return LocalVariable;
		else
			return Invalid;
	}

	bool SoundGraphNodeEditorModel::IsPropertyNode(ESoundGraphPropertyType type, const Node* node, std::string_view propertyName)
	{
		if (type == ESoundGraphPropertyType::LocalVariable ? node->Name != propertyName
			: node->Name != GetPropertyToken(type))
		{
			return false;
		}

		switch (type)
		{
		case Input: // Graph Input node can only have one output
			return  node->Outputs.size() == 1 && node->Outputs[0]->Name == propertyName;
		case Output: // Graph Output node can only have one output
			return node->Inputs.size() == 1 && node->Inputs[0]->Name == propertyName;
		case LocalVariable: // Graph Local Variable node can only have one input or output
			return node->Outputs.empty() ? node->Inputs[0]->Name == propertyName : node->Outputs[0]->Name == propertyName;
		}

		return false;
	}

	bool SoundGraphNodeEditorModel::IsPropertyNode(const Node* node) const
	{
		return (node->Category == GetPropertyToken(ESoundGraphPropertyType::Input)
			&& node->Inputs.empty()
			&& GetPropertySet(ESoundGraphPropertyType::Input).HasValue(node->Outputs[0]->Name))
			|| (node->Category == GetPropertyToken(ESoundGraphPropertyType::Output)
				&& node->Outputs.empty()
				&& GetPropertySet(ESoundGraphPropertyType::Output).HasValue(node->Inputs[0]->Name))
			|| (node->Category == GetPropertyToken(ESoundGraphPropertyType::LocalVariable)
				&& GetPropertySet(ESoundGraphPropertyType::LocalVariable).HasValue(node->Outputs.empty() ? node->Inputs[0]->Name : node->Outputs[0]->Name));
	}

	ESoundGraphPropertyType SoundGraphNodeEditorModel::GetPropertyTypeOfNode(const Node* node)
	{
		if (!IsPropertyNode(node))
			return Invalid;

		return GetPropertyType(node->Category);
	}

	bool SoundGraphNodeEditorModel::IsLocalVariableNode(const Node* node)
	{
		return node->Category == GetPropertyToken(ESoundGraphPropertyType::LocalVariable)
			&& ((node->Inputs.size() == 1 && node->Outputs.size() == 0)
				|| (node->Inputs.size() == 0 && node->Outputs.size() == 1));
	}

	bool SoundGraphNodeEditorModel::IsSameLocalVariableNodes(const Node* nodeA, const Node* nodeB)
	{
		if (!IsLocalVariableNode(nodeA) || !IsLocalVariableNode(nodeB))
			return false;

		if (nodeA == nodeB)
			return true;

		return (nodeA->Inputs.empty() ? nodeA->Outputs[0]->Name : nodeA->Inputs[0]->Name)
			== (nodeB->Inputs.empty() ? nodeB->Outputs[0]->Name : nodeB->Inputs[0]->Name);
	}

	Pin* SoundGraphNodeEditorModel::FindLocalVariableSource(std::string_view localVariableName)
	{
		Node* setterNode = nullptr;

		// Find setter node for the local variable
		for (auto& node : GetNodes())
		{
			if (IsLocalVariableNode(node) && node->Name == localVariableName && node->Inputs.size() == 1)
			{
				setterNode = node;
				break;
			}
		}

		if (!setterNode)
			return nullptr;

		// find link and pin connected to the input pin of the setter node
		if (auto* link = GetLinkConnectedToPin(setterNode->Inputs[0]->ID))
			return FindPin(link->StartPinID);

		return nullptr;
	}

	Utils::PropertySet& SoundGraphNodeEditorModel::GetPropertySet(ESoundGraphPropertyType type)
	{
		ANT_CORE_ASSERT(type != Invalid);

		switch (type)
		{
		case Input:
			return m_GraphAsset->GraphInputs;
		case Output:
			return m_GraphAsset->GraphOutputs;
		case LocalVariable:
			return m_GraphAsset->LocalVariables;
		case Invalid:
		default:
			return m_GraphAsset->GraphInputs;
		}
	}

	const Utils::PropertySet& SoundGraphNodeEditorModel::GetPropertySet(ESoundGraphPropertyType type) const
	{
		ANT_CORE_ASSERT(type != Invalid);

		switch (type)
		{
		case Input:
			return m_GraphAsset->GraphInputs;
		case Output:
			return m_GraphAsset->GraphOutputs;
		case LocalVariable:
			return m_GraphAsset->LocalVariables;
		case Invalid:
		default:
			return m_GraphAsset->GraphInputs;
		}
	}

	void SoundGraphNodeEditorModel::AddPropertyToGraph(ESoundGraphPropertyType type, const choc::value::ValueView& value)
	{
		if (type == Invalid)
		{
			ANT_CORE_ASSERT(false);
			return;
		}

		std::string name;

		const auto getUniqueName = [&](const Utils::PropertySet& propertySet, std::string_view defaultName)
		{
			return Ant::Utils::AddSuffixToMakeUnique(std::string(defaultName), [&](const std::string& newName)
				{
					auto properties = propertySet.GetNames();
					return std::any_of(properties.begin(), properties.end(), [&newName](const std::string name) { return name == newName; });
				});
		};

		auto& properties = GetPropertySet(type);

		switch (type)
		{
		case ESoundGraphPropertyType::Input:
			name = getUniqueName(properties, "New Input");
			properties.Set(name, value);
			break;
		case ESoundGraphPropertyType::Output:
			name = getUniqueName(properties, "New Output");
			properties.Set(name, value);
			break;
		case ESoundGraphPropertyType::LocalVariable:
			name = getUniqueName(properties, "New Variable");
			properties.Set(name, value);
			break;
		default:
			break;
		}

		InvalidatePlayer();

		//? DBG
		//ANT_CORE_WARN("Added Input {} to the graph. Inputs:", name);
		//ANT_LOG_TRACE(m_GraphInputs.ToJSON());
	}

	void SoundGraphNodeEditorModel::RemovePropertyFromGraph(ESoundGraphPropertyType type, const std::string& name)
	{
		if (type == Invalid)
		{
			ANT_CORE_ASSERT(false);
			return;
		}

		auto& properties = GetPropertySet(type);
		properties.Remove(name);

		std::vector<UUID> nodesToDelete;

		for (const auto& node : m_GraphAsset->Nodes)
		{
			if (IsPropertyNode(type, node, name))
				nodesToDelete.push_back(node->ID);
		}

		RemoveNodes(nodesToDelete);

		InvalidatePlayer();

		//? DBG
		//ANT_CORE_WARN("Removed Input {} from the graph. Inputs:", name);
	}

	void SoundGraphNodeEditorModel::SetPropertyValue(ESoundGraphPropertyType type, const std::string& propertyName, const choc::value::ValueView& value)
	{
		if (type == Invalid)
		{
			ANT_CORE_ASSERT(false);
			return;
		}

		auto& properties = GetPropertySet(type);
		if (properties.HasValue(propertyName))
		{
			properties.Set(propertyName, value);
			OnGraphPropertyValueChanged(type, propertyName);
		}
	}

	void SoundGraphNodeEditorModel::RenameProperty(ESoundGraphPropertyType type, const std::string& oldName, const std::string& newName)
	{
		if (type == Invalid)
		{
			ANT_CORE_ASSERT(false);
			return;
		}

		auto& properties = GetPropertySet(type);
		if (properties.HasValue(oldName))
		{
			choc::value::Value value = properties.GetValue(oldName);

			properties.Remove(oldName);
			properties.Set(newName, value);

			OnGraphPropertyNameChanged(type, oldName, newName);
		}
	}

	void SoundGraphNodeEditorModel::ChangePropertyType(ESoundGraphPropertyType type, const std::string& propertyName, const choc::value::ValueView& valueOfNewType)
	{
		if (type == Invalid)
		{
			ANT_CORE_ASSERT(false);
			return;
		}

		auto& properties = GetPropertySet(type);
		if (properties.HasValue(propertyName))
		{
			choc::value::Value value = properties.GetValue(propertyName);
			if (value.getType() != valueOfNewType.getType())
			{
				properties.Set(propertyName, valueOfNewType);

				OnGraphPropertyTypeChanged(type, propertyName);
			}
		}
	}

	void SoundGraphNodeEditorModel::OnGraphPropertyNameChanged(ESoundGraphPropertyType type, const std::string& oldName, const std::string& newName)
	{
		if (type == Invalid)
			return;

		const auto getPropertyValuePin = [](Node* node, std::string_view propertyName) -> Pin*
		{
			ANT_CORE_ASSERT(!node->Outputs.empty() || !node->Inputs.empty());

			Pin* pin = nullptr;

			if (!node->Outputs.empty())
				pin = node->Outputs[0];
			else if (!node->Inputs.empty())
				pin = node->Inputs[0];

			ANT_CORE_ASSERT(!pin ^ pin->Name == propertyName);

			return pin;
		};

		bool renamed = false;

		for (auto& node : m_GraphAsset->Nodes)
		{
			if (IsPropertyNode(type, node, oldName))
			{
				if (Pin* pin = getPropertyValuePin(node, oldName))
				{
					pin->Name = newName;

					if (pin->Kind == PinKind::Output)
					{
						if (Link* link = GetLinkConnectedToPin(pin->ID))
						{
							if (Pin* endPin = FindPin(link->EndPinID))
								endPin->Value = choc::value::createString(newName);
						}
					}

					renamed = true;
				}

				if (type == ESoundGraphPropertyType::LocalVariable)
				{
					node->Name = newName;
					renamed = true;
				}
			}
		}

		if (renamed)
			InvalidatePlayer();
	}

	void SoundGraphNodeEditorModel::OnGraphPropertyTypeChanged(ESoundGraphPropertyType type, const std::string& inputName)
	{
		if (type == Invalid)
			return;

		auto newProperty = GetPropertySet(type).GetValue(inputName);

		const bool isArray = newProperty.isArray();
		auto [pinType, storageKind] = SoundGraphNodeGraphEditor::GetPinTypeAndStorageKindForValue(newProperty);

		const auto getPropertyValuePin = [](Node* node, std::string_view propertyName) -> Pin**
		{
			ANT_CORE_ASSERT(!node->Outputs.empty() || !node->Inputs.empty());

			if (!node->Outputs.empty())
				return &node->Outputs[0];
			else if (!node->Inputs.empty())
				return &node->Inputs[0];

			return nullptr;
		};

		bool pinChagned = false;

		for (auto& node : m_GraphAsset->Nodes)
		{
			if (IsPropertyNode(type, node, inputName))
			{
				if (Pin** pin = getPropertyValuePin(node, inputName))
				{
					// create new pin of the new type
					Pin* newPin = Nodes::SGTypes::CreatePinForType(pinType);

					// copy data from the old pin to the new pin
					*newPin = **pin;

					// delete old pin
					delete (*pin);

					// emplace new pin instead of the old pin
					*pin = newPin;

					//pin->Type = pinType;
					newPin->Storage = storageKind;
					newPin->Value = newProperty; // TODO: should use one or the other
					node->Color = SoundGraphNodeGraphEditor::GetTypeColour(newProperty);

					if (Link* connectedLink = GetLinkConnectedToPin(newPin->ID))
					{
						ax::NodeEditor::DeleteLink(ax::NodeEditor::LinkId(connectedLink->ID));
						RemoveLink(connectedLink->ID);
						// TODO: add visual indication for the user that the node pin has been disconnected, or that the link is invalid
					}

					pinChagned = true;
				}
			}
		}

		if (pinChagned)
			InvalidatePlayer();
	}

	void SoundGraphNodeEditorModel::OnGraphPropertyValueChanged(ESoundGraphPropertyType type, const std::string& inputName)
	{
		if (type == Invalid)
			return;

		auto newValue = GetPropertySet(type).GetValue(inputName);
		const auto& valueType = newValue.getType();

		// If wave asset input changed, need to initialize audio data readers for new files
		// which we do when recompiling.
		// TODO: perhaps we could somehow do a clean swap of the readers while the player is playing?
		if (valueType.isObjectWithClassName("AssetHandle") || (valueType.isArray() && valueType.getElementType().isObjectWithClassName("AssetHandle"))
			|| (NodeGraphEditorBase::IsAssetHandle(valueType) || (valueType.isArray() && NodeGraphEditorBase::IsAssetHandle(valueType.getElementType()))))
		{
			InvalidatePlayer();
		}

		const auto getPropertyValuePin = [](Node* node, std::string_view propertyName) -> Pin*
		{
			ANT_CORE_ASSERT(!node->Outputs.empty() || !node->Inputs.empty());

			if (!node->Outputs.empty())
				return node->Outputs[0];
			else if (!node->Inputs.empty())
				return node->Inputs[0];

			return nullptr;
		};

		// Set Outputing value for the "Graph Input" node
		for (auto& node : m_GraphAsset->Nodes)
		{
			if (IsPropertyNode(type, node, inputName))
			{
				if (Pin* pin = getPropertyValuePin(node, inputName))
					pin->Value = newValue;
			}
		}

		if (type == ESoundGraphPropertyType::LocalVariable)
			InvalidatePlayer();

		// If player is dirty, need to recompile it first and update the parameter set
		// before sending new values from the GUI
		if (!IsPlayerDirty() && type == ESoundGraphPropertyType::Input)
		{
			// Call parameter change for the live preview player
			bool set = m_SoundGraphSource->SetParameter(choc::text::replace(inputName, " ", ""), newValue);
			ANT_CORE_ASSERT(set);
		}
	}

	//==================================================================================
	Node* SoundGraphNodeEditorModel::SpawnGraphInputNode(const std::string& inputName)
	{
		Node* newNode = Nodes::SoundGraphNodeFactory::SpawnGraphPropertyNode(m_GraphAsset, inputName, ESoundGraphPropertyType::Input);

		if (!newNode)
			return nullptr;

		OnNodeSpawned(newNode);
		//BuildNode(&GetNodes().back());
		// std::vector of nodes migh have reallocated when resized to add new node,
		// so pointers might have been invalidated
		BuildNodes();

		OnNodeCreated();

		return GetNodes().back();
	}

	Node* SoundGraphNodeEditorModel::SpawnLocalVariableNode(const std::string& variableName, bool getter)
	{
		// We don't allow to create multiple setter nodes for graph Local Variables
		if (!getter)
		{
			for (const auto& node : m_GraphAsset->Nodes)
			{
				if (node->Category == GetPropertyToken(ESoundGraphPropertyType::LocalVariable)
					&& node->Name == variableName
					&& node->Inputs.size() == 1
					&& node->Inputs[0]->Name == variableName)
				{
					ANT_CONSOLE_LOG_TRACE("Setter node for the variable '{}' already exists!", variableName);
					ax::NodeEditor::SelectNode((uint64_t)node->ID, false);
					ax::NodeEditor::NavigateToSelection();
					return nullptr;
				}
			}
		}

		Node* newNode = Nodes::SoundGraphNodeFactory::SpawnGraphPropertyNode(m_GraphAsset, variableName, ESoundGraphPropertyType::LocalVariable, getter);

		if (!newNode)
			return nullptr;

		OnNodeSpawned(newNode);

		// mostly to handle dangling inputs/outpus
		BuildNodes();

		OnNodeCreated();

		return GetNodes().back();
	}

	uint64_t SoundGraphNodeEditorModel::GetCurrentPlaybackFrame() const
	{
		if (!m_SoundGraphSource || m_SoundGraphSource->IsSuspended())
			return 0;

		return m_SoundGraphSource->GetCurrentFrame();
	}

	void SoundGraphNodeEditorModel::OnUpdate(Timestep ts)
	{
		//! Care should be taken as this may be called not from Audio Thread
		if (m_SoundGraphSource)
			m_SoundGraphSource->Update(ts);
	}

} // namespace Ant