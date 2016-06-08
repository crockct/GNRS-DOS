/*
 *
 * User: wontoniii
 *
 * File: Parser.cpp
 *
 * Description: classes that contains the parser of the pair file and the functions to access it
 *
 * Created on: Dec 12, 2013
 */

#include <stdlib.h>
#include "parser.h"

Parser::Parser(){
	errorOccurred = false;
}

Parser::~Parser(){
	
}

char *Parser::removeSpaces(char *ptr){
	char *init = ptr, *end;
	while(*init && (*init == ' '))init++;
	end = init;
	while(*end && *end != ' ' && *end != '\t')end++;
	if(*end == ' ' || *end == '\t' || *end == '\n') *(end) = '\0';
	return init;
}

NamePair *Parser::parsePair(char *ptr){
	char *temp;
	NamePair *newPair = new NamePair();
	temp = strtok(ptr, ",");
	temp = removeSpaces(temp);
	if(strlen(temp) >= 1){
		newPair->setName(temp);
	}
	else{
		errorOccurred = true;
		errDescr.append("error: empty name");
	}
	temp = strtok(NULL, " \n\t#");
	temp = removeSpaces(temp);
	newPair->setGUID(atoi(temp));
	return newPair;
}

void Parser::init(const char *filename){
	std::ifstream in(filename, std::ios::in);
	if (!in){
		//std::cout << "error opening file " << filename << std::endl;
		errDescr.append("error opening file ");
		errDescr.append(filename);
		errorOccurred = true;
		return;
	}
	char *ptr,buffer[512];
	NamePair *temp;
	while (!in.eof()){
		//extracting all pairs key-value
		in.getline(buffer,512);
		if(!*buffer || *buffer == '#')continue;
		ptr = buffer;
		temp = parsePair(ptr);
		if(!errorOccurred){
			pairList.push_back(temp);
			//std::cout << "New pair parsed:" << std::endl;
			//temp->print();
		}
		else{
			std::cerr << "Error parsing settings: " << errDescr.c_str() << std::endl;
			break;
		}
		memset(buffer, 0, 512);
	}
	in.close();
	if(!errorOccurred && pairList.size() == 0){
		errDescr.append("Error: no pair specified");
		errorOccurred = true;
		std::cerr << "No pair specified in file" << std::endl;
	}
}

void Parser::init(){
	init(".settings");
}

void Parser::init(std::string filename){
	init(filename.c_str());
}
