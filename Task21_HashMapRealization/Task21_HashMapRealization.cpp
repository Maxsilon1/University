#include <iostream>
#include <unordered_map>

template<typename Key, typename Value,  class Hasher = std::hash<Key>, class Alloc = std::allocator<std::pair<const Key, Value>>>
class HashMap
{
public:
	using value_type = std::pair<const Key, Value>;
private:
	struct BaseNode
	{
		BaseNode* next_;//In buckets it pointed to ptr before begin

		BaseNode() : next_(nullptr) {}
		BaseNode(BaseNode* next) : next_(next) {}
	};

	struct Node : BaseNode
	{
		value_type data;
		size_t hash_;

		Node(BaseNode* next, size_t hash, const Key& key, const Value& value) : BaseNode(next), hash_(hash_), data(key, value) {}
	};
	
	using NodeAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
	using BaseNodeAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<BaseNode>;

	using AllocTraits = std::allocator_traits<Alloc>;
	using NodeAllocTraits = std::allocator_traits<NodeAlloc>;
	using BaseNodeAllocTraits = std::allocator_traits<BaseNodeAlloc>;

	Alloc alloc_;
	NodeAlloc node_alloc_;
	BaseNodeAlloc base_node_alloc_;

	BaseNode* buckets_;
	BaseNode before_begin_;//pointer to 0 bucket
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
			NodeAllocTraits::deallocate(node_alloc_, new_node, 1);
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
		base_iterator(BaseNode* ptr): ptr(ptr) {}
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

		Node* get_ptr() { return static_cast<Node*>(ptr); }
	};

	using iterator = base_iterator<false>;
	using const_iterator = base_iterator<true>;

	iterator begin() { return iterator{ before_begin_, before_begin_.next_ };}
	const_iterator begin() const {return const_iterator{ before_begin_, before_begin_.next_ }; }
	const_iterator cbegin() const { return const_iterator{ before_begin_, before_begin_.next_ }; }

	iterator end() { return iterator{ nullptr }; }
	const_iterator end() const { return const_iterator{ nullptr }; }
	const_iterator cend() const { return const_iterator{ nullptr }; }

	size_t getBucketIndex(const Key& key) const
	{
		return hasher_(key) % buckets_count_;
	}

	void rehash(size_t new_count_)
	{
		if (new_count_ <= buckets_count_)return;

		BaseNode* new_buckets = BaseNodeAllocTraits::allocate(base_node_alloc_, new_count_);

		BaseNode* cur = before_begin_.next_;
		before_begin_.next_ = nullptr;
		
		while (cur != nullptr)
		{
			BaseNode* next_node = cur->next_;

			Node* node = static_cast<Node*>(cur);
			size_t new_idx = node->hash_ % new_count_;

			node->next_= new_buckets[new_idx].next_;
			new_buckets[new_idx].next_ = node;

			cur = next_node;
		}

		BaseNode* tail = &before_begin_;
		for (size_t i = 0; i < new_count_; ++i)
		{
			if (new_buckets[i].next_ != nullptr)
			{
				tail->next_ = new_buckets[i].next_;

				while (tail->next_ != nullptr)
				{
					Node* next_node = static_cast<Node*>(tail->next_);
					if (next_node->hash_ % new_count_ != i)
						break;
					tail = tail->next_;
				}
			}
		}
		
		BaseNodeAllocTraits::destroy(base_node_alloc_, buckets_);
		buckets_ = new_buckets;
		buckets_count_ = new_count_;
	}

public:
	HashMap():buckets_(nullptr), size_(0), buckets_count_(0), hasher_() {}

	void insert(const Key& key, const Value& val)
	{
		size_t hash = hasher_(key);
		BaseNode* node_before = &buckets_[hash % buckets_count_];

		Node* new_node = alloc_node(
			std::piecewise_construct,
			std::forward_as_tuple(key),
			std::forward_as_tuple(val)
		);;
		new_node->next_ = node_before->next_;
		node_before->next_ = new_node;
		new_node->hash_ = hash;
		
		if (static_cast<float>(size_) / static_cast<float>(buckets_count_) > max_load_factor_)
		{
			rehash(buckets_count_ * 2);//I need to add more cooler ver of this maybe later
		}
	}
	void insert(const std::pair<Key, Value>& val)
	{
		insert(val.first, val.second);
	}
	void insert(std::pair<Key, Value>&& val)
	{
		size_t hash = hasher_(val.first);
		BaseNode* node_before = buckets_[hash % buckets_count_];
		
		Node* new_node = alloc_node(
			std::piecewise_construct,
			std::forward_as_tuple(val.first),
			std::forward_as_tuple(val.second)
		);

		new_node->next_ = node_before->next_;
		node_before->next_ = new_node;
		new_node->hash_ = hash;

		++size_;
		if (static_cast<float>(size_) / static_cast<float>(buckets_count_) > max_load_factor_)
		{
			rehash(2 * buckets_count_);
		}
	}

	void erase(const Key& key) 
	{
		size_t hash = hasher_(key);
		BaseNode* cur = &buckets_[hash % buckets_count_];
		bool outOfBucket = false;
		while (cur->next_ != nullptr)
		{
			Node* next = static_cast<Node*>(cur->next_);
			if (next->hash_ % buckets_count_ != hash % buckets_count_) {
				outOfBucket = true; break;
			}
			if (next->data.first == key) break;
			cur = cur->next_;
		}
		if (cur == nullptr || outOfBucket)return;

		Node* node_to_del = static_cast<Node*>(cur->next_);
		cur->next_ = cur->next_->next_;

		NodeAllocTraits::destroy(node_alloc_, node_to_del);
		NodeAllocTraits::deallocate(node_alloc_, node_to_del, 1);
		--size_;
	}
	//Return next iterator after delete
	iterator erase(iterator it)
	{
		BaseNode* cur = it.get_ptr();
		for(;iterator(cur->next_) != it; cur = cur->next_)
		{}

		Node* node_to_del = static_cast<Node*>(cur->next_);
		cur->next_ = cur->next_->next_;

		NodeAllocTraits::destroy(node_alloc_, node_to_del);
		NodeAllocTraits::deallocate(node_alloc_, node_to_del, 1);
		--size_;
	}
	void erase(iterator first, iterator last) 
	{
		while (first != last)
		{
			first = erase(first);
		}
	}

	bool contains(const Key& key) 
	{
		return buckets_[hasher_(key)] != nullptr;
	}

	void clear()
	{
		
		for (size_t i = 0; i < buckets_count_; ++i)
		{
			BaseNode* cur = buckets_[i].next_;
			while (cur)
			{
				cur = cur->next_;
				Node* n = static_cast<Node*>(cur);
				NodeAllocTraits::destroy(node_alloc_, n);
				NodeAllocTraits::deallocate(node_alloc_, n, 1);
			}
			buckets_[i]->next_ = nullptr;
		}
		size_ = 0;
	}

	template<typename Container>
	Container entrySet()
	{
		Container out(begin(), end());
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
		
		BaseNode* cur = buckets_[idx].next_;

		while (cur != nullptr && static_cast<Node*>(cur)->hash_ % buckets_count_ == idx
			&& static_cast<Node*>(cur)->data.first != key)
		{
			cur = cur->next_;
		}

		if (cur != nullptr && static_cast<Node*>(cur)->hash_ % buckets_count_ == idx &&
			static_cast<Node*>(cur)->data.first == key)
			return { true, iterator{cur} };

		return {false, end()};
	}
};

class Hash {
public:
	size_t operator()(const std::string& msg) const
	{
		long long m = 31, n = 1'000'000'000, val = msg[0] * m;
		for (size_t i = 1; i < msg.length(); ++i)
			val = (val * m + msg[i] * m) % n;
		return static_cast<size_t>(val);
	}
};

int main()
{
	HashMap<int, int> hp;
	hp.insert(1, 5);
	hp.insert(2, 7);
	hp.insert(3, 6);
	hp.erase(1);
	hp.erase(2);
	std::cout << hp.get(3).first << ' ' << hp.get(2).first;
}