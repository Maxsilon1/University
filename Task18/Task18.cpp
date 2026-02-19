#include <iostream>

#include <vector>
#include <map>

template<typename K, typename T, class Alloc = std::allocator<T>, class Comparator = std::less<K>>
class BSTMap
{
private:
	struct Node
	{
		const K Key;
		T Val;

		Node* parent;
		Node* left;
		Node* right;
		
		Node(Node* l, Node* r) : left(l), right(r) {}
		~Node()
		{
			~left();
			delete left;
			~right();
			delete right;
		}
	};

	using AllocTraits = std::allocator_traits <Alloc>;
	using NodeAllocTraits = typename AllocTraits::template rebind_alloc<Node>;

	using difference_type = std::ptrdiff_t;

	Alloc Alloc_;
	NodeAllocTraits nodeAlloc_;

	Node* root_;
	size_t size_;

	Comparator comp;
public:
	BSTMap() : size_(0) { root_->left = nullptr; root_->right = nullptr; }
	
	void clear()
	{
		~root_();
		delete root_;
	}

	bool containsKey(const K& key)
	{
		Node* cur = root_;
		while (cur != nullptr)
		{
			if (comp(key, cur->Key) > 0)
			{
				cur = cur->right;
			}
			else if(comp(key, cur->Key) < 0)
			{
				cur = cur->left;
			}
			else
			{
				return true;
			}
		}
		return false;
	}

	bool containsValue(const T& value)
	{
		Node* cur = root_;
		while (cur != nullptr)
		{
			if (comp(value, cur->Val) > 0)
			{
				cur = cur->right;
			}
			else if(comp(value, cur->value) < 0)
			{
				cur = cur->left;
			}
			else 
			{
				return true;
			}
		}
		return false;
	}

	//Непонятно что возвращать
	template<typename Container>
	Container EntrySet()
	{

	}

	//Get
	T& operator[](const K& other)
	{
		Node* cur, prev;
		cur = prev = root_;
		while (cur != nullptr)
		{
			if (comp(cur, other) > 0)
			{
				prev = cur;
				cur = cur->right;
			}
			else if (comp(cur, other) < 0)
			{
				prev = cur;
				cur = cur->left;
			}
			else
			{
				return cur->data;
			}
		}
		Node* tmp = NodeAllocTraits::allocate(nodeAlloc_, 1);
		tmp->parent = prev;
		if (comp(cur, other) > 0)
		{
			prev->right = tmp;
		}
		else
		{
			prev->left = tmp;
		}

	}

	bool isEmpty()
	{
		return size_ == 0;
	}

	template<typename Container>
	void keySet()
	{
		Node* cur = root_;
		auto func = [](Node* tmp) {std::cout << "{ " << tmp->Key << ":" << tmp->Val << " }"; };
		TraversePreOrder(cur, func);
	}

	template<typename Func, typename... Args>
	void TraversePreOrder(Node* cur, Func func, Args&... args)
	{
		if (cur == nullptr)return;

		TraversePreOrder(cur->left, args...);
		func(cur);
		TraversePreOrder(cur->right, args...);
	}

	//Функция put реализована в []

	void remove(const K& key)
	{
		Node* cur = root_;
		while (cur != nullptr)
		{

		}
		throw std::bad_exception("Remove null el...");
	}

	size_t size()
	{
		return size_;
	}

	const K& firstKey()
	{
		Node* cur = root_;
		while (cur->left != nullptr)
		{
			cur = cur->left;
		}
		return cur->Key;
	}

	const K& lastKey()
	{
		Node* cur = root_;
		while (cur->right != nullptr)
		{
			cur = cur->right;
		}
		return cur->Key;
	}

	std::ostream& operator<<(std::ostream& os) const
	{
		Node* cur = root_;
		
		auto func = [&os](Node* cur) {os << "{ " << cur->Key << " :" << cur->Val << " }"; };
		TraverseFunc(cur, func);

		return os;
	}

	friend std::ostream& operator<<(std::ostream& os, const BSTMap& map)
	{
		auto func = [&os](Node* cur) {os << "{ " << cur->Key << " :" << cur->Val << " }"; };
		map.TraverseFunc(map.root_, func);

		return os;
	}
	template<typename... Args, typename Func>
	void TraverseFunc(Node* cur, Func func, const Args&... args) const
	{
		if (cur == nullptr)return;

		TraverseFunc(cur->left, func, args...);
		func(cur, args...);
		TraverseFunc(cur->right, func, args...);
	}

	//Рекурсивная версия
	//Нужно добавить компаратор!!!!
	std::vector<Node> headMap(const K& key)
	{
		std::vector<Node> out;
		out.reserve(size_);

		auto func = [&out](Node* cur, const K& key) {
				if (comp(key, cur->Key) > 0) {
					out.push_back(*cur);
				}
			};
		TraverseFunc(root_, func, key);
		return out;
	}
	std::vector<Node> subMap(const K& start, const K& end)
	{
		std::vector<Node> out;
		out.reserve(size_);

		auto func = [&out](Node* cur, const K& start, const K& end)
			{
				if (comp(start, cur->Key) < 0 && comp(cur->Key, end) < 0)
				{
					out.push_back(cur);
				}
			};
		TraverseFunc(root_, func, start, end);
		return out;
	}
	std::vector<Node> tailMap(const K& key)
	{
		std::vector<Node> out;
		out.reserve(size_);

		auto func = [&out](Node* cur, const K& key)
			{
				if (comp(key, cur->Key) < 0)
				{
					out.push_back(cur);
				};
			};
		TraverseFunc(root_, func, key);
		return out;
	}

	const Node& lowerEntry(const K& key)
	{
		Node* out = nullptr;

		auto func = [&out](Node* cur, const K& key) {
				if (comp(key, cur->Key) > 0)
				{
					out = key;
				}
			};
		return out;
		TraverseFunc(root_, func);
	}
	const Node& floorEntry(const K& key)
	{
		Node* out = nullptr;

		auto func = [&out](Node* cur, const K& key)
			{
				if (comp(key, cur->Key) >= 0)
				{
					out = key;
				}
			};
		TraverseFunc(root_, func);
		return out;
	}
	const Node& higherEntry(const K& key)
	{
		Node* out = nullptr;

		auto func = [&out](Node* cur, const K& key)
			{
				if (comp(key, cur->Key) < 0)
				{
					out = key;
				}
			};
		TraverseFunc(root_, func);
		return out;
	}
	const Node& ceilingEntry(const K& key)
	{
		Node* out = nullptr;

		auto func = [&out](Node* cur, const K& key)
			{
				if (comp(key, cur->Key) <= 0)
				{
					out = key;
				}
			};
		TraverseFunc(root_, func);
		return out;
	}

	const K& lowerK(const K& K)
	{
		Node* node = lowerEntry(K);
		return node->Key;
	}
	const K& floorK(const K& key)
	{
		Node* node = floorEntry(key);
		return node->Key;
	}
	const K& higherK(const K& key)
	{
		Node* node = higherEntry(key);
		return node->Key;
	}
	const K& ceilingK(const K& key)
	{
		Node* node = ceilingEntry(key);
		return node->Key;
	}

	Node poolFirstElem()
	{
		Node tmp;
		
		auto func = [&tmp](Node* cur) {remove(cur->Key); };
		tmp = FirstEntryTemplate(func);

		return tmp;
	}
	Node poolLastElem()
	{
		Node tmp;

		auto func = [&tmp](Node* cur) {remove(cur->Key); };
		tmp = LastEntryTemplate(func);

		return tmp;
	}

	const Node& firstEntry()
	{
		auto emptyFunc = [](Node*) {};
		return FirstEntryTemplate(emptyFunc);
	}
	const Node& lastEntry()
	{
		auto emptyFunc = [](Node*) {};
		return LastEntryTemplate(emptyFunc);
	}

	template<typename Func>
	const Node& FirstEntryTemplate(Func func)
	{
		Node* cur = root_, tmp;
		while (cur->left != nullptr) 
		{ 
			cur = cur->left; 
		}

		func(tmp);

		return *tmp;
	}

	template<typename Func>
	const Node& LastEntryTemplate(Func func)
	{
		Node* cur = root_, tmp;

		while (cur->right != nullptr) 
		{ 
			cur = cur->right; 
		}

		func(tmp);

		return *tmp;
	}
};

int main()
{
	BSTMap<int, int> map;
	map << std::cout;
	std::cout << map;
}