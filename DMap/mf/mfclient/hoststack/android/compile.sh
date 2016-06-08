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
	mkdir stack
	mkdir stack/jni
	mkdir stack/jni/include
	mkdir bin
}

stack()
{
	echo "Building stack"
	if [[ -z "$MF_HOME" ]];
	then
		echo "ERROR: MF_HOME is not defined!"
		exit 1
	fi
	cp libs/libpcap.so stack/jni
	cp libs/libcrypto.so stack/jni
	cp -r include/pcap/* stack/jni/include
	cp -r include/openssl stack/jni/include/
	cp src/* stack/jni/
	cp -r $PJ_DIR/src/* stack/jni
	cp resources/Android.mk.stack stack/jni/Android.mk
	cp resources/Application.mk.stack stack/jni/Application.mk
	cd stack
	ndk-build
	cd ..
	mv stack/libs/armeabi-v7a/libgnustl_shared.so bin/
	mv stack/libs/armeabi-v7a/libpcap.so bin/
	mv stack/libs/armeabi-v7a/mfandroidstack bin/
	echo "stack succesfully built and available in bin folder"
}

if [ $# -gt 1 ]
then
	usage
fi

CLEAN="clean"
ST="stack"

if [ $# -eq 1 ]
then
	if [ "$1" == "$CLEAN" ]
	then
		echo "cleaning up the folder"
		rm -rf stack
		rm -rf bin
		exit
	elif [ "$1" == "$ST" ]
	then
		prepare
		stack
		exit
	fi
fi

prepare
stack
