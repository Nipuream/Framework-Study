LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)


LOCAL_C_INCLUDES := \
    $(LOCAL_PATH) \
	
LOCAL_SRC_FILES:= \
    smoperator.cpp \
	main.cpp \
	
LOCAL_SHARED_LIBRARIES := \
	libcutils \
    liblog \
	libnativehelper \


LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE:= sm
include $(BUILD_SHARED_LIBRARY)