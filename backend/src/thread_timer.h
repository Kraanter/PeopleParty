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
	ThreadTimer() : active(false) { }
	~ThreadTimer() { clear(); }
	void setInterval(std::function<void()> ptr, int timer);
	void setTimeout(std::function<void()> ptr, int timer);
	void clear();
private:
	void threadProcessInterval(std::function<void()> handler,int timeout);
	void threadProcessTimeout(std::function<void()> handler,int timeout);
private:
	bool active;
	std::mutex m;
	std::condition_variable cv;
};

inline void ThreadTimer::threadProcessInterval(std::function<void()> handler,int timeout)
{
	while(active){
		std::unique_lock<std::mutex> lock(m);
		auto status = cv.wait_for(lock,std::chrono::milliseconds(timeout));
		if (status == std::cv_status::timeout){
			if(!active) return;
			if (handler != nullptr)
				handler();
		}
	}
	
}
inline void ThreadTimer::threadProcessTimeout(std::function<void()> handler,int timeout)
{
		std::unique_lock<std::mutex> lock(m);
		auto status = cv.wait_for(lock,std::chrono::milliseconds(timeout));
		if (status == std::cv_status::timeout){
			if(!active) return;
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
	cv.notify_all();
}

#endif //THREAD_TIMER_H