#include <iostream>
using namespace std;

void merge2seq(int* newSeq, int* left, int* right, int lenL, int lenR) {
	int i = 0, j = 0, k = 0;
	while (i < lenL && j < lenR) {
		if (left[i] < right[j]) {
			newSeq[k] = left[i];
			++i;
		}
		else {
			newSeq[k] = right[j];
			++j;
		}
		++k;
	}
	while (i < lenL) {
		newSeq[k] = left[i];
		++i; ++k;
	}
	while (j < lenR) {
		newSeq[k] = right[j];
		++j; ++k;
	}
}

void MergeSort(int* seq, int len) {
	if (len > 1) {
		int lenL = len / 2;
		MergeSort(seq, lenL);
		MergeSort(seq + lenL, len - lenL);
		int* newSeq = new int[len];
		merge2seq(newSeq, seq, seq + lenL, lenL, len - lenL);
		
		for (int i = 0; i < len; ++i) {
			seq[i] = newSeq[i];
		}
		delete[] newSeq;
	}
}

int main(){
	int n;
	cin >> n;
	int* seq = new int[n];
	for (int i = 0; i < n; ++i) {
		cin >> seq[i];
	}

	MergeSort(seq, n);

	for (int i = 0; i < n; ++i) {
		cout << seq[i] << " ";
	}
	cout << endl;

	delete[] seq;
	return 0;
}