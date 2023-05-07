#pragma once


namespace Ant {

	class ScriptsEngine 
	{
	public:
		static void Init();
		static void Shutdown();
	private:
		static void InitMono();
		static void ShutdownMono();
	};
}