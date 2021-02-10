#include <iostream>
#include <vector>
using namespace std;

int powN(int x, int n) {
	if (n == 0) {
		return 1;
	}
	return x*powN(x, n - 1);
}

void powerSum(int x, int n, vector<int>& v, vector<int>& keeper, int& vSize, int avoid = 0) {
	if (x == 0) {
		if (v.size() < vSize || vSize == 0) {
			vSize = v.size();
			keeper.clear();
			for (vector<int>::iterator it = v.begin(); it != v.end(); ++it) {
				keeper.push_back(*it);
			}
		}
	}
	int i = avoid + 1;
	int iN = powN(i, n);
	while (iN < x + 1) {
		v.push_back(i);
		powerSum(x - iN, n, v, keeper, vSize, i++);
		v.pop_back();
		iN = powN(i, n);
	}
}

int main() {
	int x, n;
	cout << "x and n: ";
	cin >> x >> n;
	vector<int> temp, result;
	vector<int>::iterator it;
	int smallest = 0;
	powerSum(x, n, temp, result, smallest);
	cout << smallest << endl;
	
	for (it = result.begin(); it != result.end(); ++it) {
		cout << *it << " ";
	}
	cout << endl;
	return 0;
}