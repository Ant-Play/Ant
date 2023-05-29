#pragma once

#include <ostream>

namespace Ant {
	typedef enum class KeyCode : uint16_t
	{
		// From glfw3.h
		Space = 32,
		Apostrophe = 39, /* ' */
		Comma = 44, /* , */
		Minus = 45, /* - */
		Period = 46, /* . */
		Slash = 47, /* / */

		D0 = 48, /* 0 */
		D1 = 49, /* 1 */
		D2 = 50, /* 2 */
		D3 = 51, /* 3 */
		D4 = 52, /* 4 */
		D5 = 53, /* 5 */
		D6 = 54, /* 6 */
		D7 = 55, /* 7 */
		D8 = 56, /* 8 */
		D9 = 57, /* 9 */

		Semicolon = 59, /* ; */
		Equal = 61, /* = */

		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,

		LeftBracket = 91,  /* [ */
		Backslash = 92,  /* \ */
		RightBracket = 93,  /* ] */
		GraveAccent = 96,  /* ` */

		World1 = 161, /* non-US #1 */
		World2 = 162, /* non-US #2 */

		/* Function keys */
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,

		/* Keypad */
		KP0 = 320,
		KP1 = 321,
		KP2 = 322,
		KP3 = 323,
		KP4 = 324,
		KP5 = 325,
		KP6 = 326,
		KP7 = 327,
		KP8 = 328,
		KP9 = 329,
		KPDecimal = 330,
		KPDivide = 331,
		KPMultiply = 332,
		KPSubtract = 333,
		KPAdd = 334,
		KPEnter = 335,
		KPEqual = 336,

		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348
	} Key;

	enum class KeyState
	{
		None = -1,
		Pressed,
		Held,
		Released
	};

	enum class CursorMode
	{
		Normal = 0,
		Hidden = 1,
		Locked = 2
	};

	typedef enum class MouseButton : uint16_t
	{
		Button0 = 0,
		Button1 = 1,
		Button2 = 2,
		Button3 = 3,
		Button4 = 4,
		Button5 = 5,
		Left = Button0,
		Right = Button1,
		Middle = Button2
	} Button;

	inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
	{
		os << static_cast<int32_t>(keyCode);
		return os;
	}

	inline std::ostream& operator<<(std::ostream& os, MouseButton button)
	{
		os << static_cast<int32_t>(button);
		return os;
	}
}

// From glfw3.h
#define ANT_KEY_SPACE           ::Ant::Key::Space
#define ANT_KEY_APOSTROPHE      ::Ant::Key::Apostrophe    /* ' */
#define ANT_KEY_COMMA           ::Ant::Key::Comma         /* , */
#define ANT_KEY_MINUS           ::Ant::Key::Minus         /* - */
#define ANT_KEY_PERIOD          ::Ant::Key::Period        /* . */
#define ANT_KEY_SLASH           ::Ant::Key::Slash         /* / */
#define ANT_KEY_0               ::Ant::Key::D0
#define ANT_KEY_1               ::Ant::Key::D1
#define ANT_KEY_2               ::Ant::Key::D2
#define ANT_KEY_3               ::Ant::Key::D3
#define ANT_KEY_4               ::Ant::Key::D4
#define ANT_KEY_5               ::Ant::Key::D5
#define ANT_KEY_6               ::Ant::Key::D6
#define ANT_KEY_7               ::Ant::Key::D7
#define ANT_KEY_8               ::Ant::Key::D8
#define ANT_KEY_9               ::Ant::Key::D9
#define ANT_KEY_SEMICOLON       ::Ant::Key::Semicolon     /* ; */
#define ANT_KEY_EQUAL           ::Ant::Key::Equal         /* = */
#define ANT_KEY_A               ::Ant::Key::A
#define ANT_KEY_B               ::Ant::Key::B
#define ANT_KEY_C               ::Ant::Key::C
#define ANT_KEY_D               ::Ant::Key::D
#define ANT_KEY_E               ::Ant::Key::E
#define ANT_KEY_F               ::Ant::Key::F
#define ANT_KEY_G               ::Ant::Key::G
#define ANT_KEY_H               ::Ant::Key::H
#define ANT_KEY_I               ::Ant::Key::I
#define ANT_KEY_J               ::Ant::Key::J
#define ANT_KEY_K               ::Ant::Key::K
#define ANT_KEY_L               ::Ant::Key::L
#define ANT_KEY_M               ::Ant::Key::M
#define ANT_KEY_N               ::Ant::Key::N
#define ANT_KEY_O               ::Ant::Key::O
#define ANT_KEY_P               ::Ant::Key::P
#define ANT_KEY_Q               ::Ant::Key::Q
#define ANT_KEY_R               ::Ant::Key::R
#define ANT_KEY_S               ::Ant::Key::S
#define ANT_KEY_T               ::Ant::Key::T
#define ANT_KEY_U               ::Ant::Key::U
#define ANT_KEY_V               ::Ant::Key::V
#define ANT_KEY_W               ::Ant::Key::W
#define ANT_KEY_X               ::Ant::Key::X
#define ANT_KEY_Y               ::Ant::Key::Y
#define ANT_KEY_Z               ::Ant::Key::Z
#define ANT_KEY_LEFT_BRACKET    ::Ant::Key::LeftBracket   /* [ */
#define ANT_KEY_BACKSLASH       ::Ant::Key::Backslash     /* \ */
#define ANT_KEY_RIGHT_BRACKET   ::Ant::Key::RightBracket  /* ] */
#define ANT_KEY_GRAVE_ACCENT    ::Ant::Key::GraveAccent   /* ` */
#define ANT_KEY_WORLD_1         ::Ant::Key::World1        /* non-US #1 */
#define ANT_KEY_WORLD_2         ::Ant::Key::World2        /* non-US #2 */

/* Function keys */
#define ANT_KEY_ESCAPE          ::Ant::Key::Escape
#define ANT_KEY_ENTER           ::Ant::Key::Enter
#define ANT_KEY_TAB             ::Ant::Key::Tab
#define ANT_KEY_BACKSPACE       ::Ant::Key::Backspace
#define ANT_KEY_INSERT          ::Ant::Key::Insert
#define ANT_KEY_DELETE          ::Ant::Key::Delete
#define ANT_KEY_RIGHT           ::Ant::Key::Right
#define ANT_KEY_LEFT            ::Ant::Key::Left
#define ANT_KEY_DOWN            ::Ant::Key::Down
#define ANT_KEY_UP              ::Ant::Key::Up
#define ANT_KEY_PAGE_UP         ::Ant::Key::PageUp
#define ANT_KEY_PAGE_DOWN       ::Ant::Key::PageDown
#define ANT_KEY_HOME            ::Ant::Key::Home
#define ANT_KEY_END             ::Ant::Key::End
#define ANT_KEY_CAPS_LOCK       ::Ant::Key::CapsLock
#define ANT_KEY_SCROLL_LOCK     ::Ant::Key::ScrollLock
#define ANT_KEY_NUM_LOCK        ::Ant::Key::NumLock
#define ANT_KEY_PRINT_SCREEN    ::Ant::Key::PrintScreen
#define ANT_KEY_PAUSE           ::Ant::Key::Pause
#define ANT_KEY_F1              ::Ant::Key::F1
#define ANT_KEY_F2              ::Ant::Key::F2
#define ANT_KEY_F3              ::Ant::Key::F3
#define ANT_KEY_F4              ::Ant::Key::F4
#define ANT_KEY_F5              ::Ant::Key::F5
#define ANT_KEY_F6              ::Ant::Key::F6
#define ANT_KEY_F7              ::Ant::Key::F7
#define ANT_KEY_F8              ::Ant::Key::F8
#define ANT_KEY_F9              ::Ant::Key::F9
#define ANT_KEY_F10             ::Ant::Key::F10
#define ANT_KEY_F11             ::Ant::Key::F11
#define ANT_KEY_F12             ::Ant::Key::F12
#define ANT_KEY_F13             ::Ant::Key::F13
#define ANT_KEY_F14             ::Ant::Key::F14
#define ANT_KEY_F15             ::Ant::Key::F15
#define ANT_KEY_F16             ::Ant::Key::F16
#define ANT_KEY_F17             ::Ant::Key::F17
#define ANT_KEY_F18             ::Ant::Key::F18
#define ANT_KEY_F19             ::Ant::Key::F19
#define ANT_KEY_F20             ::Ant::Key::F20
#define ANT_KEY_F21             ::Ant::Key::F21
#define ANT_KEY_F22             ::Ant::Key::F22
#define ANT_KEY_F23             ::Ant::Key::F23
#define ANT_KEY_F24             ::Ant::Key::F24
#define ANT_KEY_F25             ::Ant::Key::F25

/* Keypad */
#define ANT_KEY_KP_0            ::Ant::Key::KP0
#define ANT_KEY_KP_1            ::Ant::Key::KP1
#define ANT_KEY_KP_2            ::Ant::Key::KP2
#define ANT_KEY_KP_3            ::Ant::Key::KP3
#define ANT_KEY_KP_4            ::Ant::Key::KP4
#define ANT_KEY_KP_5            ::Ant::Key::KP5
#define ANT_KEY_KP_6            ::Ant::Key::KP6
#define ANT_KEY_KP_7            ::Ant::Key::KP7
#define ANT_KEY_KP_8            ::Ant::Key::KP8
#define ANT_KEY_KP_9            ::Ant::Key::KP9
#define ANT_KEY_KP_DECIMAL      ::Ant::Key::KPDecimal
#define ANT_KEY_KP_DIVIDE       ::Ant::Key::KPDivide
#define ANT_KEY_KP_MULTIPLY     ::Ant::Key::KPMultiply
#define ANT_KEY_KP_SUBTRACT     ::Ant::Key::KPSubtract
#define ANT_KEY_KP_ADD          ::Ant::Key::KPAdd
#define ANT_KEY_KP_ENTER        ::Ant::Key::KPEnter
#define ANT_KEY_KP_EQUAL        ::Ant::Key::KPEqual

#define ANT_KEY_LEFT_SHIFT      ::Ant::Key::LeftShift
#define ANT_KEY_LEFT_CONTROL    ::Ant::Key::LeftControl
#define ANT_KEY_LEFT_ALT        ::Ant::Key::LeftAlt
#define ANT_KEY_LEFT_SUPER      ::Ant::Key::LeftSuper
#define ANT_KEY_RIGHT_SHIFT     ::Ant::Key::RightShift
#define ANT_KEY_RIGHT_CONTROL   ::Ant::Key::RightControl
#define ANT_KEY_RIGHT_ALT       ::Ant::Key::RightAlt
#define ANT_KEY_RIGHT_SUPER     ::Ant::Key::RightSuper
#define ANT_KEY_MENU            ::Ant::Key::Menu

// Mouse
#define ANT_MOUSE_BUTTON_LEFT    ::Ant::Button::Left
#define ANT_MOUSE_BUTTON_RIGHT   ::Ant::Button::Right
#define ANT_MOUSE_BUTTON_MIDDLE  ::Ant::Button::Middle