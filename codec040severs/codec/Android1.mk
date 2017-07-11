
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := \
        libcutils
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := ld_dspcfg.cpp\
	./ld_cfg.c\
	./zl380twolf_config.c

LOCAL_C_INCLUDES += \
        $(JNI_H_INCLUDE)

LOCAL_MODULE := libCodec_jni

include $(BUILD_SHARED_LIBRARY)
