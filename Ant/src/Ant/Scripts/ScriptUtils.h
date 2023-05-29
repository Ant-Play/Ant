#pragma once

#include "ScriptCache.h"
#include "ValueWrapper.h"
#include "Ant/Scene/Entity.h"

#include <mono/utils/mono-error.h>
#include <yaml-cpp/yaml.h>

#ifdef ANT_PLATFORM_WINDOWS
#define ANT_MONO_STDCALL __stdcall
#else
#define ANT_MONO_STDCALL
#endif

#ifdef ANT_DEBUG
#define ANT_CHECK_MANAGED_METHOD(x) ANT_CORE_ASSERT(x)
#else
#define ANT_CHECK_MANAGED_METHOD(x) ANT_CORE_VERIFY(x)
#endif

extern "C" {
	typedef struct _MonoType MonoType;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoString MonoString;
	typedef struct _MonoArray MonoArray;
	typedef struct _MonoException MonoException;
}

#ifndef ANT_DIST
#define ANT_THROW_INVALID_OPERATION(msg) mono_raise_exception(mono_get_exception_invalid_operation(msg))
#else
#define ANT_THROW_INVALID_OPERATION(msg)
#endif

namespace Ant{

	class ScriptUtils;

	template<typename... TParameterTypes>
	struct ManagedMethodThunk
	{
		typedef void(ANT_MONO_STDCALL* M)(TParameterTypes... params, MonoException**);

		M Method = nullptr;

		ManagedMethodThunk() = default;

		ManagedMethodThunk(const ManagedMethod* method)
		{
			SetThunkFromMethod(method);
		}

		void Invoke(TParameterTypes... params, MonoException** exception)
		{
			Method(params..., exception);
		}

		void SetThunkFromMethod(const ManagedMethod* method)
		{
			ANT_CHECK_MANAGED_METHOD(method->Method);
			//ANT_CHECK_MANAGED_METHOD(method->ReturnType.IsVoid());

			if (method->IsStatic)
			{
				ANT_CHECK_MANAGED_METHOD(method->ParameterCount == sizeof...(TParameterTypes));
			}
			else
			{
				ANT_CHECK_MANAGED_METHOD(method->ParameterCount == sizeof...(TParameterTypes) - 1);
			}

			Method = (M)ScriptUtils::GetUnmanagedMethodTunk(method->Method);
		}
	};

	/*template<typename TReturn, typename... TParameterTypes>
	struct ManagedMethodThunkR
	{
		typedef TReturn(ANT_MONO_STDCALL* M)(TParameterTypes... params, MonoException**);

		M Method = nullptr;

		ManagedMethodThunkR() = default;

		ManagedMethodThunkR(const ManagedMethod* method)
		{
			SetThunkFromMethod(method);
		}

		TReturn Invoke(TParameterTypes... params, MonoException** exception)
		{
			return Method(params..., exception);
		}

		void SetThunkFromMethod(const ManagedMethod* method)
		{
			ANT_CHECK_MANAGED_METHOD(method->Method);
			ANT_CHECK_MANAGED_METHOD(!method->ReturnType.IsVoid());

			if (method->IsStatic)
			{
				ANT_CHECK_MANAGED_METHOD(method->ParameterCount == sizeof...(TParameterTypes));
			}
			else
			{
				ANT_CHECK_MANAGED_METHOD(method->ParameterCount == sizeof...(TParameterTypes) - 1);
			}

			Method = (M)ScriptUtils::GetUnmanagedMethodTunk(method->Method);
		}
	};*/

	class ScriptUtils
	{
	public:
		static void Init();
		static void Shutdown();

		static bool CheckMonoError(MonoError& error);
		static void HandleException(MonoObject* exception);

		static Buffer GetFieldValue(MonoObject* classInstance, const std::string& fieldName, FieldType fieldType, bool isProperty);
		static MonoObject* GetFieldValueObject(MonoObject* classInstance, const std::string& fieldName, bool isProperty);
		static void SetFieldValue(MonoObject* classInstance, const FieldInfo* fieldInfo, const void* data);

		// Type Utils
		static Buffer MonoObjectToValue(MonoObject* obj, FieldType fieldType);
		static MonoObject* ValueToMonoObject(const void* data, FieldType dataType);
		//static Utils::ValueWrapper GetDefaultValueForType(const ManagedType& type);
		//static MonoObject* GetDefaultValueObjectForType(const ManagedType& type);
		static FieldType GetFieldTypeFromMonoType(MonoType* monoType);

		static std::string ResolveMonoClassName(MonoClass* monoClass);

		// String
		static MonoString* EmptyMonoString();
		static std::string MonoStringToUTF8(MonoString* monoString);
		static MonoString* UTF8StringToMono(const std::string& str);

		// Boxing
		template<typename TValueType>
		static TValueType Unbox(MonoObject* obj) { return *(TValueType*)mono_object_unbox(obj); }
		template<typename TValueType>
		static TValueType UnboxAddress(MonoObject* obj) { return (TValueType*)mono_object_unbox(obj); }

		static MonoObject* BoxValue(MonoClass* valueClass, const void* value);
	private:
		static void* GetUnmanagedMethodTunk(MonoMethod* managedMethod);
	private:
		template<typename... TParameterTypes>
		friend struct ManagedMethodThunk;

		/*template<typename TReturn, typename... TParameterTypes>
		friend struct ManagedMethodThunkR;*/
	};

	class ManagedArrayUtils
	{
	public:
		static Utils::ValueWrapper GetValue(MonoArray* arr, uintptr_t index);

		template<typename TValueType>
		static void SetValue(MonoArray* arr, uintptr_t index, TValueType value)
		{
			if constexpr (std::is_same<TValueType, MonoObject*>::value)
				SetValueInternal(arr, index, value);
			else
				SetValueInternal(arr, index, &value);
		}

		static uintptr_t Length(MonoArray* arr);
		static void Resize(MonoArray** arr, uintptr_t newLength);
		static void RemoveAt(MonoArray** arr, uintptr_t index);
		static MonoArray* Copy(MonoArray* arr);

		template<typename TValueType>
		static MonoArray* FromVector(const std::vector<TValueType>& vec)
		{
			MonoArray* arr = Create<TValueType>(vec.size());
			for (size_t i = 0; i < vec.size(); i++)
				SetValue<TValueType>(arr, i, vec[i]);
			return arr;
		}

		template<typename TValueType>
		static std::vector<TValueType> ToVector(MonoArray* arr)
		{
			uintptr_t length = Length(arr);

			std::vector<TValueType> vec;
			vec.resize(length);

			if constexpr (std::is_same_v<TValueType, Utils::ValueWrapper>)
			{
				for (uintptr_t i = 0; i < length; i++)
					vec[i] = GetValue(arr, i);
			}
			else
			{
				for (uintptr_t i = 0; i < length; i++)
					vec[i] = GetValue(arr, i).Get<TValueType>();
			}

			return vec;
		}
	public:
		static MonoArray* Create(const std::string& arrayClass, uintptr_t length);
		static MonoArray* Create(ManagedClass* arrayClass, uintptr_t length);

		template<typename T>
		static MonoArray* Create(uintptr_t length) { static_assert(false, "Unsupported Native Type!"); }

		template<> static MonoArray* Create<bool>(uintptr_t length) { return Create("System.Boolean", length); }
		template<> static MonoArray* Create<int8_t>(uintptr_t length) { return Create("System.SByte", length); }
		template<> static MonoArray* Create<int16_t>(uintptr_t length) { return Create("System.Int16", length); }
		template<> static MonoArray* Create<int32_t>(uintptr_t length) { return Create("System.Int32", length); }
		template<> static MonoArray* Create<int64_t>(uintptr_t length) { return Create("System.Int64", length); }
		template<> static MonoArray* Create<uint8_t>(uintptr_t length) { return Create("System.Byte", length); }
		template<> static MonoArray* Create<uint16_t>(uintptr_t length) { return Create("System.UInt16", length); }
		template<> static MonoArray* Create<uint32_t>(uintptr_t length) { return Create("System.UInt32", length); }
		template<> static MonoArray* Create<uint64_t>(uintptr_t length) { return Create("System.UInt64", length); }
		template<> static MonoArray* Create<float>(uintptr_t length) { return Create("System.Single", length); }
		template<> static MonoArray* Create<double>(uintptr_t length) { return Create("System.Double", length); }
		template<> static MonoArray* Create<char>(uintptr_t length) { return Create("System.Char", length); }
		template<> static MonoArray* Create<std::string>(uintptr_t length) { return Create("System.String", length); }
		template<> static MonoArray* Create<Entity>(uintptr_t length) { return Create("Ant.Entity", length); }
		template<> static MonoArray* Create<Prefab>(uintptr_t length) { return Create("Ant.Prefab", length); }
		template<> static MonoArray* Create<glm::vec2>(uintptr_t length) { return Create("Ant.Vector2", length); }
		template<> static MonoArray* Create<glm::vec3>(uintptr_t length) { return Create("Ant.Vector3", length); }
		template<> static MonoArray* Create<glm::vec4>(uintptr_t length) { return Create("Ant.Vector4", length); }
	private:
		static void SetValueInternal(MonoArray* arr, uintptr_t index, void* data);
		static void SetValueInternal(MonoArray* arr, uintptr_t index, MonoObject* value);
	};

	class MethodThunks
	{
	public:
		static void OnEntityCreate(GCHandle entityHandle);
		static void OnEntityUpdate(GCHandle entityHandle, float ts);
		static void OnEntityPhysicsUpdate(GCHandle entityHandle, float ts);
		static void OnEntityDestroyed(GCHandle entityHandle);

		static void IEditorRunnable_OnInstantiate(GCHandle scriptHandle);
		static void IEditorRunnable_OnUIRender(GCHandle scriptHandle);
		static void IEditorRunnable_OnShutdown(GCHandle scriptHandle);
	};
}
