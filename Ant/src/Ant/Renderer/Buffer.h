#pragma once

#include <string>
#include <vector>

namespace Ant{

	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:			return 4;
			case ShaderDataType::Float2:		return 4 * 2;
			case ShaderDataType::Float3:		return 4 * 3;
			case ShaderDataType::Float4:		return 4 * 4;
			case ShaderDataType::Mat3:			return 4 * 3 * 3;
			case ShaderDataType::Mat4:			return 4 * 4 * 4;
			case ShaderDataType::Int:			return 4;
			case ShaderDataType::Int2:			return 4 * 2;
			case ShaderDataType::Int3:			return 4 * 3;
			case ShaderDataType::Int4:			return 4 * 4;
			case ShaderDataType::Bool:			return 1;
		}

		ANT_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	// 缓冲区元素结构体
	struct BufferElements
	{
		std::string Name;
		ShaderDataType Type;
		uint32_t Size;
		uint32_t Offset;
		bool Normalized;
		
		BufferElements() = default;

		BufferElements(ShaderDataType type, const std::string& name, bool normalized = false)
			: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
		{

		}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
				case ShaderDataType::Float:			return 1;
				case ShaderDataType::Float2:		return 2;
				case ShaderDataType::Float3:		return 3;
				case ShaderDataType::Float4:		return 4;
				case ShaderDataType::Mat3:			return 3 * 3;
				case ShaderDataType::Mat4:			return 4 * 4;
				case ShaderDataType::Int:			return 1;
				case ShaderDataType::Int2:			return 2;
				case ShaderDataType::Int3:			return 3;
				case ShaderDataType::Int4:			return 4;
				case ShaderDataType::Bool:			return 1;
			}

			ANT_CORE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}
	};

	class BufferLayout
	{
	public:

		BufferLayout() = default;

		BufferLayout(const std::initializer_list<BufferElements>& elements) 
			: m_Elements(elements)
		{
			CalculateOffsetAndStride();
		}

		inline const uint32_t GetStride() const { return m_Stride; }
		inline const std::vector<BufferElements>& GetElements() const { return m_Elements; }

		// 为了使用迭代器遍历元素列表而添加的方法
		std::vector<BufferElements>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElements>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElements>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElements>::const_iterator end() const { return m_Elements.end(); }

	private:
		// 计算元素的偏移量和Layout的步长
		void CalculateOffsetAndStride()
		{
			uint32_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}
	private:
		std::vector<BufferElements> m_Elements;
		uint32_t m_Stride = 0;
	};

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;

		// 绑定/解绑顶点缓冲区
		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		// 设置顶点数据
		virtual void SetData(const void* data, uint32_t size) = 0;

		// 获取/设置顶点缓冲区布局
		virtual const BufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;

		// 创建并初始化顶点缓冲区
		static Ref<VertexBuffer> Create(uint32_t size);
		static Ref<VertexBuffer> Create(float* vertices, uint32_t size);
	};

	// Currently Ant only supports 32-bit index buffers
	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;

		// 绑定/解绑索引缓冲区
		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		// 获取索引数量
		virtual uint32_t GetCount() const = 0;

		// 创建索引缓冲区
		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t size);
	};
}
