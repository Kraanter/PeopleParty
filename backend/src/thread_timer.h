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
private:
	bool active;
	std::mutex m;
	std::condition_variable cv;
	void threadProcessInterval(std::function<void()> handler,int timeout)
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
	void threadProcessTimeout(std::function<void()> handler,int timeout)
	{
			std::unique_lock<std::mutex> lock(m);
			auto status = cv.wait_for(lock,std::chrono::milliseconds(timeout));
			if (status == std::cv_status::timeout){
				if(!active) return;
				if (handler != nullptr)
					handler();
			}
	}
public:
	void setInterval(std::function<void()> ptr, int timer)
	{
		active = true;
		std::thread t(([=] {
			threadProcessInterval(ptr,timer);
			}));
		t.detach();
		
	}
	void setTimeout(std::function<void()> ptr, int timer)
	{	
		active = true;
		std::thread t(([=] {
			threadProcessTimeout(ptr,timer);
			}));
		t.detach();
		
	}
	void clear(){
		active = false;
		cv.notify_all();
	}
};

#endif //THREAD_TIMER_H