#include <timer.h>

namespace util
{
	//----------------------------------------------------------------------------------------------------------------------------------------------------------
	// Timer:public
	//----------------------------------------------------------------------------------------------------------------------------------------------------------

	Timer::Timer()
	{
		restart();
	}

    static inline std::conditional<std::chrono::high_resolution_clock::is_steady,
                    std::chrono::high_resolution_clock,
                    std::chrono::steady_clock>::type::time_point getNow()
    {
        return std::conditional<std::chrono::high_resolution_clock::is_steady,
                std::chrono::high_resolution_clock,
                std::chrono::steady_clock>::type::now();
    }

	void Timer::restart()
	{
        _start = getNow();
	}

	double Timer::elapsedSec()
	{
		return elapsedNanoSec() * 1e-9;
	}

	double Timer::elapsedMilliSec()
	{
		return elapsedNanoSec() * 1e-6;
	}

	double Timer::elapsedMicroSec()
	{
		return elapsedNanoSec() * 1e-3;
	}

	double Timer::elapsedNanoSec()
	{
        return static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(getNow() - _start).count());
	}
}
