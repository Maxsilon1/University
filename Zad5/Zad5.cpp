#include <iostream>

#include <type_traits>

#include <vector>
#include <queue>
#include <deque>

template<typename T, class Container = vector<T>>
class Stack
{
private:
	Container data;

public:
	Stack() {}
};

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

template<class T, class Alloc = std::allocator<T>>
class vector
{
private:
	T* arr_;
	size_t cap_;
	size_t size_;
	Alloc alloc_;

	using AllocTraits = std::allocator_traits<Alloc>;
public:
	vector(): arr_(nullptr), cap_(0), size_(0) {}
	vector(size_t sz): arr_(AllocTraits::allocate(alloc_, sz)), cap_(sz), size_(sz) {}
	vector(std::initializer_list<T> ilist)
	{
		reserve(ilist.size());
		for (const T& i : ilist)
			push_back(i);
	}
	explicit vector(const vector& other) : arr_(nullptr), cap_(), size_() 
	{
		resize(other.size_);
		for (size_t i = 0; i < other.size_; ++i)
			push_back(other.arr_[i]);
	}
	vector(vector&& other): arr_(other.arr_), cap_(other.cap_), size_(other.size_) 
	{
		other.arr_ = nullptr;
		other.size_ = 0;
		other.cap_ = 0;
	}
private:
	template<bool isConst>
	class base_iterator
	{
	private:
		using iterator_category = std::random_access_iterator_tag;
		using reference = std::conditional_t<isConst, const T&, T&>;
		using pointer = std::conditional_t<isConst, const T*, T*>;
		using value_type = T;
		using difference_type = std::ptrdiff_t;//любой целочисленный тип для смещения итераторов

		T* ptr;
	public:
		base_iterator(T* p = nullptr): ptr(p) {}

		base_iterator (const base_iterator&) = default;
		base_iterator& operator=(const base_iterator& other) = default;

		reference operator*() { return *ptr; }
		pointer operator->() const { return ptr; }

		pointer operator&() { return ptr; }

		base_iterator& operator++() { ++ptr; return *this; }
		base_iterator operator++(int) { base_iterator old = *this; ++ptr; return old; }
		
		base_iterator& operator--() { return --ptr; }
		base_iterator operator--(int) { base_iterator old = *this; --ptr; return old; }

		base_iterator& operator+=(difference_type other)
		{
			ptr += other;
			return *this;
		}
		base_iterator& operator-=(difference_type other)
		{
			ptr -= other;
			return *this;
		}

		friend base_iterator operator+(base_iterator it, difference_type other)
		{
			return it += other;
		}
		friend base_iterator operator+(difference_type other, base_iterator it)
		{
			return it += other;
		}
		
		friend base_iterator operator-(base_iterator it, difference_type other)
		{
			return it -= other;
		}
		friend base_iterator operator-(const base_iterator& it1, const base_iterator& it2)
		{
			return it1.ptr - it2.ptr;
		}

		bool operator==(const base_iterator& other) { return ptr == other.ptr; }
		bool operator!=(const base_iterator& other) { return ptr != other.ptr; }

		bool operator<(const base_iterator& other) { return *this < other; }
		bool operator>(const base_iterator& other) { return *this > other; }

		bool operator<=(const base_iterator& other) { return *this <= other; }
		bool operator>=(const base_iterator& other) { return *this >= other; }
	};
	using iterator = base_iterator<false>;
	using const_iterator = base_iterator<true>;
public:

	T& operator[](size_t i) const { return *(arr_ + i); }
	vector& operator=(const vector& other) noexcept
	{
		Alloc newalloc_ = AllocTraits::propagate_on_container_copy_assigment::value ? other.alloc_ : alloc_;
		T* newarr = AllocTraits::allocate(newalloc_, other.cap_);
		size_t i = 0;
		try
		{
			for (; i < other.sz; ++i)
				AllocTraits::construct(newalloc_, newarr + i, other[i]);
			
		} catch (...)
		{
				for (size_t j = 0; j < i; ++j)
					AllocTraits::deallocate(newalloc_, newarr + j, other.cap_);
				throw;
		}

		for (i = 0; i < size_; ++i)
		{
			AllocTraits::destroy(alloc_, arr_ + i);

		}
		AllocTraits::deallocate(alloc_, arr_ + i);

		alloc_ = newalloc_;
		arr_ = newarr;
		size_ = other.size_;
		cap_ = other.cap_;
	}
	
	vector* operator+=(const T& value)
	{
		for (size_t i = 0; i < size_; ++i)
			arr_[i] += value;
		return *this;
	}
	vector& operator+=(const vector& other) 
	{
		if (size_ != other.size_)throw std::length_error("bad sizes");
		for (size_t i = 0; i < size_; ++i)
			arr_[i] += other.arr_[i];
		return *this;
	}

	vector& operator-=(const T& value)
	{
		for (size_t i = 0; i < size_; ++i)
			arr_[i] -= value;
		return *this;
	}
	vector& operator-=(const vector& other) 
	{
		if (size_ != other)throw std::length_error("bad sizes");
		for (size_t i = 0; i < size_; ++i)
			arr_[i] -= other.arr_[i];
		return *this;
	}

	friend vector& operator+(vector first, const vector& second)
	{
		return first += second;
	}
	friend vector& operator+(vector first, T& second)
	{
		return first += second;
	}
	friend vector& operator+(T& first, vector second) 
	{
		return second += first;
	}

	friend vector& operator-(vector first, const vector& second)
	{
		return first -= second;
	}
	friend vector& operator-(vector first, const T& second) { return first -= second; }
	friend vector& operator-(const T& first, vector second) { return second -= first; }
	
	friend std::ostream& operator<<(std::ostream& os, const vector v)//У std::ostream - удален конструктор копирования, можно вызывать только по ссылке
	{
		os << "{";//Вводим в поток os все наши данные
		for (size_t i = 0; i < v.size_; ++i)
		{
			os << v[i] << (i < v.size() - 1 ? " " : "");
		}
		os << "}";
		return os;
	}

	void range_initialize(iterator first, iterator end)
	{
		//используется в реализация forward, чуть позже нужно доделать
	}
	void push_back(const T& value)
	{
		if (size_ == cap_)
		{
			reserve(cap_ > 0 ? cap_ * 2 : 1);
		}
		AllocTraits::construct(alloc_, arr_ + size_, value);
		++size_;
	}
	void push_back(T&& value)
	{
		if (size_ == cap_)
		{
			reserve(cap_ > 0 ? cap_ * 2 : 1);
		}
		AllocTraits::construct(alloc_, arr_ + size_, std::move(value));
		++size_;
	}

	void reserve(size_t newcap)
	{
		if (newcap < cap_)
			return;

		T* newarr = AllocTraits::allocate(alloc_, newcap);//operator new();
		size_t i = 0;
		try
		{
			for (; i < size_; ++i)
			{
				AllocTraits::construct(alloc_, newarr + i, std::move_if_noexcept(arr_[i]));//Если перемещение безопасно, то перемещаем
			}
		}
		catch (...)
		{
			for (size_t old_i = 0; old_i < i; ++old_i)
			{
				AllocTraits::destroy(alloc_, newarr + old_i);
			}
			AllocTraits::deallocate(alloc_, newarr, newcap);
			throw;
		}

		for(size_t index = 0; index < size_; ++ index)
		{
			AllocTraits::destroy(alloc_, arr_ + i);
		}
		AllocTraits::deallocate(alloc_, arr_, cap_);

		arr_ = newarr;
		cap_ = newcap;
	}//Используем доп память, пока без move семантики
	
	size_t size() const noexcept {return size_;}
	size_t capacity() const noexcept{ return cap_; }
	bool isEmpty()
	{
		return size_ == 0;
	}

	void remove(int idx) noexcept
	{
		if (idx < 0)return;

		AllocTraits::deallocate(alloc_, arr_ + idx);
	}
	void remove(iterator& it) noexcept
	{
		AllocTraits::deallocate(alloc_, it);
	}

	iterator begin() noexcept { return iterator{ arr_ }; }
	iterator end() noexcept {return iterator{ arr_ + size_ };}

	const_iterator begin() const noexcept { return const_iterator{ arr_ }; }
	const_iterator end() const noexcept { return const_iterator{ arr_ + size_ }; }

	const_iterator cbegin() const noexcept { return const_iterator{ arr_ }; }
	const_iterator cend() const noexcept { return const_iterator{ arr_ + size_ }; }

	/*iterator rbegin() noexcept { return iterator{ arr_ + size_ }; }
	iterator rend() noexcept { return iterator{ arr_ }; }*/
	//Нужно использовать reverse iterator
};


int main()
{
	std::vector<int> stl_v;
	vector<int> v;
	v.push_back(5);
	v.push_back(6);
	v.reserve(10);
	std::cout << v;
}