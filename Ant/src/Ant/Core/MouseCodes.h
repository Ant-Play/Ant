#pragma once

namespace Ant {

		typedef enum class MouseCode : uint16_t
		{
			// From glfw3.h
			Button0								= 0,
			Button1								= 1,
			Button2								= 2,
			Button3								= 3,
			Button4								= 4,
			Button5								= 5,
			Button6								= 6,
			Button7								= 7,

			ButtonLast							= Button7,
			ButtonLeft							= Button0,
			ButtonRight							= Button1,
			ButtonMiddle						= Button2
		} Mouse;

		inline std::ostream& operator<<(std::ostream& os, MouseCode mouseCode)
		{
			os << static_cast<int32_t>(mouseCode);
			return os;
		}
}

#define ANT_MOUSE_BUTTON_0      ::Ant::Mouse::Button0
#define ANT_MOUSE_BUTTON_1      ::Ant::Mouse::Button1
#define ANT_MOUSE_BUTTON_2      ::Ant::Mouse::Button2
#define ANT_MOUSE_BUTTON_3      ::Ant::Mouse::Button3
#define ANT_MOUSE_BUTTON_4      ::Ant::Mouse::Button4
#define ANT_MOUSE_BUTTON_5      ::Ant::Mouse::Button5
#define ANT_MOUSE_BUTTON_6      ::Ant::Mouse::Button6
#define ANT_MOUSE_BUTTON_7      ::Ant::Mouse::Button7
#define ANT_MOUSE_BUTTON_LAST   ::Ant::Mouse::ButtonLast
#define ANT_MOUSE_BUTTON_LEFT   ::Ant::Mouse::ButtonLeft
#define ANT_MOUSE_BUTTON_RIGHT  ::Ant::Mouse::ButtonRight
#define ANT_MOUSE_BUTTON_MIDDLE ::Ant::Mouse::ButtonMiddle
