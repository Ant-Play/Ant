#pragma once


namespace Ant{

	//用于计算每一帧渲染的时间，较少不同刷新率机器上对于物体运动的影响
	class Timestep
	{
	public:
		Timestep(float time = 0.0f)
			: m_Time(time)
		{
		}
		float GetSeconds() const { return m_Time; }
		float GetMilliseconds() const { return m_Time * 1000.0f; }

		operator float() const { return m_Time; }


	private:
		float m_Time = 0.0f;
	};
}
