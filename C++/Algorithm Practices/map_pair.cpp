#include <iostream>
#include <map>
#include <string>
using namespace std;

class myPair {
public:
	string storeName;
	double ICprice;
};

int main() {
	map<string, myPair> myMap;
	map<string, myPair>::iterator it;
	string flavor, store;
	double price;
	myPair temp1;

	cin >> flavor >> store >> price;
	while (price > 0.0) {
		if (myMap.find(flavor) == myMap.end()) {
			temp1.ICprice = price;
			temp1.storeName = store;
			myMap[flavor] = temp1;
		}
		else {
			if (myMap[flavor].ICprice > price) {
				myMap[flavor].ICprice = price;
				myMap[flavor].storeName = store;
			}
		}
		cin >> flavor >> store >> price;
	}

	for (it = myMap.begin(); it != myMap.end(); ++it) {
		cout << (*it).first << " is the cheapest when made by ";
		cout << (*it).second.storeName << " for the price of ";
		cout << (*it).second.ICprice << endl;
	}

	return 0;
}