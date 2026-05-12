#include "RBTree.h"

int main()
{
	Map<int, int> m;
	m.insert({5, 2});m.insert({3, 2});m.insert({1, 5});
	auto it = m.find(5);
	m.erase(m.find(5));
	std::cout << m;
}