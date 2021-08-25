#ifndef TIMER_H
#define TIMER_H

#include<chrono>

class Timer
{
private:
	bool m_isRunning;
	std::chrono::time_point<std::chrono::steady_clock> m_startTime;
	std::chrono::time_point<std::chrono::steady_clock> m_stopTime;
	int m_timeAdd;
	bool m_remember;
	double m_resumeValue;

public:
	Timer(const bool remember = false)
	{
		m_startTime = std::chrono::high_resolution_clock::now();
		m_stopTime = std::chrono::high_resolution_clock::now();
		m_isRunning = false;
		m_timeAdd = 0;
		m_remember = remember;
		m_resumeValue = 0;
	}
	double timeElapsed() const
	{
		if (m_isRunning)
		{
			auto elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - m_startTime);
			return elapsed.count() + m_timeAdd;
		}
		else
		{
			auto elapsed = std::chrono::duration<double>(m_stopTime - m_startTime);
			return elapsed.count() + m_timeAdd;
		}
	}
	bool start()
	{
		bool canStart = true;
		if (m_isRunning)
			canStart = false;
		else
		{
			if (m_remember)
			{
				addTime((int)m_resumeValue);
			}
			m_startTime = std::chrono::high_resolution_clock::now();
			m_isRunning = true;
		}

		return canStart;
	}
	bool stop()
	{
		bool canStop = true;
		if (m_isRunning)
		{
			if (m_remember)
			{
				auto time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - m_startTime);
				m_resumeValue = (double)time.count();
			}
			m_stopTime = std::chrono::high_resolution_clock::now();
			m_isRunning = false;
		}
		else
			canStop = false;

		return canStop;
	}
	void restart()
	{
		m_isRunning = true;
		m_startTime = std::chrono::high_resolution_clock::now();
	}

	void addTime(const int timeAdd)
	{
		m_timeAdd += timeAdd;
	}
	bool isRunning() const
	{
		return m_isRunning;
	}

};

#endif // !TIMER_H