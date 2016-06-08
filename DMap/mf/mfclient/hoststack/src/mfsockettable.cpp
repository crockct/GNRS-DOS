/*
 *  Copyright (c) 2010-2013, Rutgers, The State University of New Jersey
 *  All rights reserved.
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of the organization(s) stated above nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file   mfsockettable.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   August, 2013
 * @brief  Class that handles the set of created sockets.
 *
 * Class that handles the set of created sockets.
 */

#include "mfsockettable.h"

/*
 * Default constructror
 */
MF_SocketTable::MF_SocketTable(){
	
}

/*
 * Default destructror
 */
MF_SocketTable::~MF_SocketTable(){
	for(IdMap::iterator it=tableById.begin(); it!=tableById.end(); ++it)
		delete it->second;
}

/*
 * Adding a socket manager to the object
 */
void MF_SocketTable::addManager(MF_SocketManager *sockMan){
	MF_Log::mf_log(MF_DEBUG, "MF_SocketTable:addManager add manager to by ID");
	tableById[sockMan->getID()] = sockMan;
}

/*
 * Removing a socket manager to the object
 */
void MF_SocketTable::removeManager(u_int id){
	MF_Log::mf_log(MF_DEBUG, "MF_SocketTable:removeManager remove manager from available ones");
	IdMap::iterator it;
	it = tableById.find(id);
	if(it!=tableById.end()){
		//Implement getIterator;
		list <int>::iterator intID;
		list <u_int>::iterator itID;
		for(intID = it->second->getGUIDBegin(); intID!=it->second->getGUIDEnd(); ++intID){
			tableByGUID.erase(*intID);
		}
		for(itID = it->second->getTIDBegin(); itID!=it->second->getTIDEnd(); ++itID){
			tableByTID.erase(*itID);
		}
		tableById.erase(it);
	}
}

/*
 * Get socket manager by id
 */
MF_SocketManager *MF_SocketTable::getById(u_int ID){
	MF_Log::mf_log(MF_DEBUG, "MF_SocketTable:getById");
	IdMap::iterator it;
	it = tableById.find(ID);
	if(it!=tableById.end()) return it->second;
	else return NULL;
}

/*
 * Add GUID to the socket identified by id in tables
 */
void MF_SocketTable::addGUID(u_int id, int GUID){
	MF_Log::mf_log(MF_DEBUG, "MF_SocketTable:addGUID");
	IdMap::iterator it;
	it = tableById.find(id);
	if(it==tableById.end()) return;
	if(!tableByGUID.count(GUID)){
		tableByGUID[GUID].insert(make_pair(id,it->second));
	}
}

/*
 * Remove GUID to the socket identified by id in tables
 */
void MF_SocketTable::removeGUID(u_int id, int GUID){
	MF_Log::mf_log(MF_DEBUG, "MF_SocketTable:removeGUID");
	IdMap::iterator it;
	it = tableById.find(id);
	if(it==tableById.end()) return;
	if(tableByGUID.count(GUID)){
		tableByGUID[GUID].erase(id);
	}
}

/*
 * Check if there is an open socket with that GUID
 */
bool MF_SocketTable::hasGUID(int GUID){
	MF_Log::mf_log(MF_DEBUG, "MF_SocketTable:hasGUID");
	map <int, IdMap>::iterator it;
	it = tableByGUID.find(GUID);
	if(it==tableByGUID.end()){
		return false;
	}
	return true;
}

/*
 * Return a pointer to a map that contains all MF_SocketManagers open on a particular GUID
 */
IdMap *MF_SocketTable::getByGUID(int GUID){
	MF_Log::mf_log(MF_DEBUG, "MF_SocketTable:getByGUID");
	map <int, IdMap>::iterator it;
	it = tableByGUID.find(GUID);
	if(it==tableByGUID.end()){
		return NULL;
	}
	return &(it->second);
}

/*
 * Add TID to the socket identified by id
 */
void MF_SocketTable::addTID(u_int id, u_int TID){
	MF_Log::mf_log(MF_DEBUG, "MF_SocketTable:addTID");
	IdMap::iterator it;
	it = tableById.find(id);
	if(it==tableById.end()) return;
	it->second->addTID(TID);
	if(!tableByTID.count(TID)){
		tableByTID[TID].insert(make_pair(id,it->second));
	}
}

/*
 * Remove TID to the socket identified by id
 */
void MF_SocketTable::removeTID(u_int id, u_int TID){
	MF_Log::mf_log(MF_DEBUG, "MF_SocketTable:removeTID");
	IdMap::iterator it;
	it = tableById.find(id);
	if(it==tableById.end()) return;
	it->second->removeTID(TID);
	if(tableByTID.count(TID)){
		tableByTID[TID].erase(id);
	}
}

/*
 * Return a pointer to a MF_SocketManager open on a particular TID
 */
MF_SocketManager *MF_SocketTable::getByTID(u_int TID){
	MF_Log::mf_log(MF_DEBUG, "MF_SocketTable:getByTID");
	map <u_int, IdMap>::iterator it;
	it = tableByTID.find(TID);
	if(it==tableByTID.end()){
		return NULL;
	}
	IdMap::iterator it2 = it->second.begin();
	return it2->second;
}
