#include <iostream>
using namespace std;

bool CFI(const int& n) {
	for (int i = 1; i < n; ++i) {
		if (i * (i + 1) == n) {
			return true;
		}
	}
	return false;
}

int cfiCombination(const int& x, int avoid = 0) {
	if (x == 0) {
		return 1;
	}
	int counter = 0;
	int newInt = avoid + 1;
	while (newInt < x + 1) {
		if (CFI(newInt)) {
			counter += cfiCombination(x - newInt, newInt);
		}
		++newInt;
	}
	return counter;
}

int main() {
	int x;
	cin >> x;
	cout << cfiCombination(x) << endl;
	return 0;
}