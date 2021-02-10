// move_example.cpp 
// compile with 
// c++ -o m_ex move_example.cpp -std=c++11 -fno-elide-constructors 
// execute with 
// ./m_ex 

#include<iostream> 
class Basket {
public:
	Basket();
	Basket(const Basket&);
	Basket(Basket&&);
	void operator=(const Basket&);
	void operator=(Basket&&);
	~Basket();
};
Basket::Basket() { std::cout << "1"; }
Basket::Basket(const Basket& copyFrom) { std::cout << "2"; }
Basket::Basket(Basket&& moveFrom) { std::cout << "3"; }
void Basket::operator=(const Basket& copyFrom) { std::cout << "4"; }
void Basket::operator=(Basket&& moveFrom) { std::cout << "5"; }
Basket::~Basket() { std::cout << "6"; }
Basket doSomething(Basket B) {
	Basket helper;
	return helper;
}
void printResults() {
	Basket greenBasket;
	Basket redBasket(greenBasket);
	redBasket = doSomething(greenBasket);
	std::cout << "7";
}
int main() {
	printResults();
	std::cout << std::endl;
	return 0;
}

//122136566766 Linux
//122136656766 Windows