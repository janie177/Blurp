#pragma once
#include <cmath>
#include <chrono>
#include <cinttypes>

namespace utilities
{
	/*
	 * Data structure filled with information about the game loop.
	 */
	struct GameLoopData
	{
		//Actual FPS.
		std::double_t fps;

		//Actual TPS.
		std::double_t tps;

		//The time in seconds since the last frame.
		std::float_t deltaFrame;

		//The time in seconds since the last tick.
		std::float_t deltaTick;

		//Whether a tick should happen or not.
		bool tick;

		//Whether a frame should happen or not.
		bool frame;

		//The current tick count. This is increased each tick.
		std::uint64_t currentTick;

	};

	/*
	 * GameLoop class which handles the FPS and TPS control.
	 * It sleeps the main thread until a tick or frame has to be processed.
	 * This can be enabled or disabled. On mobile devices, sleeping will result in less battery usage and lower temperatures.
	 * On a PC, it will likely slow down the FPS because it is not always awakened in time.
	 */
	class GameLoop
	{
	public:
		GameLoop(std::uint32_t fps, std::uint32_t tps, bool sleep = false);

	public:
		/*
		 * This function is called in the game loop.
		 * It returns information about the TPS, FPS and whether a frame or tick should happen.
		 * 
		 * This function sleeps the main thread until a frame or tick should happen.
		 */
		GameLoopData update();

		/*
		 * Get the targeted frames per second.
		 */
		std::uint32_t getFpsTarget() const;

		/*
		 * Get the targeted ticks per second.
		 */
		std::uint32_t getTpsTarget() const;

		/*
		 * Set the sample interval at which to measure TPS and FPS.
		 * This is given in seconds.
		 */
		void setSampleInterval(std::float_t sampleInterval);

	private:
		/*
		 * Take FPS and TPS samples.
		 */
		void sample(bool tick, bool frame, std::int64_t tickTime, std::int64_t frameTime);

		/*
		 * Make a neat package of data to return.
		 */
		GameLoopData getData(bool tick, bool frame, std::int64_t deltaTick, std::int64_t deltaFrame);

	private:
		//Whether or not to sleep.
		//This is recommended for mobile devices but not PC's.
		//Sleep is never guaranteed to awaken at the right time.
		bool sleep;

		//The TPS and FPS targets.
		std::uint32_t fps;
		std::uint32_t tps;

		//The TPS and FPS targets converted into seconds.
		std::int64_t tickDelay;
		std::int64_t frameDelay;

		//The current tick number.
		std::uint64_t currentTick;

		//Timestamps of the last update, last frame and last tick.
		std::chrono::time_point<std::chrono::high_resolution_clock> lastFrame;
		std::chrono::time_point<std::chrono::high_resolution_clock> lastTick;
		std::chrono::duration<std::int64_t, std::micro> lag;


		/*
		 * Samples to measure the TPS and FPS.
		 * The first value is the current time.
		 * The second value is the total amount of frames/ticks in that time.
		 * The third value is used to store the last sample.
		 *
		 * Every time a certain time has passed, the sample is updated by dividing total amount of frames/ticks through the time that has passed.
		 * The result is stored as the third value.
		 */
		float fpsSamples[3];
		float tpsSamples[3];

		//Interval to take samples at. Defaults to 1 second.
		std::float_t sampleInterval;
	};
}
