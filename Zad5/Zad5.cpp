#include <iostream>
#include <fstream>
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

int priority(char op)
{
	if (op == '+' || op == '-') return 1;
	if (op == '*' || op == '/') return 2;
	if (op == '^') return 3;
	return 0;
}

bool isOperator(char c)
{
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
}

// Разбиение на токены
std::vector<std::string> tokenize(const std::string& expr)
{
	std::vector<std::string> tokens;
	std::string num;

	for (char c : expr)
	{
		if (std::isdigit(c) || c == '.')
		{
			num += c;
		}
		else
		{
			if (!num.empty())
			{
				tokens.push_back(num);
				num.clear();
			}
			if (isOperator(c) || c == '(' || c == ')')
				tokens.push_back(std::string(1, c));
		}
	}

	if (!num.empty())
		tokens.push_back(num);

	return tokens;
}

// Инфикс -> ОПН
std::vector<std::string> toRPN(const std::vector<std::string>& tokens)
{
	std::vector<std::string> output;
	Stack<char> ops;

	for (const std::string& t : tokens)
	{
		if (std::isdigit(t[0]) || (t.size() > 1 && t[0] == '.'))
		{
			output.push_back(t);
		}
		else if (t == "(")
		{
			ops.push('(');
		}
		else if (t == ")")
		{
			while (!ops.empty() && ops.top() != '(')
			{
				output.push_back(std::string(1, ops.top()));
				ops.pop();
			}
			ops.pop();
		}
		else if (isOperator(t[0]))
		{
			char op = t[0];
			while (!ops.empty() && ops.top() != '(' && priority(ops.top()) >= priority(op))
			{
				output.push_back(std::string(1, ops.top()));
				ops.pop();
			}
			ops.push(op);
		}
	}

	while (!ops.empty())
	{
		output.push_back(std::string(1, ops.top()));
		ops.pop();
	}

	return output;
}

// Вычисление ОПН
double calculate(const std::vector<std::string>& rpn)
{
	Stack<double> st;

	for (const std::string& t : rpn)
	{
		if (isOperator(t[0]) && t.size() == 1)
		{
			double b = st.top(); st.pop();
			double a = st.top(); st.pop();

			if (t == "+") st.push(a + b);
			else if (t == "-") st.push(a - b);
			else if (t == "*") st.push(a * b);
			else if (t == "/") st.push(a / b);
			else if (t == "^") st.push(std::pow(a, b));
		}
		else
		{
			st.push(std::stod(t));
		}
	}

	return st.top();
}

void CurvaPolskaNotation()
{
	std::string expr;
	std::cout << "Введите выражение (без пробелов): ";
	std::cin >> expr;

	auto tokens = tokenize(expr);
	auto rpn = toRPN(tokens);
	double result = calculate(rpn);

	std::cout << "Результат: " << result << std::endl;
}

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
		for (size_t i = 0; i < other.size_ && i < size_; ++i)
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
	void push_back(std::initializer_list<T> ilist)
	{
		for (auto i : ilist)
		{
			push_back(i);
		}
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
class list
{
	struct BaseNode
	{
		BaseNode* prev;
		BaseNode* next;

		BaseNode() : prev(this), next(this) {}
	};

	struct Node : BaseNode
	{
		T value;
	};

public:
	// 2. Стандартные using-объявления (Traits)
	using value_type = T;
	using allocator_type = Alloc;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = typename std::allocator_traits<Alloc>::pointer;
	using const_pointer = typename std::allocator_traits<Alloc>::const_pointer;

private:
	using AllocTraits = std::allocator_traits<Alloc>;
	using NodeAlloc = typename AllocTraits::template rebind_alloc<Node>;
	using NodeAllocTraits = std::allocator_traits<NodeAlloc>;

	using BaseNodeAlloc = typename AllocTraits::template rebind_alloc<BaseNode>;
	using BaseNodeAllocTraits = std::allocator_traits<BaseNodeAlloc>;

	BaseNode* fakeNode_;
	size_t size_;

	Alloc alloc_; 
	NodeAlloc nodeAlloc_;
	BaseNodeAlloc baseNodeAlloc_;

	void init()
	{
		// Выделяем память под фиктивный узел
		fakeNode_ = BaseNodeAllocTraits::allocate(baseNodeAlloc_, 1);
		// Конструируем BaseNode (устанавливает prev/next на себя)
		BaseNodeAllocTraits::construct(baseNodeAlloc_, fakeNode_);
		size_ = 0;
	}

	template<typename... Args>
	void emplace_impl(BaseNode* pos, Args&&... args)
	{
		Node* newNode = NodeAllocTraits::allocate(nodeAlloc_, 1);
		try
		{
			AllocTraits::construct(alloc_, std::addressof(newNode->value), std::forward<Args>(args)...);

			newNode->next = pos;
			newNode->prev = pos->prev;
			pos->prev->next = newNode;
			pos->prev = newNode;

			++size_;
		}
		catch (...)
		{
			NodeAllocTraits::deallocate(nodeAlloc_, newNode, 1);
			throw;
		}
	}

public:
	list() : size_(0) 
	{
		init();
	}
	explicit list(size_t sz) : list()
	{
		for (size_t i = 0; i < sz; ++i)
			emplace_back(); // Создаст элементы по умолчанию
	}

	list(const list& other) : list()
	{
		for (const auto& item : other)
		{
			push_back(item);
		}
	}
	list(list&& other) noexcept : list() // Делаем swap
	{
		swap(other);
	}

	~list()
	{
		clear();
		BaseNodeAllocTraits::destroy(baseNodeAlloc_, fakeNode_);
		BaseNodeAllocTraits::deallocate(baseNodeAlloc_, fakeNode_, 1);
	}

	list& operator=(const list& other)
	{
		if (this == &other) return *this;
		clear();
		for (const auto& item : other)
			push_back(item);
		return *this;
	}

	list& operator=(list&& other) noexcept
	{
		if (this == &other) return *this;
		clear();
		swap(other);
		return *this;
	}

	void swap(list& other) noexcept
	{
		std::swap(fakeNode_, other.fakeNode_);
		std::swap(size_, other.size_);
		std::swap(alloc_, other.alloc_);
		std::swap(nodeAlloc_, other.nodeAlloc_);
		std::swap(baseNodeAlloc_, other.baseNodeAlloc_);
	}

	template<typename... Args>
	void emplace_back(Args&&... args)
	{
		emplace_impl(fakeNode_, std::forward<Args>(args)...);
	}

	template<typename... Args>
	void emplace_front(Args&&... args)
	{
		emplace_impl(fakeNode_->next, std::forward<Args>(args)...);
	}

	void push_back(const T& val) { emplace_back(val); }
	void push_back(T&& val) { emplace_back(std::move(val)); }

	void push_front(const T& val) { emplace_front(val); }
	void push_front(T&& val) { emplace_front(std::move(val)); }

	bool empty() const { return size_ == 0; }
	size_t size() const { return size_; }

	void pop_back()
	{
		if (empty()) return;
		erase(iterator(fakeNode_->prev));
	}

	void pop_front()
	{
		if (empty()) return;
		erase(begin());
	}

	void clear()
	{
		while (!empty())
		{
			pop_back();
		}
	}

private:
	template<bool isConst>
	class base_iterator
	{
	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using pointer = std::conditional_t<isConst, const T*, T*>;
		using reference = std::conditional_t<isConst, const T&, T&>;

		// Внутри итератора храним BaseNode*, так как fakeNode_ это BaseNode
		using NodePtr = BaseNode*;

	private:
		NodePtr ptr;

	public:
		base_iterator(NodePtr p) : ptr(p) {}

		template<bool wasConst, typename = std::enable_if_t<isConst && !wasConst>>
		base_iterator(const base_iterator<wasConst>& other) : ptr(other.ptr) {}

		reference operator*() const {
			return static_cast<Node*>(ptr)->value; 
		}

		pointer operator->() const { 
			return std::addressof(static_cast<Node*>(ptr)->value); 
		}

		base_iterator& operator++()
		{
			ptr = ptr->next;
			return *this;
		}

		base_iterator operator++(int)
		{
			base_iterator tmp = *this;
			ptr = ptr->next;
			return tmp;
		}

		base_iterator& operator--()
		{
			ptr = ptr->prev;
			return *this;
		}

		base_iterator operator--(int)
		{
			base_iterator tmp = *this;
			ptr = ptr->prev;
			return tmp;
		}

		bool operator==(const base_iterator& other) const { return ptr == other.ptr; }
		bool operator!=(const base_iterator& other) const { return ptr != other.ptr; }

		friend class list<T, Alloc>;
	};

public:
	using iterator = base_iterator<false>;
	using const_iterator = base_iterator<true>;

	iterator begin() { return iterator(fakeNode_->next); }
	const_iterator begin() const { return const_iterator(fakeNode_->next); }
	const_iterator cbegin() const { return const_iterator(fakeNode_->next); }

	iterator end() { return iterator(fakeNode_); }
	const_iterator end() const { return const_iterator(fakeNode_); }
	const_iterator cend() const { return const_iterator(fakeNode_); }

	iterator erase(iterator pos)
	{
		BaseNode* curr = pos.ptr;
		BaseNode* nextNode = curr->next;
		BaseNode* prevNode = curr->prev;

		prevNode->next = nextNode;
		nextNode->prev = prevNode;

		Node* Node = static_cast<Node*>(curr);

		AllocTraits::destroy(alloc_, std::addressof(Node->value));
		NodeAllocTraits::deallocate(nodeAlloc_, Node, 1);

		--size_;
		return iterator(nextNode);
	}

	// Удаление по значению
	void remove(const T& val)
	{
		iterator it = begin();
		while (it != end())
		{
			if (*it == val)
				it = erase(it); // erase возвращает следующий валидный итератор
			else
				++it;
		}
	}
};

template<class T, class Alloc = std::allocator<T>>
class deque
{
public:
	using value_type = T;
	using reference = T&;
	using const_reference = const T&;
	using pointer = T*;
	using size_type = size_t;

private:
	using AllocTraits = std::allocator_traits<Alloc>;

	T* arr_;
	size_t cap_;
	size_t head_;
	size_t size_;
	Alloc alloc_;

	size_t get_physical_index(size_t index) const
	{
		return (head_ + index) % cap_;
	}

	void grow()
	{
		size_t new_cap = (cap_ == 0) ? 16 : cap_ * 2;
		T* new_arr = AllocTraits::allocate(alloc_, new_cap);

		try
		{
			for (size_t i = 0; i < size_; ++i)
			{
				size_t old_idx = get_physical_index(i);
				AllocTraits::construct(alloc_, new_arr + i, std::move(arr_[old_idx]));
			}
		}
		catch (...)
		{
			AllocTraits::deallocate(alloc_, new_arr, new_cap);
			throw;
		}

		if (arr_)
		{
			for (size_t i = 0; i < size_; ++i)
				AllocTraits::destroy(alloc_, arr_ + get_physical_index(i));
			AllocTraits::deallocate(alloc_, arr_, cap_);
		}

		arr_ = new_arr;
		cap_ = new_cap;
		head_ = 0;
	}

	void remove_at_logical(size_t idx)
	{
		size_t physical_idx = get_physical_index(idx);
		AllocTraits::destroy(alloc_, arr_ + physical_idx);

		if (idx < size_ / 2)
		{
			for (size_t i = idx; i > 0; --i)
			{
				size_t curr = get_physical_index(i);
				size_t prev = get_physical_index(i - 1);
				if (i == idx) curr = physical_idx; 
				AllocTraits::construct(alloc_, arr_ + curr, std::move(arr_[prev]));
				AllocTraits::destroy(alloc_, arr_ + prev);
			}
			head_ = (head_ + 1) % cap_;
		}
		else
		{
			for (size_t i = idx; i < size_ - 1; ++i)
			{
				size_t curr = get_physical_index(i);
				size_t next = get_physical_index(i + 1);
				if (i == idx) curr = physical_idx;
				AllocTraits::construct(alloc_, arr_ + curr, std::move(arr_[next]));
				AllocTraits::destroy(alloc_, arr_ + next);
			}
		}
		--size_;
	}

public:
	deque() : arr_(nullptr), cap_(0), head_(0), size_(0)
	{
		cap_ = 16;
		arr_ = AllocTraits::allocate(alloc_, cap_);
	}

	explicit deque(const std::vector<T>& a) : deque(a.size())
	{
		addAll(a);
	}

	explicit deque(size_t numElements) : arr_(nullptr), cap_(0), head_(0), size_(0)
	{
		if (numElements > 0)
		{
			cap_ = numElements;
			arr_ = AllocTraits::allocate(alloc_, cap_);
		}
		else
		{
			cap_ = 16;
			arr_ = AllocTraits::allocate(alloc_, cap_);
		}
	}

	~deque()
	{
		clear();
		if (arr_)
			AllocTraits::deallocate(alloc_, arr_, cap_);
	}

	void add(const T& e)
	{
		addLast(e);
	}

	void addAll(const std::vector<T>& a)
	{
		for (const auto& item : a)
		{
			addLast(item);
		}
	}

	void clear()
	{
		while (!isEmpty())
			pollLast();
	}

	bool contains(const T& o) const
	{
		for (size_t i = 0; i < size_; ++i)
		{
			if (arr_[get_physical_index(i)] == o)
				return true;
		}
		return false;
	}

	bool containsAll(const std::vector<T>& a) const
	{
		for (const auto& item : a)
		{
			if (!contains(item))
				return false;
		}
		return true;
	}

	bool isEmpty() const
	{
		return size_ == 0;
	}

	bool remove(const T& o)
	{
		return removeFirstOccurrence(o);
	}

	void removeAll(const std::vector<T>& a)
	{
		for (const auto& item : a)
		{
			while (removeFirstOccurrence(item));
		}
	}

	void retainAll(const std::vector<T>& a)
	{
		size_t i = 0;
		while (i < size_)
		{
			const T& curr = arr_[get_physical_index(i)];
			bool found = false;
			for (const auto& keep : a)
			{
				if (curr == keep)
				{
					found = true;
					break;
				}
			}

			if (!found)
				remove_at_logical(i);
			else
				++i;
		}
	}

	size_t size() const
	{
		return size_;
	}

	std::vector<T> toArray() const
	{
		std::vector<T> res;
		res.reserve(size_);
		for (size_t i = 0; i < size_; ++i)
		{
			res.push_back(arr_[get_physical_index(i)]);
		}
		return res;
	}

	void toArray(std::vector<T>& a) const
	{
		a = toArray();
	}

	T& element()
	{
		return getFirst();
	}

	bool offer(const T& obj)
	{
		addLast(obj);
		return true;
	}

	T* peek()
	{
		return peekFirst();
	}

	T poll()
	{
		return pollFirst();
	}

	void addFirst(const T& obj)
	{
		if (size_ == cap_)
			grow();

		head_ = (head_ == 0) ? cap_ - 1 : head_ - 1;
		AllocTraits::construct(alloc_, arr_ + head_, obj);
		++size_;
	}

	void addLast(const T& obj)
	{
		if (size_ == cap_)
			grow();

		size_t tail = get_physical_index(size_);
		AllocTraits::construct(alloc_, arr_ + tail, obj);
		++size_;
	}

	T& getFirst()
	{
		if (isEmpty())
			throw std::out_of_range("Deque is empty");
		return arr_[head_];
	}

	T& getLast()
	{
		if (isEmpty())
			throw std::out_of_range("Deque is empty");
		return arr_[get_physical_index(size_ - 1)];
	}

	bool offerFirst(const T& obj)
	{
		addFirst(obj);
		return true;
	}

	bool offerLast(const T& obj)
	{
		addLast(obj);
		return true;
	}

	T pop()
	{
		return removeFirst();
	}

	void push(const T& obj)
	{
		addFirst(obj);
	}

	T* peekFirst()
	{
		if (isEmpty()) return nullptr;
		return &arr_[head_];
	}

	T* peekLast()
	{
		if (isEmpty()) return nullptr;
		return &arr_[get_physical_index(size_ - 1)];
	}

	T pollFirst()
	{
		if (isEmpty())
			throw std::out_of_range("Deque is empty");
		T val = std::move(arr_[head_]);
		removeFirst();
		return val;
	}

	T pollLast()
	{
		if (isEmpty())
			throw std::out_of_range("Deque is empty");
		T val = std::move(arr_[get_physical_index(size_ - 1)]);
		removeLast();
		return val;
	}

	T removeLast()
	{
		if (isEmpty())
			throw std::out_of_range("Deque is empty");

		size_t tail_idx = get_physical_index(size_ - 1);
		T val = std::move(arr_[tail_idx]);
		AllocTraits::destroy(alloc_, arr_ + tail_idx);
		--size_;
		return val;
	}

	T removeFirst()
	{
		if (isEmpty())
			throw std::out_of_range("Deque is empty");

		T val = std::move(arr_[head_]);
		AllocTraits::destroy(alloc_, arr_ + head_);
		head_ = (head_ + 1) % cap_;
		--size_;
		return val;
	}

	bool removeLastOccurrence(const T& obj)
	{
		if (isEmpty()) return false;

		for (size_t i = size_; i > 0; --i)
		{
			size_t logical = i - 1;
			if (arr_[get_physical_index(logical)] == obj)
			{
				remove_at_logical(logical);
				return true;
			}
		}
		return false;
	}

	bool removeFirstOccurrence(const T& obj)
	{
		for (size_t i = 0; i < size_; ++i)
		{
			if (arr_[get_physical_index(i)] == obj)
			{
				remove_at_logical(i);
				return true;
			}
		}
		return false;
	}

	template<bool IsConst>
	struct DequeIterator
	{
		using pointer = std::conditional_t<IsConst, const T*, T*>;
		using reference = std::conditional_t<IsConst, const T&, T&>;
		using value_type = T;
		using iterator_category = std::random_access_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using DequePtr = std::conditional_t<IsConst, const deque*, deque*>;

		DequePtr deq_ptr;
		size_t idx;

		DequeIterator(DequePtr d, size_t i) : deq_ptr(d), idx(i) {}

		reference operator*() const { return (*deq_ptr).arr_[deq_ptr->get_physical_index(idx)]; }
		pointer operator->() const { return &(*deq_ptr).arr_[deq_ptr->get_physical_index(idx)]; }

		DequeIterator& operator++() { ++idx; return *this; }
		DequeIterator operator++(int) { DequeIterator tmp = *this; ++idx; return tmp; }
		DequeIterator& operator--() { --idx; return *this; }
		DequeIterator operator--(int) { DequeIterator tmp = *this; --idx; return tmp; }
		DequeIterator& operator+=(difference_type n) { idx += n; return *this; }
		DequeIterator& operator-=(difference_type n) { idx -= n; return *this; }
		difference_type operator-(const DequeIterator& other) const { return idx - other.idx; }
		DequeIterator operator+(difference_type n) const { return { deq_ptr, idx + n }; }
		DequeIterator operator-(difference_type n) const { return { deq_ptr, idx - n }; }
		bool operator==(const DequeIterator& other) const { return idx == other.idx; }
		bool operator!=(const DequeIterator& other) const { return idx != other.idx; }
		bool operator<(const DequeIterator& other) const { return idx < other.idx; }
	};

	using iterator = DequeIterator<false>;
	using const_iterator = DequeIterator<true>;

	iterator begin() { return iterator(this, 0); }
	iterator end() { return iterator(this, size_); }
	const_iterator begin() const { return const_iterator(this, 0); }
	const_iterator end() const { return const_iterator(this, size_); }
};

static size_t digits(const std::string& s) {
	size_t c = 0;
	for (unsigned char ch : s) if (std::isdigit(ch)) ++c;
	return c;
}

static size_t spaces(const std::string& s) {
	size_t c = 0;
	for (char ch : s) if (ch == ' ') ++c;
	return c;
}

void funcDeq() {
	deque<std::string> q;
	std::ifstream fin("input.txt");
	std::string line;
	while (std::getline(fin, line)) {
		if (q.isEmpty()) { q.addLast(line); continue; }
		size_t d1 = digits(line);
		size_t d0 = digits(*q.peekFirst());
		if (d1 > d0) q.addLast(line); else q.addFirst(line);
	}
	std::ofstream fout("sorted.txt");
	size_t k = q.size();
	for (size_t i = 0; i < k; ++i) { std::string t = q.pollFirst(); fout << t << '\n'; q.addLast(t); }
	size_t n;
	std::cin >> n;
	k = q.size();
	for (size_t i = 0; i < k; ++i) { std::string t = q.pollFirst(); if (spaces(t) <= n) q.addLast(t); }
	while (!q.isEmpty()) std::cout << q.pollFirst() << '\n';
	return 0;
}

int main()
{
	list<int> l; l.push_front(10);
	
	for (auto it = d.begin(); it != d.end(); ++it)
	{
		std::cout << *it << '\t';
	}
}