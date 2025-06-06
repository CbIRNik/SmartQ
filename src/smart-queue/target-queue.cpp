#include <cmath>
#include <condition_variable>
#include <cstddef>
#include <map>
#include <mutex>
#include <thread>

struct Task {
	int targetId;
	std::function<void()> execute;
};

class Semaphore {
protected:
	const int infinity = static_cast<int>(INFINITY);
private:
	int count;
	std::condition_variable cv;
	std::mutex mtx;
public:
  Semaphore(int count = 0) {
    this -> count = count || infinity;
  }
  
  void acquire() {
  	std::unique_lock<std::mutex> lock(this -> mtx);
   	while (this -> count <= 0) {
   		this -> cv.wait(lock);
    }
    this -> count--;
  }
  void release() {
  	std::unique_lock<std::mutex> lock(this -> mtx);
  	this -> count++;
   	this -> cv.notify_one();
  }
  
};

class TargetLocker {
private:
	std::map<int, bool> lockTargets;
	std::condition_variable cv;
	std::mutex mtx;
public:
  TargetLocker() {}
  
  void acquire(int targetId) {
  	std::unique_lock<std::mutex> lock(this -> mtx);
   	while (this -> lockTargets[targetId]) {
    	this -> cv.wait(lock);
    }
    this -> lockTargets[targetId] = true;
  }
  
  void release(int targetId) {
  	std::unique_lock<std::mutex> lock(this -> mtx);
   	this -> lockTargets[targetId] = false;
    this -> cv.notify_one();
  }
};

class Worker { 
private: 
	size_t max_threads;
	Semaphore* semaphore;
	TargetLocker* target_locker;
	
	auto init_execute() {
		return [&](Task& task) -> void {
			const int targetId = task.targetId;
			this -> target_locker -> acquire(targetId);
			this -> semaphore -> acquire();
			task.execute();
			this -> semaphore -> release();
			this -> target_locker -> release(targetId);
		};
	};
	
public:
  Worker(int max_threads = 0) {
  	this -> max_threads = max_threads;
   	this -> semaphore = new Semaphore(max_threads);
    this -> target_locker = new TargetLocker{};
  }
  
  void run(std::vector<Task> queue) {
  	const std::function execute = this -> init_execute();
    std::vector<std::thread> threads;
   	for (const Task& task : queue) {
    	threads.emplace_back(execute, task);
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
