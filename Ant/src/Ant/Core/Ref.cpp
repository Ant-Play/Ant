﻿#include "antpch.h"

#include <unordered_set>

namespace Ant{

	static std::unordered_set<void*> s_LiveReferences;
	static std::mutex s_LiveReferenceMutex;

	namespace RefUtils{

		void AddToLiveReferences(void* instance)
		{
			std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
			ANT_CORE_ASSERT(instance);
			s_LiveReferences.insert(instance);
		}

		void RemoveFromLiveReferences(void* instance)
		{
			std::scoped_lock<std::mutex> lock(s_LiveReferenceMutex);
			ANT_CORE_ASSERT(instance);
			ANT_CORE_ASSERT(s_LiveReferences.find(instance) != s_LiveReferences.end());
			s_LiveReferences.erase(instance);
		}

		bool IsLive(void* instance)
		{
			ANT_CORE_ASSERT(instance);
			return s_LiveReferences.find(instance) != s_LiveReferences.end();
		}
	}
}