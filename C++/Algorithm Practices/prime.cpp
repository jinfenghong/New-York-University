#include <iostream>
#include <set>
#include <cmath>
using namespace std;

void extendTheSetOfPrimes(set<long>& allPrimes, long x) {
	long bP;	//big prime
	set<long>::iterator it = allPrimes.end();
	--it;
	bP = *it;

	bP += 2;	//skip the even number, all primes are odd except 2
	long ind;
	while (bP <= x) {
		//check bP is prime; if it is, add it to allPrimes
		it = allPrimes.begin();
		ind = 0;
		while (it != allPrimes.end() && (*it)*(*it) < bP && (ind == 0)) {
			ind = (bP % (*it) == 0);
			++it;
		}
		if (ind == 0) {
			allPrimes.insert(bP);
		}
		bP += 2;
	}
}

long isPrime(set<long>& allPrimes, long x) {
	// first step: to check whether the x is in allPrimes already or not
	if (allPrimes.find(x) != allPrimes.end()) {
		return 1;
	}

	// second step: to check if x is not in allPrimes, x should not be less than the biggest prime number in the allPrimes, otherwise it is definitely not prime
	long theBiggestPrimeIKnow;
	set<long>::iterator it;
	it = allPrimes.end();
	--it;
	theBiggestPrimeIKnow = *it;
	if (x < theBiggestPrimeIKnow) {
		return 0;
	}

	// step three: extend the set of primes to less than or equal to x
	extendTheSetOfPrimes(allPrimes, x);
	if (allPrimes.find(x) != allPrimes.end()) {
		return 1;	// after calling extendTheSetOfPrimes, we should check again whether x is in allPrimes or not
	}

	return 0;
}

int main() {
	set<long> allPrimes;
	allPrimes.insert(2);
	allPrimes.insert(3);
	allPrimes.insert(5);

	/*int a, p, q;
	cin >> a >> p >> q;

	while (p > 0 && q > 0) {
		if (isPrime(allPrimes, p) && isPrime(allPrimes, q)) {
			cout << isPrime(allPrimes, pow(p, 2) + pow(q, 2) + a) << endl;
		}
		cin >> p >> q;
	}*/
	long x;
	cin >> x;

	if (isPrime(allPrimes, x)) {
		cout << x << " is prime" << endl;
	}
	else {
		cout << x << " is not prime" << endl;
	}

	return 0;
}