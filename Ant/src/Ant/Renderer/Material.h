#pragma once

#include "Ant/Core/Base.h"
#include "Ant/Renderer/Shader.h"
#include "Ant/Renderer/Texture.h"
#include "Ant/Core/Buffer.h"

#include <unordered_set>

namespace Ant{

	enum class MaterialFlag
	{
		None = 0,
		DepthTest			= BIT(0),
		DepthWrite			= BIT(1),
		DepthFunc			= BIT(2),
		Blend				= BIT(3),
		BlendSrc			= BIT(4),
		BlendDst			= BIT(5),
		BlendEquation		= BIT(6),
		CullFace			= BIT(7),
		CullFaceMode		= BIT(8),
		FrontFace			= BIT(9),
	};

	class Material
	{
		friend class MaterialInstance;
	public:
		Material(const Ref<Shader>& shader);
		virtual ~Material();

		void Bind() const;

		uint32_t GetFlags() const { return m_MaterialFlags; }
		void SetFlag(MaterialFlag flag) { m_MaterialFlags |= static_cast<uint32_t>(flag); };

		template <typename T>
		void Set(const std::string& name, const T& value)
		{
			auto decl = FindUniformDeclaration(name);
			// ANT_CORE_ASSERT(decl, "Could not find uniform with name '{0}'", name);
			ANT_CORE_ASSERT(decl, "Could not find uniform with name 'x'");
			auto& buffer = GetUniformBufferTarget(decl);
			buffer.Write((byte*)&value, decl->GetSize(), decl->GetOffset());

			for (auto mi : m_MaterialInstances)
				mi->OnMaterialValueUpdated(decl);
		}

		void Set(const std::string& name, const Ref<Texture>& texture)
		{
			auto decl = FindResourceDeclaration(name);
			uint32_t slot = decl->GetRegister();
			if (m_Textures.size() <= slot)
				m_Textures.resize((size_t)slot + 1);
			m_Textures[slot] = texture;
		}

		void Set(const std::string& name, const Ref<Texture2D>& texture)
		{
			Set(name, (const Ref<Texture>&)texture);
		}

		void Set(const std::string& name, const Ref<TextureCube>& texture)
		{
			Set(name, (const Ref<Texture>&)texture);
		}
	public:
		static Ref<Material> Create(const Ref<Shader>& shader);
	private:
		void AllocateStorage();
		void OnShaderReloaded();
		void BindTextures() const;

		ShaderUniformDeclaration* FindUniformDeclaration(const std::string& name);
		ShaderResourceDeclaration* FindResourceDeclaration(const std::string& name);
		Buffer& GetUniformBufferTarget(const ShaderUniformDeclaration* uniformDeclaration);
	private:
		Ref<Shader> m_Shader;
		std::unordered_set<MaterialInstance*> m_MaterialInstances;

		Buffer m_VSUniformStorageBuffer;
		Buffer m_PSUniformStorageBuffer;
		std::vector<Ref<Texture>> m_Textures;

		uint32_t m_MaterialFlags;
	};

	class MaterialInstance
	{
		friend class Material;
	public:
		MaterialInstance(const Ref<Material>& material);
		virtual ~MaterialInstance();

		template <typename T>
		void Set(const std::string& name, const T& value)
		{
			auto decl = m_Material->FindUniformDeclaration(name);
			if (!decl)
				return;
			// ANT_CORE_ASSERT(decl, "Could not find uniform with name '{0}'", name);
			ANT_CORE_ASSERT(decl, "Could not find uniform with name 'x'");
			auto& buffer = GetUniformBufferTarget(decl);
			buffer.Write((byte*)&value, decl->GetSize(), decl->GetOffset());

			m_OverriddenValues.insert(name);
		}

		void Set(const std::string& name, const Ref<Texture>& texture)
		{
			auto decl = m_Material->FindResourceDeclaration(name);
			if (!decl)
				ANT_CORE_WARN("Cannot find material property: ", name);
			uint32_t slot = decl->GetRegister();
			if (m_Textures.size() <= slot)
				m_Textures.resize((size_t)slot + 1);
			m_Textures[slot] = texture;
		}

		void Set(const std::string& name, const Ref<Texture2D>& texture)
		{
			Set(name, (const Ref<Texture>&)texture);
		}

		void Set(const std::string& name, const Ref<TextureCube>& texture)
		{
			Set(name, (const Ref<Texture>&)texture);
		}

		void Bind() const;

		uint32_t GetFlags() const { return m_Material->m_MaterialFlags; }
		bool GetFlag(MaterialFlag flag) const { return (uint32_t)flag & m_Material->m_MaterialFlags; }
		void SetFlag(MaterialFlag flag, bool value = true);

		Ref<Shader >GetShader() { return m_Material->m_Shader; }
	public:
		static Ref<MaterialInstance> Create(const Ref<Material>& material);
	private:
		void AllocateStorage();
		void OnShaderReloaded();
		Buffer& GetUniformBufferTarget(const ShaderUniformDeclaration* uniformDeclaration);
		void OnMaterialValueUpdated(ShaderUniformDeclaration* decl);
	private:
		Ref<Material> m_Material;

		Buffer m_VSUniformStorageBuffer;
		Buffer m_PSUniformStorageBuffer;
		std::vector<Ref<Texture>> m_Textures;

		// TODO: This is a temporary; come up with a pproper system to track overrides
		std::unordered_set<std::string> m_OverriddenValues;
	};
}