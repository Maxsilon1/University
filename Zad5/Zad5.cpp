#include <iostream>
#include <type_traits>

#include <vector>
#include <queue>
#include <deque>

#include <list>

template<class T, class Container = vector<T>>
class Stack
{
public:
	using value_type = Container::value_type;//Нужны для того, чтобы не использовать шаблоны
	using reference = Container::reference_type;
private:
	Container c;
public:
	Stack() {}
	Stack(std::initializer_list<int> ilist) 
	{
		c.reserve(ilist.size);
		for (size_t i = 0; i < ilist.size; ++i)
			c.push_back(ilist[i]);
	}
	Stack(const Stack& other) noexcept: c(other.c){}
	Stack(Stack&& other) noexcept(std::nothrow_move_assignable_v<Container>): c(std::move(other.c))  {}//Если мув семантика контейнера не выкинет ошибку, то и наша функция этого не сделает

	Stack& operator=(const Stack& other) noexcept(std::nothrow_move_assignable_v<Container>)
	{
		c = std::move(other.c);
		return *this;
	}

	bool empty() const { return c.empty(); }
	bool size() const { return c.size(); }

	value_type& top() { return c.back(); }
	const value_type& top() const { return c.back(); }

	reference pool()
	{
		if (empty())throw std::length_error("Stack is empty");
		value_type tmp = std::move(c.back());
		c.pop_back();
		return tmp;
	}

	void push(const value_type& val) { c.push_back(val); }
	void push(value_type&& val) { c.push_back(std::move(val)); }
	
	void pop() { c.pop_back(); }

	value_type search(const value_type& v)
	{
		size_t depth = size() - 1;//Если элемент является последним, то имеет глубину size() - 1, если является top, то 0
		for(const value_type& i : c)
		{
			if (v == i)
			{
				return depth;
			}
			--depth;
		}
		return -1;
	}

	template<typename... Args>//Передаём пакет данных
	decltype(auto) emplace(Args&&... args)
	{
		return c.emplace_back(std::forward<Args>(args)...);//forward<Args>(args) Смотрит на тип Args, сохраняет l/r - value переменных, а после распаковка
	}
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
class list
{
	struct BaseNode
	{
		BaseNode* prev;
		BaseNode* next;

		BaseNode(): prev(nullptr), next(nullptr) {}
		BaseNode(BaseNode* prev, BaseNode* next): prev(prev), next(next) {}
	};
	struct Node: BaseNode
	{
		T value;

		Node(): BaseNode() { value = 0; }
		Node(T val) : BaseNode() { value = val; }
	};
	
	using AllocTraits = std::allocator_traits<NodeAlloc>;
	using NodeAlloc = typename AllocTraits::template rebind_alloc<Node>;//Что это значит???
	using NodeAllocTraits = std::allocator_traits<NodeAlloc>;

	BaseNode* fakeNode_;
	Node* node;

	size_t size_;

	Alloc alloc_;
	NodeAlloc allocNode_;

	template<typename... Args>
	void emplace_impl(Args... args, void* link(Node*))
	{
		Node* newNode = NodeAllocTraits::allocate(allocNode_, 1);
		try
		{
			newNode = AllocTraits::construct(alloc_, newNode, std::forward(args)...);//Используем конструктор нашего дефолтного аллокатора

			link(newNode);
			++size_;
		}
		catch (...)
		{
			NodeAllocTraits::deallocate(allocNode_, newNode, 1);
		}
	}

	void link_front(Node* newNode)
	{
		fakeNode_->next->prev = newNode; newNode->next = fakeNode_->prev;
		fakeNode_->next = newNode;newNode->prev = fakeNode_;
	}
	void link_back(Node* newNode)
	{
		fakeNode_->prev->next = newNode; newNode->prev = fakeNode_->prev;
		fakeNode_->prev = newNode;newNode->next = fakeNode_;
	}
public:
	list(): fakeNode_{fakeNode_, fakeNode_}, size_(0) {}
	explicit list(size_t sz) : fakeNode_{ fakeNode_, fakeNode_ }
	{
		for (size_t i = 0; i < sz; ++i)
			push_back(T{});
	};
	explicit list(const list& other){}
	
	void push_back(const T& val)
	{
		emplace_impl(val, link_back);
	}
	void push_back(T&& val)
	{
		emplace_impl(std::move(val), link_back);
	}

	void push_front(const T& val)
	{
		emplace_impl(val, link_front);
	}
	void push_front(T&& val)
	{
		emplace_impl(std::move(val), link_front);
	}

	bool empty() 
	{ 
		if (&fakeNode_ != &fakeNode_)return false; 
		return true;
	}

	void pop_back()
	{

	}
	void pop_front()
	{

	}
private:
	template<bool isConst>
	class base_iterator
	{
	private:
		using iterator_category = std::bidirectional_iterator_tag;
		using pointer = std::conditional_t<isConst, const T*, T*);
		using reference = std::conditional_t<isConst, const T&, T&);
		using value_type = T;
		using difference_type = std::ptrdiff_t;

		Node* ptr;
	public:
		base_iterator(const base_iterator&) = default;
		base_iterator& operator=(const base_iterator&) = default;

		reference& operator*() const { return *ptr; }
		pointer operator->() const { return ptr; }
		pointer operator&() const { return ptr; }

		base_iterator& operator++()
		{
			ptr = ptr->next;
			return *this;
		}
		base_iterator& operator++(int)
		{
			base_iterator tmp = ptr;
			ptr = ptr->next;
			return tmp;
		}

		base_iterator& operator--()
		{
			ptr = ptr->prev;
			return *this;
		}
		base_iterator& operator--(int)
		{
			base_iterator tmp = *this;
			ptr = ptr->prev;
			return tmp;
		}

		base_iterator& operator+=(difference_type other)
		{
			return *this;
		}
		base_iterator& operator-=(difference_type other)
		{
			ptr -= other;
			return *this;
		}

		friend base_iterator& operator+(const base_iterator it, difference_type dif)
		{
			return it += dif;
		}
		friend base_iterator& operator+(difference_type dif, const base_iterator it)
		{
			return it += dif;
		}

		friend base_iterator& operator-(const base_iterator it, base_iterator& dif)
		{
			return it -= dif;
		}
		friend base_iterator& operator-(const base_iterator it, difference_type dif)
		{
			return it -= dif;
		}
		friend base_iterator& operator-(difference_type dif, const base_iterator it)
		{
			return it -= dif;
		}

		bool operator==(const base_iterator& other) { return ptr == ptr; }
		bool operator!=(const base_iterator& other) { return ptr != ptr; }

		bool operator<(const base_iterator& other) { return ptr < other.ptr; }
		bool operator>(const base_iterator& other) { return ptr > other.ptr; }

		bool operator<=(const base_iterator& other) { return ptr <= other.ptr; }
		bool operator>=(const base_iterator& other) { return ptr >= other.ptr; }
	};
public:
	using iterator = base_iterator<false>;
	using const_iterator = base_iterator<true>;
};

template<class T, class Alloc = std::allocator<T>>
class vector
{
public:
	using value_type = T;
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

		bool operator<(const base_iterator& other) { return ptr < other.ptr; }
		bool operator>(const base_iterator& other) { return ptr > other.ptr; }

		bool operator<=(const base_iterator& other) { return ptr <= other.ptr; }
		bool operator>=(const base_iterator& other) { return ptr >= other.ptr; }
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
	
	friend std::ostream& operator<<(std::ostream& os, const vector& v)//У std::ostream - удален конструктор копирования, можно вызывать только по ссылке
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
	
	value_type back() const noexcept
	{
		return arr_[size_ - 1];
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

template<class T, class Alloc = std::allocator<T>>
class deque
{
public:
	using value_type = T;
	using reference = T&;
	using pointer = T*;
private:
	pointer* arr;//Будет хранить указатели на массивы

	size_t head;
	size_t tail;

	size_t size_;
	size_t default_arr_size_ = 30;
	
	Alloc alloc_;

	using AllocTraits = std::allocator_traits<Alloc>;
public:
	deque(): arr(nullptr), head(0), tail(0), size_(0) {}
	deque(size_t sz): arr(nullptr), head(0), tail(0), size_(0){}

	void push_back()
	{
		if(tail % default_arr_size_== default_arr_size_ - 1)
		{
			resize(size_);//Увеличиваем количество подмассивов, которые будут распределяться между элементами
		}
	}
	void push_front()
	{
		
	}
	void resize()
	{

	}
	void reserve(size_t sz)
	{

	}
};

int main()
{
	std::list<int> l;
}