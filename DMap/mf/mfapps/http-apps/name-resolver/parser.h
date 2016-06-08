/*
 *
 * User: wontoniii
 *
 * File: Parser.h
 *
 * Description: classes that contains the parser of the pair file and the functions to access it
 *
 * Created on: Dec 12, 2013
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <fstream>
#include <list>
#include <string.h>
#include <iostream>

class NamePair{
	std::string name;
	int GUID;
	
public:
	
	NamePair(){
	}
	
	NamePair(std::string n, int g){
		name = n;
		GUID = g;
	}
	
	std::string getName(){
		return name;
	}
	
	void setName(const char *n){
		name.assign(n);
	}
	
	void setName(std::string n){
		name.assign(n.c_str());
	}
	
	int getGUID(){
		return GUID;
	}
	
	void setGUID(int g){
		GUID = g;
	}
	
	void print(){
		std::cout << "Pair: name = " << name << "  GUID = " << GUID << std::endl;
	}
};

class Parser {
	
private:
	//Place here all settings to be read
	bool errorOccurred;
	std::string errDescr;
	
	std::list <NamePair *> pairList;
	std::list<NamePair *>::iterator it;
	
	NamePair *parsePair(char *ptr);
	char *removeSpaces(char *ptr);
	
	
public:
	Parser();
	~Parser();
	
	void init(const char *filename);
	void init(std::string filename);
	void init();
	
	void setPairsIterator(){
		it = pairList.begin();
	}
	
	NamePair *getNextPair(){
		if(it == pairList.end()){
			return NULL;
		}
		NamePair *ret = (*it);
		it++;
		return ret;
	}
	
	bool error(){
		return errorOccurred;
	}
	
	const char *errstr(){
		return errDescr.c_str();
	}
};

#endif /* PARSER_H_ */
