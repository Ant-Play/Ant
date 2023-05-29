#pragma once


namespace Ant{

	//���ڼ���ÿһ֡��Ⱦ��ʱ�䣬���ٲ�ͬˢ���ʻ����϶��������˶���Ӱ��
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
