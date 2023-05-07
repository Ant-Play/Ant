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

	//�洢����������������ƣ���ʼʱ�䣬����ʱ�䣬�߳�ID����Ϣ
	struct ProfileResult
	{
		std::string Name;

		FloatingPointMiscroseconds Start;
		std::chrono::microseconds ElapsedTime;
		std::thread::id ThreadID;
	};

	// �洢�����Ự
	struct InstrumentationSession
	{
		std::string Name;
	};

	// ���� Instrumentor �࣬���ڼ�¼ profiling ����
	class Instrumentor 
	{
	public:
		// ���ÿ������ƶ����캯��
		Instrumentor(const Instrumentor&) = delete;
		Instrumentor(Instrumentor&&) = delete;

		// ��ʼһ�� profiling �Ự������Ự���ƺ�����ļ�·��
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

		// ������ǰ profiling �Ự
		void EndSession()
		{
			std::lock_guard lock(m_Mutex);
			InternalEndSession();
		}

		// ��¼һ�� profiling ���
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

		// ��ȡ Instrumentor ʵ��
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

		// ���JSON��ʽͷ����Ϣ�������
		void WriteHeader()
		{
			m_OutputStream << "{\"otherData\": {},\"traceEvents\":[{}";
			m_OutputStream.flush();
		}

		// ���JSON��ʽβ����Ϣ�������
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

	// ��ʱ����
	class InstrumentationTimer
	{
	public:
		// ���캯������¼��ʱ����ʼ��ʱ���
		InstrumentationTimer(const char* name)
			: m_Name(name), m_Stopped(false)
		{
			m_StartTimepoint = std::chrono::steady_clock::now();
		}

		// �����������ڶ�������ʱ���ã�ֹͣ��ʱ����д������
		~InstrumentationTimer()
		{
			if (!m_Stopped)
				Stop();
		}

		// ֹͣ��ʱ������¼����ʱ��㣬��������д��
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

		// ���ڱ����޸ĺ���ַ������
		template <size_t N>
		struct ChangeResult
		{
			char Data[N];
		};

		// �Ƴ��ַ�����ָ�����Ӵ�
		template<size_t N, size_t K>
		constexpr auto CleanupOutputString(const char(&expr)[N], const char(&remove)[K])
		{
			// ���ַ�����Ѱ����ָ���Ӵ���ͬ�Ĳ���
			ChangeResult<N> result = {};

			size_t srcIndex = 0;
			size_t dstIndex = 0;
			while (srcIndex < N)
			{
				size_t matchIndex = 0;
				while (matchIndex < K - 1 && srcIndex + matchIndex < N - 1 && expr[srcIndex + matchIndex] == remove[matchIndex])
					matchIndex++;

				// ����ҵ��ˣ���Դ�ַ�������������ͬ�Ĳ���
				if (matchIndex == K - 1)
					srcIndex += matchIndex;

				// ��Դ�ַ������ַ����浽�޸ĺ�Ľ���ַ�����
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
		#define ANT_FUNC_SIG __PRETTY_FUNCTION__	// GCC��Clang �ȱ������ĺ���ǩ����
	#elif defined(__DMC__) && (__DMC__ >= 0x810)
		#define ANT_FUNC_SIG __PRETTY_FUNCTION__	// Digital Mars C++ �������ĺ���ǩ����
	#elif (defined(__FUNCSIG__) || (_MSC_VER))
		#define ANT_FUNC_SIG __FUNCSIG__			// Visual Studio �������ĺ���ǩ����
	#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
		#define ANT_FUNC_SIG __FUNCTION__			// Intel C++��IBM XL C++ �������ĺ���ǩ����
	#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
		#define ANT_FUNC_SIG __FUNC__				// Borland C++ �������ĺ���ǩ����
	#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
		#define ANT_FUNC_SIG __func__				// C99 ��׼����ĺ���ǩ����
	#elif defined(__cplusplus) && (__cplusplus >= 201103)
		#define ANT_FUNC_SIG __func__				// C++11 ��׼����ĺ���ǩ����
	#else
		#define ANT_FUNC_SIG "ANT_FUNC_SIG unknown!"
	#endif

	#define ANT_PROFILE_BEGIN_SESSION(name, filepath) ::Ant::Instrumentor::Get().BeginSession(name, filepath)		// ��ʼ���ܷ����Ự
	#define ANT_PROFILE_END_SESSION() ::Ant::Instrumentor::Get().EndSession()		// �������ܷ����Ự
	#define ANT_PROFILE_SCOPE_LINE2(name, line) constexpr auto fixedName##line = ::Ant::InstrumentorUtils::CleanupOutputString(name, "__cdecl ");\
											   ::Ant::InstrumentationTimer timer##line(fixedName##line.Data)	 // ������ʱ���������ڲ��������ִ��ʱ��
	#define ANT_PROFILE_SCOPE_LINE(name, line) ANT_PROFILE_SCOPE_LINE2(name, line)
	#define ANT_PROFILE_SCOPE(name) ANT_PROFILE_SCOPE_LINE(name, __LINE__)
	#define ANT_PROFILE_FUNCTION() ANT_PROFILE_SCOPE(ANT_FUNC_SIG)
#else
	#define ANT_PROFILE_BEGIN_SESSION(name, filepath)
	#define ANT_PROFILE_END_SESSION()
	#define ANT_PROFILE_SCOPE(name)
	#define ANT_PROFILE_FUNCTION()
#endif
