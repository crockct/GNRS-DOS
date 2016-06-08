/*
 *
 * File: edu_rutgers_winlab_jmfapi_JMFAPI.c
 *
 * Author: Francesco Bronzino
 *
 * Description:
 *	JNI wrapper that connects the java code to the c API. If needed it needs to be compiled
 *	together with the main library.
 *	At the moment there is a one-to-one correspondence between the JNI functions and the c API functions
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <mobilityfirst/mfapi.h>
#include "../log.h"

#include "edu_rutgers_winlab_jmfapi_JMFAPI.h"

jint throwCException(JNIEnv *env, char *message){
	jclass exClass;
	exClass = (*env)->FindClass(env, "edu/rutgers/winlab/jmfapi/CException");
	if((*env)->ExceptionOccurred(env)){
		return (jint)-1;
	}
	return (*env)->ThrowNew(env, exClass, message);
}

const char *getStrFromJstr(JNIEnv *env, jstring jstr) {
	const char *str;
	str = (*env)->GetStringUTFChars(env, jstr, NULL);
	if (str == NULL) {
		stdLog("Some error reading a string");
		return NULL; /* OutOfMemoryError already thrown */
	}
	//At the moment should be the java class that takes care of freeing
	//the memory used by the string
	//(*env)->ReleaseStringUTFChars(env, prompt, str);
	return str;
}

char *getDataFromByteA(JNIEnv *env, jbyteArray data) {
	return (char *)(*env)->GetByteArrayElements(env, data, NULL);
}

int toJavaGuid(JNIEnv *env, jobject sGUID, int cSGUID){
	jclass c;
	jfieldID id;
	if(sGUID == NULL) return -1;
	
	c = (*env)->FindClass(env, "edu/rutgers/winlab/jmfapi/GUID");
	if (c==NULL) {
		return -1;
	}
	
	id = (*env)->GetFieldID(env, c, "mGuidVal", "I");
	if (id==NULL) {
		return -1;
	}
	
	(*env)->SetIntField(env, sGUID, id, cSGUID);
	return 0;
}


/*
 * Class:     edu_rutgers_winlab_jmfapi_JMFAPI
 * Method:    mfopen
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_edu_rutgers_winlab_jmfapi_JMFAPI_mfopen(JNIEnv *env, jobject obj, jstring profile, jint opts, jint GUID){
	const char *cProfile;
	int cGUID = (int)GUID;
	mfflag_t cOpts = (mfflag_t)opts;
	cProfile = getStrFromJstr(env, profile);
	struct Handle *h = (struct Handle*)calloc(1, sizeof(struct Handle));
	int ret = mfopen(h, cProfile, cOpts, cGUID);
	if(ret == 0){
	    return (jint)h;
	}
	else{
		char message[128];
		if(ret<0){
			sprintf(message, "%s%d", "Message returned incorrect: ", ret);
		}
		else{
			sprintf(message, "%s%d", "Errno: ", ret);
		}
		free(h);
		throwCException(env, message);
		return (jint)-1;
	}
}

/*
 * Class:     edu_rutgers_winlab_jmfapi_JMFAPI
 * Method:    mfsend
 * Signature: (I[BIILjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_edu_rutgers_winlab_jmfapi_JMFAPI_mfsend(JNIEnv *env, jobject obj, jint handle, jbyteArray buf, jint size, jint dstGUID, jint opts){
	int cSize, ret;
	struct Handle *h;
	void *cData;
	mfflag_t cOpts;
	jboolean jb;
	//Check if this is correct
	cOpts = (mfflag_t)opts;
	h = (struct Handle *)handle;
	cSize = (u_int)size;
	cData = (void *)(*env)->GetByteArrayElements(env, buf, &jb);
	if (cData == NULL) {
	    char message[128];
            sprintf(message, "%s", "Cannot extract bytes to send");
            throwCException(env, message);
	    return (jint)-1;
	}
	ret = (jint)mfsend(h,cData,(u_int)cSize,(int)dstGUID,cOpts);
	//Free up the memory in jdata if it was a copy of the original
	(*env)->ReleaseByteArrayElements(env,buf,cData,JNI_ABORT);
	if(ret >= 0){
    	return (jint)ret;
    }
    else{
    	char message[128];
    	if(ret<0){
    		sprintf(message, "%s%d", "Message returned incorrect: ", ret);
    	}
    	else{
    		sprintf(message, "%s%d", "Errno: ", ret);
    	}
    	free(h);
    	throwCException(env, message);
		return (jint)-1;
    }
}

/*
 * Class:     edu_rutgers_winlab_jmfapi_JMFAPI
 * Method:    mfrecv
 * Signature: (I[BI[II)I
 */
JNIEXPORT jint JNICALL Java_edu_rutgers_winlab_jmfapi_JMFAPI_mfrecv(JNIEnv *env, jobject obj, jint handle, jobject sGUID, jbyteArray data, jint size, jintArray srcAddrs, jint nAddr){
	int  cSize, ret;
	struct Handle *h;
	//void *cData;
	jboolean jb;
	//jbyte *jdata;
	h = (struct Handle *)handle;
	cSize = (u_int)size;
	//In this part instead of reading the data and pass it, it has to create a buffer
	char *tempData;
	tempData = (char *)malloc(cSize);
	int cNAddr = (int)nAddr;
	int *cSrcAddrs = NULL;
	if(cNAddr > 0){
		cSrcAddrs = (*env)->GetIntArrayElements(env, srcAddrs, &jb);
	}
	int cSGUID;
	if(sGUID==NULL){
		//Pass it to mfrecv and then set the data object back in the jvm
		ret = (jint)mfrecv(h, NULL, (void *)tempData,(u_int)cSize,cSrcAddrs,cNAddr);
	}
	else{
		ret = (jint)mfrecv(h, &cSGUID, (void *)tempData,(u_int)cSize,cSrcAddrs,cNAddr);
		toJavaGuid(env, sGUID, cSGUID);
	}
	if (ret>=0) {
		//The buffer need to be assigned to the java object
		(*env)->SetByteArrayRegion(env, data, (jsize)0, (jsize)ret,(jbyte*)tempData);
		free(tempData);
		return (jint)ret;
	}
	else{
    	char message[128];
    	if(ret<0){
    		sprintf(message, "%s%d", "Message returned incorrect: ", ret);
    	}
    	else{
    		sprintf(message, "%s%d", "Errno: ", ret);
    	}
    	free(h);
    	throwCException(env, message);
		return (jint)-1;
	}
}

/*
 * Class:     edu_rutgers_winlab_jmfapi_JMFAPI
 * Method:    mfrecv_blk
 * Signature: (I[BI[II)I
 */
JNIEXPORT jint JNICALL Java_edu_rutgers_winlab_jmfapi_JMFAPI_mfrecv_1blk(JNIEnv *env, jobject obj, jint handle, jobject sGUID, jbyteArray data, jint size, jintArray srcAddrs, jint nAddr){
	int  cSize, ret;
	struct Handle *h;
	//void *cData;
	jboolean jb;
	//jbyte *jdata;
	h = (struct Handle *)handle;
	cSize = (u_int)size;
	//In this part instead of reading the data and pass it, it has to create a buffer
	char *tempData;
	tempData = (char *)malloc(cSize);
	int cNAddr = (int)nAddr;
	int *cSrcAddrs = NULL;
	if(cNAddr > 0) {
		cSrcAddrs = (*env)->GetIntArrayElements(env, srcAddrs, &jb);	
	}
	int cSGUID;
	if(sGUID==NULL){
		//Pass it to mfrecv and then set the data object back in the jvm
		ret = (jint)mfrecv_blk(h, NULL, (void *)tempData,(u_int)cSize,cSrcAddrs,cNAddr);
	}
	else{
		//Pass it to mfrecv and then set the data object back in the jvm
		ret = (jint)mfrecv_blk(h,&cSGUID, (void *)tempData,(u_int)cSize,cSrcAddrs,cNAddr);
		toJavaGuid(env, sGUID, cSGUID);
	}
	if(ret>=0){
		//The buffer need to be assigned to the java object
		(*env)->SetByteArrayRegion(env, data, (jsize)0, (jsize)ret,(jbyte*)tempData);
		free(tempData);
		//Throw exception
		return (jint)ret;
	}
	else{
		char message[128];
    	if(ret<0){
    		sprintf(message, "%s%d", "Message returned incorrect: ", ret);
    	}
    	else{
    		sprintf(message, "%s%d", "Errno: ", ret);
    	}
    	free(h);
    	throwCException(env, message);
		return (jint)-1;
	}
}

/*
 * Class:     edu_rutgers_winlab_jmfapi_JMFAPI
 * Method:    mfattach
 * Signature: (I[II)I
 */
JNIEXPORT jint JNICALL Java_edu_rutgers_winlab_jmfapi_JMFAPI_mfattach(JNIEnv *env, jobject obj, jint handle, jintArray GUIDs, jint nGUID){
	jboolean jb;
	struct Handle *h = (struct Handle *)handle;
	int *cGUIDs = (*env)->GetIntArrayElements(env, GUIDs, &jb);
	int cNGUID = (int)nGUID;
	int ret = mfattach(h, cGUIDs, cNGUID);
	if(ret == 0){
    	    return (jint)ret;
    	}
    	else{
            char message[128];
            sprintf(message, "%s%d", "Message returned incorrect: ", ret);
            throwCException(env, message);
            return (jint)ret;
        }
}

/*
 * Class:     edu_rutgers_winlab_jmfapi_JMFAPI
 * Method:    mfdetach
 * Signature: (I[II)I
 */
JNIEXPORT jint JNICALL Java_edu_rutgers_winlab_jmfapi_JMFAPI_mfdetach(JNIEnv *env, jobject obj, jint handle, jintArray GUIDs, jint nGUID){
	jboolean jb;
	struct Handle *h = (struct Handle *)handle;
	int *cGUIDs = (*env)->GetIntArrayElements(env, GUIDs, &jb);
	int cNGUID = (int)nGUID;
	int ret = mfdetach(h, cGUIDs, cNGUID);
	if(ret == 0){
    	    return (jint)ret;
    	}
        else{
            char message[128];
            sprintf(message, "%s%d", "Message returned incorrect: ", ret);
            throwCException(env, message);
            return (jint)ret;
    	}
}

/*
 * Class:     edu_rutgers_winlab_jmfapi_JMFAPI
 * Method:    mfclose
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_edu_rutgers_winlab_jmfapi_JMFAPI_mfclose(JNIEnv *env, jobject obj, jint handle){
	struct Handle *h;
	h = (struct Handle *)handle;
	int ret = mfclose(h);
	if(ret){
		char message[128];
		if(ret<0){
			sprintf(message, "%s%d", "Message returned incorrect: ", ret);
		}
		else{
			sprintf(message, "%s%d", "Errno: ", ret);
		}
		free(h);
		throwCException(env, message);
		return (jint)-1;
	}
	return (jint)0;
}

