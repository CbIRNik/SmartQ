#include <cmath>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include <iostream>

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
    this -> count = (count == 0 ? this -> infinity : count);
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
		return [this](Task& task) -> void {
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
  template<class T>
  void run(T&& queue) {
  	const std::function<void(Task&)> execute = this -> init_execute();
    std::vector<std::thread> threads;
   	for (Task& task : queue) {
    	threads.emplace_back(execute, std::ref(task));
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
  Worker worker(100);
  const int max_tasks = 1000;
  std::vector<Task> tasks(max_tasks);
  for (int i = 0; i < max_tasks; ++i) {
    tasks[i] = Task{i % 10, [i]() {
      std::cout << i << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }};
  }
  worker.run(tasks);
  return 0;
}