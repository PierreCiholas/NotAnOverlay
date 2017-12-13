#pragma once
#include <Windows.h>
#include <map>
#include <deque>
#include <iostream>

#define DEFAULT_TARGET_FPS 60
#define MAX_FRAMES_BETWEEN_NON_CRITICAL_OPS 6

#define MAX_TICK_ID 10 // Define how many different "ticks" the performance manager can manage (size of the int array)
#define ID_TICK_CPS 0
#define ID_TICK_FPS 1

using namespace std;

// The class FpsManager allows to see the overlay performance in FPS and set it
// (its data can be used to decide to skip rendering)
class PerfManager {
public:
	PerfManager::PerfManager();
	PerfManager::~PerfManager();

	int PerfManager::GetTicksPerSecond(int tickID = 0);
	int PerfManager::AddTick(int tickID = 0);
	// bool drawNextFrame(); // TODO
	void PerfManager::StartBenchmark(unsigned int benchmarkID = 0);
	int PerfManager::StopBenchmark(unsigned int benchmarkID = 0);
	void PerfManager::BalanceFPS();
	bool PerfManager::ShouldExecuteNonCritOps();
	bool PerfManager::ToggleLimitFps();

private:
	SYSTEMTIME m_sysTime;
	map<unsigned int, SYSTEMTIME> m_benchmarks;
	deque<int> m_execTimesLastSecond;
	int m_targetFps = DEFAULT_TARGET_FPS, m_maxFrameSpaceNonCrits = MAX_FRAMES_BETWEEN_NON_CRITICAL_OPS, m_currentFps = 0, m_framesThisSecond = 0, m_thisSecond = -1, m_msToWaitBetweenFrames = 15, m_execTimeLastFrame = 0;
	unsigned int m_frameNbr = 1, m_execNonCritOpsEachXFrames = 1;
	int m_tickCounting[MAX_TICK_ID] = { 0 }; // Used to increment every tick
	int m_tickCounts[MAX_TICK_ID] = { 0 }; // Updated every second (these are the values to be read for further display)
	bool m_limitFps = true;
};

PerfManager::PerfManager() { // Performance manager constructor
	GetSystemTime(&m_sysTime);
}

PerfManager::~PerfManager() { // Performance manager destructor

}

int PerfManager::GetTicksPerSecond(int tickID) { return m_tickCounts[tickID]; }

int PerfManager::AddTick(int tickID) {
	// Saving the frame number
	++m_frameNbr;
	if (m_frameNbr > m_targetFps)
		m_frameNbr = 1;

	GetSystemTime(&m_sysTime); // Refresh time
	if (m_sysTime.wSecond == m_thisSecond) { // Same second = add frame to counter
		++m_tickCounting[tickID];
		return m_tickCounting[tickID];
	}
	else { // Different second, update the ticks per second in the array for display and set ticks this second to 1
		m_thisSecond = m_sysTime.wSecond;
		m_tickCounts[tickID] = m_tickCounting[tickID];
		m_tickCounting[tickID] = 1;

		std::cout << "FPS: " << m_tickCounts[tickID] << std::endl;

		/* Should go in BalanceFPS?
		// TODO: To balance when to perform non-critical operations (scanning items and other static entities)
		m_execNonCritOpsEachXFrames = (unsigned int)ceil(m_targetFps / (m_tickCounts[tickID] + 1)); // For performance adjustments (+1 to avoid possibility to /0, not the best solution)
		if (m_execNonCritOpsEachXFrames > m_maxFrameSpaceNonCrits) // Checking if not above maximum
		m_execNonCritOpsEachXFrames = m_maxFrameSpaceNonCrits;
		*/

		return 1;
	}
}

void PerfManager::StartBenchmark(unsigned int benchmarkID) {
	SYSTEMTIME sysTimeStart;
	GetSystemTime(&sysTimeStart);
	m_benchmarks[benchmarkID] = sysTimeStart;
}

int PerfManager::StopBenchmark(unsigned int benchmarkID) {
	// Getting start and stop time
	SYSTEMTIME sysTimeStart, sysTimeStop;
	GetSystemTime(&sysTimeStop);
	sysTimeStart = m_benchmarks[benchmarkID];

	// Calculating time
	int startMs = (sysTimeStart.wSecond * 1000) + sysTimeStart.wMilliseconds;
	int stopMs = (sysTimeStop.wSecond * 1000) + sysTimeStop.wMilliseconds;

	if (startMs > stopMs) // Special case, if it was the end of a minute
		stopMs += 60000;

	m_execTimeLastFrame = stopMs - startMs;

	// Adding to the queue to calculate the average execution time over the last second
	m_execTimesLastSecond.push_front(m_execTimeLastFrame);
	if (m_execTimesLastSecond.size() > m_targetFps)
		m_execTimesLastSecond.pop_back();

	return m_execTimeLastFrame;
}

void PerfManager::BalanceFPS() {
	// If last frame took more than the maximum time to get the desired FPS, we don't wait
	if ((m_msToWaitBetweenFrames - m_execTimeLastFrame) < 0 || !m_limitFps)
		return;

	// Calculating average execution time over last second
	int totalExecTimeLastSec = 0, avgOver = 1;

	if (m_execTimesLastSecond.size() < m_targetFps)
		avgOver = m_execTimesLastSecond.size();
	else
		avgOver = m_targetFps;

	for (int i(0); i < avgOver; ++i)
		totalExecTimeLastSec += m_execTimesLastSecond[i];
	int avgTimePerCycleLastSec = ceil(totalExecTimeLastSec / avgOver);

	// Calculate the time that should be waited to smooth frames
	int timeToWait = m_msToWaitBetweenFrames - avgTimePerCycleLastSec;
	if (timeToWait > 0)
		Sleep(timeToWait);
}

bool PerfManager::ShouldExecuteNonCritOps() {
	// TODO: Make dynamic with m_execNonCritOpsEachXFrames
	if ((m_frameNbr % m_maxFrameSpaceNonCrits) == 0)
		return true;
	else
		return false;
}

bool PerfManager::ToggleLimitFps() {
	if (m_limitFps) {
		m_limitFps = false;
		return false;
	}
	else {
		m_limitFps = true;
		return true;
	}
}