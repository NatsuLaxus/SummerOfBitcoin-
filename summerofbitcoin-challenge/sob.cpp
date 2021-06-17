#include<bits/stdc++.h>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
using namespace std;
#define ll long long
#define PI 3.14159265

ll mod = 1000000007;
ll z   = 1000000000;
const ll N   = 1e5 + 1;
const ll INF = 1e18;
// ll mod = 998244353;


#define mp                make_pair
#define pb                push_back
#define ppb               pop_back
#define pf                push_front
#define ppf               pop_front
#define all(x)            (x).begin(),(x).end()
#define uniq(v)           (v).erase(unique(all(v)),(v).end())
#define sz(x)             (ll)((x).size())
#define fr                first
#define sc                second
#define pii               pair<ll,ll>
#define vi                vector<ll>
#define vvi               vector<vector<ll>>
#define vp                vector<pair<ll,ll>>
#define rep(i,a,b)        for(ll i=a;i<b;i++)
#define repr(i,a,b)       for(ll i=b-1;i>=a;i--)


struct transaction
{
	string txid;
	int fee;
	int weight;
	vector<string> parent_txid;
};
vector<transaction> transactions;


vector<pair<string, vector<string>>> read_csv(string filename)
{
	vector<pair<string, vector<string>>> result;
	ifstream myFile(filename);

	if (!myFile.is_open()) throw std::runtime_error("Could not open file");

	string line, colname;
	string val;
	if (myFile.good())
	{
		getline(myFile, line);
		stringstream ss(line);
		while (std::getline(ss, colname, ',')) {
			result.push_back({colname, std::vector<string> {}});
		}
	}
	while (std::getline(myFile, line))
	{
		stringstream ss(line);
		int colIdx = 0;
		while (ss >> val) {
			result.at(colIdx).second.push_back(val);
			if (ss.peek() == ',') ss.ignore();
			colIdx++;
		}
	}
	myFile.close();
	return result;
}

void tokenize(string &str, char delim, vector<string> &out)
{
	stringstream ss(str);

	string s;
	while (std::getline(ss, s, delim)) {
		out.push_back(s);
	}
}

void read_file()
{
	vector<pair<string, vector<string>>> temp = read_csv("mempool2.csv");
	ofstream myfile;
	myfile.open ("block.txt");
	// only the zeroth index of temp consists of all the details;
	rep(j, 0, sz(temp[0].sc))
	{
		string s = temp[0].sc[j];
		char delim = ',';
		vector<string> out;
		tokenize(s, delim, out);
		vector<string> par_txids;
		if (out.size() == 4)
		{
			char delim2 = ';';
			tokenize(out[3], delim2, par_txids);
		}
		transaction t;
		t.txid = out[0];
		t.fee = stoi(out[1]);
		t.weight = stoi(out[2]);
		t.parent_txid = par_txids;
		transactions.push_back(t);
	}
	// rep(i, 0, sz(transactions))
	// myfile << transactions[i].weight << endl;
	myfile.close();
}


bool cmp(const transaction &t1, const transaction &t2)
{
	float n1 = (float)max(t1.fee, 1), n2 = max(t2.fee, 1);
	float d1 = (float)max(t1.weight, 1), d2 = max(t2.weight, 1);
	return ((n1 / d1) > (n2 / d2));
}

map<string, transaction> txid_to_transaction;
vector<transaction> copy_transactions;
map<string, bool> visited;

void dfs(transaction t, int& sum_fee, int& sum_weight)
{
	sum_weight += t.weight;
	sum_fee += t.fee;
	for (int j = 0; j < t.parent_txid.size(); j++)
	{
		transaction current = txid_to_transaction[t.parent_txid[j]];
		if (visited[current.txid] == false)
		{
			visited[current.txid] = true;
			dfs(current, sum_fee, sum_weight);
		}
	}
}


map<string, int> position_sorted_transactions;

void create_graph()
{
	for (int i = 0; i < transactions.size(); i++)
	{
		copy_transactions.push_back(transactions[i]);
	}
	for (int i = 0; i < transactions.size(); i++)
	{
		txid_to_transaction[transactions[i].txid] = transactions[i];
	}
	for (int i = 0; i < transactions.size(); i++)
	{
		visited.clear();
		visited[transactions[i].txid] = true;
		int sum_fee = 0;
		int sum_weight = 0;
		dfs(transactions[i], sum_fee, sum_weight);
		copy_transactions[i].fee = sum_fee;
		copy_transactions[i].weight = sum_weight;
	}

	sort(all(copy_transactions), cmp);
	for (int i = 0; i < transactions.size(); i++)
	{
		position_sorted_transactions[copy_transactions[i].txid] = i;
	}
}

// store parent to child relationship of each transaction
map<string, vector<transaction>> parent_to_child;
void create_parent_to_child()
{
	for (int i = 0; i < sz(copy_transactions); i++)
	{
		for (int j = 0; j < sz(copy_transactions[i].parent_txid); j++)
		{
			parent_to_child[copy_transactions[i].parent_txid[j]].push_back(copy_transactions[i]);
		}
	}
}


map<string, bool> updated_transaction;
void update_all_child_transactions(transaction t, int remove_fee, int remove_weight)
{
	for (int i = 0; i < parent_to_child[t.txid].size(); i++)
	{
		transaction child_transaction = parent_to_child[t.txid][i];

		if (updated_transaction[child_transaction.txid] == false)
		{
			child_transaction.weight -= remove_weight;
			child_transaction.fee -= remove_fee;
			int pos = position_sorted_transactions[child_transaction.txid];
			copy_transactions[pos] = child_transaction;
			updated_transaction[child_transaction.txid] = true;
			update_all_child_transactions(copy_transactions[pos], remove_fee, remove_weight);
		}
	}
}


vector<string> block;
map<string, bool> added_to_block;

void insert_transaction(transaction t)
{
	added_to_block[t.txid] = true;
	updated_transaction.clear();
	transaction orignal = txid_to_transaction[t.txid];
	// cout << (orignal.fee == t.fee) << " ";
	update_all_child_transactions(t, orignal.fee, orignal.weight);
	for (int i = 0; i < t.parent_txid.size(); i++)
	{
		transaction parent_transaction = txid_to_transaction[t.parent_txid[i]];
		if (added_to_block[parent_transaction.txid] == false)
		{
			insert_transaction(parent_transaction);
		}
	}
	block.push_back(t.txid);
}

pair<int, int> create_block()
{
	block.clear();
	int block_weight = 0;
	int block_fee = 0;
	for (int i = 0; i < copy_transactions.size(); i++)
	{
		transaction choosen_transaction = copy_transactions[i];
		if (added_to_block.find(choosen_transaction.txid) == added_to_block.end() && choosen_transaction.weight + block_weight <= 4000000)
		{
			insert_transaction(choosen_transaction);
			block_weight += choosen_transaction.weight;
			block_fee += choosen_transaction.fee;
		}
	}
	return make_pair(block_weight, block_fee);
}

pair<bool, pair<int, int>> check_valid()
{
	int weight = 0, fee = 0;
	bool valid = true;
	for (int i = 0; i < block.size(); i++)
	{
		string curr = block[i];
		transaction curr_transaction = txid_to_transaction[curr];
		for (int j = 0; j < curr_transaction.parent_txid.size(); j++)
		{
			if (added_to_block[curr_transaction.parent_txid[j]] == false)
			{
				valid = false;
			}
		}
		weight += curr_transaction.weight;
		fee += curr_transaction.fee;
	}
	return make_pair(valid, make_pair(weight, fee));
}

int main() {
	read_file();
	create_graph();
	create_parent_to_child();
	pair<int, int> p = create_block();
	pair<bool, pair<int, int>> p2 = check_valid();

	ofstream myfile;
	myfile.open ("block.txt");
	for (int i = 0; i < block.size(); i++)
	{
		myfile << block[i] << endl;
	}
	myfile.close();
	return 0;
}
