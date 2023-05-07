#pragma once

#include "Ant/Core/Log.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <string>
#include <thread>
#include <mutex>
#include <sstream>

namespace Ant {

	using FloatingPointMiscroseconds = std::chrono::duration<double, std::micro>;

	//存储分析结果，包含名称，开始时间，运行时间，线程ID等信息
	struct ProfileResult
	{
		std::string Name;

		FloatingPointMiscroseconds Start;
		std::chrono::microseconds ElapsedTime;
		std::thread::id ThreadID;
	};

	// 存储分析会话
	struct InstrumentationSession
	{
		std::string Name;
	};

	// 定义 Instrumentor 类，用于记录 profiling 数据
	class Instrumentor 
	{
	public:
		// 禁用拷贝和移动构造函数
		Instrumentor(const Instrumentor&) = delete;
		Instrumentor(Instrumentor&&) = delete;

		// 开始一个 profiling 会话，传入会话名称和输出文件路径
		void BeginSession(const std::string& name, const std::string& filepath = "results.json")
		{
			std::lock_guard lock(m_Mutex);
			if (m_CurrentSession)
			{
				// If there is already a current session, then close it before beginning new one.
				// Subsequent profiling output meant for the original session will end up in the
				// newly opened session instead.  That's better than having badly formatted
				// profiling output.
				if (Log::GetCoreLogger())
				{
					ANT_CORE_ERROR("Instrumentor::BeginSession('{0}') when session '{1}' already open.", name, m_CurrentSession->Name);
				}
				InternalEndSession();
			}
			m_OutputStream.open(filepath);

			if (m_OutputStream.is_open())
			{
				m_CurrentSession = new InstrumentationSession({ name });
				WriteHeader();
			}
			else
			{
				if (Log::GetCoreLogger())
				{
					ANT_CORE_ERROR("Instrumentor could not open results file '{0}'.", filepath);
				}
			}
		}

		// 结束当前 profiling 会话
		void EndSession()
		{
			std::lock_guard lock(m_Mutex);
			InternalEndSession();
		}

		// 记录一个 profiling 结果
		void WriteProfile(const ProfileResult& result)
		{
			std::stringstream json;

			json << std::setprecision(3) << std::fixed;
			json << ",{";
			json << "\"cat\":\"function\",";
			json << "\"dur\":" << (result.ElapsedTime.count()) << ',';
			json << "\"name\":\"" << result.Name << "\",";
			json << "\"ph\":\"X\",";
			json << "\"pid\":0,";
			json << "\"tid\":" << result.ThreadID << ",";
			json << "\"ts\":" << result.Start.count();
			json << "}";

			std::lock_guard lock(m_Mutex);
			if (m_CurrentSession)
			{
				m_OutputStream << json.str();
				m_OutputStream.flush();
			}
		}

		// 获取 Instrumentor 实例
		static Instrumentor& Get()
		{
			static Instrumentor instance;
			return instance;
		}
	private:
		Instrumentor()
			: m_CurrentSession(nullptr)
		{

		}

		~Instrumentor()
		{
			EndSession();
		}

		// 输出JSON格式头部信息到输出流
		void WriteHeader()
		{
			m_OutputStream << "{\"otherData\": {},\"traceEvents\":[{}";
			m_OutputStream.flush();
		}

		// 输出JSON格式尾部信息到输出流
		void WriteFooter()
		{
			m_OutputStream << "]}";
			m_OutputStream.flush();
		}

		// Note: you must already own lock on m_Mutex before
		// calling InternalEndSession()
		void InternalEndSession()
		{
			if (m_CurrentSession)
			{
				WriteFooter();
				m_OutputStream.close();
				delete m_CurrentSession;
				m_CurrentSession = nullptr;
			}
		}
	private:
		std::mutex m_Mutex;
		InstrumentationSession* m_CurrentSession;
		std::ofstream m_OutputStream;
	};

	// 计时器类
	class InstrumentationTimer
	{
	public:
		// 构造函数，记录计时器开始的时间点
		InstrumentationTimer(const char* name)
			: m_Name(name), m_Stopped(false)
		{
			m_StartTimepoint = std::chrono::steady_clock::now();
		}

		// 析构函数，在对象销毁时调用，停止计时器并写入数据
		~InstrumentationTimer()
		{
			if (!m_Stopped)
				Stop();
		}

		// 停止计时器，记录结束时间点，并将数据写入
		void Stop()
		{
			auto endTimepoint = std::chrono::steady_clock::now();
			auto highResStart = FloatingPointMiscroseconds{ m_StartTimepoint.time_since_epoch() };
			auto elapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() - std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch();

			Instrumentor::Get().WriteProfile({ m_Name, highResStart,elapsedTime,std::this_thread::get_id() });

			m_Stopped = true;
		}
	private:
		const char* m_Name;
		std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
		bool m_Stopped;
	};

	namespace InstrumentorUtils {

		// 用于保存修改后的字符串结果
		template <size_t N>
		struct ChangeResult
		{
			char Data[N];
		};

		// 移除字符串中指定的子串
		template<size_t N, size_t K>
		constexpr auto CleanupOutputString(const char(&expr)[N], const char(&remove)[K])
		{
			// 在字符串中寻找与指定子串相同的部分
			ChangeResult<N> result = {};

			size_t srcIndex = 0;
			size_t dstIndex = 0;
			while (srcIndex < N)
			{
				size_t matchIndex = 0;
				while (matchIndex < K - 1 && srcIndex + matchIndex < N - 1 && expr[srcIndex + matchIndex] == remove[matchIndex])
					matchIndex++;

				// 如果找到了，则将源字符串索引跳过相同的部分
				if (matchIndex == K - 1)
					srcIndex += matchIndex;

				// 将源字符串的字符保存到修改后的结果字符串中
				result.Data[dstIndex++] = expr[srcIndex] == '"' ? '\'' : expr[srcIndex];
				srcIndex++;
			}
			return result;
		}
	}
}

#define ANT_PROFILE 1
#if ANT_PROFILE
	// Resolve which function signature macro will be used. Note that this only
	// is resolved when the (pre)compiler starts, so the syntax highlighting
	// could mark the wrong one in your editor!
	#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
		#define ANT_FUNC_SIG __PRETTY_FUNCTION__	// GCC、Clang 等编译器的函数签名宏
	#elif defined(__DMC__) && (__DMC__ >= 0x810)
		#define ANT_FUNC_SIG __PRETTY_FUNCTION__	// Digital Mars C++ 编译器的函数签名宏
	#elif (defined(__FUNCSIG__) || (_MSC_VER))
		#define ANT_FUNC_SIG __FUNCSIG__			// Visual Studio 编译器的函数签名宏
	#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
		#define ANT_FUNC_SIG __FUNCTION__			// Intel C++、IBM XL C++ 编译器的函数签名宏
	#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
		#define ANT_FUNC_SIG __FUNC__				// Borland C++ 编译器的函数签名宏
	#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
		#define ANT_FUNC_SIG __func__				// C99 标准定义的函数签名宏
	#elif defined(__cplusplus) && (__cplusplus >= 201103)
		#define ANT_FUNC_SIG __func__				// C++11 标准定义的函数签名宏
	#else
		#define ANT_FUNC_SIG "ANT_FUNC_SIG unknown!"
	#endif

	#define ANT_PROFILE_BEGIN_SESSION(name, filepath) ::Ant::Instrumentor::Get().BeginSession(name, filepath)		// 开始性能分析会话
	#define ANT_PROFILE_END_SESSION() ::Ant::Instrumentor::Get().EndSession()		// 结束性能分析会话
	#define ANT_PROFILE_SCOPE_LINE2(name, line) constexpr auto fixedName##line = ::Ant::InstrumentorUtils::CleanupOutputString(name, "__cdecl ");\
											   ::Ant::InstrumentationTimer timer##line(fixedName##line.Data)	 // 创建定时器对象，用于测量代码块执行时间
	#define ANT_PROFILE_SCOPE_LINE(name, line) ANT_PROFILE_SCOPE_LINE2(name, line)
	#define ANT_PROFILE_SCOPE(name) ANT_PROFILE_SCOPE_LINE(name, __LINE__)
	#define ANT_PROFILE_FUNCTION() ANT_PROFILE_SCOPE(ANT_FUNC_SIG)
#else
	#define ANT_PROFILE_BEGIN_SESSION(name, filepath)
	#define ANT_PROFILE_END_SESSION()
	#define ANT_PROFILE_SCOPE(name)
	#define ANT_PROFILE_FUNCTION()
#endif
