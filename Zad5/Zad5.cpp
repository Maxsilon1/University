#include <iostream>
#include <fstream>

#include <type_traits>

#include <vector>
#include <queue>
#include <deque>
#include <string>
#include <list>

#include <cmath>
#include <algorithm>
#include <cstring>
#include <ctime>

namespace my {

    template<class T, class Container = std::vector<T>>
    class Stack
    {
    public:
        using value_type = typename Container::value_type;
        using reference = typename Container::reference;
    private:
        Container c;
    public:
        Stack() {}
        Stack(std::initializer_list<T> ilist)
        {
            c.reserve(ilist.size());
            for (const auto& item : ilist)
                c.push_back(item);
        }
        Stack(const Stack& other) noexcept : c(other.c) {}
        Stack(Stack&& other) noexcept(std::is_nothrow_move_assignable_v<Container>) : c(std::move(other.c)) {}

        Stack& operator=(const Stack& other) noexcept(std::is_nothrow_move_assignable_v<Container>)
        {
            c = std::move(other.c);
            return *this;
        }

        bool empty() const { return c.empty(); }
        size_t size() const { return c.size(); }

        value_type& top() { return c.back(); }
        const value_type& top() const { return c.back(); }

        value_type pool()
        {
            if (empty()) throw std::length_error("Stack is empty");
            value_type tmp = std::move(c.back());
            c.pop_back();
            return tmp;
        }

        void push(const value_type& val) { c.push_back(val); }
        void push(value_type&& val) { c.push_back(std::move(val)); }

        void pop() { c.pop_back(); }

        int search(const value_type& v)
        {
            int depth = (int)size() - 1;
            for (auto it = c.rbegin(); it != c.rend(); ++it)
            {
                if (v == *it) return (int)size() - 1 - depth;
                --depth;
            }
            return -1;
        }

        template<typename... Args>
        decltype(auto) emplace(Args&&... args)
        {
            return c.emplace_back(std::forward<Args>(args)...);
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
        vector() : arr_(nullptr), cap_(0), size_(0) {}
        vector(size_t sz) : arr_(AllocTraits::allocate(alloc_, sz)), cap_(sz), size_(sz) {
            for(size_t i=0; i<sz; ++i) AllocTraits::construct(alloc_, arr_+i);
        }
        vector(std::initializer_list<T> ilist) : vector()
        {
            reserve(ilist.size());
            for (const T& i : ilist)
                push_back(i);
        }
        explicit vector(const vector& other) : arr_(nullptr), cap_(0), size_(0)
        {
            reserve(other.size_);
            for (size_t i = 0; i < other.size_; ++i)
                push_back(other.arr_[i]);
        }
        vector(vector&& other) : arr_(other.arr_), cap_(other.cap_), size_(other.size_)
        {
            other.arr_ = nullptr;
            other.size_ = 0;
            other.cap_ = 0;
        }

        ~vector() {
            clear();
            if(arr_) AllocTraits::deallocate(alloc_, arr_, cap_);
        }

        void clear() {
            for(size_t i=0; i<size_; ++i) AllocTraits::destroy(alloc_, arr_+i);
            size_ = 0;
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
            using difference_type = std::ptrdiff_t;

            T* ptr;
        public:
            base_iterator(T* p = nullptr) : ptr(p) {}
            base_iterator(const base_iterator&) = default;
            base_iterator& operator=(const base_iterator& other) = default;

            reference operator*() { return *ptr; }
            pointer operator->() const { return ptr; }

            base_iterator& operator++() { ++ptr; return *this; }
            base_iterator operator++(int) { base_iterator old = *this; ++ptr; return old; }

            base_iterator& operator--() { --ptr; return *this; }
            base_iterator operator--(int) { base_iterator old = *this; --ptr; return old; }

            base_iterator& operator+=(difference_type other) { ptr += other; return *this; }
            base_iterator& operator-=(difference_type other) { ptr -= other; return *this; }

            friend base_iterator operator+(base_iterator it, difference_type other) { return it += other; }
            friend base_iterator operator+(difference_type other, base_iterator it) { return it += other; }
            friend base_iterator operator-(base_iterator it, difference_type other) { return it -= other; }
            friend difference_type operator-(const base_iterator& it1, const base_iterator& it2) { return it1.ptr - it2.ptr; }

            bool operator==(const base_iterator& other) { return ptr == other.ptr; }
            bool operator!=(const base_iterator& other) { return ptr != other.ptr; }
            bool operator<(const base_iterator& other) { return ptr < other.ptr; }
            bool operator>(const base_iterator& other) { return ptr > other.ptr; }
            bool operator<=(const base_iterator& other) { return ptr <= other.ptr; }
            bool operator>=(const base_iterator& other) { return ptr >= other.ptr; }
        };

    public:
        using iterator = base_iterator<false>;
        using const_iterator = base_iterator<true>;

        T& operator[](size_t i) const { return *(arr_ + i); }

        vector& operator=(const vector& other)
        {
            if (this == &other) return *this;
            clear();
            reserve(other.size_);
            for(size_t i=0; i<other.size_; ++i) push_back(other.arr_[i]);
            return *this;
        }

        vector& operator+=(const T& value) {
            for (size_t i = 0; i < size_; ++i) arr_[i] += value;
            return *this;
        }
        vector& operator+=(const vector& other) {
            if (size_ != other.size_) throw std::length_error("bad sizes");
            for (size_t i = 0; i < size_; ++i) arr_[i] += other.arr_[i];
            return *this;
        }
        vector& operator-=(const T& value) {
            for (size_t i = 0; i < size_; ++i) arr_[i] -= value;
            return *this;
        }
        vector& operator-=(const vector& other) {
            if (size_ != other.size_) throw std::length_error("bad sizes");
            for (size_t i = 0; i < size_; ++i) arr_[i] -= other.arr_[i];
            return *this;
        }

        friend vector operator+(vector first, const vector& second) { return first += second; }
        friend vector operator+(vector first, const T& second) { return first += second; }
        friend vector operator+(const T& first, vector second) { return second += first; }
        friend vector operator-(vector first, const vector& second) { return first -= second; }
        friend vector operator-(vector first, const T& second) { return first -= second; }

        friend std::ostream& operator<<(std::ostream& os, const vector& v) {
            os << "{";
            for (size_t i = 0; i < v.size_; ++i)
                os << v[i] << (i < v.size() - 1 ? " " : "");
            os << "}";
            return os;
        }

        void push_back(const T& value) {
            if (size_ == cap_) reserve(cap_ > 0 ? cap_ * 2 : 1);
            AllocTraits::construct(alloc_, arr_ + size_, value);
            ++size_;
        }
        void push_back(T&& value) {
            if (size_ == cap_) reserve(cap_ > 0 ? cap_ * 2 : 1);
            AllocTraits::construct(alloc_, arr_ + size_, std::move(value));
            ++size_;
        }

        void reserve(size_t newcap) {
            if (newcap <= cap_) return;
            T* newarr = AllocTraits::allocate(alloc_, newcap);
            size_t i = 0;
            try {
                for (; i < size_; ++i) {
                    AllocTraits::construct(alloc_, newarr + i, std::move_if_noexcept(arr_[i]));
                }
            } catch (...) {
                for (size_t old_i = 0; old_i < i; ++old_i) AllocTraits::destroy(alloc_, newarr + old_i);
                AllocTraits::deallocate(alloc_, newarr, newcap);
                throw;
            }
            for (size_t k = 0; k < size_; ++k) AllocTraits::destroy(alloc_, arr_ + k);
            if(arr_) AllocTraits::deallocate(alloc_, arr_, cap_);
            arr_ = newarr;
            cap_ = newcap;
        }

        size_t size() const noexcept { return size_; }
        size_t capacity() const noexcept { return cap_; }
        bool isEmpty() { return size_ == 0; }
        value_type& back() { return arr_[size_ - 1]; }

        iterator begin() noexcept { return iterator{ arr_ }; }
        iterator end() noexcept { return iterator{ arr_ + size_ }; }
        const_iterator begin() const noexcept { return const_iterator{ arr_ }; }
        const_iterator end() const noexcept { return const_iterator{ arr_ + size_ }; }
    };

    template<class T, class Alloc = std::allocator<T>>
    class list
    {
        struct BaseNode {
            BaseNode* prev;
            BaseNode* next;
            BaseNode() : prev(this), next(this) {}
        };
        struct Node : BaseNode { T value; };

    public:
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
        using BaseNodeAlloc = typename AllocTraits::template rebind_alloc<BaseNode>;
        using NodeAllocTraits = std::allocator_traits<NodeAlloc>;
        using BaseNodeAllocTraits = std::allocator_traits<BaseNodeAlloc>;

        BaseNode* fakeNode_;
        size_t size_;
        Alloc alloc_;
        NodeAlloc nodeAlloc_;
        BaseNodeAlloc baseNodeAlloc_;

        void init() {
            fakeNode_ = BaseNodeAllocTraits::allocate(baseNodeAlloc_, 1);
            BaseNodeAllocTraits::construct(baseNodeAlloc_, fakeNode_);
            size_ = 0;
        }

        template<typename... Args>
        void emplace_impl(BaseNode* pos, Args&&... args) {
            Node* newNode = NodeAllocTraits::allocate(nodeAlloc_, 1);
            try {
                AllocTraits::construct(alloc_, std::addressof(newNode->value), std::forward<Args>(args)...);
                newNode->next = pos;
                newNode->prev = pos->prev;
                pos->prev->next = newNode;
                pos->prev = newNode;
                ++size_;
            } catch (...) {
                NodeAllocTraits::deallocate(nodeAlloc_, newNode, 1);
                throw;
            }
        }

    public:
        list() : size_(0) { init(); }
        explicit list(size_t sz) : list() { for (size_t i = 0; i < sz; ++i) emplace_back(); }
        list(const list& other) : list() { for (const auto& item : other) push_back(item); }
        list(list&& other) noexcept : list() { swap(other); }
        ~list() {
            clear();
            BaseNodeAllocTraits::destroy(baseNodeAlloc_, fakeNode_);
            BaseNodeAllocTraits::deallocate(baseNodeAlloc_, fakeNode_, 1);
        }

        list& operator=(const list& other) {
            if (this == &other) return *this;
            clear();
            for (const auto& item : other) push_back(item);
            return *this;
        }
        void swap(list& other) noexcept {
            std::swap(fakeNode_, other.fakeNode_);
            std::swap(size_, other.size_);
            std::swap(alloc_, other.alloc_);
            std::swap(nodeAlloc_, other.nodeAlloc_);
            std::swap(baseNodeAlloc_, other.baseNodeAlloc_);
        }

        template<typename... Args> void emplace_back(Args&&... args) { emplace_impl(fakeNode_, std::forward<Args>(args)...); }
        template<typename... Args> void emplace_front(Args&&... args) { emplace_impl(fakeNode_->next, std::forward<Args>(args)...); }
        void push_back(const T& val) { emplace_back(val); }
        void push_back(T&& val) { emplace_back(std::move(val)); }
        void push_front(const T& val) { emplace_front(val); }
        bool empty() const { return size_ == 0; }
        size_t size() const { return size_; }

        template<bool isConst>
        class base_iterator {
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = std::conditional_t<isConst, const T*, T*>;
            using reference = std::conditional_t<isConst, const T&, T&>;
            using NodePtr = BaseNode*;
        private:
            NodePtr ptr;
        public:
            base_iterator(NodePtr p) : ptr(p) {}
            template<bool wasConst, typename = std::enable_if_t<isConst && !wasConst>>
            base_iterator(const base_iterator<wasConst>& other) : ptr(other.ptr) {}

            reference operator*() const { return static_cast<Node*>(ptr)->value; }
            pointer operator->() const { return std::addressof(static_cast<Node*>(ptr)->value); }
            base_iterator& operator++() { ptr = ptr->next; return *this; }
            base_iterator operator++(int) { base_iterator tmp = *this; ptr = ptr->next; return tmp; }
            base_iterator& operator--() { ptr = ptr->prev; return *this; }
            base_iterator operator--(int) { base_iterator tmp = *this; ptr = ptr->prev; return tmp; }
            bool operator==(const base_iterator& other) const { return ptr == other.ptr; }
            bool operator!=(const base_iterator& other) const { return ptr != other.ptr; }
            friend class list<T, Alloc>;
        };

        using iterator = base_iterator<false>;
        using const_iterator = base_iterator<true>;
        iterator begin() { return iterator(fakeNode_->next); }
        iterator end() { return iterator(fakeNode_); }
        const_iterator begin() const { return const_iterator(fakeNode_->next); }
        const_iterator end() const { return const_iterator(fakeNode_); }
        const_iterator cbegin() const { return const_iterator(fakeNode_->next); }
        const_iterator cend() const { return const_iterator(fakeNode_); }

        iterator erase(iterator pos) {
            BaseNode* curr = pos.ptr;
            BaseNode* nextNode = curr->next;
            BaseNode* prevNode = curr->prev;
            prevNode->next = nextNode;
            nextNode->prev = prevNode;
            Node* nodePtr = static_cast<Node*>(curr);
            AllocTraits::destroy(alloc_, std::addressof(nodePtr->value));
            NodeAllocTraits::deallocate(nodeAlloc_, nodePtr, 1);
            --size_;
            return iterator(nextNode);
        }
        void pop_back() { if (!empty()) erase(iterator(fakeNode_->prev)); }
        void pop_front() { if (!empty()) erase(begin()); }
        void clear() { while (!empty()) pop_back(); }
        void remove(const T& val) {
            iterator it = begin();
            while (it != end()) {
                if (*it == val) it = erase(it);
                else ++it;
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
        size_t cap_, head_, size_;
        Alloc alloc_;

        size_t get_physical_index(size_t index) const { return (head_ + index) % cap_; }
        void grow() {
            size_t new_cap = (cap_ == 0) ? 16 : cap_ * 2;
            T* new_arr = AllocTraits::allocate(alloc_, new_cap);
            try {
                for (size_t i = 0; i < size_; ++i) {
                    size_t old_idx = get_physical_index(i);
                    AllocTraits::construct(alloc_, new_arr + i, std::move(arr_[old_idx]));
                }
            } catch (...) { AllocTraits::deallocate(alloc_, new_arr, new_cap); throw; }
            if (arr_) {
                for (size_t i = 0; i < size_; ++i) AllocTraits::destroy(alloc_, arr_ + get_physical_index(i));
                AllocTraits::deallocate(alloc_, arr_, cap_);
            }
            arr_ = new_arr; cap_ = new_cap; head_ = 0;
        }
        void remove_at_logical(size_t idx) {
            size_t phys = get_physical_index(idx);
            AllocTraits::destroy(alloc_, arr_ + phys);
            if(idx < size_/2) {
                for(size_t i=idx; i>0; --i) {
                    size_t c = get_physical_index(i);
                    size_t p = get_physical_index(i-1);
                    if(i==idx) c = phys;
                    AllocTraits::construct(alloc_, arr_+c, std::move(arr_[p]));
                    AllocTraits::destroy(alloc_, arr_+p);
                }
                head_ = (head_+1)%cap_;
            } else {
                for(size_t i=idx; i<size_-1; ++i) {
                    size_t c = get_physical_index(i);
                    size_t n = get_physical_index(i+1);
                    if(i==idx) c = phys;
                    AllocTraits::construct(alloc_, arr_+c, std::move(arr_[n]));
                    AllocTraits::destroy(alloc_, arr_+n);
                }
            }
            --size_;
        }

    public:
        deque() : arr_(nullptr), cap_(0), head_(0), size_(0) { cap_ = 16; arr_ = AllocTraits::allocate(alloc_, cap_); }
        explicit deque(const std::vector<T>& a) : deque(a.size()) { addAll(a); }
        explicit deque(size_t numElements) : arr_(nullptr), cap_(0), head_(0), size_(0) {
            if (numElements > 0) { cap_ = numElements; arr_ = AllocTraits::allocate(alloc_, cap_); }
            else { cap_ = 16; arr_ = AllocTraits::allocate(alloc_, cap_); }
        }
        ~deque() { clear(); if (arr_) AllocTraits::deallocate(alloc_, arr_, cap_); }

        bool isEmpty() const { return size_ == 0; }
        size_t size() const { return size_; }
        void clear() { while (!isEmpty()) pollLast(); }

        void addLast(const T& obj) {
            if (size_ == cap_) grow();
            size_t tail = get_physical_index(size_);
            AllocTraits::construct(alloc_, arr_ + tail, obj);
            ++size_;
        }
        void addFirst(const T& obj) {
            if (size_ == cap_) grow();
            head_ = (head_ == 0) ? cap_ - 1 : head_ - 1;
            AllocTraits::construct(alloc_, arr_ + head_, obj);
            ++size_;
        }
        void add(const T& e) { addLast(e); }
        void addAll(const std::vector<T>& a) { for (const auto& item : a) addLast(item); }

        T pollFirst() {
            if (isEmpty()) throw std::out_of_range("Deque empty");
            T val = std::move(arr_[head_]);
            AllocTraits::destroy(alloc_, arr_ + head_);
            head_ = (head_ + 1) % cap_;
            --size_;
            return val;
        }
        T pollLast() {
            if (isEmpty()) throw std::out_of_range("Deque empty");
            size_t tail = get_physical_index(size_-1);
            T val = std::move(arr_[tail]);
            AllocTraits::destroy(alloc_, arr_ + tail);
            --size_;
            return val;
        }
        T* peekFirst() { return isEmpty() ? nullptr : &arr_[head_]; }
        T* peekLast() { return isEmpty() ? nullptr : &arr_[get_physical_index(size_ - 1)]; }

        bool contains(const T& o) const {
            for (size_t i = 0; i < size_; ++i) if (arr_[get_physical_index(i)] == o) return true;
            return false;
        }
        bool containsAll(const std::vector<T>& a) const {
            for (const auto& item : a) if (!contains(item)) return false;
            return true;
        }
        bool remove(const T& o) { return removeFirstOccurrence(o); }
        void removeAll(const std::vector<T>& a) { for (const auto& item : a) while (removeFirstOccurrence(item)); }
        void retainAll(const std::vector<T>& a) {
            size_t i = 0;
            while (i < size_) {
                const T& curr = arr_[get_physical_index(i)];
                bool found = false;
                for (const auto& keep : a) if (curr == keep) { found = true; break; }
                if (!found) remove_at_logical(i); else ++i;
            }
        }
        std::vector<T> toArray() const {
            std::vector<T> res; res.reserve(size_);
            for (size_t i = 0; i < size_; ++i) res.push_back(arr_[get_physical_index(i)]);
            return res;
        }
        void toArray(std::vector<T>& a) const { a = toArray(); }
        T& element() { return getFirst(); }
        bool offer(const T& obj) { addLast(obj); return true; }
        T* peek() { return peekFirst(); }
        T poll() { return pollFirst(); }
        T& getFirst() { if (isEmpty()) throw std::out_of_range("Deque empty"); return arr_[head_]; }
        T& getLast() { if (isEmpty()) throw std::out_of_range("Deque empty"); return arr_[get_physical_index(size_ - 1)]; }
        bool offerFirst(const T& obj) { addFirst(obj); return true; }
        bool offerLast(const T& obj) { addLast(obj); return true; }
        T pop() { return removeFirst(); }
        void push(const T& obj) { addFirst(obj); }
        T removeLast() { return pollLast(); }
        T removeFirst() { return pollFirst(); }

        bool removeLastOccurrence(const T& obj) {
            if (isEmpty()) return false;
            for (size_t i = size_; i > 0; --i) {
                size_t logical = i - 1;
                if (arr_[get_physical_index(logical)] == obj) { remove_at_logical(logical); return true; }
            }
            return false;
        }
        bool removeFirstOccurrence(const T& obj) {
            for (size_t i = 0; i < size_; ++i) {
                if (arr_[get_physical_index(i)] == obj) { remove_at_logical(i); return true; }
            }
            return false;
        }

        template<bool IsConst> struct DequeIterator {
            using pointer = std::conditional_t<IsConst, const T*, T*>;
            using reference = std::conditional_t<IsConst, const T&, T&>;
            using value_type = T;
            using iterator_category = std::random_access_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using DequePtr = std::conditional_t<IsConst, const deque*, deque*>;
            DequePtr d; size_t idx;
            DequeIterator(DequePtr ptr, size_t i) : d(ptr), idx(i) {}
            reference operator*() const { return d->arr_[d->get_physical_index(idx)]; }
            pointer operator->() const { return &d->arr_[d->get_physical_index(idx)]; }
            DequeIterator& operator++() { ++idx; return *this; }
            DequeIterator operator++(int) { DequeIterator tmp = *this; ++idx; return tmp; }
            DequeIterator& operator--() { --idx; return *this; }
            DequeIterator operator--(int) { DequeIterator tmp = *this; --idx; return tmp; }
            DequeIterator& operator+=(difference_type n) { idx += n; return *this; }
            DequeIterator& operator-=(difference_type n) { idx -= n; return *this; }
            difference_type operator-(const DequeIterator& other) const { return idx - other.idx; }
            DequeIterator operator+(difference_type n) const { return { d, idx + n }; }
            DequeIterator operator-(difference_type n) const { return { d, idx - n }; }
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

    template<class T, class Container = std::vector<T>, class Comparator = std::less<T>>
    class Heap
    {
    private:
        Container data;
        Comparator comp;
        void SiftDown(size_t idx) {
            while (true) {
                size_t largest = idx;
                size_t left = 2 * idx + 1;
                size_t right = 2 * idx + 2;
                if (left < data.size() && comp(data[largest], data[left])) largest = left;
                if (right < data.size() && comp(data[largest], data[right])) largest = right;
                if (largest == idx) return;
                std::swap(data[idx], data[largest]);
                idx = largest;
            }
        }
        void SiftUp(size_t idx) {
            while (idx > 0) {
                size_t parent = (idx - 1) / 2;
                if (!comp(data[parent], data[idx])) return;
                std::swap(data[parent], data[idx]);
                idx = parent;
            }
        }
    public:
        Heap(size_t sz = 10, const Comparator& c = Comparator()) : comp(c) {}
        Heap(const Container& arr) : data(arr) {}
        Heap(Container&& arr) : data(std::move(arr)) {}
        void push(const T& value) noexcept { data.push_back(value); SiftUp(data.size() - 1); }
        void push(T&& value) noexcept { data.push_back(std::move(value)); SiftUp(data.size() - 1); }
        void pop() noexcept {
            data[0] = std::move(data.back());
            data.pop_back();
            if (!empty()) SiftDown(0);
        }
        T pool() {
            if (data.empty()) throw std::out_of_range("Heap empty");
            T val = std::move(data[0]);
            data[0] = std::move(data.back());
            data.pop_back();
            if (!data.empty()) SiftDown(0);
            return val;
        }
        const T& top() const { return data.front(); }
        T& top() noexcept { return data[0]; }
        bool empty() noexcept { return data.empty(); }
        size_t size() const { return data.size(); }
        bool contains(const T& val) noexcept { return std::find(data.begin(), data.end(), val) != data.end(); }
        void clear() noexcept { data.clear(); }
        ~Heap() {
            if constexpr (std::is_pointer<T>::value) {}
        }
    };
}

int priority(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    if (op == '^') return 3;
    return 0;
}
bool isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
}
std::vector<std::string> toExpr(const std::string& expr) {
    std::vector<std::string> value;
    std::string num;
    for (char c : expr) {
        if (std::isdigit(c) || c == '.') num += c;
        else {
            if (!num.empty()) { value.push_back(num); num.clear(); }
            if (isOperator(c) || c == '(' || c == ')') value.push_back(std::string(1, c));
        }
    }
    if (!num.empty()) value.push_back(num);
    return value;
}
std::vector<std::string> toRPN(const std::vector<std::string>& value) {
    std::vector<std::string> output;
    my::Stack<char> ops;
    for (const std::string& t : value) {
        if (std::isdigit(t[0]) || (t.size() > 1 && t[0] == '.')) output.push_back(t);
        else if (t == "(") ops.push('(');
        else if (t == ")") {
            while (!ops.empty() && ops.top() != '(') {
                output.push_back(std::string(1, ops.top()));
                ops.pop();
            }
            if(!ops.empty()) ops.pop();
        }
        else if (isOperator(t[0])) {
            char op = t[0];
            while (!ops.empty() && ops.top() != '(' && priority(ops.top()) >= priority(op)) {
                output.push_back(std::string(1, ops.top()));
                ops.pop();
            }
            ops.push(op);
        }
    }
    while (!ops.empty()) {
        output.push_back(std::string(1, ops.top()));
        ops.pop();
    }
    return output;
}
double calculate(const std::vector<std::string>& rpn) {
    my::Stack<double> st;
    for (const std::string& t : rpn) {
        if (isOperator(t[0]) && t.size() == 1) {
            double b = st.pool();
            double a = st.pool();
            if (t == "+") st.push(a + b);
            else if (t == "-") st.push(a - b);
            else if (t == "*") st.push(a * b);
            else if (t == "/") st.push(a / b);
            else if (t == "^") st.push(std::pow(a, b));
        } else {
            st.push(std::stod(t));
        }
    }
    return st.top();
}
void CurvaPolskaNotation() {
    std::string expr;
    std::cout << "Введите выражение (без пробелов): ";
    std::cin >> expr;
    auto tokens = toExpr(expr);
    auto rpn = toRPN(tokens);
    double result = calculate(rpn);
    std::cout << "Результат: " << result << std::endl;
}

struct Request {
    int id;
    int priority;
    int arrivalStep;
    bool operator<(const Request& other) const {
        return priority < other.priority;
    }
};

void RequestTask() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    int n;
    std::cout << "Введите количество шагов генерации: ";
    if (!(std::cin >> n)) return;

    my::Heap<Request> pq(0);
    std::ofstream logFile("log.txt");

    int globalIdCounter = 1;
    int currentStep = 1;
    Request maxWaitRequest = {0, 0, 0};
    int maxWaitTime = -1;

    for (; currentStep <= n; ++currentStep) {
        int numToGen = std::rand() % 10 + 1;
        for (int i = 0; i < numToGen; ++i) {
            Request req;
            req.id = globalIdCounter++;
            req.priority = std::rand() % 5 + 1;
            req.arrivalStep = currentStep;
            pq.push(req);
            logFile << "ADD " << req.id << " " << req.priority << " " << currentStep << "\n";
        }
        if (!pq.empty()) {
            Request topReq = pq.pool();
            int waitTime = currentStep - topReq.arrivalStep;
            if (waitTime > maxWaitTime) { maxWaitTime = waitTime; maxWaitRequest = topReq; }
            logFile << "REMOVE " << topReq.id << " " << topReq.priority << " " << currentStep << "\n";
        }
    }
    while (!pq.empty()) {
        currentStep++;
        Request topReq = pq.pool();
        int waitTime = currentStep - topReq.arrivalStep;
        if (waitTime > maxWaitTime) { maxWaitTime = waitTime; maxWaitRequest = topReq; }
        logFile << "REMOVE " << topReq.id << " " << topReq.priority << " " << currentStep << "\n";
    }
    logFile.close();
    if (maxWaitTime != -1) {
        std::cout << "ID: " << maxWaitRequest.id << ", Priority: " << maxWaitRequest.priority
            << ", Wait: " << maxWaitTime << "\n";
    }
}

bool isValidPart(const std::string& s) {
    if (s.empty() || s.length() > 3) return false;
    for (char c : s) if (!std::isdigit(c)) return false;
    if (s.length() > 1 && s[0] == '0') return false;
    int val = std::stoi(s);
    return val >= 0 && val <= 255;
}
bool isValidIPv4(const std::string& block) {
    int dotCount = 0;
    for (char c : block) if (c == '.') dotCount++;
    if (dotCount != 3) return false;
    std::string part = "";
    int partsFound = 0;
    for (size_t i = 0; i <= block.length(); ++i) {
        if (i == block.length() || block[i] == '.') {
            if (!isValidPart(part)) return false;
            part = "";
            partsFound++;
        } else { part += block[i]; }
    }
    return partsFound == 4;
}
void vecTask() {
    my::vector<std::string> lines;
    std::ifstream inFile("input.txt");
    if (!inFile) { std::cerr << "No input.txt\n"; return; }
    std::string line;
    while (std::getline(inFile, line)) { lines.push_back(line); }
    inFile.close();

    my::vector<std::string> ips;
    for (size_t k = 0; k < lines.size(); ++k) {
        std::string l = lines[k];
        for (size_t i = 0; i < l.length(); ) {
            if (std::isdigit(l[i]) || l[i] == '.') {
                size_t start = i;
                while (i < l.length() && (std::isdigit(l[i]) || l[i] == '.')) i++;
                std::string block = l.substr(start, i - start);
                if (isValidIPv4(block)) ips.push_back(block);
            } else { i++; }
        }
    }
    std::ofstream outFile("output.txt");
    for (size_t k=0; k<ips.size(); ++k) outFile << ips[k] << "\n";
    outFile.close();
    std::cout << "IPs processed to output.txt\n";
}

size_t digits(const std::string& s) {
    size_t c = 0; for (unsigned char ch : s) if (std::isdigit(ch)) ++c; return c;
}
size_t spaces(const std::string& s) {
    size_t c = 0; for (char ch : s) if (ch == ' ') ++c; return c;
}
void funcDeq() {
    my::deque<std::string> q;
    std::ifstream fin("input.txt");
    if (!fin) { std::cerr << "No input.txt\n"; return; }
    std::string line;
    while (std::getline(fin, line)) {
        if (q.isEmpty()) { q.addLast(line); }
        else {
            size_t d1 = digits(line);
            size_t d0 = digits(*q.peekFirst());
            if (d1 > d0) q.addLast(line); else q.addFirst(line);
        }
    }
    std::ofstream fout("sorted.txt");
    size_t k = q.size();
    for (size_t i = 0; i < k; ++i) {
        std::string t = q.pollFirst();
        fout << t << '\n';
        q.addLast(t);
    }
    fout.close();

    std::cout << "Введите max пробелов n: ";
    size_t n;
    std::cin >> n;
    k = q.size();
    for (size_t i = 0; i < k; ++i) {
        std::string t = q.pollFirst();
        if (spaces(t) <= n) q.addLast(t);
    }
    std::cout << "Оставшиеся строки:\n";
    while (!q.isEmpty()) std::cout << q.pollFirst() << '\n';
}

int main()
{
    while(true) {
        std::cout << "\nВыберите задачу:\n";
        std::cout << "1. Обратная польская запись (Stack)\n";
        std::cout << "2. Приоритетная очередь заявок (Heap)\n";
        std::cout << "3. Поиск IP адресов (Vector)\n";
        std::cout << "4. Сортировка строк (Deque)\n";
        std::cout << "0. Выход\n";
        std::cout << "> ";

        char choice;
        std::cin >> choice;

        if (choice == '0') break;
        switch(choice) {
        case '1': CurvaPolskaNotation(); break;
        case '2': RequestTask(); break;
        case '3': vecTask(); break;
        case '4': funcDeq(); break;
        default: std::cout << "Неверный выбор.\n";
        }
    }
    return 0;
}
```