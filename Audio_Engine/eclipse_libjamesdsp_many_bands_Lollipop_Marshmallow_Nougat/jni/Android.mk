LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    := fftw3
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
LOCAL_SRC_FILES := libfftw3f.a
else ifeq ($(TARGET_ARCH_ABI), x86)
LOCAL_SRC_FILES := libfftw3fx86.a
endif
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)
LOCAL_MODULE := libjamesdsp
LOCAL_PRELINK_MODULE := false
LOCAL_SRC_FILES := \
	jamesdsp.cpp \
	Effect.cpp \
	EffectDSPMain.cpp \
	iir/Biquad.cpp \
	iir/Butterworth.cpp \
	iir/Cascade.cpp \
	iir/PoleFilter.cpp \
	iir/RootFinder.cpp \
	gverb.cpp \
	reverb.cpp \
	compressor.cpp \
	libHybridConv.cpp \
# terminator

LOCAL_C_INCLUDES += \
	frameworks/base/include \
	hardware/libhardware/include \
	system/core/include \
	system/media/audio_effects/include \
# terminator

LOCAL_STATIC_LIBRARIES := fftw3
LOCAL_LDLIBS := -llog
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
LOCAL_CPPFLAGS += -ffunction-sections -fdata-sections -Ofast -march=armv7-a -mfpu=neon -ftree-vectorize -DNDEBUG -DDEBUG
LOCAL_CFLAGS += -ffunction-sections -fdata-sections -Ofast -march=armv7-a -mfpu=neon -ftree-vectorize -DNDEBUG -DDEBUG
else ifeq ($(TARGET_ARCH_ABI), x86)
LOCAL_CPPFLAGS += -ffunction-sections -fdata-sections -Ofast -ftree-vectorize -DNDEBUG -DHYBRIDCONV_USE_SSE# -DDEBUG
LOCAL_CFLAGS += -ffunction-sections -fdata-sections -Ofast -ftree-vectorize -DNDEBUG -DHYBRIDCONV_USE_SSE# -DDEBUG
endif
LOCAL_LDFLAGS += -Wl,--gc-sections -DNDEBUG
include $(BUILD_SHARED_LIBRARY)