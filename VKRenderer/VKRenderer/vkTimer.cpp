#include "stdafx.h"
#include "vkTimer.h"

//Static variable declaration
vkTimer* vkTimer::m_instance = nullptr;

vkTimer * vkTimer::getInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new vkTimer();
	}

	return m_instance;
}


vkTimer::vkTimer()
{
	Reset();
}


vkTimer::~vkTimer()
{
}


unsigned getSystemTime()
{
	return (unsigned)timeGetTime();
}

void vkTimer::Reset()
{
	m_startTime = getSystemTime();
}

unsigned int vkTimer::FrameStart(bool is_Reset)
{
	unsigned currentTime = getSystemTime();
	unsigned elapsedTime = currentTime - m_startTime;
	
	if (is_Reset)
		m_startTime = currentTime;

	return elapsedTime;
}

