#include <iostream>
#include <set>
#include <string>
using namespace std;

/*struct SortOrder {
	bool operator () (const string& first, const string& second) {
		int len1 = first.length();
		int len2 = second.length();

		if (len1 == len2) {
			return (first < second);
		}
		return (len1 < len2);
	}
};*/

class SortOrder {
public:
	bool operator () (const string& first, const string& second) {
		int len1 = first.length();
		int len2 = second.length();

		if (len1 == len2) {
			return (first < second);
		}
		return (len1 < len2);
	}
};

int main() {
	//set<string, SortOrder> names;
	set<string, SortOrder> names;
	set<string>::iterator it;

	names.insert("Peter");
	names.insert("CodingMadeEasy");

	for (it = names.begin(); it != names.end(); ++it) {
		cout << *it << endl;
	}

	return 0;
}