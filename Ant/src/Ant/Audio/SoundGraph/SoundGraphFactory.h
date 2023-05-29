#pragma once

#include "NodeProcessor.h"
#include "Ant/Core/Identifier.h"

namespace Ant::SoundGraph {

	class Factory
	{
		Factory() = delete;
	public:
		[[nodiscard]] static NodeProcessor* Create(Identifier nodeTypeID, UUID nodeID);
		static bool Contains(Identifier nodeTypeID);
	};
}
