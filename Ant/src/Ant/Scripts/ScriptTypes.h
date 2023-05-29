#pragma once

#include "Ant/Core/Base.h"

#include <string>
#include <unordered_map>
#include <filesystem>

extern "C" {
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoClassField MonoClassField;
	typedef struct _MonoProperty MonoProperty;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoType MonoType;
}

namespace Ant{

	namespace Utils {

		// NOTE: Stolen from Boost
		template<typename T>
		inline void HashCombine(size_t& result, const T& value)
		{
			std::hash<T> h;
			result ^= h(value) + 0x9e3779b9 + (result << 6) + (result >> 2);
		}

	}

	namespace TypeUtils {
		bool ContainsAttribute(void* attributeList, const std::string& attributeName);
	}

	struct ManagedClass;

	struct ManagedMethod
	{
		uint32_t ID = 0;
		std::string FullName = "";
		bool IsVirtual = false;
		bool IsStatic = false;
		uint32_t ParameterCount = 0;

		MonoMethod* Method = nullptr;

		~ManagedMethod()
		{
			Method = nullptr;
		}
	};

	struct ManagedClass
	{
		uint32_t ID = 0;
		std::string FullName = "";
		std::vector<uint32_t> Fields;
		std::vector<uint32_t> Methods;
		uint32_t Size = 0;

		// Will also be true if class is static
		bool IsAbstract = false;
		bool IsStruct = false;

		uint32_t ParentID = 0;

		MonoClass* Class = nullptr;

		~ManagedClass()
		{
			Class = nullptr;
		}
	};

	struct AssemblyMetadata
	{
		std::string Name;
		uint32_t MajorVersion;
		uint32_t MinorVersion;
		uint32_t BuildVersion;
		uint32_t RevisionVersion;

		bool operator==(const AssemblyMetadata& other) const
		{
			return Name == other.Name && MajorVersion == other.MajorVersion && MinorVersion == other.MinorVersion && BuildVersion == other.BuildVersion && RevisionVersion == other.RevisionVersion;
		}

		bool operator!=(const AssemblyMetadata& other) const { return !(*this == other); }
	};

	struct AssemblyInfo : public RefCounted
	{
		std::filesystem::path FilePath = "";
		MonoAssembly* Assembly = nullptr;
		MonoImage* AssemblyImage = nullptr;
		std::vector<uint32_t> Classes;
		bool IsCoreAssembly = false;
		AssemblyMetadata Metadata;
		std::vector<AssemblyMetadata> ReferencedAssemblies;
	};

}

namespace std {

	template<>
	struct hash<Ant::AssemblyMetadata>
	{
		size_t operator()(const Ant::AssemblyMetadata& metadata) const
		{
			size_t result = 0;
			Ant::Utils::HashCombine(result, metadata.Name);
			Ant::Utils::HashCombine(result, metadata.MajorVersion);
			Ant::Utils::HashCombine(result, metadata.MinorVersion);
			Ant::Utils::HashCombine(result, metadata.BuildVersion);
			Ant::Utils::HashCombine(result, metadata.RevisionVersion);
			return result;
		}
	};

}
