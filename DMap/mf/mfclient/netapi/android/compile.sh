#!/bin/bash

set -e

PJ_DIR=".."

usage()
{
	echo "USAGE: compile [clean]"
	exit $E_BADARGS
}

prepare()
{
	echo "starting the compilation process"
	mkdir -p mfapi
	mkdir -p mfapi/jni
	mkdir -p mfapi/jni/jni
	mkdir -p mfapi/jmfapi
	mkdir -p bin
}

mfapi()
{
	echo "Building libmfapi"
	cp $PJ_DIR/c/src/*.c mfapi/jni
	cp $PJ_DIR/c/src/*.h mfapi/jni
	cp $PJ_DIR/c/src/jni/*.c mfapi/jni/jni
	cp $PJ_DIR/c/src/jni/*.h mfapi/jni/jni
	cp resources/Android.mk.mfapi mfapi/jni/Android.mk
	cd mfapi
	ndk-build
	cd ..
	mv mfapi/libs/armeabi/libmfapi_jni.so bin/
	echo "libmfapi succesfully built and available in bin folder"
}

jmfapi()
{
	echo "Building java api"
	cp $PJ_DIR/java/src/main/java/edu/rutgers/winlab/jmfapi/*.java mfapi/jmfapi
	cd mfapi/jmfapi
	javac -d . *.java
	jar cf jmfapi.jar edu/rutgers/winlab/jmfapi/
	cd ../../
	mv mfapi/jmfapi/jmfapi.jar bin/
	echo "jmfapi.jar succesfully built and available in bin folder"
}

if [ $# -gt 1 ]
then
	usage
fi

CLEAN="clean"
MFA="mfapi"
JMF="jmfapi"

if [ $# -eq 1 ]
then
	if [ "$1" == "$CLEAN" ]
	then
		echo "cleaning up the folder"
		rm -rf mfapi
		rm -rf bin
		exit
	elif [ "$1" == "$MFA" ]
	then
		prepare
		mfapi
		exit
	elif [ "$1" == "$JMF" ]
	then
		prepare
		jmfapi
		exit
	fi
fi

prepare
mfapi
jmfapi
