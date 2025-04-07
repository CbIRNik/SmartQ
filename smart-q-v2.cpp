#include <atomic>
#include <functional>
#include <thread>
#include <vector>

template <class T>
class smartQ
{
protected:
	std::vector<std::atomic<bool>>* state;
  std::vector<std::thread> threads;

private:
	template<class F>
  auto init_worker(std::vector<std::vector<T>>* arr, std::function<F>& func)
  {
    std::vector<std::atomic<bool>>* state = this -> state;

    return [state, arr, &func]() -> void
    {
      for (size_t state_index{0}; state_index < arr -> size(); ++state_index)
      {
        if (!state -> at(state_index))
        {
          state -> at(state_index).store(true);
          func(arr -> at(state_index), state_index);
        }
      }
      return;
    };
  }

public:
	template<class F>
  smartQ(std::vector<std::vector<T>>& tasks_pool, std::function<F>& func, size_t threads_number = 3)
  {
    this -> state = new std::vector<std::atomic<bool>>(tasks_pool.size());
    auto worker = this -> init_worker(&tasks_pool, func);
    
    for (size_t thread_id{0}; thread_id < threads_number; ++thread_id)
    {
      this -> threads.emplace_back(worker);
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
  std::function func = [](std::vector<int>& data, size_t& task_number) -> int
  {
  	int summary = 0; 
   	for (size_t i{0}; i < data.size(); ++i) 
    {
    	summary += data[i];
    }
    return summary;
  };
  smartQ<int> q{arr, func, 5};
  return 0;
}
