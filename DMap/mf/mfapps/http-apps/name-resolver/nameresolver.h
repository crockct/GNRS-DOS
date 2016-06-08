/*
 *
 * User: wontoniii
 *
 * File: nameresolver.h
 *
 * Description: class that contains the name service that replies maintain name to GUIDs bindings and reverse lookups.
 *
 * Created on: Jun, 2014
 */

#ifndef NAMERESOLVER_H_
#define NAMERESOLVER_H_

#include <map>
#include <vector>
#include <list>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include "parser.h"

class NameResolver{
	
public:
	
	NameResolver();
	~NameResolver();
	
	void init(std::string filename);
	
	std::vector <int> *getGuids(const std::string name);
	std::string getName(const int guid);
	std::list<int> *getAllGuids();
	std::list<std::string> *getAllNames();
	
private:
	std::map <std::string, std::vector <int> *> nameToGuid;
	std::map <int, std::string> guidToName;
	int replyTo();
	int getNext();
	
};

#endif /* NAMERESOLVER_H_ */
