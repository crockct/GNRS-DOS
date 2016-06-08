#include "nameresolver.h"

int main(int argc, char **argv){
	NameResolver test;
	std::vector <int> *l;
	std::string file("mappings.txt");
	std::string name1("name1");
	std::string name2("name2");
	test.init(file);
	l = test.getGuids(name1);
	std::cout << (*l)[0] << std::endl;
	l = test.getGuids(name2);
	std::cout << (*l)[0] << std::endl;
	std::cout << (*l)[1] << std::endl;
	std::cout << test.getName(1) << std::endl;
	std::cout << test.getName(2) << std::endl;
	std::cout << test.getName(22) << std::endl;
}
