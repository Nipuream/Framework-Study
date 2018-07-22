LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)


LOCAL_C_INCLUDES := \
    bionic \
    $(my_ndk_stl_include_path) \
    $(LOCAL_PATH) \
    $(call include-path-for, corecg graphics)

LOCAL_SRC_FILES:= \
    base_socket.cpp \
	main.cpp \
	yanghuiService.cpp \
	IyanghuiService.cpp \
	socket_test.cpp \
	file_operator.cpp \

LOCAL_SHARED_LIBRARIES := \
	libcutils \
    liblog \
	$(my_ndk_stl_shared_lib) \
	libutils \
	libbinder \

LOCAL_LDLIBS += -pthread

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES


LOCAL_PRELINK_MODULE := false

LOCAL_MODULE:= yanghui11
include $(BUILD_EXECUTABLE)
