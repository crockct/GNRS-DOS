#include <pthread.h>

#include "../cpp/gnrs_cxx.h"
#include "../cpp/guid.h"

#define MAX_GNRS_OBJECTS 32
#define MAX_GUID_OBJECTS 1024

/* errors returned by some object pool methods */
#define ERR_OBJPOOL_CAPACITY_REACHED -10
#define ERR_OBJPOOL_NOT_INITED -11
#define ERR_OBJPOOL_NOT_FOUND -12

/** 
 * Status flag to indicate object pool has been inited and synchronization 
 * vars have been set up
 */
bool objectPoolInited = false;

Gnrs* gnrsObjArr[MAX_GNRS_OBJECTS];
pthread_mutex_t gnrsObjPoolMutex = PTHREAD_MUTEX_INITIALIZER;

Guid* guidObjArr[MAX_GUID_OBJECTS];
pthread_mutex_t guidObjPoolMutex = PTHREAD_MUTEX_INITIALIZER;

void initObjectPool() {
    
    pthread_mutex_lock(&gnrsObjPoolMutex);
    pthread_mutex_lock(&guidObjPoolMutex);
    if(!objectPoolInited) {
        for(int i = 0; i < MAX_GNRS_OBJECTS; i++) {
            gnrsObjArr[i] = NULL;
        }
        for(int i = 0; i < MAX_GUID_OBJECTS; i++) {
            guidObjArr[i] = NULL;
        }
        objectPoolInited = true;
    }
    pthread_mutex_unlock(&gnrsObjPoolMutex);
    pthread_mutex_unlock(&guidObjPoolMutex);
}

Gnrs* getGnrsObj(int objIndex) {

    Gnrs *gnrsObj = NULL;
    if(!objectPoolInited) return NULL;

    pthread_mutex_lock(&gnrsObjPoolMutex);
    if(objIndex < MAX_GNRS_OBJECTS) {
        gnrsObj = gnrsObjArr[objIndex];
    }
    pthread_mutex_unlock(&gnrsObjPoolMutex);

    return gnrsObj;
}

int putGnrsObj(Gnrs* gnrsObj) {

    int objIndex = ERR_OBJPOOL_CAPACITY_REACHED;
    if(!objectPoolInited) return ERR_OBJPOOL_NOT_INITED;

    pthread_mutex_lock(&gnrsObjPoolMutex);
    for(int i = 0; i < MAX_GNRS_OBJECTS; i++) {
        if(gnrsObjArr[i] == NULL) {
            gnrsObjArr[i] = gnrsObj;
            objIndex = i;
            break;
        }
    }
    pthread_mutex_unlock(&gnrsObjPoolMutex);

    return objIndex;
}

int deleteGnrsObj(int objIndex) {

    int result = ERR_OBJPOOL_NOT_FOUND;
    if(!objectPoolInited) return ERR_OBJPOOL_NOT_INITED;

    pthread_mutex_lock(&gnrsObjPoolMutex);
    if(objIndex < MAX_GNRS_OBJECTS && gnrsObjArr[objIndex] != NULL) {
        delete(gnrsObjArr[objIndex]);
        result = 0;
    }
    pthread_mutex_unlock(&gnrsObjPoolMutex);

    return result;
}

Guid* getGuidObj(int objIndex) {

    Guid *guidObj = NULL;
    if(!objectPoolInited) return NULL;

    pthread_mutex_lock(&guidObjPoolMutex);
    if(objIndex < MAX_GUID_OBJECTS) {
        guidObj = guidObjArr[objIndex];
    }
    pthread_mutex_unlock(&guidObjPoolMutex);

    return guidObj;
}

int putGuidObj(Guid* guidObj) {

    int objIndex = ERR_OBJPOOL_CAPACITY_REACHED;
    if(!objectPoolInited) return ERR_OBJPOOL_NOT_INITED;

    pthread_mutex_lock(&guidObjPoolMutex);
    for(int i = 0; i < MAX_GUID_OBJECTS; i++) {
        if(guidObjArr[i] == NULL) {
            guidObjArr[i] = guidObj;
            objIndex = i;
            break;
        }
    }
    pthread_mutex_unlock(&guidObjPoolMutex);

    return objIndex;
}

int deleteGuidObj(int objIndex) {

    int result = ERR_OBJPOOL_NOT_FOUND;
    if(!objectPoolInited) return ERR_OBJPOOL_NOT_INITED;

    pthread_mutex_lock(&guidObjPoolMutex);
    if(objIndex < MAX_GNRS_OBJECTS && gnrsObjArr[objIndex] != NULL) {
        delete(guidObjArr[objIndex]);
        result = 0;
    }
    pthread_mutex_unlock(&guidObjPoolMutex);
    
    return result;
}

