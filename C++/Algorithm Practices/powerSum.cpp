#include <iostream>
using namespace std;

int powN(int x, int n) {
	if (n == 0) {
		return 1;
	}
	return x*powN(x, n - 1);
}

int powerSum(int x, int n, int avoid = 0) {
	if (x == 0) {
		return 1;
	}
	int counter = 0;
	int i = avoid + 1;
	int iN = powN(i, n);
	while (iN < x + 1) {
		counter += powerSum(x - iN, n, i++);
		iN = powN(i, n);
	}
	return counter;
}

int main() {
	int x, n;
	cout << "x and n: ";
	cin >> x >> n;
	cout << powerSum(x, n) << endl;
	return 0;
}