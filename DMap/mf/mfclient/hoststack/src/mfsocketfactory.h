/*
 * mfsocketfactory.h
 *
 *  Created on: Mar 14, 2015
 *      Author: wontoniii
 */

#ifndef MFSOCKETFACTORY_H_
#define MFSOCKETFACTORY_H_


#include "mfsocketmanager.h"
#include "mfsystem.h"

class MF_SocketFactory {

public:

	static MF_SocketManager *getInstance(MFSystem *_system, int type, u_int UID);
	static MF_SocketManager *getInstance(MFSystem *_system,const char *profile, u_int UID);
	static MF_SocketManager *getInstance(MFSystem *_system,int type, u_int UID, int nChunks);
	static MF_SocketManager *getInstance(MFSystem *_system,const char *profile, u_int UID, int nChunks, TransportSetting ts);
};


#endif /* MFSOCKETFACTORY_H_ */
