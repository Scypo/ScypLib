#pragma once
#include <chrono>

using namespace std::chrono;
namespace sl
{
	class FrameTimer
	{
	public:
		FrameTimer()
		{
			last = steady_clock::now();
		}
		float Mark()
		{
			const auto old = last;
			last = steady_clock::now();
			const duration<float> frameTime = last - old;
			return frameTime.count();
		}
		float GetTimeSeconds()
		{
			auto now = std::chrono::system_clock::now();
			auto duration = now.time_since_epoch();
			float seconds = std::chrono::duration<float>(duration).count();
			return seconds;
		}
	private:
		std::chrono::steady_clock::time_point last;
	};
}