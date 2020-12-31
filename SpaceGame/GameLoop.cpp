#include "GameLoop.h"
#include <thread>
#include <algorithm>

namespace utilities
{
	GameLoop::GameLoop(std::uint32_t fps, std::uint32_t tps, bool sleep) :
		sleep(sleep), 
		fps(fps), tps(tps),
		currentTick(0),
		lag(0),
		fpsSamples{0.f, 0.f, 1.f},
		tpsSamples{0.f, 0.f, 1.f},
		sampleInterval(1.f)
	{
		frameDelay = 1000000 / fps;
		tickDelay = 1000000 / tps;
		lastFrame = std::chrono::high_resolution_clock::now();
		lastTick = std::chrono::high_resolution_clock::now();
	}

	GameLoopData GameLoop::update()
	{
		const auto currentTime = std::chrono::high_resolution_clock::now();
		const auto timeSinceTick = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastTick).count();
		const auto timeSinceFrame = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastFrame).count();

		//If it is not yet time for a tick or frame, sleep till it is.
		if(timeSinceFrame < frameDelay && timeSinceTick < tickDelay)
		{
			const auto timeToTick = tickDelay - timeSinceTick;
			const auto timeToFrame = frameDelay - timeSinceFrame;

			const auto sleepTime = std::chrono::microseconds(std::min<std::int64_t>(timeToFrame, timeToTick));

			//If sleep is set to true, sleep the thread to save power and prevent overheating.
			if(sleep)
			{
				std::this_thread::sleep_for(sleepTime);
			}
			//Sleep set to false, keep going till it's time.
			else
			{
				const auto endTime = currentTime + sleepTime;
				bool end = false;
				while(!end)
				{
					if(std::chrono::high_resolution_clock::now() >= endTime)
					{
						end = true;
					}
				}
			}
		}

		//Calculate how much time has passed.
		const auto now = std::chrono::high_resolution_clock::now();
		const auto passedTime = std::chrono::duration_cast<std::chrono::microseconds>(now - currentTime).count();

		//Time since frame and tick.
		const auto deltaFrame = timeSinceFrame + passedTime;
		const auto deltaTick = timeSinceTick + passedTime;

		//Whether a frame or tick have happened.
		const bool tick = deltaTick >= tickDelay;
		const bool frame = deltaFrame >= frameDelay;

		//Sample the TPS and FPS.
		sample(tick, frame, deltaTick, deltaFrame);

		if(tick)
		{
			++currentTick;
			lastTick = now - lag;
		}

		if(frame)
		{
			lastFrame = now - lag;
		}

		//Calculate lag value to compensate next frame.
		lag = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - now);

		return getData(tick, frame, deltaTick, deltaFrame);
	}

	std::uint32_t GameLoop::getFpsTarget() const
	{
		return fps;
	}

	std::uint32_t GameLoop::getTpsTarget() const
	{
		return tps;
	}

	void GameLoop::setSampleInterval(std::float_t sampleInterval)
	{
		this->sampleInterval = sampleInterval;
	}

	void GameLoop::sample(bool tick, bool frame, std::int64_t tickTime, std::int64_t frameTime)
	{
		//Samples for FPS and TPS. Also resetting counter.
		if (frame)
		{
			//Increment time and count.
			fpsSamples[0] += (frameTime / 1000000.0f);
			++(fpsSamples[1]);

			//If a second has passed, 
			if (fpsSamples[0] > sampleInterval)
			{
				//Calculate frames per second.
				fpsSamples[2] = fpsSamples[1] / fpsSamples[0];
				fpsSamples[0] = 0.f;
				fpsSamples[1] = 0.f;
			}
		}
		if (tick)
		{
			//Increment time and count.
			tpsSamples[0] += (tickTime / 1000000.0f);
			++(tpsSamples[1]);

			//If a second has passed, 
			if (tpsSamples[0] > sampleInterval)
			{
				//Calculate frames per second.
				tpsSamples[2] = tpsSamples[1] / tpsSamples[0];
				tpsSamples[0] = 0.f;
				tpsSamples[1] = 0.f;
			}
		}
	}

	GameLoopData GameLoop::getData(bool tick, bool frame, std::int64_t deltaTick, std::int64_t deltaFrame)
	{
		return GameLoopData
		{
			fpsSamples[2],
			tpsSamples[2],
			static_cast<std::float_t>(deltaFrame / 1000000.0f),
			static_cast<std::float_t>(deltaTick / 1000000.0f),
			tick,
			frame,
			currentTick
		};
	}
}


