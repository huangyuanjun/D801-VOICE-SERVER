LOCAL_PATH:=$(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=seiver.c \
		./codec\
		./powerspeaker

LOCAL_MODULE:=codecdsp_seiver

include $(BUILD_EXECUTABLE)
