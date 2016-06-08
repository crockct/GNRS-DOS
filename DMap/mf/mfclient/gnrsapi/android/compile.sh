#!/bin/bash

set -e

PJ_DIR=".."

usage()
{
	echo "USAGE: c_android [clean]"
	exit $E_BADARGS
}

prepare()
{
	echo "starting the compilation process"
	mkdir gnrsapi
	mkdir gnrsapi/jni
	mkdir gnrsapi/java_api
	mkdir bin
}


gnrsapi()
{
	echo "Building libgnrs"
	cp $PJ_DIR/cpp/gnrs_cxx.cpp $PJ_DIR/cpp/edu_rutgers_winlab_jgnrs_JGNRS.cpp $PJ_DIR/cpp/log.cpp $PJ_DIR/cpp/udpipv4_endpoint.cpp gnrsapi/jni
	cp $PJ_DIR/cpp/*.h gnrsapi/jni
	cp Android.mk gnrsapi/jni/
	cp Application.mk gnrsapi/jni/
	cd gnrsapi
	ndk-build
	cd ..
	mv gnrsapi/libs/armeabi/libgnrs.so bin/
	echo "libgnrs succesfully built and available in bin folder"
}

jgnrsapi()
{
	echo "Building java api"
	cp $PJ_DIR/java/src/main/edu/rutgers/winlab/jgnrs/*.java gnrsapi/java_api
	cd gnrsapi/java_api
	javac -d . *.java
	jar cf jgnrs.jar edu/rutgers/winlab/jgnrs/
	cd ../../
	mv gnrsapi/java_api/jgnrs.jar bin/
	echo "jgnrs.jar succesfully built and available in bin folder"
}

if [ $# -gt 1 ]
then
	usage
fi

CLEAN="clean"

if [ $# -eq 1 ]
then
	if [ "$1" == "$CLEAN" ]
	then
		echo "cleaning up the folder"
		rm -rf gnrsapi
		rm -rf bin
		exit
	fi
fi

prepare
gnrsapi
jgnrsapi
