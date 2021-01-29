#include"iostream"
#include"algorithm"
#include"vector"
#include <numeric>
#include <stdlib.h>
#include <math.h>
#include<random>  
#include<time.h> 
#include<list>
using namespace std;
vector<int> all_prices = { 99,199,299,399,499,599,699,799,899,999 };
std::default_random_engine random(time(NULL));  //产生两种均匀分布
std::uniform_int_distribution<int> dis1(0, 1000);
std::uniform_int_distribution<int> dis2(0, 600);
std::uniform_int_distribution<int> dis3(0, 9);
std::uniform_int_distribution<int> dis4(0, 5);
//产生高斯分布
int gaussrand(int V ,int mean)
{
	static double V1, V2, S;
	static int phase = 0;
	double X;

	if (phase == 0) {
		do {
			//srand((unsigned)time(NULL));
			double U1 = (double)rand() / RAND_MAX;
			double U2 = (double)rand() / RAND_MAX;

			V1 = 2 * U1 - 1;
			V2 = 2 * U2 - 1;
			S = V1 * V1 + V2 * V2;
		} while (S >= 1 || S == 0);

		X = V1 * sqrt(-2 * log(S) / S);
	}
	else
		X = V2 * sqrt(-2 * log(S) / S);

	phase = 1 - phase;

	return int((X*V+mean)/3);
} // 产生高斯分布    //产生高斯分布

//节点对象
class node
{
public:
	vector<int> prices = { 999,899,799,699,599,499,399,299,199,99 };
	int deep = 0;
	vector<node> child;
	int price;
	int visit = 0;
	float reward_money = 1;
	node *parent=nullptr;
	node()
	{
		deep = 0;
		price = 0;
		visit = 0;
	}
	node(int depth, int action, node* last) :parent(last)
	{
		deep = depth;
		price = action;
	}
	void create_child()
	{
		if (!(this->is_expand()))
		{
			int price = prices[prices.size() - 1];
			prices.pop_back();
			node children(deep + 1, price, this);
			child.push_back(children);
		}
	}
	bool is_expand()
	{
		if (deep < 6)
		{
			return child.size() == 10;
		}
		else
		{
			return child.size() == 6;
		}
 
	}

	bool is_terminal()
	{
		return deep == 10;
	}
	bool is_root() { return deep == 0; }
	const node& most_visit_child()
	{
		if (child.size() > 0)
		{
			int max = 0;
			for (int i = 1; i < child.size(); i++)
			{
				max = child[max].visit > child[i].visit ? max : i;
			}
			return child[max];
		}
		else
		{
			return *this;
		}
	}
	void update(float reward)
	{
		reward_money = (reward_money * float(visit) + reward) / (visit + 1.0);
		visit += 1;
	}
	void back_ward(float get_money)
	{
		node* the_last = this;
		while (the_last != NULL)
		{
			the_last->update(get_money);
			the_last = the_last->parent;
		} 
	}
	vector<int> action_path()
	{
		vector<int> path;
		node* current = this;
		while (!current->is_root())
		{
			path.push_back((*current).price);
			current = (*current).parent;
		}
		reverse(path.begin(), path.end());
		return path;
	}
};
//给入路径 进行模拟
pair<int,int> step_simulation(int remain, int money_acc, int action, int depth)
{
	int num_custom_each_sku = 0;
	vector<int> offer;
	if (depth > 6)
	{
		num_custom_each_sku = gaussrand(200, 1000);
	}
	else
	{
		num_custom_each_sku =gaussrand(400, 2000);
	}
	for(int i=0;i<num_custom_each_sku;i++)
	{
		if (depth < 7)
		{
			offer.push_back(dis1(random));
		}
		else
		{
			offer.push_back(dis2(random));
		}
	}
	for (int j = 0; j < offer.size(); j++)
	{
		if (offer[j] >= action)
		{
			remain--;
			money_acc += action;
			if (remain == 0)
			{
				break;
			}
		}
	}
	return make_pair(remain, money_acc);

}

int all_simulation(vector<int> actions)
{
	int remain = 500;
	int money = 0;
	int depth = 1;
	pair<int,int> remain_money = make_pair(remain, money);
	while (depth < 11 && remain_money.first>0)
	{
		remain_money = step_simulation(remain_money.first, remain_money.second, actions[depth-1], depth);
		depth++;
	}
	return remain_money.second;
}
void show(vector<int> path)
{
	for (int i = 0; i < path.size(); i++)
	{
		cout << path[i] << ',';
	}
}

// MCTS过程
class MCTS
{
private:
	node* current_node;
	float c;
	int sim_step_num=100;
public:
	node most_visit_node;
	MCTS() {};
	MCTS(node& self, int num) :current_node(&self), c(num) { most_visit_node = *current_node; };
	node*  uct(node* father)
	{
		int max = 0;
		vector<float> scores;
		for (int i = 0; i < (*father).child.size(); i++)
		{
			scores.push_back(((*father).child[i].reward_money / (*father).reward_money) +
				1 * sqrt(log((*father).visit)/(*father).child[i].visit));
				//child.reward / node.reward + self.c * np.sqrt((np.log(node.visit) / child.visit))
		}
		for (int j = 1; j < scores.size(); j++)
		{
			max = scores[max] > scores[j] ? max : j;
		}
		return &((*father).child[max]);
	}
	node* search( node** running)
	{
		node* now_pointer = *running;
		while (!(now_pointer)->is_terminal())
		{
			if ((now_pointer)->is_expand())
			{
				now_pointer=uct(now_pointer);
			}
			else
			{
				(now_pointer)->create_child();
				return &(now_pointer)->child[((now_pointer)->child.size())- 1];
			}
		}
		return *running;
	}
	int fast_action(int depth)
	{
		if (depth < 7) return all_prices[dis3(random)];
		else return all_prices[dis4(random)];
	}
	float cycle(node current_node)
	{
		vector<int> money;
		vector<int> path = (current_node).action_path();
		for (int i = 0; i < sim_step_num; i++)
		{
			for (int j = path.size()+1; j < 11; j++)
			{
				path.push_back(fast_action(j));
			}
			money.push_back(all_simulation(path));
			vector<int> path = (current_node).action_path();
		}
		for (int i = 1; i < money.size(); i++)
		{
			money[0] += money[i];
		}
		cout <<current_node.price<<','<< current_node.deep<<','<< float(money[0]) / float(money.size()) <<endl;
		return float(money[0])/ float(money.size());
	}
	
	void run(int echo)
	{
		int reward;// 一次模拟完得到的奖励
		node* run_node = nullptr;
		for (int i = 0; i < echo; i++)
		{
			run_node=search(&current_node);
			reward = this->cycle(*run_node);
			(*run_node).back_ward(reward);
			if (!(*run_node).is_terminal())
			{
				most_visit_node = (*run_node).most_visit_child();
			}
			else
			{
				most_visit_node = *run_node;
			}
			show(most_visit_node.action_path());
			cout << ', ';
			cout << &most_visit_node;
			cout << endl;
			/*cout << current_node;
			cout << endl;*/
		}
		
	}
};	

int main()
{
	node root(0,0,nullptr);
	MCTS test(root, 1);
	test.run(900000);
	
}



