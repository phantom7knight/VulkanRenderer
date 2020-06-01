#pragma once
class vkTimer
{
public:
	vkTimer();
	~vkTimer();

	unsigned int FrameStart(bool is_Reset);
	static vkTimer* getInstance();

	double					m_dIdealDeltaTime;


private:
	static vkTimer*			m_instance;
	unsigned int			m_startTime;

	void	Reset();

};

