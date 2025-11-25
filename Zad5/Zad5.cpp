#include <iostream>

#include <type_traits>

#include <vector>
#include <queue>
#include <deque>

//Аллокатор используется косвенно
template<class T, class Container = std::vector<T>, class Comparator = std::less<T>>
class Heap
{
private:
	Container data;
	Comparator comp;

	void SiftDown(size_t idx)
	{
		while (true)
		{
			size_t largest = idx;

			size_t left = 2 * idx + 1;
			size_t right = 2 * idx + 2;

			if (left < data.size() && comp(data[largest], data[left]))
				largest = left;
			if (right < data.size() && comp(data[largest], data[right]))
				largest = right;

			if (largest == idx)return;
			
			std::swap(data[idx], data[largest]);
			idx = largest;
		}
	}
	void SiftUp(size_t idx)
	{
		while (idx > 0)
		{
			size_t parent = (idx - 1) / 2;
			if (!comp(data[parent], data[idx]))return;//При построении этого достаточно
			
			std::swap(data[parent], data[idx]);
			idx = parent;
		}
	}
public:
	Heap(size_t sz = 10, const Comparator& c = Comparator()) :comp(c) { data.resize(sz); }
	Heap(const Container& arr): data(arr) {}
	Heap(Container&& arr): data(std::move(arr)) {}

	void push(const T& value) noexcept
	{
		data.push_back(value);
		SiftUp(data.size() - 1);
	}
	void push(T&& value) noexcept
	{
		data.push_back(std::move(value));
		SiftUp(data.size() - 1);
	}

	void pop() noexcept
	{
		data[0] = std::move(data.back());
		data.pop_back();
		if (!empty())
		{
			SiftDown(0);
		}
	}
	bool remove(T val) noexcept
	{
		auto it = std::find(data.begin(), data.end(), val);
		
		if (it == data.end())return false;

		size_t idx = it - data.begin();
		data[idx] = std::move(data.end());
		data.pop_back();

		if (!empty() && idx < data.size())
		{
			SiftDown(idx);
			SiftUp(idx);
		}
		return true;
	}

	const T& top() const noexcept{ return data[0]; }
	T& top() noexcept { return data[0]; }

	bool empty() noexcept
	{
		return (data.size() == 0);
	}
	
	bool contains(const T& val) noexcept
	{
		return std::find(data.begin(), data.end(), val) != data.end();
	}
	bool contains(Container& arr) noexcept
	{
		bool out = true;
		for (const T& val : arr)
		{
			out &= contains(val);
		}
		return out;
	}

	void clear() noexcept
	{
		data.clear();
	}

	//Оставить только указанные элементы
	void retain(T val)
	{
		retain(val, true);
	}
	void retain(T val, bool isAlone)
	{
		if (auto it = std::find(data.begin(), data.end(), val); it != data.end())
		{
			if (isAlone)//;(
			{
				data = Container{};
			}
			data.push(*it);
		}
	}
	void retain(Container& arr)
	{
		bool res = true;
		for (const T& v : arr)
		{
			retain(v, false);
		}
	}

	T& peek() noexcept
	{
		if (empty())throw std::out_of_range("Heap::peek() out of range");
		return data[0];
	}
	const T pool()
	{
		if (empty())throw std::out_of_range("Heap::pool() out of range");
		
		T val = std::move(data[0]);
		
		data[0] = std::move(data.back());
		data.pop_back();

		if (!empty())
			SiftDown(0);

		return val;
	}

	~Heap()
	{
		if constexpr (std::is_pointer<T>::value)
		{
			for (auto ptr : data)
				delete ptr;
		}
	}
};

template<class T, class Allocator = std::allocator<T>>
class vector
{
private:
	T* arr_;
	size_t cap_;
	size_t size_;
public:
	vector() {}
	vector(size_t sz): size_(sz) {}
	vector(vector& other): arr_(std::move(other)) {}

private:
	template<bool isConst>
	class base_iterator
	{
	private:
		using reference_type = std::conditional<isConst, const T&, T&>;
		using pointer_type = std::conditional<isConst, const T*, T*>;
		using value_type = T;

		T* ptr;
	public:
		base_iterator (const base_iterator&) = default;
		base_iterator& operator=(const base_iterator& other) = default;

		reference_type operator*() { return *ptr; }
		reference_type operator->() {}
		pointer_type operator&() { return ptr; }

		pointer_type operator++() { ++ptr; return *this; }
		pointer_type operator++(int) { base_iterator old = *this; ++ptr; return old; }
		pointer_type operator--() { return --ptr; }
		pointer_type operator--(int) { base_iterator old = *this; ++ptr; return old; }


	};
	using iterator = base_iterator<false>;
	using const_iterator = base_iterator<true>;
public:

	void push_back(const T& tmp) noexcept
	{
		if (size_ == cap_)
		{
			reserve(cap_ > 0 ? cap_ * 2 : 1);
		}
	}
	void push_back(T&& tmp) noexcept
	{
		if (size_ == cap_)
		{
			reserve(cap_ > 0 ? cap_ * 2 : 1);
		}
	}
	void reserve(size_t _newcap)
	{
		if (_newcap < _cap)
			return;

		T* newarr;
		size_t idx = 0;
		try
		{
			for (; idx < size_; ++idx)
			{
				new(newarr + ind) T(arr_[ind]);
			}
		}
		catch (...)
		{
			for (size_t old_idx; old_idx < idx; ++old_idx)
			{
				(newarr + old_idx)->~T;
			}
		}
	}
	bool isEmpty()
	{
		return size_ == 0;
	}

	iterator begin() { return iterator{ arr_ }; }
	iterator end() {return iterator{ arr_ + size_ };}

	const_iterator cbegin() const { return iterator{ arr_ }; }
	const_iterator cend() const { return iterator{ arr_ + size_ }; }

	iterator rbegin() { return iterator{ arr_ + size_ }; }
	iterator rend() { return iterator{ arr_ }; }
};


int main()
{
	Heap<int> smth(5);
	int val;
	for (int i = 0; i < 10; ++i)
	{
		std::cin >> val;
		smth.push(std::move(val));
	}
	std::cout << smth.pool() << '\t';
	std::cout << smth.peek() << '\n';
	std::cout << smth.contains(5);
}