
LOCAL_PATH := $(call my-dir)

#--- build binder demo with linkToDeath
include $(CLEAR_VARS)
LOCAL_MODULE := binder_demo
## LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := binder.cpp
LOCAL_SRC_FILES := binder_demo.cpp
LOCAL_SHARED_LIBRARIES := libutils libcutils libbinder liblog
LOCAL_C_INCLUDES += frameworks/native/include system/core/include
include $(BUILD_EXECUTABLE)

#--- enable binder demo to run from single executable
include $(CLEAR_VARS)
LOCAL_MODULE := binder_single
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).cpp
LOCAL_SHARED_LIBRARIES := libutils libcutils libbinder liblog
LOCAL_C_INCLUDES += frameworks/native/include system/core/include
include $(BUILD_EXECUTABLE)

#--- build binder demo with listener
include $(CLEAR_VARS)
LOCAL_MODULE := binder_listener
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).cpp
LOCAL_SHARED_LIBRARIES := libutils libcutils libbinder liblog
LOCAL_C_INCLUDES += frameworks/native/include system/core/include
include $(BUILD_EXECUTABLE)

