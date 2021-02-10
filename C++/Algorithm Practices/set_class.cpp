#include <iostream>
#include <string>
#include <set>
using namespace std;

class ProductOffer {
public:
	string storeName;
	string pName;
	double pPrice;
	int operator< (const ProductOffer& _PO) const;
};

int ProductOffer::operator< (const ProductOffer& _PO) const {
	if (pName < _PO.pName) {
		return 1;
	}
	return 0;
}

int main() {
	set<ProductOffer> cheapest;
	set<ProductOffer>::iterator it;
	ProductOffer temp;
	string flavor, store;
	double price;

	cin >> flavor >> store >> price;

	while (price > 0.0) {
		temp.pName = flavor;
		temp.pPrice = price;
		temp.storeName = store;
		it = cheapest.find(temp);
		if (it == cheapest.end()) {
			cheapest.insert(temp);
		}
		else {
			if ((*it).pPrice > price) {
				cheapest.erase(*it);
				cheapest.insert(temp);
			}
		}
		cin >> flavor >> store >> price;
	}

	for (it = cheapest.begin(); it != cheapest.end(); ++it) {
		cout << (*it).pName << " is the cheapest when made by ";
		cout << (*it).storeName << " for the price of ";
		cout << (*it).pPrice << endl;
	}

	return 0;
}