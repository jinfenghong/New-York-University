#include <iostream>
using namespace std;

int XOR(int x, int newInt, int k) {
	int multiplier = 1;
	int result = 0;
	while (x > 0 || newInt > 0) {
		result += multiplier * ((x + newInt) % k);
		multiplier *= k;
		x /= k; newInt /= k;
	}
	return result;
}

int main() {
	int n, k;
	cout << "n and k: ";
	cin >> n >> k;

	int uI, temp = 0;
	for (int i = 0; i < n*k + 1; ++i) {
		cin >> uI;
		temp = XOR(uI, temp, k);
	}

	cout << temp << endl;
	return 0;
}