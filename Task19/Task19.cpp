#include <iostream>

#include <map>

#include <type_traits>

template<typename Key, typename T, class Comparator = std::less<Key>, class Alloc = std::allocator<std::pair<const Key, T>>>
class Map
{
public:
	using value_type = std::pair<const Key, T>;
private:
	struct BaseNode
	{
		BaseNode* left;
		BaseNode* right;
		BaseNode* parent;
		bool red;

		BaseNode(bool isRed = false): left(nullptr), right(nullptr), parent(nullptr), red(isRed) {}
	};
	struct Node : BaseNode
	{
		std::pair<const Key, T> kv;

		Node(std::pair<const Key, T> in) : BaseNode() { kv(in); }
		
		template<typename... Args> 
		Node(Args&&... val): kv(std::forward<Args>(val)...) {}
	};
	
	using AllocTraits = std::allocator_traits<Alloc>;
	using NodeAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
	using BaseNodeAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<BaseNode>;

	using NodeAllocTraits = std::allocator_traits<NodeAlloc>;
	using BaseNodeAllocTraits = std::allocator_traits<BaseNodeAlloc>;

	NodeAlloc alloc_;
	BaseNodeAlloc base_alloc_;

	BaseNode* header_;//parent - root, left - begin(), right - end() - 1

	size_t size_;
	Comparator comp;

	void reset_header()
	{
		header_->parent = nullptr;
		header_->left = header_;
		header_->right = header_;
		size_ = 0;
	}

public:
	Map(const Alloc& alloc = Alloc()) : alloc_(alloc), base_alloc_(alloc)
	{
		header_ = BaseNodeAllocTraits::allocate(base_alloc_, 1);
		BaseNodeAllocTraits::construct(base_alloc_, header_);
		reset_header(); 
	}
	Map(const Map& other) : alloc_(other.alloc_), base_alloc_(alloc), comp(other.comp)
	{
		header_ = BaseNodeAllocTraits::allocate(base_alloc_, 1);
		BaseNodeAllocTraits::construct(base_alloc_, header_);
		reset_header();

		for (auto it = other.begin(); it != other.end(); ++it)
			insert(*it);
	}
	Map(Map&& other) noexcept : alloc_(std::move(other.alloc_)), base_alloc_(std::move(other.alloc_)), comp(std::move(other.comp))
	{
		header_ = BaseNodeAllocTraits::allocate(base_alloc_, 1);
		BaseNodeAllocTraits::construct(base_alloc_, header_);
		reset_header();
		
		if (other.size_ > 0)
		{
			header_->parent = other.header_->parent;
			header_->left = other.header_->left;
			header_->right = other.header_->right;
			size_ = other.size_;

			header_->parent->parent = header_;//don't let the pointer of the root be on deleted tree

			other.reset_header();
		}
	}

	Map& operator=(const Map& other)	
	{
		if (this == &other)return *this;

		clear();
		alloc_ = other.alloc_;
		base_alloc_ = other.base_alloc_;
		comp = other.comp;

		for (auto it = other.begin(); it != other.end(); ++it)
			insert(*it);
		
		return *this;
	}

	Map& operator=(Map&& other) noexcept
	{
		if (this == &other)return *this;

		clear();

		alloc_ = std::move(other.alloc_);
		base_alloc_ = other.base_alloc_;
		comp = other.comp;

		if (other.size_ > 0)
		{
			header_->parent = other.header_->parent;
			header_->left = other.header_->left;
			header_->right = other.header_->right;
			size_ = other.size_;

			header_->parent->parent = header_;

			other.reset_header();
		}
		return *this;
	}

	template<typename... Args>
	Node* allocate_node(Args&&... val) 
	{
		Node* ptr = NodeAllocTraits::allocate(alloc_, 1);
		try {
			NodeAllocTraits::construct(alloc_, ptr, std::forward<Args>(val)...);
		}
		catch (...) 
		{
			NodeAllocTraits::deallocate(alloc_, ptr, 1);
			throw;
		}
		return ptr;
	}

	template<bool isConst>
	class base_iterator
	{
	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = Map::value_type;
		using difference_type = std::ptrdiff_t;
		using pointer = std::conditional_t<isConst, const value*, value*>;
		using reference = std::conditional_t<isConst, const value&, value&>;
	private:
		BaseNode* header_;
		BaseNode* ptr;
	public:
		base_iterator(BaseNode* p, BaseNode* header): header_(header), ptr(p) {}

		base_iterator& operator++()
		{
			if (ptr == header_)return *this;

			if(ptr->right != nullptr)
			{
				ptr = ptr->right;
				while (ptr->left != nullptr)
					ptr = ptr->left;
			}
			else
			{
				while (ptr->parent != header_ && ptr->parent->right == ptr)
					ptr = ptr->parent;
				ptr = ptr->parent;
			}
			return *this;
		}
		base_iterator operator++(int)
		{
			base_iterator copy = *this;
			++(*this);
			return copy;
		}

		base_iterator& operator--()
		{
			if (ptr == header_)
			{
				ptr = header_->right;
			}
			else if (ptr->left != nullptr)
			{
				ptr = ptr->left;
				while (ptr->right != nullptr)
					ptr = ptr->right;
			}
			else
			{
				while (ptr->parent != header_ && ptr->parent->left == ptr)
					ptr = ptr->parent;
				ptr = ptr->parent;
			}
			return *this;
		}
		base_iterator operator--(int)
		{
			base_iterator copy = *this;
			--(*this);
			return copy;
		}

		reference operator*() const { return static_cast<Node*>(ptr)->kv; }
		pointer operator->() const { return &static_cast<Node*>(ptr)->kv; }//Or like this &(*(*this)) | *this - out iterator, * - operator*(),& - adress

		friend bool operator==(const base_iterator& first, const base_iterator& second) { return first.ptr == second.ptr; }
		friend bool operator!=(const base_iterator& first, const base_iterator& second) { return first.ptr != second.ptr; }
	};

	using iterator = base_iterator<false>;
	using const_iterator = base_iterator<true>;
	using reverse_iterator = std::reverse_iterator<iterator>;

	iterator begin()
	{
		return iterator(header_->left, header_);
	}
	const_iterator begin() const
	{
		return const_iterator(header_->left, header_);
	}
	
	const_iterator cbegin() const 
	{ 
		return const_iterator{header_->left, header_}; 
	}
	reverse_iterator rbegin()
	{
		return reverse_iterator(end());
	}

	iterator end()
	{
		return iterator(header_, header_);
	}
	const_iterator end() const
	{
		return const_iterator{ header_, header };
	}
	const const_iterator cend() const 
	{ 
		return const_iterator{ header_, header_ }; 
	}
	reverse_iterator rend()
	{
		return reverse_iterator(begin());
	}

private:
	BaseNode* find_impl(const Key& key) const
	{
		BaseNode* cur = header_->parent;
		while (cur != header_)
		{
			if (comp(key, static_cast<Node*>(cur)->kv.first))
				cur = cur->left;
			else if (comp(static_cast<Node*>(cur)->kv.first, key))
				cur = cur->right;
			else
				return cur;
		}
		return header_;
	}
public:
	iterator find(const Key& key)
	{
		return iterator(find_impl(key), header_);
	}
	const_iterator find(const Key& key) const
	{
		return iterator(find_impl(key), header_);
	}

	friend std::ostream& operator<<(std::ostream& os, const Map& map)
	{
		os << "{ ";
		for (auto it = map.begin(); it != end(); ++it)
			os << it->first << ":" << it->second << ", ";
		os << " }";
		return os;
	}
	
private:
	//Rotations
	void leftRotate(BaseNode* x)
	{
		BaseNode* y = x->right;

		x->right = y->left;
		if (y->left != nullptr)
			y->left->parent = x;

		y->parent = x->parent;
		if (x->parent == header_)header_->parent = y;
		else if (x->parent->left == x)x->parent->left = y;
		else x->parent->right = y;

		y->left = x;
		x->parent = y;
	}
	void rightRotate(BaseNode* x)
	{
		BaseNode* y = x->left;

		x->left = y->right;
		if (y->right != nullptr)
			y->right->parent = x;

		y->parent = x->parent;
		if (x->parent == header_)header_->parent = y;
		else if (x->parent->left == x)x->parent->left = y;
		else x->parent->right = y;

		y->right = x;
		x->parent = y;
	}

	bool is_red(BaseNode* n)
	{
		return n != nullptr && n->red;
	}
	void InsertFix(BaseNode* t) 
	{
		if (t == header_->parent)
		{
			t->red = false;
			return;
		}

		while (t->parent != header_ && t->parent->red	)
		{
			//Родитель - левый
			if (t->parent->parent->left == t->parent)
			{
				BaseNode* uncle = t->parent->parent->right;
				if (uncle != nullptr && uncle->red == true)
				{
					t->parent->red = false;
					t->parent->parent->red = true;
					uncle->red = false;
					t = t->parent->parent;
				}
				else
				{
					if (t == t->parent->right)
					{
						t = t->parent;
						leftRotate(t);
					}
					t->parent->red = false;
					t->parent->parent->red = true;
					rightRotate(t->parent->parent);
				}
			}
			else
			{
				BaseNode* uncle = t->parent->parent->left;
				if (uncle != nullptr && uncle->red == true)
				{
					t->parent->red = false;
					t->parent->parent->red = true;
					uncle->red = false;
					t = t->parent->parent;
				}
				else
				{
					if (t->parent->left == t)
					{
						t = t->parent;
						rightRotate(t);
					}
					t->parent->red = false;
					t->parent->parent->red = true;
					leftRotate(t->parent->parent);
				}
			}
		}
		header_->parent->red = false;
	}
	void EraseFix(BaseNode* t) 
	{
		if (t == nullptr)return;

		while (t != header_->parent && !is_red(t))
		{
			if (t->parent->left == t)
			{
				BaseNode* brother = t->parent->right;

				//First case - red brother
				if (is_red(brother))
				{
					brother->red = false;
					t->parent->red = true;
					leftRotate(t->parent);
					brother = t->parent->right;
				}
				
				if (!is_red(brother->left) && !is_red(brother->right))
				{
					brother->red = true;
					t = t->parent;
				}
				else
				{
					if (!is_red(brother->right))//Making this case to go to the next case
					{
						brother->left->red = false;
						brother->red = true;
						rightRotate(brother);
						brother = t->parent->right;
					}
					brother->red = t->parent->red;
					brother->right->red = false;
					t->parent->red = false;
					leftRotate(t->parent);

					break;
				}
			}
			else
			{
				BaseNode* brother = t->parent->left;

				if (is_red(brother))
				{
					brother->red = false;
					t->parent->red = true;
					rightRotate(t->parent);
					brother = t->parent->left;
				}

				if (!is_red(brother->left) && !is_red(brother->right))
				{
					brother->red = true;
					t = t->parent;
				}
				else
				{
					if (!is_red(brother->left))
					{
						brother->right->red = false;
						brother->red = true;
						leftRotate(brother);
						brother = t->parent->left;
					}
					brother->left->red = false;
					brother->red = t->parent->red;
					t->parent->red = false;
					rightRotate(t->parent);

					break;
				}
			}
		}
		t->red = false;
	}
public:
	//For value&& and const value&
	template<typename U>
	std::pair<iterator, bool> insert(U&& value)
	{
		BaseNode* parent = header_;
		BaseNode* cur = header_->parent;
		bool go_left = false;

		while (cur != nullptr)
		{
			parent = cur;
			if (comp(value.first, static_cast<Node*>(cur)->kv.first))
			{
				cur = cur->left;
				go_left = true;
			}
			else if (comp(static_cast<Node*>(cur)->kv.first, value.first))
			{
				cur = cur->right;
				go_left = false;
			}
			else
			{
				return {iterator(cur, header_), false};
			}
		}

		Node* new_node = allocate_node(std::forward<U>(value));
		new_node->parent = parent;
		new_node->left = nullptr;
		new_node->right = nullptr;
		new_node->red = true;

		if (parent == header_)
		{
			header_->parent = new_node;
		}
		else if (go_left)
		{
			parent->left = new_node;
		}
		else
		{
			parent->right = new_node;
		}

		if (header_->left == header_ || comp(new_node->kv.first, static_cast<Node*>(header_->left)->kv.first))
			header_->left = new_node;

		if (header_->right == header_ || comp(static_cast<Node*>(header_->right)->kv.first, new_node->kv.first))
			header_->right = new_node;

		InsertFix(new_node);
		++size_;

		return {iterator(new_node, header_), true};
	}

	template<typename... Args>
	std::pair<iterator, bool> try_emplace(const Key& k, Args&&... args) 
	{
		BaseNode* parent;
		BaseNode* cur = header_->parent;

		while (cur != nullptr)
		{
			if (comp(k, static_cast<Node*>(cur)))
			{
				parent = cur; go_left = true;
			}
			else if (comp(static_cast<Node*>(cur), k))
			{
				parent = cur; go_left = false;
				cur = cur->right;
			}
			else
			{
				return {iterator(cur), false};
			}
		}

		Node* new_node = allocate_node(
		std::piecewise_construct,
		std::forward_as_tuple(k),
		std::forward_as_tuple(std::forward<Args>(args)...)
		);
		
		new_node->parent = parent;
		new_node->left = nullptr;
		new_node->right = nullptr;
		new_node->red = true;

		if (parent == header_)
		{
			header_->left = new_node;
		}
		else if (go_left)
		{
			parent->left = new_node;
		}
		else
		{
			parent->right = new_node;
		}

		if (header_->left == header_ || comp(new_node->kv.first, static_cast<Node*>(header_->left)))
			header_->left = new_node;
		if(header_->right == header || comp(static_cast<Node*>(header_->right), new_node->kv.first))
			header_->right = new_node;

		InsertFix(new_node);

		return { iterator(new_node), true };
	}
	T& operator[](const Key& k)
	{
		BaseNode* parent = header_;
		BaseNode* cur = header_->parent;

		while (cur != nullptr)
		{
			if (comp(k, static_cast<Node*>(cur)->kv.first))
				cur = cur->left;
			else if(comp(static_cast<Node*>(cur)->kv.first, k))
				cur = cur->right;
			else
				return static_cast<Node*>(cur)->kv.second;
		}

		Node* new_node = allocate_node(
		std::piecewise_construct,
		std::forward_as_tuple(k),
		std::forward_as_tuple(T())
		);

		new_node->parent = parent;
		new_node->left = nullptr;
		new_node->right = nullptr;
		new_node->red = true;
		
		if (parent == header_)
			new_node->parent = header_->parent;
		else if (parent->left == new_node)
			parent->left = new_node;
		else
			parent->right = new_node;

		if (header_->left == header_ || comp(new_node->kv.first, static_cast<Node*>(header_->left)->kv.first))
			header_->left = new_node;
		if (header_->right = header_ || comp(static_cast<Node*>(header_->right)->kv.first, new_node->kv.first))
			header_->right = new_node;
	}
	T& operator[](Key&& k)
	{
		BaseNode* parent = header_;
		BaseNode* cur = header_->parent;
		bool go_left = false;

		while (cur != nullptr)
		{
			if (comp(k, cur->kv.first))
			{
				parent = cur;
				cur = cur->left;
				go_left = true;
			}
			else if(comp(cur->kv.first, k))
			{
				parent = cur;
				cur = cur->right;
				go_left = false;
			}
			else
			{
				return cur->kv.second;
			}
		}

		Node* new_node = allocate_node(
			std::piecewise_construct,//Говорим компилятору, что будем упаковывать элементы в кортеж
			std::forward_as_tuple(std::forward<Key>(k)),
			std::forward_as_tuple()
		);

		new_node->parent = parent;
		new_node->left = nullptr;
		new_node->right = nullptr;

		if (go_left)
		{
			parent->left = new_node;
		}
		else
		{
			parent->right = new_node;
		}
		InsertFix(new_node);

		++size_;
		return new_node->kv.second;
	}
	
	template<typename U>
	bool contains(const U& value) const
	{
		Node* cur = begin;

		while (cur != nullptr)
		{
			if (comp(value.first, cur->kv.first))
				cur = cur->left;
			else if (comp(cur->kv.first, value.first))
				cur = cur->right;
			else
				return true;
		}
		return false;
	}

	template<typename... Args>
	bool containsAll(Args&&... args) const
	{
		return (contains(std::forward<Args>(args)) && ...);
	}
	template<typename InputIt>
	bool containsAll(InputIt& first, InputIt& last) const
	{
		for (; first != last; ++first)
			if (!contains(*first))
				return false;
		return true;
	}
	template<typename Range>
	bool containsAll(const Range& range) const
	{
		for (const auto& x : range)
			if (!contains(x))
				return false;
		return true;
	}

	bool empty() const { return size_ == 0; }
	size_t size() const { return size_; }

	//Доделать
	std::pair<iterator, bool> erase(const Key& k) 
	{
		Node* cur = begin();
		while(cur != nullptr)
		{
			if (comp(k, cur->kv.first)) cur = cur->left;
			else if (comp(cur->kv.first, k)) cur = cur->right;
			else break;
		}
		if (cur == nullptr)return { end(), false };

		Node* del_node = cur;
		Node* x = nullptr;
		bool orig_is_red;

		auto transplant = [&](Node* u, Node* v) {
			if (u->parent == nullptr)begin = v;
			else if (u->parent->left == u)u->parent->left = v;
			else u->parent->right = v;

			if (v != nullptr)v->parent = u->parent;
			};

		if(del_node->left == nullptr)
		{
			x = del_node->right;
			transplant(del_node, x);
		}
		else if(del_node->right == nullptr)
		{
			x = del_node->left;
			transplant(del_node, x);
		}
		else
		{
			x = del_node->right;
			while (x->left != nullptr)
				x = x->left;

			orig_is_red = x->red;
			Node* child = x->right;

			if (x->parent != del_node)
			{
				transplant(x, child);
				x->right = del_node->right;
				x->right->parent = x;
			}
			else if (child != nullptr)
			{
				child->parent = x;
			}
			transplant(del_node, x);
			x->left = del_node->left;
			x->left->parent = x;
			x->color = del_node->color;
		}

		if(del_node == nullptr || !del_node->color)
		{
			EraseFix(x);
		}

		NodeAllocTraits::destroy(alloc_, del_node);
		NodeAllocTraits::deallocate(alloc_, del_node, 1);

		return { iterator(), true};
	}

	template<typename... Args>
	size_t erase(Args&&... args)
	{
		return ((erase(std::forward<Args>(args)).second ? 1 : 0) + ...);
	}
	template<typename Range>
	size_t erase(const Range& range)
	{
		for (const auto& x : range)
			erase(x);
		return size_;
	}
	template<typename InputIt>
	size_t erase(InputIt first, InputIt last)
	{
		size_t cnt = 0;
		for (; first != last; ++first)
		{
			if (erase(*first).second)
				++cnt;
		}
		return cnt;
	}
	iterator erase(iterator first, iterator last)
	{
		while (first != last)
			first = erase(first);
		return last;
	}

	BaseNode* lower_bound(const Key& key)
	{
		BaseNode* cur = header_->parent;
		BaseNode* start_node = header_;
		while (cur != nullptr)
		{
			if (comp(key, cur->kv.first))
			{
				cur = cur->right;
			}
			else
			{
				start_node = cur;
				cur = cur->left;
			}
		}
		return start_node;
	}
	BaseNode* upper_bound(const Key& key)
	{
		BaseNode* cur = header_->parent;
		BaseNode* start_node = header_;
		while (cur != nullptr)
		{
			if (comp(key, static_cast<Node*>(cur)->kv.first))
			{
				start_node = cur;
				cur = cur->right;
			}
			else
			{
				cur = cur->left;
			}
		}
		return start_node;
	}
	void clear()
	{
		BaseNode* cur = header_->parent;
		while (cur != &header_ && cur != nullptr)
		{
			if (cur->left != nullptr)
			{
				cur = cur->left;
			}
			else if (cur->right != nullptr)
			{
				cur = cur->right;
			}
			else
			{
				BaseNode* parent = cur->parent;

				if (parent != header_)
				{
					if (parent->left == cur)parent->left = nullptr;
					else parent->right = nullptr;
				}

				Node* node_to_delete = static_cast<Node*>(cur);

				NodeAllocTraits::destroy(alloc_, node_to_delete);
				NodeAllocTraits::deallocate(alloc_, node_to_delete, 1);

				cur = parent;
			}
		}
		reset_header();
	}

	template<typename Container>
	Container subSet(iterator first, iterator last) const
	{
		return Container(first, last);
	}

	template<typename Container>
	Container headSet(iterator head) const
	{
		return Container(begin, head);
	}
	template<typename Container>
	Container headSet(const Key& key) const
	{
		Container tmp;

		auto it = begin();
		for (auto other = upper_boud(key);it != other; ++it)
			tmp.insert(tmp.end(), *it);
		tmp.insert(tmp.end(), *(++it));

		return tmp;
	}

	template<typename Container>
	Container tailSet(iterator tail) const
	{
		return Container(tail, end());
	}
	template<typename Container>
	Container tailSet(const Key& key)
	{
		Container tmp;

		for (auto it = lower_bound(key); it != end(); ++it)
			tmp.insert(tmp.end(), *it);

		return tmp;
	}

	template<typename Container>
	Container descending_set()
	{
		Container tmp;
		for (auto it = rbegin(); it != rend(); ++it)
			tmp.insert(tmp.end(), *it);
		return tmp;
	}
};

int main()
{
	Map<int, int> mp;
	std::map<int, int> m;
	m.insert(5, 3);
	std::cout << mp;
	mp.rbegin();
}