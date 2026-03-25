#include <iostream>
#include <unordered_map>

template<typename Key, typename Value,  class Hasher, class Alloc = std::allocator<std::pair<const Key, Value>>>
class HashMap
{
public:
	using value_type = std::pair<const Key, Value>;
private:
	struct BaseNode
	{
		BaseNode* next_;//In buckets it pointed to ptr before begin

		BaseNode(BaseNode* next) : next_(next) {}
	};

	struct Node : BaseNode
	{
		value_type data;
		size_t hash_;

		Node(BaseNode* next, size_t hash, const Key& key, const Value& value) : BaseNode(next), hash_(hash_), data(key, value) {}
	};
	
	using NodePtr = typename std::allocator_traits<Alloc>::template rebind_alloc<Node*>;
	using NodeAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
	using BaseNodePtr = typename std::allocator_traits<Alloc>::template rebind_alloc<BaseNode*>;
	using BaseNodeAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Node*>;

	using AllocTraits = std::allocator_traits<Alloc>;
	using NodePtrTraits = std::allocator_traits<NodePtr>;
	using NodeAllocTraits = std::allocator_traits<NodeAlloc>;
	using BaseNodePtrTraits = std::allocator_traits<BaseNodePtr>;
	using BaseNodeAllocTraits = std::allocator_traits<BaseNodeAlloc>;

	Alloc alloc_;
	NodePtr ptr_node_alloc_;
	NodeAlloc node_alloc_;
	BaseNodePtr ptr_base_node_alloc_;
	BaseNodeAlloc base_node_alloc_;

	BaseNode* buckets_;
	BaseNode* before_begin_;
	Hasher hasher_;

	size_t size_;
	size_t buckets_count_;
	float max_load_factor_ = 0.75f;

	template<typename... Args>
	Node* alloc_node(Args&&... args)
	{
		Node* new_node = NodeAllocTraits::allocate(node_alloc_, 1);
		try
		{
			NodeAllocTraits::construct(node_alloc_, new_node, std::forward<Args>(args)...);
		}
		catch (...)
		{
			NodeAllocTraits::deallocate(new_node, 1);
			throw;
		}
		return new_node;
	}

	template<bool isConst>
	class base_iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = HashMap::value_type;
		using difference_type = std::ptrdiff_t;
		using pointer_type = std::conditional_t<isConst, const value_type*, value_type*>;
		using reference_type = std::conditional_t < isConst, const value_type&, value_type&>;
	private:
		BaseNode* ptr;
	public:
		base_iterator(Node* ptr): ptr(ptr) {}
		base_iterator(const base_iterator& other): ptr(other.ptr) {}

		base_iterator& operator++()
		{
			ptr = ptr->next_;
			return *this;
		}
		base_iterator& operator++(int)
		{
			base_iterator tmp(ptr);
			ptr = ptr->next_;
			return tmp;
		}

		reference_type operator*() { return static_cast<Node*>(ptr)->data; }
		pointer_type operator->() { return &(static_cast<Node*>(ptr)->data); }

		friend bool operator==(const base_iterator& first, const base_iterator& second) { return first.ptr == second.ptr; }
		friend bool operator!=(const base_iterator& first, const base_iterator& second) { return first.ptr != second.ptr; }

		Node* get() { return static_cast<Node*>(ptr); }
	};

	using iterator = base_iterator<false>;
	using const_iterator = base_iterator<true>;

	size_t getBucketIndex(const Key& key) const
	{
		return hasher_(key) % buckets_count_;
	}

	void rehash(size_t new_count_)
	{
		if (new_count_ <= buckets_count_)return;

		BaseNode* new_buckets = BaseNodeAllocTraits(base_node_alloc, new_count_);

		BaseNode* cur = before_begin_->next_;
		before_begin_->next_ = nullptr;
		
		while (cur != nullptr)
		{
			Node* next_node = cur->next_;

			Node* node = static_cast<Node*>(current);
			size_t new_idx = node->hash_ % buckets_count_;

			node_->next = new_bucket[new_idx];
			new_buckets[new_idx] = node_;

			cur = next_node;
		}
		BaseNodeAlloc::destroy(buckets_);
		buckets_ = new_buckets;
		buckets_count_ = new_count_;
	}

public:
	void insert(const Key& key, const Value& val)
	{
		size_t hash = hasher_(key);
		BaseNode* before_first_ = buckets_[(hash % buckets_count_)];
		before_first_->next_->hash_ = hash;

		Node* new_node = alloc_node(
			std::piecewise_construct,
			std::forward_as_tuple(key),
			std::forward_as_tuple(val)
		);;
		new_node->next = before_first_->next;
		before_first_->next_ = new_node;
		
		if (size_ / buckets_ > max_load_factor_)
		{
			rehash(buckets_count_ * 2);//I need to add more cooler ver of this maybe later
		}
	}
	void insert(const std::pair<Key, Value>& val)
	{
		size_t idx;
		try
		{
			idx = hasher_(val.first);
		}
		catch (...)
		{
			throw;
		}
		
		Node* cur = buckets_[idx];
		if (cur == nullptr)
			++buckets_count_;
		else
			while (cur != nullptr)
				cur = cur->next;
		
		cur = alloc_node(
			std::piecewise_construct,
			std::forward_as_tuple(val.first),
			std::forward_as_tuple(val.second)
		);

		++size_;
		if (size_ / buckets_count_ > max_load_factor_)
		{
			rehash(2 * size_);
		}
	}
	void insert(std::pair<Key, Value>&& val)
	{
		size_t idx;
		try {
			idx = hasher_(val.first);
		}
		catch (...)
		{
			throw;
		}

		Node* cur = buckets_[idx];
		if (cur == nullptr)
			++buckets_count_;
		else
			while (cur != nullptr)
				cur = cur->next;

		cur = alloc_node(
			std::piecewise_construct,
			std::forward_as_tuple(val.first),
			std::forward_as_tuple(val.second)
		);

		++size_;
		if (size_ / buckets_count_ > max_load_factor_)
		{
			rehash(2 * size_);
		}
	}
	
	Value& operator[](const Key& key)
	{
		size_t hash = hasher_(key);
		size_t idx = hash % buckets_;


	}

	void erase(const Key& key) {}
	void erase(iterator first)
	{

	}
	void erase(iterator first, iterator last) {}

	bool contains(const Key& key) 
	{
		return buckets_[hasher_(key)]->next_ != nullptr;
	}

	void clear()
	{
		Node* prev;
		for (auto it = this->begin(); it != this->end(); ++it)
		{
			NodeAllocTraits::destroy(node_alloc_, prev);
			prev = static_cast<Node*>(it.get());
		}
		NodeAllocTraits::destroy(node_alloc_, prev);

		BaseNodeAllocTraits::deallocate(base_node_alloc_, size_);
	}

	template<typename Container>
	Container entrySet()
	{
		Container out(it.begin(), it.end());
		return out;
	}
	template<typename Container>
	Container keySet()
	{
		Container out;
		for (iterator it = begin(); it != end(); ++it)
			out.insert(out.end(), *it.first);
	}

	size_t buckets_count() { return buckets_count_; }
	size_t size() { return size_; }

	bool isEmpty() { return size_ == 0; }

	std::pair<bool, iterator> get(const Key& key)
	{
		size_t hash = hasher_(key);
		size_t idx = hash % buckets_count_;
		if (buckets_[idx]->next_ != nullptr)
		{
			BaseNode* cur = buckets_[]->next_;
			while (cur != nullptr || static_cast<Node*>(cur)->hash_ == idx || static_cast<Node*>(cur)->first != key)
			{
				cur = cur->next_;
			}
			if (static_cast<Node*>(cur)->first == key)
			{
				return {true, iterator{cur}};
			}
		}
		return {false, iterator{end()}};
			
	}
};

int main()
{
	std::unordered_map<int, int> um;
	auto it = um.begin();
	auto it1 = um.erase(it);
	std::cout << it1->first;
}