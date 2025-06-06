#include <iostream>
#include <cstddef>
#include <initializer_list>

template <class T>
class Vector 
{
protected:
	T* data;
	size_t last;
	size_t capacity;
public:
	Vector(std::initializer_list<T>&& arr)
	{
		this -> last = arr.size() - 1;
		this -> capacity = arr.size();
		this -> data = new T[this -> capacity];
		for (size_t i{0}; i < this -> capacity; ++i)
		{
			this -> data[i] = *(arr.begin() + i);
		}
	}
	
	Vector* push_back(T&& new_element)
	{
		if (this -> capacity > this -> last) 
		{
			this -> data[last + 1] = new_element;
			this -> last++;
			return this;
		}
		this -> capacity *= 2;
		memcpy(new T[this -> capacity], this -> data, this -> capacity);
		this -> data[last + 1] = new_element;
		this -> last++;
		return this;
	}
	
	void log()
	{
		for (size_t i{0}; i <= this -> last; ++i)
		{
			std::cout << this -> data[i] << "\n";
		}
	}
	
	~Vector() {
		delete [] this -> data;
	}
};



int main()
{
	Vector<int> arr = {1, 2, 3};
	arr.push_back(4);
	arr.push_back(5);
	arr.push_back(6);
	arr.push_back(7);
	arr.push_back(8);
	arr.push_back(9);
	arr.log();
	return 0;
}