#include <cmath>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include <iostream>

template<class TargetIdType>
struct Task {
  TargetIdType targetId;
	std::function<void()> execute;
};

class Semaphore {
private:
	size_t count = static_cast<size_t>(INFINITY);
	std::condition_variable cv;
	std::mutex mtx;
public:
  explicit Semaphore(size_t count = 0) {
    this -> count = count == 0 ? this -> count : count;
  }
  void acquire() {
  	std::unique_lock<std::mutex> lock(this -> mtx);
   	this -> cv.wait(lock, [this]{return this -> count > 0;});
    this -> count--;
  }
  void release() {
  	std::unique_lock<std::mutex> lock(this -> mtx);
  	this -> count++;
   	this -> cv.notify_one();
  }
};

template<class T>
class TargetLocker {
private:
	std::map<T, bool> lockTargets;
	std::condition_variable cv;
	std::mutex mtx;
public:
  TargetLocker() {}
  
  void acquire(T targetId) {
  	std::unique_lock<std::mutex> lock(this -> mtx);
    this -> cv.wait(lock, [&]{return !this -> lockTargets[targetId];});
    this -> lockTargets[targetId] = true;
  }
  
  void release(T targetId) {
  	std::unique_lock<std::mutex> lock(this -> mtx);
   	this -> lockTargets[targetId] = false;
    this -> cv.notify_one();
  }
};

template<class TargetIdType>
class Worker { 
private: 
	Semaphore* semaphore;
	TargetLocker<TargetIdType>* target_locker;
  std::function<void(Task<TargetIdType>&)> execute;
	
	auto init_execute() {
		return [this](Task<TargetIdType>& task) -> void {
			const TargetIdType targetId = task.targetId;
			task.execute();
			this -> semaphore -> release();
			this -> target_locker -> release(targetId);
		};
	};
	
public:
  Worker(size_t max_threads = 0) {
   	this -> semaphore = new Semaphore(max_threads);
    this -> target_locker = new TargetLocker<TargetIdType>{};
    this -> execute = this -> init_execute();
  }
  void run(std::vector<Task<TargetIdType>>&& queue) {
    std::vector<std::thread> threads;
   	for (Task<TargetIdType>& task : queue) {
			this -> target_locker -> acquire(task.targetId);
      this -> semaphore -> acquire();
      threads.emplace_back(this -> execute, std::ref(task));
    }
    for (std::thread& t : threads) {
      t.join();
    }
  }
  ~Worker() {
 		delete this -> semaphore;
  	delete this -> target_locker;
  }
};


int main() {
  Worker<long long> worker{100};
  const int max_tasks = 1000000;
  std::vector<Task<long long>> tasks(max_tasks);
  for (long long i = 0; i < max_tasks; ++i) {
    tasks[i] = Task<long long>{i, [i]() {
      std::cout << i << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }};
  }
  worker.run(std::move(tasks));
  return 0;
}