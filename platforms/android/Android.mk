/*
    This file is part of Prototype.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Prototype is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Prototype is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Prototype.  If not, see <http://www.gnu.org/licenses/>.
*/

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

APP_DIR := $(LOCAL_PATH)

LOCAL_MODULE := prototype

LOCAL_CFLAGS := -DANDROID_NDK \
                -DDISABLE_IMPORTGL \
				-I$(LOCAL_PATH)/..

LOCAL_CXXFLAGS := -DANDROID_NDK \
                -DDISABLE_IMPORTGL \
            -I$(LOCAL_PATH)/../sources \
				-I$(LOCAL_PATH)/.. \
				-I$(LOCAL_PATH)/../sac/ \
				-I$(LOCAL_PATH)/../sac/libs/libpng/jni/ \
				-I$(LOCAL_PATH)/../sac/libs/

LOCAL_SRC_FILES := \
    prototype.cpp \
    ../sources/PrototypeGame.cpp \
    ../sources/states/LogoStateManager.cpp \
    ../sources/states/MenuStateManager.cpp \
    ../sources/states/TransitionStateManager.cpp \
    ../sac/android/sacjnilib.cpp

LOCAL_STATIC_LIBRARIES := sac png tremor jsoncpp
LOCAL_LDLIBS := -lGLESv2 -lGLESv1_CM -lEGL -llog -lz

include $(BUILD_SHARED_LIBRARY)

include $(APP_DIR)/../sac/build/android/Android.mk
include $(APP_DIR)/../sac/libs/build/android/tremor/Android.mk
include $(APP_DIR)/../sac/libs/build/android/libpng/Android.mk
include $(APP_DIR)/../sac/libs/build/android/jsoncpp/Android.mk

$(call import-module,android/native_app_glue)
