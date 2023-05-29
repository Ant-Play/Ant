#pragma once


namespace Ant{

	//用于计算每一帧渲染的时间，较少不同刷新率机器上对于物体运动的影响
	class Timestep
	{
	public:
		Timestep() {}
		Timestep(float time);

		inline float GetSeconds() const { return m_Time; }
		inline float GetMilliseconds() const { return m_Time * 1000.0f; }

		operator float() { return m_Time; }
	private:
		float m_Time = 0.0f;
	};
}
