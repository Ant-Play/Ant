﻿#include "antpch.h"
#include "Audio.h"

#include "Ant/Debug/Profiler.h"

#include <chrono>
using namespace std::chrono_literals;

namespace Ant::Audio {

	static std::queue<AudioFunctionCallback*> s_AudioThreadJobsLocal;

	//==============================================================================
	bool AudioThread::Start()
	{
		if (s_ThreadActive)
			return false;

		s_ThreadActive = true;
		s_AudioThread = anew std::thread([]
			{
				ANT_PROFILE_THREAD("AudioThread");
#if defined(ANT_PLATFORM_WINDOWS)

				HRESULT r;
				r = SetThreadDescription(
					GetCurrentThread(),
					L"Ant Audio Thread"
				);
#endif

				ANT_CORE_INFO_TAG("Audio", "Spinning up Audio Thread.");
				while (s_ThreadActive)
				{
					OnUpdate();
				}
				ANT_CORE_INFO_TAG("Audio", "Audio Thread stopped.");
			});

		s_AudioThreadID = s_AudioThread->get_id();
		s_AudioThread->detach();

		return true;
	}

	bool AudioThread::Stop()
	{
		if (!s_ThreadActive)
			return false;

		s_ThreadActive = false;
		return true;
	}

	bool AudioThread::IsRunning()
	{
		return s_ThreadActive;
	}

	bool AudioThread::IsAudioThread()
	{
		return std::this_thread::get_id() == s_AudioThreadID;
	}

	std::thread::id AudioThread::GetThreadID()
	{
		return s_AudioThreadID;
	}

	void AudioThread::AddTask(AudioFunctionCallback*&& funcCb)
	{
		ANT_PROFILE_FUNC();

		std::scoped_lock lock(s_AudioThreadJobsLock);
		s_AudioThreadJobs.emplace(std::move(funcCb));
	}

	void AudioThread::OnUpdate()
	{
		ANT_PROFILE_FUNC();

		s_Timer.Reset();

		//---------------------------
		//--- Handle AudioThread Jobs

		{
			ANT_PROFILE_FUNC("AudioThread::OnUpdate - Execution");

			auto& jobs = s_AudioThreadJobsLocal;
			{
				std::scoped_lock lock(s_AudioThreadJobsLock);
				s_AudioThreadJobsLocal = s_AudioThreadJobs;
				s_AudioThreadJobs = std::queue<AudioFunctionCallback*>();
			}
			if (!jobs.empty())
			{
				// Should not run while jobs are not complete, instead re-add them to run on the next update loop
				for (int i = (int)jobs.size() - 1; i >= 0; i--)
				{
					auto job = jobs.front();
					//ANT_CONSOLE_LOG_INFO("AudioThread. Executing: {}", job->GetID());
					job->Execute();

					// TODO: check if job ran successfully, if not, notify and/or add back to the queue
					jobs.pop();
					delete job; // TODO: better allocation strategy
				}
			}
		}

		ANT_CORE_ASSERT(onUpdateCallback.IsBound(), "Update Function is not bound");
		onUpdateCallback.Invoke(s_TimeStep);

		// Sleeping to let at least a single audio block to render before next OnUpdate
		// This should ensure that we don't pass too many updates to render thread
		// before it can handle previous ones.
		const uint32_t sleepDurationMS = glm::min((uint32_t)s_Timer.ElapsedMillis() - PCM_FRAME_CHUNK_MS, PCM_FRAME_CHUNK_MS);
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepDurationMS));

		s_TimeStep = s_Timer.Elapsed();
		s_LastFrameTime = s_TimeStep.GetMilliseconds();
	}

	//==============================================================================
	AudioThreadFence::AudioThreadFence()
	{
		ANT_CORE_ASSERT(!IsAudioThread());
	}

	AudioThreadFence::~AudioThreadFence()
	{
		ANT_CORE_ASSERT(!IsAudioThread());

		Wait();
		delete m_Counter;
	}

	void AudioThreadFence::Begin()
	{
		ANT_CORE_ASSERT(!IsAudioThread());

		if (!AudioThread::IsRunning())
			return;

		Wait();

		m_Counter->IncRefCount();

		AudioThread::AddTask(new AudioFunctionCallback(
			[counter = m_Counter]
			{
				counter->DecRefCount();
			}
			, "AudioThreadFence"));
	}

	void AudioThreadFence::BeginAndWait()
	{
		Begin();
		Wait();
	}

	void AudioThreadFence::Wait() const
	{
		ANT_CORE_ASSERT(!IsAudioThread());

		while (!IsReady())
		{
		}
	}
}