#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>

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

		Node(const K& k, const T& v, Node* p = nullptr) // FIX: добавил parent
			: Key(k), Val(v), parent(p), left(nullptr), right(nullptr) {}

		friend std::ostream& operator<<(std::ostream& os, const Node& node)
		{
			os << "{ " << node.Key << " :" << node.Val << " }";
			return os;
		}
	};

	using AllocTraits = std::allocator_traits<Alloc>;
	using NodeAlloc = typename AllocTraits::template rebind_alloc<Node>;
	using NodeAllocTraits = std::allocator_traits<NodeAlloc>;

	using difference_type = std::ptrdiff_t;

	Alloc alloc_;
	NodeAlloc nodeAlloc_;

	Node* root_;
	size_t size_;

	Comparator comp;

	Node* findNode(const K& key) const
	{
		Node* cur = root_;
		while (cur != nullptr)
		{
			if (comp(key, cur->Key))
				cur = cur->left;
			else if (comp(cur->Key, key))
				cur = cur->right;
			else
				return cur;
		}
		return nullptr;
	}

	static Node* minNode(Node* n)
	{
		if (!n) return nullptr;
		while (n->left) n = n->left;
		return n;
	}
	static Node* maxNode(Node* n)
	{
		if (!n) return nullptr;
		while (n->right) n = n->right;
		return n;
	}

public:
	BSTMap(): root_(nullptr), size_(0), comp(Comparator()){}

	void clear()
	{
		destroy_tree(root_);
		root_ = nullptr;
		size_ = 0;
	}

	void destroy_tree(Node* node)
	{
		if (node == nullptr) return;

		destroy_tree(node->left);
		destroy_tree(node->right);

		NodeAllocTraits::destroy(nodeAlloc_, node);
		NodeAllocTraits::deallocate(nodeAlloc_, node, 1);
	}

	bool containsKey(const K& key)
	{
		Node* cur = root_;
		while (cur != nullptr)
		{
			if (comp(key, cur->Key))// key < cur->Key
				cur = cur->left;
			else if (comp(cur->Key, key))// cur->Key < key
				cur = cur->right;
			else
				return true;
		}
		return false;
	}

	bool containsValue(const T& value)
	{
		// FIX: сравнивать значения через Comparator< K > нельзя, а у тебя он только по K.
		// Делам линейный обход и сравниваем ==.
		bool found = false;
		auto func = [&found, &value](Node* cur)
			{
				if (cur->Val == value) found = true;
			};
		TraversePreOrder(root_, func);
		return found;
	}

	//Непонятно что возвращать
	template<typename Container>
	Container EntrySet()
	{
		// FIX: реализуем как контейнер пар (K, T)
		Container c;
		auto func = [&c](Node* cur)
			{
				c.emplace_back(cur->Key, cur->Val);
			};
		TraversePreOrder(root_, func);
		return c;
	}

	//Get / put
	T& operator[](const K& other)
	{
		// FIX: в твоём коде тут множество ошибок (Node* cur, prev; comp(cur, other) и т.п.)
		// Сохраняем идею: поиск, если нет – вставка.
		if (root_ == nullptr)
		{
			Node* n = NodeAllocTraits::allocate(nodeAlloc_, 1);
			NodeAllocTraits::construct(nodeAlloc_, n, other, T{}, nullptr);
			root_ = n;
			++size_;
			return root_->Val;
		}

		Node* cur = root_;
		Node* parent = nullptr;

		while (cur != nullptr)
		{
			parent = cur;
			if (comp(other, cur->Key))
				cur = cur->left;
			else if (comp(cur->Key, other))
				cur = cur->right;
			else
				return cur->Val;
		}

		Node* n = NodeAllocTraits::allocate(nodeAlloc_, 1);
		NodeAllocTraits::construct(nodeAlloc_, n, other, T{}, parent);
		if (comp(other, parent->Key))
			parent->left = n;
		else
			parent->right = n;
		++size_;
		return n->Val;
	}

	bool isEmpty() const
	{
		return size_ == 0;
	}

	template<typename Container>
	void keySet()
	{
		// FIX: сохранил логику печати
		auto func = [](Node* tmp) { std::cout << "{ " << tmp->Key << ":" << tmp->Val << " } "; };
		TraversePreOrder(root_, func);
		std::cout << '\n';
	}

	template<typename Func, typename... Args>
	void TraversePreOrder(Node* cur, Func func, Args&... args)
	{
		if (cur == nullptr)return;

		func(cur, args...);                 // FIX: префиксный порядок – сначала вузел
		TraversePreOrder(cur->left, func, args...);
		TraversePreOrder(cur->right, func, args...);
	}

	//Функция put реализована в []

	void remove(const K& key)
	{
		Node* node = findNode(key);
		if (!node) return;

		if (!node->left && !node->right)
		{
			if (node->parent)
			{
				if (node->parent->left == node) node->parent->left = nullptr;
				else node->parent->right = nullptr;
			}
			else
			{
				root_ = nullptr;
			}
		}
		else if (!node->left || !node->right)
		{
			Node* child = node->left ? node->left : node->right;
			if (node->parent)
			{
				if (node->parent->left == node) node->parent->left = child;
				else node->parent->right = child;
			}
			else
			{
				root_ = child;
			}
			child->parent = node->parent;
		}
		else
		{
			Node* succ = minNode(node->right);
			const_cast<K&>(node->Key) = succ->Key; // Key константный, но логика BSTMap предполагает перенос
			node->Val = succ->Val;

			Node* child = succ->right;
			if (succ->parent->left == succ) succ->parent->left = child;
			else succ->parent->right = child;
			if (child) child->parent = succ->parent;

			NodeAllocTraits::destroy(nodeAlloc_, succ);
			NodeAllocTraits::deallocate(nodeAlloc_, succ, 1);
			--size_;
			return;
		}

		NodeAllocTraits::destroy(nodeAlloc_, node);
		NodeAllocTraits::deallocate(nodeAlloc_, node, 1);
		--size_;
	}

	size_t size()
	{
		return size_;
	}

	const K& firstKey()
	{
		Node* cur = root_;
		if (!cur) throw std::runtime_error("Map is empty");
		while (cur->left != nullptr)
		{
			cur = cur->left;
		}
		return cur->Key;
	}

	const K& lastKey()
	{
		Node* cur = root_;
		if (!cur) throw std::runtime_error("Map is empty");
		while (cur->right != nullptr)
		{
			cur = cur->right;
		}
		return cur->Key;
	}

	std::ostream& operator<<(std::ostream& os) const
	{
		auto func = [&os](Node* cur) { os << *cur << ' '; };
		TraverseFunc(root_, func);

		if (size_ == 0)os << "Map is empty...";
		return os;
	}

	friend std::ostream& operator<<(std::ostream& os, const BSTMap& map)
	{
		auto func = [&os](Node* cur) { os << *cur << ' '; };
		map.TraverseFunc(map.root_, func);

		if (map.isEmpty())os << "Map is empty...";
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

	std::vector<Node> headMap(const K& key)
	{
		std::vector<Node> out;
		out.reserve(size_);

		auto func = [this, &out](Node* cur, const K& key) {
			if (comp(cur->Key, key)) {
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

		auto func = [this, &out](Node* cur, const K& start, const K& end)
			{
				if (!comp(cur->Key, start) && comp(cur->Key, end))
				{
					out.push_back(*cur);
				}
			};
		TraverseFunc(root_, func, start, end);
		return out;
	}
	std::vector<Node> tailMap(const K& key)
	{
		std::vector<Node> out;
		out.reserve(size_);

		auto func = [this, &out](Node* cur, const K& key)
			{
				if (!comp(cur->Key, key))
				{
					out.push_back(*cur);
				};
			};
		TraverseFunc(root_, func, key);
		return out;
	}

	const Node& lowerEntry(const K& key)
	{
		// max el < key
		Node* out = nullptr;
		Node* cur = root_;
		while (cur)
		{
			if (comp(cur->Key, key))
			{
				out = cur;
				cur = cur->right;
			}
			else
			{
				cur = cur->left;
			}
		}
		if (!out) throw std::runtime_error("No lowerEntry");
		return *out;
	}
	const Node& floorEntry(const K& key)
	{
		// max el <= key
		Node* out = nullptr;
		Node* cur = root_;
		while (cur)
		{
			if (!comp(key, cur->Key)) // cur->Key <= key
			{
				out = cur;
				cur = cur->right;
			}
			else
			{
				cur = cur->left;
			}
		}
		if (!out) throw std::runtime_error("No floorEntry");
		return *out;
	}
	const Node& higherEntry(const K& key)
	{
		// min el > key
		Node* out = nullptr;
		Node* cur = root_;
		while (cur)
		{
			if (comp(key, cur->Key)) // cur->Key > key
			{
				out = cur;
				cur = cur->left;
			}
			else
			{
				cur = cur->right;
			}
		}
		if (!out) throw std::runtime_error("No higherEntry");
		return *out;
	}
	Node& ceilingEntry(const K& key) const
	{
		// min el >= key
		Node* out = nullptr;
		Node* cur = root_;
		while (cur)
		{
			if (!comp(cur->Key, key)) // cur->Key >= key
			{
				out = cur;
				cur = cur->left;
			}
			else
			{
				cur = cur->right;
			}
		}
		if (!out) throw std::runtime_error("No ceilingEntry");
		return *out;
	}

	const K& lowerK(const K& key)
	{
		const Node& node = lowerEntry(key);
		return node.Key;
	}
	const K& floorK(const K& key)
	{
		const Node& node = floorEntry(key);
		return node.Key;
	}
	const K& higherK(const K& key)
	{
		const Node& node = higherEntry(key);
		return node.Key;
	}
	const K& ceilingK(const K& key)
	{
		Node& node = ceilingEntry(key);
		return node.Key;
	}

	Node poolFirstElem()
	{
		//return min
		Node* mn = minNode(root_);
		if (!mn) throw std::runtime_error("Map is empty");
		Node copy = *mn;
		remove(mn->Key);
		return copy;
	}
	Node poolLastElem()
	{
		// удаляет и возвращает максимальный
		Node* mx = maxNode(root_);
		if (!mx) throw std::runtime_error("Map is empty");
		Node copy = *mx;
		remove(mx->Key);
		return copy;
	}

	const Node& firstEntry()
	{
		Node* mn = minNode(root_);
		if (!mn) throw std::runtime_error("Map is empty");
		return *mn;
	}
	const Node& lastEntry()
	{
		Node* mx = maxNode(root_);
		if (!mx) throw std::runtime_error("Map is empty");
		return *mx;
	}

	template<typename Func>
	const Node& FirstEntryTemplate(Func func)
	{
		Node* cur = minNode(root_);
		if (!cur) throw std::runtime_error("Map is empty");
		func(cur);
		return *cur;
	}

	template<typename Func>
	const Node& LastEntryTemplate(Func func)
	{
		Node* cur = maxNode(root_);
		if (!cur) throw std::runtime_error("Map is empty");
		func(cur);
		return *cur;
	}
};

int main()
{
	BSTMap<int, int> map;

	std::cout << "Initial map: " << map << "\n\n";

	int n;
	for (int i = 0; i < 5; ++i)
	{
		std::cin >> n;
		map[n] = i;
	}
	std::cin >> n;
	std::cout << "map[" << n << "] = " << map[n] << '\n';

	std::cout << "\nMap: " << map << "\n\n";

	std::cout << "containsKey(3): " << map.containsKey(3) << "\n";
	std::cout << "containsValue(2): " << map.containsValue(2) << "\n\n";

	auto v_tail = map.tailMap(3);
	std::cout << "tailMap(3):\n";
	for (auto& i : v_tail)
	{
		std::cout << i << '\n';
	}
	std::cout << '\n';

	auto v_head = map.headMap(3);
	std::cout << "headMap(3):\n";
	for (auto& i : v_head)
	{
		std::cout << i << '\n';
	}
	std::cout << '\n';

	auto v_sub = map.subMap(2, 4);
	std::cout << "subMap(2,4):\n";
	for (auto& i : v_sub)
	{
		std::cout << i << '\n';
	}
	std::cout << '\n';

	std::cout << "firstKey: " << map.firstKey() << "\n";
	std::cout << "lastKey: " << map.lastKey() << "\n\n";

	std::cout << "lowerEntry(3): " << map.lowerEntry(3) << "\n";
	std::cout << "floorEntry(3): " << map.floorEntry(3) << "\n";
	std::cout << "higherEntry(3): " << map.higherEntry(3) << "\n";
	std::cout << "ceilingEntry(3): " << map.ceilingEntry(3) << "\n\n";

	std::cout << "lowerK(3): " << map.lowerK(3) << "\n";
	std::cout << "floorK(3): " << map.floorK(3) << "\n";
	std::cout << "higherK(3): " << map.higherK(3) << "\n";
	std::cout << "ceilingK(3): " << map.ceilingK(3) << "\n\n";

	std::cout << "firstEntry: " << map.firstEntry() << "\n";
	std::cout << "lastEntry: " << map.lastEntry() << "\n\n";

	std::cout << "poolFirstElem(): " << map.poolFirstElem() << "\n";
	std::cout << "poolLastElem(): " << map.poolLastElem() << "\n";
	std::cout << "After pools: " << map << "\n\n";

	std::cout << "keySet(): ";
	map.keySet<std::vector<int>>();

	auto entries = map.EntrySet<std::vector<std::pair<int,int>>>();
	std::cout << "EntrySet():\n";
	for (auto& p : entries)
		std::cout << "{ " << p.first << " : " << p.second << " }\n";

	std::cout << "\nsize(): " << map.size() << "\n";
	std::cout << "isEmpty(): " << map.isEmpty() << "\n";

	std::cout << "\nclear()\n";
	map.clear();
	std::cout << "After clear: " << map << "\n";
}