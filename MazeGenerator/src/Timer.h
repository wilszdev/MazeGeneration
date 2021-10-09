#pragma once

#include <inttypes.h>
#include <chrono>

class Timer
{
private:
	std::chrono::steady_clock::time_point m_tp;
public:
	Timer()
	{
		Mark();
	}

	/// <summary>
	/// set internal timepoint
	/// </summary>
	void Mark() noexcept
	{
		m_tp = std::chrono::high_resolution_clock::now();
	}

	/// <summary>
	/// time since last Mark() in seconds
	/// </summary>
	float Peek() const noexcept
	{
		return (std::chrono::high_resolution_clock::now() - m_tp).count() / 1000000000.0f;
	}
};