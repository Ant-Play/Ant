#pragma once

#include "Ant/Core/Hash.h"

namespace Ant::Audio {

	class CommandID
	{
	private:
		uint32_t ID;
	private:
		friend struct std::hash<CommandID>;
		uint32_t GetID() const noexcept { return ID; };
	public:
		CommandID() : CommandID("") {}
		explicit CommandID(const char* str) { ID = Hash::CRC32(str); }
		static CommandID FromString(const char* sourceString) { return CommandID(sourceString); }

		bool operator==(const CommandID& other) const { return ID == other.ID; }
		bool operator!=(const CommandID& other) const { return !(*this == other); }
		operator uint32_t() const { return GetID(); }

		static CommandID FromUnsignedInt(uint32_t ID)
		{
			CommandID commandID;
			commandID.ID = ID;
			return commandID;
		}

		static CommandID InvalidID() { return CommandID(""); }
	};
}

namespace std {

	template<>
	struct hash<Ant::Audio::CommandID>
	{
		size_t operator()(const Ant::Audio::CommandID& commandID) const noexcept
		{
			static_assert(noexcept(hash<uint32_t>()(commandID.GetID())), "hash function should not throw");
			return hash<uint32_t>()(commandID.GetID());
		}
	};
}
