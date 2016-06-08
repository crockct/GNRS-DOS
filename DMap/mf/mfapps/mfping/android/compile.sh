#!/bin/bash

set -e

PJ_DIR="../../../"

usage()
{
	echo "USAGE: c_android [clean]"
	exit $E_BADARGS
}

prepare()
{
	echo "starting the compilation process"
	mkdir mfping
	mkdir mfping/jni
	mkdir mfping/jni/include
	mkdir bin
}

mfping()
{
	echo "Building mfping"
	cp $PJ_DIR/netapi/android/bin/libmfapi.so mfping/jni
	cp -r $PJ_DIR/netapi/c/*.h mfping/jni/include
	cp ../src/*.c mfping/jni
	cp ../src/*.h mfping/jni
	cp resources/Android.mk.ping mfping/jni/Android.mk
	cd mfping
	ndk-build
	cd ..
	mv mfping/libs/armeabi/* bin/
	echo "mfping succesfully built and available in bin folder"
}

if [ $# -gt 1 ]
then
	usage
fi

CLEAN="clean"
PI="mfping"

if [ $# -eq 1 ]
then
	if [ "$1" == "$CLEAN" ]
	then
		echo "cleaning up the folder"
		rm -rf mfping
		rm -rf bin
		exit
	fi
fi

prepare
mfping
