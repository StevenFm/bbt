LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := luna

LOCAL_MODULE_FILENAME := libluna

LOCAL_SRC_FILES := $(wildcard $(LOCAL_PATH)/../src/*.c)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../src/*.cpp)
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES:$(LOCAL_PATH)/%=%)

$(info SRC= $(LOCAL_SRC_FILES))

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../include/luna

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../include/luna
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../cocos2dx/cocos2dx
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../cocos2dx/cocos2dx/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../cocos2dx/cocos2dx/platform
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../cocos2dx/cocos2dx/platform/android

include $(BUILD_STATIC_LIBRARY)
