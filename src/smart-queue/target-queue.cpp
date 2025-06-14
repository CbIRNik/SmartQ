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
  std::function<void(Task&)> execute;
	
	auto init_execute() {
		return [this](Task& task) -> void {
			const int targetId = task.targetId;
			this -> target_locker -> acquire(targetId);
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
    this -> execute = this -> init_execute();
  }
  void run(std::vector<Task>&& queue) {
    std::vector<std::thread> threads;
   	for (Task& task : queue) {
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
  Worker worker{};
  const int max_tasks = 1000000;
  std::vector<Task> tasks(max_tasks);
  for (int i = 0; i < max_tasks; ++i) {
    tasks[i] = Task{i, [i]() {
      std::cout << i << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }};
  }
  worker.run(std::move(tasks));
  return 0;
}