#pragma once

class Timer
{
public:
	Timer();

	
	double GetDeltaTime() const;

	
	double GetTotalTime() const;
	void Start();
	void Stop();
	void Reset();
	void Tick();
protected:
	double m_SecondsPerCount;

	double  m_DeltaTimeInSeconds;
	__int64 m_CurrTime;
	__int64 m_PrevTime;

	// Time since the last Reset() was called e.g since the application started
	double  m_TotalTime;

	// Time when Reset() was called
	__int64 m_BaseTime; 

	// Time when Stop() was called
	__int64 m_StopTime;

	// Time since the last Stop() was called
	__int64 m_PausedTime; 

	bool m_Stopped;
};