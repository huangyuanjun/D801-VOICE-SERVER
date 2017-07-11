LOCAL_PATH:=$(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=codec_set.c
LOCAL_MODULE:=codec

include $(BUILD_EXECUTABLE)
