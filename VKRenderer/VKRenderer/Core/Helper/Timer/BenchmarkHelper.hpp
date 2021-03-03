#include <chrono>

class Timer
{
public:
	Timer() : m_sessionName("Testing")
	{
		m_StartPoint = std::chrono::high_resolution_clock::now();
	}

	Timer(std::string_view SessionName)
		:m_sessionName(SessionName)
	{
		m_StartPoint = std::chrono::high_resolution_clock::now();
	}

	~Timer()
	{
		Stop();
	}

	void Stop()
	{
		auto endPoint = std::chrono::high_resolution_clock::now();

		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartPoint).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endPoint).time_since_epoch().count();
		
		auto duration = end - start;
		double ms = duration * 0.001;

		std::cout << "Duration of this scope " << m_sessionName <<" : "<< ms << "ms (" << duration << "us)\n";
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_StartPoint;
	std::string_view m_sessionName;
};