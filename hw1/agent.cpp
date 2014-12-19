#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <queue>
#include <stack>
#include <map>
#include <cstring>
#include <climits>
using namespace std;

struct Node
{
	string name;
	map<int, int> neighbor;
};

struct queue_node
{
	int w, nid;
	string n;
	queue_node(int node_id, int weight, string name):nid(node_id), w(weight), n(name){}
};

bool operator<(const queue_node &n1, const queue_node &n2)
{
	if(n1.w>n2.w) return true;
	else if(n1.w == n2.w)
	{
		if(n1.n>n2.n) return true;
		else return false;
	}
	else return false;
}

struct myless
{
	bool operator()(const queue_node &n1, const queue_node &n2)
	{
		if(n1.w<n2.w) return true;
		else if(n1.w == n2.w)
		{
			if(n1.n>n2.n) return true;
			else return false;
		}
		else return false;
	}
};

struct param
{
	int cost;
	vector<int> exp_front;
	int *pre;
	param(int nnod)
	{
		cost = 0;
		pre = new int[nnod];
		memset(pre, -1, sizeof(int));
	}
	~param()
	{
		delete[] pre;
		pre = NULL;
	}
};

vector<Node> network;

void output_path(const int s, const int t, int *pre, ofstream &f)
{
	stack<int> stk;
	stk.push(t);
	int p = pre[t];
	while(p!=s)
	{
		stk.push(p);
		p = pre[p];
	}
	
	f<<network[p].name;
	while(!stk.empty())
	{
		f<<"-"<<network[stk.top()].name;
		stk.pop();
	}
	f<<endl;
}

int get_cost(const int s, const int t, int *pre)
{
	int a = pre[t], b = t, w = 0;
	while(a!=s)
	{
		w += network[a].neighbor[b];
		b = a;
		a = pre[a];
	}
	w += network[a].neighbor[b];
	return w;
}

void output_front(vector<int> &frontier, ofstream &f)
{
	f<<network[frontier[0]].name;
	for(int i = 1;i<frontier.size();i++)
	{
		f<<"-"<<network[frontier[i]].name;
	}
	f<<endl;
}

bool bfs(const int s, const int t, param &res)
{
	priority_queue<queue_node> q;
	bool *visited = new bool[network.size()];
	for(int i = 0;i<network.size();i++) visited[i] = false;
	
	q.push(queue_node(s, 0, network[s].name));
	visited[s] = true;

	int cur_level, cur_idx;
	while(!q.empty())
	{
		queue_node cur_node = q.top();
		q.pop();
		cur_level = cur_node.w;
		cur_idx = cur_node.nid;
	
		res.exp_front.push_back(cur_idx);
		
		if(cur_idx == t)
		{
			res.cost = cur_level;
			delete[] visited;
			return true;
		}
		for(map<int, int>::iterator iter = network[cur_idx].neighbor.begin();
			iter!=network[cur_idx].neighbor.end();
			iter++)
		{
			if(!visited[iter->first])
			{
				visited[iter->first] = true;
				res.pre[iter->first] = cur_idx;
				q.push(queue_node(iter->first, cur_level+1, network[iter->first].name));
			}
		}
	}
	delete[] visited;
	return false;
}


bool dfs(const int s, const int t, param &res)
{
	priority_queue<queue_node, vector<queue_node>, myless> q;
	bool *visited = new bool[network.size()];
	for(int i = 0;i<network.size();i++) visited[i] = false;
	
	q.push(queue_node(s, 0, network[s].name));
	visited[s] = true;
	int cur_level, cur_idx;
	while(!q.empty())
	{
		queue_node cur_node = q.top();
		q.pop();
		cur_level = cur_node.w;
		cur_idx = cur_node.nid;
	
		res.exp_front.push_back(cur_idx);
		
		if(cur_idx == t)
		{
			res.cost = cur_level;
			delete[] visited;
			return true;
		}
		for(map<int, int>::iterator iter = network[cur_idx].neighbor.begin();
			iter!=network[cur_idx].neighbor.end();
			iter++)
		{
			if(!visited[iter->first])
			{
				visited[iter->first] = true;
				res.pre[iter->first] = cur_idx;
				q.push(queue_node(iter->first, cur_level+1, network[iter->first].name));
			}
		}
	}
	delete[] visited;
	return false;
}

bool ucs(const int s, const int t, param &res)
{
	priority_queue<queue_node> q;
	bool *visited = new bool[network.size()];
	for(int i = 0;i<network.size();i++) visited[i] = false;
	int *cost = new int[network.size()];
	for(int i = 0;i<network.size();i++) cost[i] = INT_MAX;
	
	q.push(queue_node(s, 0, network[s].name));
	cost[s] = 0;
		
	int cur_idx, cur_cost;
	while(!q.empty())
	{
		queue_node cur_node = q.top();
		q.pop();
		cur_idx = cur_node.nid;
		cur_cost = cur_node.w;

		if(!visited[cur_idx])
		{
			visited[cur_idx] = true;
			res.exp_front.push_back(cur_idx);
		
			if(cur_idx == t)
			{
				res.cost = cur_cost;
				delete[] visited;
				delete[] cost;
				return true;
			}
				
			for(map<int, int>::iterator iter = network[cur_idx].neighbor.begin();
				iter!=network[cur_idx].neighbor.end();
				iter++)
			{
				if(!visited[iter->first])
				{
					if(iter->second+cur_cost < cost[iter->first])
					{
						cost[iter->first] = iter->second+cur_cost;
						res.pre[iter->first] = cur_idx;
						q.push(queue_node(iter->first, cur_cost+iter->second, network[iter->first].name));
					}
				}
			}
		}
	}
	delete[] visited;
	delete[] cost;
	return false;
}

int main()
{
	ifstream f1("input.txt");
	int task_id = 0, nnod = 0, idx = 0, w;
	string s, t, name;
	map<string, int> name_list;
	f1>>task_id>>s>>t>>nnod;
	vector<int> parent(nnod, -1);
	param ret(nnod);
	bool valid_path;
	
	for(int i = 0;i<nnod;i++)
	{
		f1>>name;
		name_list[name] = idx++;
	}

	network.resize(nnod);
	for(map<string, int>::iterator iter = name_list.begin();iter!=name_list.end();iter++)
	{
		network[iter->second].name = iter->first;
	}
	
	for(int i = 0;i<nnod;i++)
	{
		for(int j = 0;j<nnod;j++)
		{
			f1>>w;
			if(w>0)
			{
				network[i].neighbor[j] = w;
				network[j].neighbor[i] = w;
			}
		}
	}
	
	f1.close();

	switch(task_id)
	{
	case 1:
		valid_path = bfs(name_list[s], name_list[t], ret);
		break;
	case 2:
		valid_path = dfs(name_list[s], name_list[t], ret);
		break;
	case 3:
		valid_path = ucs(name_list[s], name_list[t], ret);
		break;
	default:
		break;
	}

	ofstream f2("output.txt");
	if(valid_path)
	{
		output_front(ret.exp_front, f2);
		output_path(name_list[s], name_list[t], ret.pre, f2);
		f2<<get_cost(name_list[s], name_list[t], ret.pre)<<endl;
	}
	else
	{
		f2<<"NoPathAvailable"<<endl;
	}
	f2.close();
	return 0;
}

