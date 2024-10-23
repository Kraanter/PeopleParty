#ifndef THREAD_TIMER_H
#define THREAD_TIMER_H

#include <iostream>
#include <thread>
#include <chrono>
#include <functional>
#include <mutex>
#include <condition_variable>

class ThreadTimer
{
public:
	ThreadTimer() : active(false), paused(false) { }
	~ThreadTimer() { clear(); }
	void setInterval(std::function<void()> ptr, int timer);
	void setTimeout(std::function<void()> ptr, int timer);
	void clear();
	void pause();
	void pause(int timeout);
	void resume();
private:
	void threadProcessInterval(std::function<void()> handler,int timeout);
	void threadProcessTimeout(std::function<void()> handler,int timeout);
private:
	bool paused;
	bool active;
	std::mutex m;
	std::condition_variable cv;
};

inline void ThreadTimer::pause()
{
	if (!active || paused) return;
	paused = true;
	cv.notify_all();
}

inline void ThreadTimer::pause(int timeout)
{
	paused = true;
	std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
	paused = false;
}

inline void ThreadTimer::resume()
{
	if (!active || !paused) return;
	paused = false;
	cv.notify_all();
}

inline void ThreadTimer::threadProcessInterval(std::function<void()> handler, int timeout)
{
    while (active) {
        std::unique_lock<std::mutex> lock(m);
        
        // Wait while paused, or timeout for the interval duration
        if (paused) {
            cv.wait(lock, [this] { return !paused || !active; }); // Wait until unpaused or inactive
        }
        
        // After pause, check if still active
        if (!active) return;

        // Wait for the interval duration
        auto status = cv.wait_for(lock, std::chrono::milliseconds(timeout));
        if (status == std::cv_status::timeout && !paused && active) {
            if (handler != nullptr)
                handler();
        }
    }
}

inline void ThreadTimer::threadProcessTimeout(std::function<void()> handler, int timeout)
{
    std::unique_lock<std::mutex> lock(m);
    
    // Wait while paused
    if (paused) {
        cv.wait(lock, [this] { return !paused || !active; });
    }

    if (!active) return;

    // Wait for the timeout duration
    auto status = cv.wait_for(lock, std::chrono::milliseconds(timeout));
    if (status == std::cv_status::timeout && !paused && active) {
        if (handler != nullptr)
            handler();
    }
}

inline void ThreadTimer::setInterval(std::function<void()> ptr, int timer)
{
	active = true;
	std::thread t(([this, ptr, timer] {
		threadProcessInterval(ptr,timer);
		}));
	t.detach();
	
}
inline void ThreadTimer::setTimeout(std::function<void()> ptr, int timer)
{	
	active = true;
	std::thread t(([this, ptr, timer] {
		threadProcessTimeout(ptr,timer);
		}));
	t.detach();
	
}
inline void ThreadTimer::clear(){
	active = false;
	paused = false;
	cv.notify_all();
}

#endif //THREAD_TIMER_H