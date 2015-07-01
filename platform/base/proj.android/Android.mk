LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := base_static

LOCAL_MODULE_FILENAME := lib_base

BASE_CPP_FILES := $(wildcard $(LOCAL_PATH)/../src/*.cpp)
BASE_CPP_FILES := $(BASE_CPP_FILES:$(LOCAL_PATH)/%=%) 

BASE_C_FILES := $(wildcard $(LOCAL_PATH)/../src/*.c)
BASE_C_FILES := $(BASE_C_FILES:$(LOCAL_PATH)/%=%) 

LOCAL_SRC_FILES := $(BASE_CPP_FILES) 
LOCAL_SRC_FILES += $(BASE_C_FILES) 

$(info Base.LOCAL_SRC_FILES= $(LOCAL_SRC_FILES))

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../include

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../include
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../include/base
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../cocos2dx/cocos2dx
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../cocos2dx/cocos2dx/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../cocos2dx/cocos2dx/platform
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../cocos2dx/cocos2dx/platform/android

include $(BUILD_STATIC_LIBRARY)
