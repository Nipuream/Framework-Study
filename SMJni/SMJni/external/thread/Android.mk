LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES:=\
  bionic \
  $(LOCAL_PATH) \
  $(LOCAL_PATH)/../include \
  frameworks/native/include
  
LOCAL_SRC_FILES:=\
  main.cpp
  
LOCAL_SHARED_LIBRARIES:=\
  libcutils \
  liblog \
  libutils \
  libbinder

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := thread
include $(BUILD_EXECUTABLE)