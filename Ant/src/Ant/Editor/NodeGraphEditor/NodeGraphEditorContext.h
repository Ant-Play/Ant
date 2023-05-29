#pragma once

// This file exists to avoid having to include imgui_node_editor.h in NodeGraphEditor.h

#include "NodeGraphEditor.h"
#include "imgui-node-editor/imgui_node_editor.h"

namespace Ant {

	struct NodeGraphEditorBase::ContextState
	{
		bool CreateNewNode = false;
		UUID NewNodeLinkPinId = 0;
		Pin* NewLinkPin = nullptr;
		bool DraggingInputField = false;

		ax::NodeEditor::NodeId ContextNodeId = 0;
		ax::NodeEditor::LinkId ContextLinkId = 0;
		ax::NodeEditor::PinId ContextPinId = 0;
	};

}