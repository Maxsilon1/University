#include <iostream>

#include <map>
#include <type_traits>

template<typename Key, typename T, class Alloc = std::allocator<T>, class Comparator = std::less<Key>>//Comparator(F, S) == true -> F < S
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

		Node(std::pair<const Key, T> in) : BaseNode() { kv{ in }; }
		
		template<typename... Args> 
		Node(Args&&... val): kv(std::forward<Args>(val)...) {}
	};
	
	using AllocTraits = std::allocator_traits<Alloc>;
	using NodeAlloc = std::allocator_traits<Alloc>::template rebind_alloc<Node>;
	using BaseNodeAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<BaseNode>;

	using NodeAllocTraits = std::allocator_traits<NodeAlloc>;
	using BaseNodeAllocTraits = std::allocator_traits<BaseNodeAlloc>;

	NodeAlloc _alloc;
	BaseNodeAlloc _base_alloc;

	BaseNode* begin;
	BaseNode* fakeNode_;

	size_t sz_;
	Comparator comp;
public:
	Map(const Alloc& alloc = Alloc()) : _alloc(alloc) {}

	//Привести в нормальную форму
	Map(const Map& other) = default;
	Map(Map&& other)
	{

	}
	Map& operator = (const Map& other)
	{

	}
	Map& operator=(Map&& other)
	{
		 
	}

	template<typename... Args>
	Node* allocate_node(Args&&... val) 
	{
		Node* ptr = NodeAllocTraits::alloc(alloc_, 1);
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
	private:
		using dif_type = std::ptrdiff_t;
		using ptr_type = std::conditional_t<isConst, const BaseNode*, BaseNode*>;
		using ref_type = std::conditional_t<isConst, const BaseNode&, BaseNode&>;

		BaseNode* ptr;
		
	public:

		base_iterator& operator++() {}
		
		base_iterator& operator+=(dif_type& other) {}
		base_iterator& operator-=(dif_type& other) {}

		friend base_iterator& operator+(base_iterator& it, dif_type other) {}
		friend base_iterator& operator+(dif_type other, base_iterator& it) {}

		friend base_iterator& operator-(base_iterator& it, dif_type other) {}
		friend base_iterator& operator-(dif_type other, base_iterator& it) {}


	};

	using iterator = base_iterator<false>;
	using const_iterator = base_iterator<true>;

	iterator& find(const Key& key)
	{}
	const_iterator& find(const Key& key) const {}
	
	

	//Rotations
	void LL(Node* t)//<-<- 
	{}
	void RR(Node* t)//->-> 
	{}
	void leftRotate(Node* t)//<-|->
	{}
	void rightRotate(Node* t)//->|<- 
	{}
	//Fix RB tree
	void InsertFix(Node* t) 
	{
		if (t == begin)
		{
			t->red = false;
			return;
		}

		while (t->parent->red == true && t->parent != begin)
		{
			//Родитель - левый
			if (t->parent->parent->left == t->parent)
			{
				Node* uncle = t->parent->parent->right;
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
				Node* uncle = t->parent->parent->left;
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
		begin->red = false;
	}
	void EraseFix() {}
	//- - - - - -
private:
	
public:
	template<typename P>
	std::pair<iterator<const Key, P>, bool> insert(P&& val)
	{
		Node* parent;
		Node* cur = begin;
		bool go_left;
		while (cur != nullptr)
		{
			parent = cur;
			if (comp(cur->kv.first, key))
			{
				cur = cur->left;
				go_left = true;
			}
			else if (comp(key, cur->kv.first))
			{
				cur = cur->right;
				go_left = false;
			}
			else
			{
				return {iterator(cur), false};
			}
		}

		Node* new_node = allocate_node(std::forward<P>(val));
		
		new_node->parent = parent;
		if (parent == nullptr)
		{
			begin = new_node;
		}
		else if (go_left)
		{
			parent->left = new_node;
		}
		else
		{
			parent->right = new_node;
		}

		new_node->red = true;
		InsertFix(new_node);
		++size_;

		return {new_node, true};
	}
	template<typename... Args>
	std::pair<iterator, bool> try_emplace(const Key& k, Args&&... args) 
	{
		Node* parent;
		Node* cur = begin;
		bool go_left;

		while (cur != nullptr)
		{
			if (comp(k, cur))
			{
				parent = cur; go_left = true;
				cur = cur->left;
			}
			else if (comp(cur, k))
			{
				parent = cur; go_left = false;
				cur = cur->right;
			}
			else
			{
				return {iterator(cur), false};
			}
		}

		Node* new_node = allocate_node(std::forward<Args>(args)...);
		
		new_node->parent = parent;
		if (parent == fakeNode_)
		{
			begin = new_node;
		}
		else if (go_left)
		{
			parent->left = new_node;
		}
		else
		{
			parent->right = new_node;
		}

		InsertFix(new_node);

		return { iterator(new_node), true };
	}
	T& operator[](Key&& k) const
	{
		Node* parent;
		Node* cur = begin;
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
			std::forward_as_tuple(std::forward<K>(k)),
			std::forward_as_tuple()
		);

		new_node->parent = parent;
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

	std::pair<iterator, bool> erase(const Key& k) 
	{
		Node* cur = begin;

		Node* parent;
		while (cur->kv.first != k)
		{
			parent = cur;
			if (cur == nullptr)
				return {iterator(), false};
			else if (comp(cur, k))
			{
				cur = cur->left;
			}
			else
			{
				cur = cur->right;
			}
		}

		if (t->right == nullptr && t->left == nullptr)
		{

		}
		else if ((t->right == nullptr && t->left != nullptr) || (t->right != nullptr && t->left == nullptr))
		{

		}
		else
		{
			cur = cur->right;
			while (cur->left != nullptr)
				cur = cur->left;
			t->kv = std::move(cur->kv);

			NodeAllocTraits::destroy(alloc_, cur);
			NodeAllocTraits::deallocate(cur, 1);
		}
		EraseFix(t);
	}
};

int main()
{
	
}