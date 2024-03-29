#pragma once

#include<queue>
#include<mutex>

template <typename T>
class MutexQueue
{
public:
	T pop() {
		std::unique_lock<std::mutex> l(lock_);
		auto val = queue_.front();
		queue_.pop();
		return val;
	}

	void push(T item) {
		std::unique_lock<std::mutex> l(lock_);
		queue_.push(item);
	}

	bool isEmpty() { return queue_.empty(); }

	void clear() { queue_.empty(); }
	
	//queue_ = default;
	//queue_ (const queue&);
private:
	std::queue<T> queue_;
	std::mutex lock_;
	//std::condition_variable cond_;
};

