#include "edu_rutgers_winlab_jgnrs_JGNRS.h"
#include "log.h"
#include "../cpp/gnrs_cxx.h"
#include "../cpp/guid.h"
#include "../cpp/net_addr.h"

/*
 * Convert a Java string to a c "string"
 */
char *getStrFromJstr(JNIEnv *env, jstring jstr) {
	char *str;
	str = (char*)env->GetStringUTFChars(jstr, NULL);
	if (str == NULL) {
		return NULL; /* OutOfMemoryError already thrown */
	}
	return str;
}

/*
 * From jbyteArray to array of chars
 */
char *getDataFromByteA(JNIEnv *env, jbyteArray data) {
	return (char *)(env->GetByteArrayElements(data, NULL));
}

/*
 * Class:     edu_rutgers_winlab_jgnrs_JGNRS
 * Method:    setNativeGNRS
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_edu_rutgers_winlab_jgnrs_JGNRS_setNativeGNRS(JNIEnv *env, jobject obj, jstring remote, jstring local){
	try{
		//Set up the gnrs object
		string server_addr_s(getStrFromJstr(env, remote));
		string local_addr_s(getStrFromJstr(env, local));
		NetAddr server_addr(NET_ADDR_TYPE_IPV4_PORT, server_addr_s);
		NetAddr local_addr(NET_ADDR_TYPE_IPV4_PORT, local_addr_s);
		Gnrs *gnrs = new Gnrs(server_addr, local_addr);
		//Return the pointer at the memory used for the gnrs
		return (jint) gnrs;
	} catch(string error){
		errLog(error.c_str());
		return (jint)0;
	} catch(...){
		errLog("Default exception");
		return (jint)0;
	}
}

/*
 * Class:     edu_rutgers_winlab_jgnrs_JGNRS
 * Method:    freeNativeGNRS
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_edu_rutgers_winlab_jgnrs_JGNRS_freeNativeGNRS(JNIEnv *env, jobject obj, jint gnrs){
	try{
		Gnrs *g = (Gnrs *)gnrs;
		if(g!=NULL){
			delete g;
		}
	} catch(...){
		errLog("Default exception");
	}
}

/*
 * Class:     edu_rutgers_winlab_jgnrs_JGNRS
 * Method:    setNativeGUID
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_edu_rutgers_winlab_jgnrs_JGNRS_setNativeGUID(JNIEnv *env, jobject obj, jint guid){
	try{
		Guid *g = new Guid();
		Guid temp = Guid::from_unsigned_int(guid);
		for(int i = 0; i<GUID_BINARY_SIZE; i++){
			g->bytes[i] = temp.bytes[i];
		}
		g->str = temp.str;
		return (jint)g;
	} catch(string error){
		errLog(error.c_str());
		return (jint)0;
	} catch(...){
		errLog("Default exception");
		return (jint)0;
	}
}

/*
 * Class:     edu_rutgers_winlab_jgnrs_JGNRS
 * Method:    freeNativeGUID
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_edu_rutgers_winlab_jgnrs_JGNRS_freeNativeGUID(JNIEnv *env, jobject obj, jint guid){
	try{
		Guid *g = (Guid *)guid;
		free(g);
	} catch(...){
		errLog("Default exception");
	}
}

/*
 * Class:     edu_rutgers_winlab_jgnrs_JGNRS
 * Method:    gnrs_lookup
 * Signature: (IILjava/lang/String;)[I
 */
JNIEXPORT jintArray JNICALL Java_edu_rutgers_winlab_jgnrs_JGNRS_gnrs_1lookup(JNIEnv *env, jobject obj, jint gnrs, jint guid){
	try{
		Guid *nguid = (Guid *)guid;
		Gnrs *ngnrs = (Gnrs *)gnrs;
		stdLog("Performing lookup");
		list<NetAddr> lkup_addrs = ngnrs->lookup(*nguid);
		stdLog("Lookup finished. Starting preparing data for upper layers");
		int guids[lkup_addrs.size()];
		int i=0;
		stdLog("Variables initialized");
		for (list<NetAddr>::const_iterator it = lkup_addrs.begin();
			 it != lkup_addrs.end(); it++) {
			guids[i] = atoi((*it).get_value().c_str());
			i++;
		}
		stdLog("Variables set");
		jintArray out_ints = env->NewIntArray(lkup_addrs.size());
		stdLog("Java variables ready");
		env->SetIntArrayRegion(out_ints, 0, lkup_addrs.size(), guids);
		stdLog("Java variables set");
		return out_ints;
	} catch(string error){
		errLog(error.c_str());
		return env->NewIntArray(0);
	} catch(...){
		errLog("Default exception");
		return  env->NewIntArray(0);
	}
}

/*
 * Class:     edu_rutgers_winlab_jgnrs_JGNRS
 * Method:    gnrs_add
 * Signature: (I[IILjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_edu_rutgers_winlab_jgnrs_JGNRS_gnrs_1add(JNIEnv *env, jobject obj, jint gnrs, jint guid, jintArray nas, jint size){
	try{
		Gnrs *ngnrs = (Gnrs *)gnrs;
		Guid *nguid = (Guid *)guid;
		char tempAddr[32];
		int *nnas = env->GetIntArrayElements(nas, NULL);
		int s = (int)size;
		list<NetAddr> addrs;
		for(int i = 0; i<s; i++){
			sprintf(tempAddr, "%d", nnas[i]);
            NetAddr na(NET_ADDR_TYPE_GUID, tempAddr);
            addrs.push_back(na);
		}
		ngnrs->add(*nguid, addrs);
		return (jint)1;
	} catch(string error){
		errLog(error.c_str());
		return (jint)0;
	} catch(...){
		errLog("Default exception");
		return (jint)0;
	}
}


