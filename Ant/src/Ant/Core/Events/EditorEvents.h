#pragma once

#include "Event.h"

#include <sstream>

namespace Ant{

	class EditorExitPlayModeEvent : public Event
	{
	public:
		EditorExitPlayModeEvent() = default;

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "EditorExitPlayModeEvent";
			return ss.str();
		}

		EVENT_CLASS_TYPE(EditorExitPlayMode)
		EVENT_CLASS_CATEGORY(EventCategoryEditor)
	};
}