LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:=\
	edu_rutgers_winlab_jgnrs_JGNRS.cpp\
	gnrs_cxx.cpp\
	log.cpp\
	udpipv4_endpoint.cpp 
	
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog 

LOCAL_MODULE:= libgnrs

include $(BUILD_SHARED_LIBRARY)
