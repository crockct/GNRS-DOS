/*
 *
 * User: wontoniii
 *
 * File: nameresolver.cpp
 *
 * Description: class that contains the name service that replies maintain name to GUIDs bindings and reverse lookups.
 *
 * Created on: Jun, 2014
 */


#include <map>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include "nameresolver.h"
#include "parser.h"

std::list<int> *NameResolver::getAllGuids(){
	std::list<int> *returnList = new std::list<int>();
	std::map <int, std::string>::iterator it;
	for(it = guidToName.begin(); it != guidToName.end(); it++){
		returnList->push_back((int)it->first);
	}
	return returnList;
}

std::list<std::string> *NameResolver::getAllNames(){
	std::list<std::string> *returnList = new std::list<std::string>();
	std::map<std::string, std::vector <int> *>::iterator it;
	for (it = nameToGuid.begin(); it!=nameToGuid.end(); it++) {
		returnList->push_back(it->first);
	}
	return returnList;
}

std::vector <int> *NameResolver::getGuids(const std::string name){
	std::map<std::string, std::vector <int> *>::iterator it;
	it = nameToGuid.find(name);
	if(it == nameToGuid.end()){
		return NULL;
	}
	else{
		return it->second;
	}
}

std::string NameResolver::getName(int guid){
	std::map<int, std::string>::iterator it;
	it = guidToName.find(guid);
	if(it == guidToName.end()){
		return std::string("");
	}
	else{
		return it->second;
	}
}
	
NameResolver::NameResolver(){
	
}

NameResolver::~NameResolver(){
	
}

void NameResolver::init(const std::string filename){
	std::cout << "Initializing NameResolver" << std::endl;
	Parser par;
	par.init(filename);
	NamePair *temp;
	par.setPairsIterator();
	std::map<std::string, std::vector <int> *>::iterator it;
	while((temp = par.getNextPair()) != NULL){
		it = nameToGuid.find(temp->getName());
		if(it == nameToGuid.end()){
			nameToGuid.insert(make_pair(temp->getName(), new std::vector<int>()));
			it = nameToGuid.find(temp->getName());
			it->second->push_back(temp->getGUID());
			guidToName.insert(make_pair(temp->getGUID(), temp->getName()));
		}
		else{
			it->second->push_back(temp->getGUID());
			guidToName.insert(make_pair(temp->getGUID(), temp->getName()));
		}
	}
	if(par.error()){
		std::cout << "ERROR: parsing the file -> " << par.errstr() << std::endl;
		return;
	}
	std::cout << "Initialized NameResolver" << std::endl;
}
