#include <iostream>

#include <algorithm>

#include <vector>
#include <queue>

//4 Поиск компонент сильной связности. Kosaraju's algorithm
std::vector<int> SCC(const std::vector<std::vector<int>>& adj)
{
	int n = adj.size();
	std::vector<int> order;
	std::vector<bool> vis(n, false);

	std::vector<std::vector<int>> radj(n);
	for (int v = 0; v < n; ++v)
		for (int to : adj[v])
			radj[to].push_back(v);

	std::vector<int> comp(n, -1);

	auto dfs1 = [&](auto& dfs, int v)->void {
		vis[v] = false;
		for (const int& to : adj[v])
		{
			if (!vis[to])
			{
				dfs(dfs, to);
			}
		}
		order.push_back(v);
	};

	auto dfs2 = [&](auto& dfs, int v, int c)->void {
		vis[v] = false;
		comp[v] = c;
		for (const int& to : radj[v])
		{
			if (!vis[v])
			{
				dfs(dfs, to, c);
			}
		}
	};

	for (int i = 0; i < n; ++i)
		if (!vis[i])
			dfs1(dfs1, i);

	std::fill(vis.begin(), vis.end(), false);

	int c = 0;

	for(int i = 0; i < n; ++ i)
	{
		if (!vis[i])
		{
			dfs2(dfs2, i, c++);
		}
	}	
	return comp;
}

void KosarjuExamples(int num_of_examples = 3)
{
	std::cout << "SCC\n";

	int i = 0;
	while (i < num_of_examples)
	{
		int n; std::cin >> n;
		std::vector<std::vector<int>> adj(n);
		
		for (int i = 0; i < n; ++i)
		{
			int u, v; std::cin >> u >> v;
			--u; --v;
			
			adj[u].push_back(v);
			adj[v].push_back(u);
		}

		std::vector<int> out = SCC(adj);
		std::cout << "Example " << i++ + 1 << ":";
		for (int& x : out)
			std::cout << x << ' ';
		std::cout << '\n';
	}
	std::cout << "...\n";
}

//10 Max Flow. Edmonds-Carp's Algorithm.
int bfs(int t, int s, std::vector<int>& parent, std::vector<std::vector<int>>& cap, const std::vector<std::vector<int>>& adj)
{
	std::fill(parent.begin(), parent.end(), -1);
	parent[s] = -2;
	
	std::queue<std::pair<int, int>> q;
	q.push({s, INT32_MAX});
	
	while (!q.empty())
	{
		auto [vertex, flow] = q.front();
		q.pop();

		for (const int& to : adj[vertex])
		{
			if (parent[to] == -1 && cap[vertex][to] > 0)
			{
				parent[to] = vertex;
				int new_flow = std::min(flow, cap[vertex][to]);
				if (to == t)
					return new_flow;
				q.push({to, new_flow});
			}
		}
	}
	return 0;
}

//Finding flows if the flow uses wrong way we still can reuse it because of back-way algorithm and cap
int maxFlow(int s, int t, std::vector<std::vector<int>>& adj, std::vector<std::vector<int>>& cap, int n)
{
	std::vector<int> parent(n);

	int flow = 0, new_flow = 0;
	while ((new_flow = bfs(s, t, parent, cap, adj)) > 0)
	{
		flow += new_flow;
		int cur = t;
		while (cur != s)
		{
			int prev = parent[cur];
			cap[prev][cur] -= new_flow;
			cap[cur][prev] += new_flow;
			cur = prev;
		}
	}
	return flow;
}

void maxFlowExamples(int num_of_examples = 3)
{
	for (int i = 0; i < num_of_examples; ++i)
	{
		std::cout << "Input cnt of vertexes and edges...\n";
		int n, m; std::cin >> n >> m;
		std::cout << "Input s(from) and t(to)...\n";
		int s, t; std::cin >> s >> t;
		--s; --t;

		std::vector<std::vector<int>> adj(n);
		std::vector<std::vector<int>> cap(n, std::vector<int>(n));
		for (int i = 0; i < m; ++i)
		{
			int u, v, c; std::cin >> u >> v >> c;
			--u; --v;
			adj[u].push_back(v);
			adj[v].push_back(u);
			cap[u][v] += c;
		}

		std::cout << "Max Flow " << i + 1 << ": " << maxFlow(s, t, adj, cap, n) << '\n';
	}
}

/*
4 4
1 4
1 2 10
2 4 5
1 3 7
3 4 8

Ответ: Max Flow 1: 12

Ввод:
5 6
1 5
1 2 3
2 5 2
2 4 3
1 3 2
3 4 3
4 5 4

Ответ: Max Flow 3: 5
*/

//16 Finding Articular Points | Tarjan's Algorithm
std::vector<int> findArticulations(const std::vector<std::vector<int>>& adj)
{
	int n = adj.size();
	int timer = 0;
	int children = 0;

	std::vector<int> tin(n, -1), low(n, -1);
	std::vector<bool> vis(n, false), is_cut(n, true);

	//auto& self - to have opportunity to have recursion
	auto dfs = [&](auto& self, int v, int p = -1) -> void {
		vis[v] = true;
		tin[v] = low[v] = timer++;
		for (int neighbor : adj[v])
		{
			if (neighbor == p)continue;

			if (vis[neighbor])
			{
				low[neighbor] = std::min(low[neighbor], low[v]);
			}
			else
			{
				self(self, neighbor, v);

				low[v] = std::min(low[v], low[neighbor]);
				if (p != -1 && low[neighbor] >= tin[v])
				{
					is_cut[v] = true;
				}
				children++;
			}
		}
		if (p == -1 && children > 0)
			is_cut[v] = true;
	};

	for (int i = 0; i < n; ++i)
		if (!vis[i])
			dfs(dfs, 0);

	std::vector<int> out;
	for (int i = 0; i < n; ++i)
		if (is_cut[i])
			out.push_back(i);

	return out;
}

void ArtPointsExamples(int num_of_examples = 3)
{
	int i = 0;
	while (i < num_of_examples)
	{
		int n; std::cin >> n;
		std::vector<std::vector<int>> adj(n);
		
		for (int i = 0; i < n; ++i)
		{
			int u, v; std::cin >> u >> v;
			--u; --v;
			adj[u].push_back(v);
			adj[v].push_back(u);
		}

		std::vector<int> out = findArticulations(adj);
	}
}

int main()
{
	std::cout << "Input what of the variants you need...\n";
	
	std::cout << "1 - SCC Kosaraju's algorithm\n";
	std::cout << "2 - Max Flow - Edmonds-Carp's Algorithm\n";
	std::cout << "3 - Articulation Points - Tarjan's Algorithm\n";

	std::cout << "4 - All Random Examples...\n";
	
	int n; std::cin >> n;
	
	std::cout << "Input count of examples of choosen algorithm:\n";
	int cnt; std::cin >> cnt;
	switch (n)
	{
	case 1:

		KosarjuExamples(cnt);
		break;

	case 2:
		break;

	case 3:
		
		break;

	case 4:
		
		break;
	}
}