#include <chrono>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <alloca.h>
#include <thread>

class LogDuration {
public:
	LogDuration(std::string id): id_(std::move(id)) {}

  ~LogDuration() {
  	const auto end_time = std::chrono::high_resolution_clock::now();
    const auto dur = end_time - start_time_;
    std::cout << id_ << ": ";
    std::cout << "operation time"
              << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(dur).count()
              << " ms" << std::endl;
  }

private:
  const std::string id_;
  const std::chrono::high_resolution_clock::time_point start_time_ = std::chrono::high_resolution_clock::now();
};


inline void f() { 
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	std::cout << "inline f" << std::endl;
	return;
}


template <class T>
T max(T&& arg1, T&& arg2)
{
	return arg1 / arg2;
}


int main() 
{
	LogDuration log("log");
	std::cout << max(2, 3);
	

	// void* ptr = operator new(8);
	// std::cout << *static_cast<int*>(ptr) << std::endl;
	// {
	// 	LogDuration lg("log");
	// 	std::vector<long long> big_copy(std::move(big_vec)); 
	// }
 	return 0;
}