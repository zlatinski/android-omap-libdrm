# Copyright 2010-2012, The Android-x86 Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0

ifneq ($(strip $(BOARD_GPU_DRIVERS)),)

LOCAL_PATH := $(call my-dir)

################ DRM Lib #######################

include $(CLEAR_VARS)

LOCAL_SRC_FILES :=			\
	xf86drm.c			\
	xf86drmHash.c			\
	xf86drmMode.c			\
	xf86drmRandom.c			\
	xf86drmSL.c

LOCAL_C_INCLUDES +=			\
	$(LOCAL_PATH)/include/drm

LOCAL_MODULE := libdrm
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

################ OMAP DRM Lib #######################

include $(CLEAR_VARS)

LOCAL_SRC_FILES :=			\
	omap/omap_drm.c			\

LOCAL_C_INCLUDES +=			\
	$(LOCAL_PATH)/include/drm	\
	$(LOCAL_PATH)/omap

LOCAL_CFLAGS += -DHAVE_LIBDRM_ATOMIC_PRIMITIVES=1

LOCAL_MODULE := libdrm_omap
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := libdrm

include $(BUILD_SHARED_LIBRARY)

################ KMS Lib #######################

include $(CLEAR_VARS)

LOCAL_SRC_FILES :=			\
	libkms/api.c			\
	libkms/dumb.c			\
	libkms/omap.c			\
	libkms/intel.c			\
	libkms/linux.c

LOCAL_C_INCLUDES +=				\
	$(LOCAL_PATH)/include/drm	\
	$(LOCAL_PATH)/omap			\

LOCAL_MODULE := libkms
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := libdrm

include $(BUILD_SHARED_LIBRARY)

################ Test Apps #######################

include $(CLEAR_VARS)

LOCAL_SRC_FILES :=				\
	tests/modetest/modetest.c		\
	tests/modetest/buffers.c

LOCAL_C_INCLUDES +=				\
	$(LOCAL_PATH)/include/drm		\
	$(LOCAL_PATH)/libkms			\
	$(LOCAL_PATH)/omap			\
	$(LOCAL_PATH)/tests/modetest

LOCAL_MODULE := modetest
LOCAL_MODULE_TAGS := debug

LOCAL_SHARED_LIBRARIES :=			\
	libdrm							\
	libkms							\
	libdrm_omap

include $(BUILD_EXECUTABLE)

endif
