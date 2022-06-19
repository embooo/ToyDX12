#include "pch.h"
#include "Timer.h"

#include <Windows.h>

Timer::Timer()
	: m_DeltaTimeInSeconds(-1.0), m_Stopped(false), m_BaseTime(0), m_PausedTime(0), m_PrevTime(0), m_CurrTime(0), m_SecondsPerCount(0.0)
{
	__int64 perfCounterFrequency;
	QueryPerformanceFrequency((LARGE_INTEGER*)&perfCounterFrequency);

	m_SecondsPerCount = 1.0 / (double)perfCounterFrequency;
}

double Timer::GetDeltaTime() const
{
	return m_DeltaTimeInSeconds;
}

double Timer::GetTotalTime() const
{
	if (m_Stopped)
	{
		// If the timer is stopped, don't count the paused duration when returning the total time 
		return ((m_StopTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount;
	}

	// Return the total time since last Reset() without counting paused time
	return ((m_CurrTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount;
}

void Timer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	if (m_Stopped)
	{
		// Accumulate paused time
		m_PausedTime += (startTime - m_StopTime);

		// Restarting timer 
		m_PrevTime = startTime;

		m_StopTime = 0;
		m_Stopped = false;
	}
}

void Timer::Stop()
{
	if (!m_Stopped)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		m_PausedTime = currTime;
		m_Stopped = true;
	}
}

void Timer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	m_PrevTime = currTime;
}

void Timer::Tick()
{
	if (m_Stopped)
	{
		m_DeltaTimeInSeconds = 0.0;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_CurrTime = currTime;

	m_DeltaTimeInSeconds = (m_CurrTime - m_PrevTime) * m_SecondsPerCount;

	m_PrevTime = m_CurrTime;

}
