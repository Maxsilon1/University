#include "HashMap.h"

int main()
{
    using namespace std;
    HashMap<int, std::string> hmap;

    hmap.insert(1, "AAAA");
    hmap.insert(3, "AAA");
    hmap.insert(1, "AAAAA");
    std::cout << hmap.get(3).second->second << '\t' << hmap.get(1).second->second << '\n';
    std::cout << (++hmap.get(1).second)->second;
}