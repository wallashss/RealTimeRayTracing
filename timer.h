#pragma once
#include <chrono>

namespace util
{
	class Timer
	{
	public:
		Timer();

		void restart();

		double elapsedSec();

		double elapsedMilliSec();

		double elapsedMicroSec();

		double elapsedNanoSec();

	private:
        std::conditional<std::chrono::high_resolution_clock::is_steady,
                                                  std::chrono::high_resolution_clock,
                                                  std::chrono::steady_clock>::type::time_point _start;
	};
}
