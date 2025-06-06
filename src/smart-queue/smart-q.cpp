#include <atomic>
#include <iostream>
#include <thread>
#include <vector>


template <typename T>
class smartQ
{
protected:
	std::vector<std::atomic<bool>>* state;
  std::vector<std::thread> threads;

private:
  auto init_sum(std::vector<std::vector<T>>* arr)
  {
    std::vector<std::atomic<bool>>* state = this -> state;

    return [state, arr](size_t id) -> void
    {
      for (size_t state_index{0}; state_index < arr -> size(); ++state_index)
      {
        if (!state -> at(state_index))
        {
          state -> at(state_index).store(true);
          T summary = 0;
          for (T& el : arr -> at(state_index))
          {
            summary += el;
          }
          std::cout << id << " " << summary << "\n";
        }
      }
    };
  }

public:
  smartQ(std::vector<std::vector<T>>& arr, size_t threads_number = 3)
  {
    this -> state = new std::vector<std::atomic<bool>>(arr.size());
    auto sum = this -> init_sum(&arr);
    
    for (size_t thread_id{0}; thread_id < threads_number; ++thread_id)
    {
      this -> threads.emplace_back(sum, thread_id);
    }
    for (std::thread& thread : this -> threads)
    {
      thread.join();
    }
  }
};

int main()
{
  std::vector<std::vector<int>> arr{
    {1, 2, 3, 10, 102, 2, 234, 34, 234, 23, 4, 224, 2},
    {0, 2, 3, 200, 22},
    {1, 2, 0},
    {3, 4, 5, 4, 63, 34, 34, 2, 656},
    {10, 15, 13, 45, 7, 35}
  };
  smartQ<int> q{arr, 5};
  return 0;
}
