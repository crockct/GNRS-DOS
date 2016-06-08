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
 * @file   mfsockettable.h
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   August, 2013
 * @brief  Class that handles the set of created sockets.
 *
 * Class that handles the set of created sockets.
 */

#ifndef MF_SOCKET_TAB
#define MF_SOCKET_TAB

#include <map>
#include <list>

#include "mftypes.h"
#include "mfsocketmanager.h"

using namespace std;

class MF_SocketManager;

typedef map<u_int, MF_SocketManager *> IdMap;

class MF_SocketTable{
	//Map <socket id, socketManager>
	IdMap tableById;
	//Map <GUID, <socket id, socketManager>>
	map <int, IdMap > tableByGUID;
	//Map <TID, <socket id, socketManager>>
	map <u_int, IdMap> tableByTID;

public:
	MF_SocketTable();
	~MF_SocketTable();

	void addManager(MF_SocketManager *);
	void removeManager(u_int);
	MF_SocketManager *getById(u_int);
	
	void addGUID(u_int, int);
	void removeGUID(u_int, int);
	bool hasGUID(int);
	IdMap *getByGUID(int);
	
	void addTID(u_int, u_int);
	void removeTID(u_int, u_int);
	MF_SocketManager *getByTID(u_int);
};

#endif /* MF_SOCKET_TAB */
