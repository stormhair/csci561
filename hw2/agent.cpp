#include <iostream>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <string>
#include <climits>
#include <utility>
#include <algorithm>
using namespace std;

//user-built type
typedef int DISC;

//constant
const int BOARD_SIZE = 8;
const int BLACK = 0;
const int WHITE = 1;
int weights[BOARD_SIZE][BOARD_SIZE] = {
{99, -8, 8, 6, 6, 8, -8, 99},
{-8, -24, -4, -3, -3, -4, -24, -8},
{8, -4, 7, 4, 4, 7, -4, 8},
{6, -3, 4, 0, 0, 4, -3, 6},
{6, -3, 4, 0, 0, 4, -3, 6},
{8, -4, 7, 4, 4, 7, -4, 8},
{-8, -24, -4, -3, -3, -4, -24, -8},
{99, -8, 8, 6, 6, 8, -8, 99}
};

//global var.
int board[BOARD_SIZE][BOARD_SIZE];
vector<string> logs;
DISC Player;
int current_pass = 0;
bool pass_end = false;

//functions
	//definition
void init();
void output_current_board();
void out_next_state(ofstream &, int);
bool place_disc(int, int, DISC, vector<pair<int, int> > &);
void undo(const int, const int, vector<pair<int, int> > &);
int eval(DISC);
bool greedy_next_move(DISC);
void minimax_next_move(DISC, int);
void ab_next_move(DISC, int);
int max_value(int, int, DISC, string);
int min_value(int, int, DISC, string);
int max_value_ab(int, int, int, int, DISC, string);
int min_value_ab(int, int, int, int, DISC, string);

inline string coord2name(const int i, const int j)
{
	string name;
	name+=('a'+j);
	name+=('1'+i);
	return name;
}

inline pair<int, int> name2coord(string name)
{
	if(name.size()!=2) return pair<int, int>(-1, -1);
	else return pair<int, int>(name[1] - '1', name[0] - 'a');
}

	//implementation
	//initialize the board, -1 stands for no disc in this check
void init()
{
	for(int i = 0;i<BOARD_SIZE;i++)
		for(int j = 0;j<BOARD_SIZE;j++)
			board[i][j] = -1;
}

	//for debugging
void output_current_board()
{
	for(int i = 0;i<BOARD_SIZE;i++)
	{
		for(int j = 0;j<BOARD_SIZE;j++)
		{
			switch(board[i][j])
			{
			case -1:
				printf("*");
				break;
			case BLACK:
				printf("X");
				break;
			case WHITE:
				printf("O");
				break;
			default:
				break;
			}
		}
		printf("\n");
	}
}

void output_next_state(ofstream &f, int opt)
{
	for(int i = 0;i<BOARD_SIZE;i++)
	{
		for(int j = 0;j<BOARD_SIZE;j++)
		{
			switch(board[i][j])
			{
			case -1:
				f<<"*";
				break;
			case BLACK:
				f<<"X";
				break;
			case WHITE:
				f<<"O";
				break;
			default:
				break;
			}
		}
		f<<endl;
	}
	//output traversing logs
	if(opt>1)
	{
		for(int i = 0;i<logs.size();i++)
		{
			f<<logs[i]<<endl;
		}
	}
}

//place d on (i, j), return whether d is placed on (i, j)
bool place_disc(int i, int j, DISC d, vector<pair<int, int> > &fs)
{
	bool flag = false;
	//there is one disc already
	if(board[i][j] != -1) return flag;
	
	//the place is not adjacent to the opponent's disc
	DISC op = d^1;
	//up check
	if(i-1>0&&board[i-1][j] == op)
	{
		int loc = i-1;
		for(int k = loc-1;k>=0;k--)
		{
			if(board[k][j] == op)
				continue;
			else if(board[k][j] == -1)
				break;
			else if(board[k][j] == d)
			{
				flag = true;
				for(int t = k+1;t<=loc;t++)
				{
					board[t][j]^=1;
					fs.push_back(pair<int, int>(t, j));
				}
				break;
			}
		}
	}
	//down check
	if(i+1<BOARD_SIZE&&board[i+1][j] == op)
	{
		int loc = i+1;
		for(int k = loc+1;k<BOARD_SIZE;k++)
		{
			if(board[k][j] == op)
				continue;
			else if(board[k][j] == -1)
				break;
			else if(board[k][j] == d)
			{
				flag = true;
				for(int t = k-1;t>=loc;t--)
				{
					board[t][j]^=1;
					fs.push_back(pair<int, int>(t, j));
				}
				break;
			}
		}
	}
	//left check
	if(j-1>0&&board[i][j-1] == op)
	{
		int loc = j-1;
		for(int k = loc-1;k>=0;k--)
		{
			if(board[i][k] == op)
				continue;
			else if(board[i][k] == -1)
				break;
			else if(board[i][k] == d)
			{
				flag = true;
				for(int t = k+1;t<=loc;t++)
				{
					board[i][t]^=1;
					fs.push_back(pair<int, int>(i, t));
				}
				break;
			}
		}
	}
	//right check
	if(j+1<BOARD_SIZE&&board[i][j+1] == op)
	{
		int loc = j+1;
		for(int k = loc+1;k<BOARD_SIZE;k++)
		{
			if(board[i][k] == op)
				continue;
			else if(board[i][k] == -1)
				break;
			else if(board[i][k] == d)
			{
				flag = true;
				for(int t = k-1;t>=loc;t--)
				{
					board[i][t]^=1;
					fs.push_back(pair<int, int>(i, t));
				}
				break;
			}
		}
	}
	//up-left diag
	if(i-1>0&&j-1>0&&board[i-1][j-1] == op)
	{
		int locx = i-1, locy = j-1, x = i-2, y = j-2;
		while(x>=0&&y>=0)
		{
			if(board[x][y] == op)
			{
				x--;y--;
				continue;
			}
			else if(board[x][y] == -1)
				break;
			else if(board[x][y] == d)
			{
				flag = true;
				for(int xx = x+1, yy = y+1;xx<=locx&&yy<=locy;xx++, yy++)
				{
					board[xx][yy]^=1;
					fs.push_back(pair<int, int>(xx, yy));
				}
				break;
			}
		}
	}
	//down-left diag
	if(i+1<BOARD_SIZE&&j-1>0&&board[i+1][j-1] == op)
	{
		int locx = i+1, locy = j-1, x = i+2, y = j-2;
		while(x<BOARD_SIZE&&y>=0)
		{
			if(board[x][y] == op)
			{
				x++;y--;
				continue;
			}
			else if(board[x][y] == -1)
				break;
			else if(board[x][y] == d)
			{
				flag = true;
				for(int xx = x-1, yy = y+1;xx>=locx&&yy<=locy;xx--, yy++)
				{
					board[xx][yy]^=1;
					fs.push_back(pair<int, int>(xx, yy));
				}
				break;
			}
		}
	}
	//up-right
	if(i-1>0&&j+1<BOARD_SIZE&&board[i-1][j+1] == op)
	{
		int locx = i-1, locy = j+1, x = i-2, y = j+2;
		while(x>=0&&y<BOARD_SIZE)
		{
			if(board[x][y] == op)
			{
				x--;y++;
				continue;
			}
			else if(board[x][y] == -1)
				break;
			else if(board[x][y] == d)
			{
				flag = true;
				for(int xx = x+1, yy = y-1;xx<=locx&&yy>=locy;xx++, yy--)
				{
					board[xx][yy]^=1;
					fs.push_back(pair<int, int>(xx, yy));
				}
				break;
			}
		}
	}
	//down-right
	if(i+1<BOARD_SIZE&&j+1<BOARD_SIZE&&board[i+1][j+1] == op)
	{
		int locx = i+1, locy = j+1, x = i+2, y = j+2;
		while(x<BOARD_SIZE&&y<BOARD_SIZE)
		{
			if(board[x][y] == op)
			{
				x++;y++;
				continue;
			}
			else if(board[x][y] == -1)
				break;
			else if(board[x][y] == d)
			{
				flag = true;
				for(int xx = x-1, yy = y-1;xx>=locx&&yy>=locy;xx--, yy--)
				{
					board[xx][yy]^=1;
					fs.push_back(pair<int, int>(xx, yy));
				}
				break;
			}
		}
	}
	
	if(flag) board[i][j] = d;
	return flag;
}

void undo(const int i, const int j, vector<pair<int, int> > &fs)
{
	board[i][j] = -1;
	for(int i = 0;i<fs.size();i++)
	{
		board[fs[i].first][fs[i].second]^=1;
	}
}

int eval(DISC d)
{
	int dw = 0, opw = 0;
	DISC op = d^1;
	for(int i = 0;i<BOARD_SIZE;i++)
	{
		for(int j = 0;j<BOARD_SIZE;j++)
		{
			if(board[i][j] == d) dw+=weights[i][j];
			else if(board[i][j] == op) opw+=weights[i][j];
		}
	}
	return dw-opw;
}

bool check_end()
{
	DISC d = -1;
	for(int i = 0;i<BOARD_SIZE;i++)
		for(int j = 0;j<BOARD_SIZE;j++)
		{
			if(board[i][j]!=-1)
			{
				if(d == -1)
					d = board[i][j];
				else if(board[i][j]!=d)
				{
					return false;
				}
			}
		}
	return true;
}

	//greedy: return false means pass
bool greedy_next_move(DISC player)
{
	//fliped sequece
	vector<pair<int, int> > fs;
	pair<int, int> best_move;
	int max_profits = INT_MIN;
	bool success = false;
	for(int i = 0;i<BOARD_SIZE;i++)
		for(int j = 0;j<BOARD_SIZE;j++)
		{
			if(place_disc(i, j, player, fs))
			{
				success = true;
				int w = eval(Player);
				if(w>max_profits)
				{
					best_move = pair<int, int>(i, j);
					max_profits = w;
				}
				undo(i, j, fs);
				fs.clear();
			}
		}
	if(success)
	{
		place_disc(best_move.first, best_move.second, player, fs);
	}
	return success;
}

	//minimax
void minimax_next_move(DISC player, int max_dep)
{
	char buf[100];
	int w = INT_MIN;
	pair<int, int> best_move;
	vector<pair<int, int> > s;
	bool flag = false;
	//
	if(check_end())
	{
		logs.push_back("Node,Depth,Value");
		sprintf(buf, "root,0,%d", eval(Player));
		logs.push_back(buf);
		return;
	}
	//
	logs.push_back("Node,Depth,Value");
	logs.push_back("root,0,-Infinity");
	for(int i = 0;i<BOARD_SIZE;i++)
	{
		for(int j = 0;j<BOARD_SIZE;j++)
		{
			if(place_disc(i, j, player, s))
			{
				int v = min_value(1, max_dep, player^1, coord2name(i, j));
				if(v>w)
				{
					flag = true;
					w = v;
					best_move = pair<int, int>(i, j);	
				}
				undo(i, j, s);
				s.clear();
				if(w>INT_MIN)
					sprintf(buf, "root,0,%d",w);
				else
					sprintf(buf, "root,0,-Infinity");
				logs.push_back(buf);
			}
		}
	}
	if(flag)
	{
		place_disc(best_move.first, best_move.second, player, s);
	}
	else
	{
		int v = min_value(1, max_dep, player^1, "pass");
		if(v>w)
		{
			w = v;
		}
		if(w>INT_MIN)
			sprintf(buf, "root,0,%d",w);
		else
			sprintf(buf, "root,0,-Infinity");
		logs.push_back(buf);
	}
}

int max_value(int dep, int max_dep, DISC player, string state)
{
	char buf[100];
	if(dep == max_dep||check_end())
	{
		int vv = eval(Player);

		sprintf(buf, "%s,%d,%d", state.c_str(), dep, vv);
		logs.push_back(buf);

		return vv;
	}

	if(state == "pass")
		current_pass++;
	if(current_pass == 2)
		pass_end = true;
	//pass end
	if(pass_end)
	{
		int vv = eval(Player);

		sprintf(buf, "pass,%d,%d",dep, vv);
		logs.push_back(buf);
		
		return vv;
	}
	int v = INT_MIN;

	sprintf(buf, "%s,%d,-Infinity", state.c_str(), dep);
	logs.push_back(buf);

	//move successfully	
	bool flag = false;

	vector<pair<int, int> > seq;
	for(int i = 0;i<BOARD_SIZE;i++)
	{
		for(int j = 0;j<BOARD_SIZE;j++)
		{
			if(place_disc(i, j, player, seq))
			{
				flag = true;
				int tmp = min_value(dep+1, max_dep, player^1, coord2name(i, j));
				if(v<tmp)
				{
					v = tmp;
				}
				undo(i, j, seq);
				seq.clear();
				if(v>INT_MIN)
					sprintf(buf, "%s,%d,%d", state.c_str(), dep, v);
				else
					sprintf(buf, "%s,%d,-Infinity", state.c_str(), dep);
				logs.push_back(buf);
			}
		}
	}
	//pass
	
	if(!flag)
	{
		int tmp = min_value(dep+1, max_dep, player^1,"pass");
		current_pass--;
		if(v<tmp)
		{
			v = tmp;
		}
		if(v > INT_MIN)
			sprintf(buf, "%s,%d,%d", state.c_str(), dep, v);
		else
			sprintf(buf, "%s,%d,-Infinity", state.c_str(), dep);
		logs.push_back(buf);
	}
	
	//
	return v;
}

int min_value(int dep, int max_dep, DISC player, string state)
{
	char buf[100];
	if(dep == max_dep||check_end())
	{
		int vv = eval(Player);

		sprintf(buf, "%s,%d,%d", state.c_str(), dep, vv);
		logs.push_back(buf);

		return vv;
	}
	if(state == "pass")
		current_pass++;
	if(current_pass == 2)
		pass_end = true;
	//pass end
	if(pass_end)
	{
		int vv = eval(Player);

		sprintf(buf, "pass,%d,%d", dep, vv);
		logs.push_back(buf);

		return vv;
	}

	int v = INT_MAX;

	sprintf(buf, "%s,%d,Infinity", state.c_str(), dep);
	logs.push_back(buf);

	//
	bool flag = false;
	vector<pair<int, int> > seq;
	for(int i = 0;i<BOARD_SIZE;i++)
	{
		for(int j = 0;j<BOARD_SIZE;j++)
		{
			if(place_disc(i, j, player, seq))
			{
				flag = true;
				int tmp = max_value(dep+1, max_dep, player^1, coord2name(i, j));
				if(tmp<v)
				{
					v = tmp;
				}
				undo(i, j, seq);
				seq.clear();
				if(v<INT_MAX)
					sprintf(buf, "%s,%d,%d", state.c_str(), dep, v);
				else
					sprintf(buf, "%s,%d,Infinity", state.c_str(), dep);
				logs.push_back(buf);
			}
		}
	}
	//pass
	
	if(!flag)
	{
		int tmp = max_value(dep+1, max_dep, player^1, "pass");
		current_pass--;
		if(tmp<v)
		{
			v = tmp;
		}
		if(v < INT_MAX)
			sprintf(buf, "%s,%d,%d", state.c_str(), dep, v);
		else
			sprintf(buf, "%s,%d,Infinity", state.c_str(), dep);
		logs.push_back(buf);
	}
	
	//
	return v;
}

	//minimax with alpha-beta pruning
void ab_next_move(DISC player, int max_dep)
{
	int w = INT_MIN, a = INT_MIN, b = INT_MAX;
	pair<int, int> best_move;
	vector<pair<int, int> > s;
	bool flag = false;

	if(check_end())
	{
		logs.push_back("Node,Depth,Value,Alpha,Beta");
		stringstream tmp_str;
		tmp_str<<"root,0,"<<eval(Player)<<",";
		if(a == INT_MIN)
			tmp_str<<"-Infinity,";
		else
			tmp_str<<a<<",";
		if(b == INT_MAX)
			tmp_str<<"Infinity";
		else
			tmp_str<<b;		
		logs.push_back(tmp_str.str());
		return;
	}

	logs.push_back("Node,Depth,Value,Alpha,Beta");
	logs.push_back("root,0,-Infinity,-Infinity,Infinity");
	char buf[100];
	for(int i = 0;i<BOARD_SIZE;i++)
	{
		for(int j = 0;j<BOARD_SIZE;j++)
		{
			if(place_disc(i, j, player, s))
			{
				int v = min_value_ab(a, b, 1, max_dep, player^1, coord2name(i, j));
				stringstream tmp_str;
				if(v>w)
				{
					flag = true;
					w = v;
					best_move = pair<int, int>(i, j);
				}
				undo(i, j, s);
				s.clear();
				if(w>b)
				{
					//logs
					tmp_str<<"root,0,";
					if(w == INT_MIN)
						tmp_str<<"-Infinity,";
					else
						tmp_str<<w<<",";
					if(a == INT_MIN)
						tmp_str<<"-Infinity,";
					else
						tmp_str<<a<<",";
					if(b == INT_MAX)
						tmp_str<<"Infinity";
					else
						tmp_str<<b;		
					logs.push_back(tmp_str.str());
					if(flag)
						place_disc(best_move.first, best_move.second, player, s);
					return;
				}
				a = max(a, w);
				//logs
				tmp_str<<"root,0,";
				if(w == INT_MIN)
					tmp_str<<"-Infinity,";
				else
					tmp_str<<w<<",";
				if(a == INT_MIN)
					tmp_str<<"-Infinity,";
				else
					tmp_str<<a<<",";
				if(b == INT_MAX)
					tmp_str<<"Infinity";
				else
					tmp_str<<b;		
				logs.push_back(tmp_str.str());
			}
		}
	}
	if(flag)
	{
		place_disc(best_move.first, best_move.second, player, s);
	}
	else
	{
		int v = min_value_ab(a, b, 1, max_dep, player^1, "pass");
		if(v>w) w = v;
		a = max(a, w);
		stringstream tmp_str;
		tmp_str<<"root,0,";
		if(w == INT_MIN)
			tmp_str<<"-Infinity,";
		else
			tmp_str<<w<<",";
		if(a == INT_MIN)
			tmp_str<<"-Infinity,";
		else
			tmp_str<<a<<",";
		if(b == INT_MAX)
			tmp_str<<"Infinity";
		else
			tmp_str<<b;		
		logs.push_back(tmp_str.str());
	}
}

int max_value_ab(int a, int b, int dep, int max_dep, DISC player, string state)
{
	if(dep == max_dep||check_end())
	{
		int vv = eval(Player);
		//logs
		stringstream tmp_str;
		tmp_str<<state<<","<<dep<<","<<vv<<",";
		if(a == INT_MIN)
			tmp_str<<"-Infinity,";
		else
			tmp_str<<a<<",";
		if(b == INT_MAX)
			tmp_str<<"Infinity";
		else
			tmp_str<<b;		
		logs.push_back(tmp_str.str());

		return vv;
	}

	if(state == "pass")
		current_pass++;
	if(current_pass == 2)
		pass_end = true;
	//pass end
	if(pass_end)
	{
		int vv = eval(Player);

		stringstream tmp_str;
		tmp_str<<state<<","<<dep<<","<<vv<<",";
		if(a == INT_MIN)
			tmp_str<<"-Infinity,";
		else
			tmp_str<<a<<",";
		if(b == INT_MAX)
			tmp_str<<"Infinity";
		else
			tmp_str<<b;		
		logs.push_back(tmp_str.str());
		
		return vv;
	}

	int v = INT_MIN;
	//logs
	stringstream tmp_str;
	tmp_str<<state<<","<<dep<<","<<"-Infinity,";
	if(a == INT_MIN)
		tmp_str<<"-Infinity,";
	else
		tmp_str<<a<<",";
	if(b == INT_MAX)
		tmp_str<<"Infinity";
	else
		tmp_str<<b;		
	logs.push_back(tmp_str.str());

	//move successfully	
	bool flag = false;

	vector<pair<int, int> > seq;
	for(int i = 0;i<BOARD_SIZE;i++)
	{
		for(int j = 0;j<BOARD_SIZE;j++)
		{
			if(place_disc(i, j, player, seq))
			{
				stringstream tmp_str;
				flag = true;
				int tmp = min_value_ab(a, b, dep+1, max_dep, player^1, coord2name(i, j));
				if(v<tmp)
				{
					v = tmp;
				}
				undo(i, j, seq);
				seq.clear();
				
				if(v>=b)
				{
					//logs
					tmp_str<<state<<","<<dep<<",";
					if(v == INT_MIN)
						tmp_str<<"-Infinity,";
					else
						tmp_str<<v<<",";
					if(a == INT_MIN)
						tmp_str<<"-Infinity,";
					else
						tmp_str<<a<<",";
					if(b == INT_MAX)
						tmp_str<<"Infinity";
					else
						tmp_str<<b;		
					logs.push_back(tmp_str.str());
					return v;
				}
				
				a = max(a, v);
				//logs
				tmp_str<<state<<","<<dep<<",";
				if(v == INT_MIN)
					tmp_str<<"-Infinity,";
				else
					tmp_str<<v<<",";
				if(a == INT_MIN)
					tmp_str<<"-Infinity,";
				else
					tmp_str<<a<<",";
				if(b == INT_MAX)
					tmp_str<<"Infinity";
				else
					tmp_str<<b;		
				logs.push_back(tmp_str.str());
			}
		}
	}
	if(!flag)
	{
		int tmp = min_value_ab(a, b, dep+1, max_dep, player^1, "pass");
		current_pass--;
		if(v<tmp)
		{
			v = tmp;
		}
		stringstream tmp_str;
		if(v>=b)
		{
			//logs
			tmp_str<<state<<","<<dep<<",";
			if(v == INT_MIN)
				tmp_str<<"-Infinity,";
			else
				tmp_str<<v<<",";
			if(a == INT_MIN)
				tmp_str<<"-Infinity,";
			else
				tmp_str<<a<<",";
			if(b == INT_MAX)
				tmp_str<<"Infinity";
			else
				tmp_str<<b;		
			logs.push_back(tmp_str.str());
			return v;
		}
				
		a = max(a, v);
		//logs
		tmp_str<<state<<","<<dep<<",";
		if(v == INT_MIN)
			tmp_str<<"-Infinity,";
		else
			tmp_str<<v<<",";
		if(a == INT_MIN)
			tmp_str<<"-Infinity,";
		else
			tmp_str<<a<<",";
		if(b == INT_MAX)
			tmp_str<<"Infinity";
		else
			tmp_str<<b;		
		logs.push_back(tmp_str.str());
	}
	return v;
}

int min_value_ab(int a, int b, int dep, int max_dep, DISC player, string state)
{
	if(dep == max_dep||check_end())
	{
		int vv = eval(Player);

		//logs
		stringstream tmp_str;
		tmp_str<<state<<","<<dep<<","<<vv<<",";
		if(a == INT_MIN)
			tmp_str<<"-Infinity,";
		else
			tmp_str<<a<<",";
		if(b == INT_MAX)
			tmp_str<<"Infinity";
		else
			tmp_str<<b;		
		logs.push_back(tmp_str.str());

		return vv;
	}

	if(state == "pass")
		current_pass++;
	if(current_pass == 2)
		pass_end = true;
	//pass end
	if(pass_end)
	{
		int vv = eval(Player);

		stringstream tmp_str;
		tmp_str<<state<<","<<dep<<","<<vv<<",";
		if(a == INT_MIN)
			tmp_str<<"-Infinity,";
		else
			tmp_str<<a<<",";
		if(b == INT_MAX)
			tmp_str<<"Infinity";
		else
			tmp_str<<b;		
		logs.push_back(tmp_str.str());
		
		return vv;
	}

	int v = INT_MAX;

	//logs
	stringstream tmp_str;
	tmp_str<<state<<","<<dep<<","<<"Infinity,";
	if(a == INT_MIN)
		tmp_str<<"-Infinity,";
	else
		tmp_str<<a<<",";
	if(b == INT_MAX)
		tmp_str<<"Infinity";
	else
		tmp_str<<b;		
	logs.push_back(tmp_str.str());

	//
	bool flag = false;
	vector<pair<int, int> > seq;
	for(int i = 0;i<BOARD_SIZE;i++)
	{
		for(int j = 0;j<BOARD_SIZE;j++)
		{
			if(place_disc(i, j, player, seq))
			{
				flag = true;
				stringstream tmp_str;
				int tmp = max_value_ab(a, b, dep+1, max_dep, player^1, coord2name(i, j));
				if(tmp<v)
				{
					v = tmp;
				}
				undo(i, j, seq);
				seq.clear();
				if(v<=a)
				{
					//logs
					tmp_str<<state<<","<<dep<<",";
					if(v == INT_MAX)
						tmp_str<<"-Infinity,";
					else
						tmp_str<<v<<",";
					if(a == INT_MIN)
						tmp_str<<"-Infinity,";
					else
						tmp_str<<a<<",";
					if(b == INT_MAX)
						tmp_str<<"Infinity";
					else
						tmp_str<<b;		
					logs.push_back(tmp_str.str());
					return v;
				}
				b = min(b, v);
				//logs
				tmp_str<<state<<","<<dep<<",";
				if(v == INT_MAX)
					tmp_str<<"-Infinity,";
				else
					tmp_str<<v<<",";
				if(a == INT_MIN)
					tmp_str<<"-Infinity,";
				else
					tmp_str<<a<<",";
				if(b == INT_MAX)
					tmp_str<<"Infinity";
				else
					tmp_str<<b;		
				logs.push_back(tmp_str.str());
				
			}
		}
	}
	if(!flag)
	{
		int tmp = max_value_ab(a, b, dep+1, max_dep, player^1, "pass");
		current_pass--;
		if(tmp<v)
		{
			v = tmp;
		}
		stringstream tmp_str;
		if(v<=a)
		{
			//logs
			tmp_str<<state<<","<<dep<<",";
			if(v == INT_MAX)
				tmp_str<<"-Infinity,";
			else
				tmp_str<<v<<",";
			if(a == INT_MIN)
				tmp_str<<"-Infinity,";
			else
				tmp_str<<a<<",";
			if(b == INT_MAX)
				tmp_str<<"Infinity";
			else
				tmp_str<<b;		
			logs.push_back(tmp_str.str());
			return v;
		}
		b = min(b, v);
		//logs
		tmp_str<<state<<","<<dep<<",";
		if(v == INT_MAX)
			tmp_str<<"-Infinity,";
		else
			tmp_str<<v<<",";
		if(a == INT_MIN)
			tmp_str<<"-Infinity,";
		else
			tmp_str<<a<<",";
		if(b == INT_MAX)
			tmp_str<<"Infinity";
		else
			tmp_str<<b;		
		logs.push_back(tmp_str.str());
	}
	return v;
}

//
int main()
{
	init();
	ifstream f("input.txt");
	int stgy, cut_depth;
	DISC player;
	char toplay;
	f>>stgy>>toplay>>cut_depth;
	if(toplay == 'X')
	{
		player = BLACK;
	}
	else if(toplay == 'O')
	{
		player = WHITE;
	}
	Player = player;
	string line;
	int r = 0, c;
	while(f>>line)
	{
		for(c = 0;c<line.size();c++)
		{
			switch(line[c])
			{
			case '*':
				board[r][c] = -1;
				break;
			case 'X':
				board[r][c] = BLACK;
				break;
			case 'O':
				board[r][c] = WHITE;
				break;
			default:
				break;
			}
		}
		r++;
	}
	f.close();
	
	ofstream of("output.txt");
	switch(stgy)
	{
	case 1:
		greedy_next_move(player);
		output_next_state(of, 1);
		break;
	case 2:
		minimax_next_move(player, cut_depth);
		output_next_state(of, 2);
		break;
	case 3:
		ab_next_move(player, cut_depth);
		output_next_state(of, 3);
		break;
	default:
		break;
	}
	of.close();
	logs.clear();

	return 0;
}

